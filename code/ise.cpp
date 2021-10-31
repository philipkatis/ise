#include <stdlib.h>
#include <stdio.h>

#include "ise_base.h"
#include "ise_parser.h"
#include "ise.h"

#include "ise_base.cpp"
#include "ise_parser.cpp"

/*

  NOTE(philip): Deduplication should happen to both the query keywords and the document contents. The most ideal
  way to achieve this has to be a hash table, where we store all the words and only compare them when their hash
  values are the same.

  TODO(philip): Can the keyword list not be a list??? It makes more sence for it to be a hash table for O(1)
  access, since that will be the most common case. A hash table can also accelerate the deduplication.

  TODO(philip): Investigate how we choose what keyword matching function is used each time.

  TODO(philip): Replace calloc() and free() with custom function that can be instrumented later on.

  NOTE(philip): There are many ways to optimize the keyword matching functions. If there's a need to in the
  future, we should investigate whether the use of intrinsics (SSE) is allowed.

*/

function u32
IsExactMatch(char *A, char *B)
{
    u32 Result = 1;

    u64 LengthA = StringLength(A);
    u64 LengthB = StringLength(B);

    if (LengthA == LengthB)
    {
        for (u64 Index = 0;
             Index < LengthA;
             ++Index)
        {
            if (A[Index] != B[Index])
            {
                Result = 0;
                break;
            }
        }
    }
    else
    {
        Result = 0;
    }

    return Result;
}

function u32
CalculateHammingDistance(char *A, char *B)
{
    s32 Result = 0;

    u64 LengthA = StringLength(A);
    u64 LengthB = StringLength(B);

    if (LengthA == LengthB)
    {
        for (u64 Index = 0;
             Index < LengthA;
             ++Index)
        {
            if (A[Index] != B[Index])
            {
                ++Result;
            }
        }
    }

    return Result;
}

/*

  NOTE(philip): There are several algorithm implementations for calculating the Levenshtein distance between
  two strings, but here we are using the one with the matrix cache. The regular implementation uses 3 recursive
  function calls. Appart from the fact that function calls are expensive for a function called so many times,
  that specific implementation recalculates the length between two substrings multiple times, thus wasting CPU
  time. The version used here is not recursive and uses a 2D matrix to cache distance calculation results that
  might need to be used in the future.

  One small implementation detail is that this matrix is stored as a 1D array in memory. This should be more
  cache friendly. Additionally, the matrix is a global variable and it is reused on every function call.

  Implementation Reference: https://en.wikipedia.org/wiki/Wagner%E2%80%93Fischer_algorithm

*/

global u32 LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * MAX_KEYWORD_LENGTH];

function u32
CalculateLevenshteinDistance(char *A, char *B)
{
    ZeroMemory(LevenshteinDistanceCache, MAX_KEYWORD_LENGTH * MAX_KEYWORD_LENGTH * sizeof(u32));

    u64 LengthA = StringLength(A);
    u64 LengthB = StringLength(B);

    for (u64 Index = 1;
         Index <= LengthA;
         ++Index)
    {
        LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * 0 + Index] = Index;
    }

    for (u64 Index = 1;
         Index <= LengthB;
         ++Index)
    {
        LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * Index + 0] = Index;
    }

    for (u64 IndexB = 1;
         IndexB <= LengthB;
         ++IndexB)
    {
        for (u64 IndexA = 1;
             IndexA <= LengthA;
             ++IndexA)
        {
            u32 DeletionDistance  = LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * IndexB + (IndexA - 1)] + 1;
            u32 InsertionDistance = LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * (IndexB - 1) + IndexA] + 1;

            b32 RequiresSubstitution = (A[IndexA - 1] != B[IndexB - 1]);
            u32 SubstitutionDistance = LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * (IndexB - 1) + (IndexA - 1)] +
                RequiresSubstitution;

            u32 *Distance = &LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * IndexB + IndexA];
            *Distance = Min(Min(DeletionDistance, InsertionDistance), SubstitutionDistance);
        }
    }

    return LevenshteinDistanceCache[MAX_KEYWORD_LENGTH * LengthB + LengthA];
}

// NOTE(philip): This array stores all the different keyword matching functions in such a way where the match_type
// can be used to index into it.
global match_function MatchFunctions[3] = { IsExactMatch, CalculateHammingDistance, CalculateLevenshteinDistance };

function keyword_list_node *
KeywordList_AllocateNode(char *Word, u64 QueryID, match_type MatchType, u32 MatchDistance)
{
    keyword_list_node *Result = (keyword_list_node *)calloc(1, sizeof(keyword_list_node));
    Result->Word = Word;
    Result->QueryID = QueryID;
    Result->MatchType = MatchType;
    Result->MatchDistance = MatchDistance;

    return Result;
}

function void
KeywordList_DeallocateNode(keyword_list_node *Node)
{
    free(Node);
}

function keyword_list
KeywordList_Create()
{
    keyword_list Result = { };
    return Result;
}

function void
KeywordList_Insert(keyword_list *List, keyword_list_node *Node)
{
    if (List->Head == 0)
    {
        List->Head = Node;
        List->Tail = Node;
    }
    else
    {
        keyword_list_node *Next = List->Head;
        Next->Previous = Node;

        Node->Next = Next;
        List->Head = Node;
    }

    ++List->Count;
}

function void
KeywordList_Destroy(keyword_list *List)
{
    keyword_list_node *Node = List->Head;
    while (Node)
    {
        keyword_list_node *Next = Node->Next;
        KeywordList_DeallocateNode(Node);
        Node = Next;
    }

    List->Head  = 0;
    List->Tail  = 0;
    List->Count = 0;
}

function void
KeywordList_Visualize(keyword_list *List)
{
    keyword_list_node *Node = List->Head;
    u64 Index = 0;

    while (Node)
    {
        printf("Index: %llu, Word: %s\n", Index, Node->Word);
        Node = Node->Next;
        Index++;
    }
}

function bk_tree_node *
BKTree_AllocateNode(keyword_list_node *Keyword)
{
    bk_tree_node *Result = (bk_tree_node *)calloc(1, sizeof(bk_tree_node));
    Result->Keyword = Keyword;

    return Result;
}

function bk_tree_node *
BKTree_Create(keyword_list_node *RootKeyword)
{
    return BKTree_AllocateNode(RootKeyword);
}

function bk_tree_node *
BKTree_Insert(bk_tree_node *Tree, keyword_list_node *Keyword)
{
    bk_tree_node *Result = 0;

    bk_tree_node *Node = Tree;
    b32 Complete = false;

    while (!Complete)
    {
        u32 Distance = CalculateLevenshteinDistance(Node->Keyword->Word, Keyword->Word);
        if (Distance > 0)
        {
            if (Node->Children[Distance - 1])
            {
                Node = Node->Children[Distance - 1];
            }
            else
            {
                Result = BKTree_AllocateNode(Keyword);
                Node->Children[Distance - 1] = Result;

                Complete = true;
            }
        }
        else
        {
            // NOTE(philip): If the keyword is already in the BK-tree, do not insert it.
            Complete = true;
        }
    }

    return Result;
}

function keyword_list
BKTree_Search(bk_tree_node *Tree, char *Word)
{
    keyword_list Result = KeywordList_Create();

    // TODO(philip): Total jank.
    bk_tree_node *CandidateKeywords[1024];
    u64 CandidateKeywordCount = 0;

    // NOTE(philip): Initialize the candidate keyword list.
    CandidateKeywords[CandidateKeywordCount] = Tree;
    ++CandidateKeywordCount;

    for (u64 Index = 0;
         Index < CandidateKeywordCount;
         ++Index)
    {
        // NOTE(philip): Pop the last item from the array.
        bk_tree_node *CandidateKeyword = CandidateKeywords[CandidateKeywordCount - Index - 1];
        --CandidateKeywordCount;

        keyword_list_node *Keyword = CandidateKeyword->Keyword;

        // TODO(philip): I do not know if this is the correct way to approach this. We are using the current
        // keyword's query-specific matching type and matching distance. This might prevent words later down the
        // tree from being searched.

        // NOTE(philip): Get the correct match function and calculate the distance.
        match_function MatchFunction = MatchFunctions[Keyword->MatchType];
        u32 Distance = MatchFunction(Word, Keyword->Word);

        // NOTE(philip): Account for the difference in maximum match distance.
        b32 IsValid = false;
        if (Keyword->MatchType == MatchType_Exact)
        {
            IsValid = Distance;
        }
        else
        {
            IsValid = (Distance <= Keyword->MatchDistance);
        }

        if (IsValid)
        {
            keyword_list_node *Node = KeywordList_AllocateNode(Keyword->Word, Keyword->QueryID, Keyword->MatchType,
                                                               Keyword->MatchDistance);
            KeywordList_Insert(&Result, Node);
        }

        // TODO(philip): Again, this doesn't seem right.
        if (Keyword->MatchType != MatchType_Exact)
        {
            s32 MinDistanceFromParent = Distance - Keyword->MatchDistance;
            Assert(MinDistanceFromParent >= 0);

            s32 MaxDistanceFromParent = Distance + Keyword->MatchDistance;
            Assert(MaxDistanceFromParent > MinDistanceFromParent);

            for (s32 Index = MinDistanceFromParent;
                 Index < MaxDistanceFromParent;
                 Index++)
            {

                if (CandidateKeyword->Children[Index])
                {
                    CandidateKeywords[CandidateKeywordCount] = CandidateKeyword->Children[Index];
                    ++CandidateKeywordCount;
                }
            }
        }
    }

    return Result;
}

function void
BKTree_Destroy(bk_tree_node *Node)
{
    for (u64 Index = 0;
         Index < MAX_KEYWORD_LENGTH - 1;
         ++Index)
    {
        if (Node->Children[Index])
        {
            BKTree_Destroy(Node->Children[Index]);
        }
    }

    free(Node);
}

function void
PrintSpaces(u64 Count)
{
    for (u64 Index = 0;
         Index < Count;
         ++Index)
    {
        printf(" ");
    }
}

function void
BKTree_Visualize(bk_tree_node *Node, u64 Depth = 0)
{
    PrintSpaces(Depth * 4);
    printf("{\n");

    PrintSpaces(Depth * 4 + 4);
    printf("Word: %s\n", Node->Keyword->Word);

    for (u64 Index = 0;
         Index < MAX_KEYWORD_LENGTH - 1;
         ++Index)
    {
        if (Node->Children[Index])
        {
            printf("\n");
            PrintSpaces(Depth * 4 + 4);
            printf("Distance: %llu\n", Index + 1);

            BKTree_Visualize(Node->Children[Index], Depth + 1);
        }
    }

    PrintSpaces(Depth * 4);
    printf("}\n");
}

function entry *
Allocate_entry(char* Word)
{
    entry* Result = (entry*)calloc(1, sizeof(entry));

    u64 WordLength = StringLength(Word);
    CopyMemory(Word, WordLength * sizeof(char), Result->Word);

    return Result;
}

function void
Deallocate_entry(entry *Entry)
{
    free(Entry);
}

function entry *
Create_entry(char *Word)
{
    entry *Entry = Allocate_entry(Word);

    /*

    NOTE(Alex): Initialize payload here too.

    */

    return Entry;
}

s32 main()
{
#if 0
    char *CommandFilePath = "data/commands.txt";

    buffer CommandFileContents = { };
    if (LoadTextFile(CommandFilePath, &CommandFileContents))
    {
        ParseCommandFile(CommandFileContents);
        DeallocateBuffer(CommandFileContents);
    }
    else
    {
        printf("[Error]: Could not open command file! (Path: \"%s\")\n", CommandFilePath);
    }
#endif

#if 1
    char *Words[] = { "hell", "help", "fall", "felt", "fell", "small", "melt" };

    keyword_list List = KeywordList_Create();
    for (u64 Index = 0;
         Index < ArrayCount(Words);
         ++Index)
    {
        keyword_list_node *Node = KeywordList_AllocateNode(Words[Index], 123, MatchType_Levenshtein, 2);
        KeywordList_Insert(&List, Node);
    }

    // KeywordList_Visualize(&List);
    // printf("\n\n");

    bk_tree_node *Tree = BKTree_Create(List.Tail);

    for (keyword_list_node *Node = List.Tail->Previous;
         Node;
         Node = Node->Previous)
    {
        BKTree_Insert(Tree, Node);
    }

    // BKTree_Visualize(Tree);

    keyword_list SearchResult = BKTree_Search(Tree, "henn");

    KeywordList_Visualize(&SearchResult);
    KeywordList_Destroy(&SearchResult);

    BKTree_Destroy(Tree);
    KeywordList_Destroy(&List);
#endif

    return 0;
}
