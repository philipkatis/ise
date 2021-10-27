#include <stdlib.h>
#include <stdio.h>

#include "ise_base.h"
#include "ise.h"

#include "ise_base.cpp"

/*

  NOTE(philip): The queries will come from a file. Each query will be on a separate line, with the line number
  acting as the query number/ID. Each query will have up to 5 keywords (based on files from the competition),
  separated by a space. The keywords must follow the regular keyword restrictions.

  NOTE(philip): Deduplication should happen to both the query keywords and the document contents. The most ideal
  way to achieve this has to be a hash table, where we store all the words and only compare them when their hash
  values are the same.

  NOTE(philip): The entry list seems to simply be a list of keywords where each node contains the keyword but also
  additional information. (For example the query ID that that particular keyword is part of.) Based on the
  assignment, this list has to be used in two cases. 1) To supply the query keywords to the BK-tree for
  construction, and 2) as return value for the BK-tree lookup function.

  TODO(philip): What is the final resting point of the query keywords? Are they stored in a keyword list and then
  referenced to by a pointer? Are they stored in the BK-tree for faster accesss?

  TODO(philip): Can the keyword list not be a list??? It makes more sence for it to be a hash table for O(1)
  access, since that will be the most common case. A hash table can also accelerate the deduplication.

  TODO(philip): Implement an Assert macro and use it to catch very wrong outputs from the keyword matching
  functions.

  TODO(philip): Investigate how we choose what keyword matching function is used each time.

  TODO(philip): Replace calloc() and free() with custom function that can be instrumented later on.

  NOTE(philip): There are many ways to optimize the keyword matching functions. If there's a need to in the
  future, we should investigate whether the use of intrinsics (SSE) is allowed.

*/

function b32
IsExactMatch(char *A, char *B)
{
    b32 Result = true;

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
                Result = false;
                break;
            }
        }
    }
    else
    {
        Result = false;
    }

    return Result;
}

// TODO(philip): What if we give this strings of different lengths??? Is that ever a case??? If so, does this
// function handle it??? For now a special value of -1 is returned.
function s32
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
    else
    {
        Result = -1;
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

function keyword_list_node *
KeywordList_AllocateNode(char *Word)
{
    keyword_list_node *Result = (keyword_list_node *)calloc(1, sizeof(keyword_list_node));

    u64 WordLength = StringLength(Word);
    CopyMemory(Word, WordLength * sizeof(char), Result->Word);

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

    List->Head = 0;
    List->Tail = 0;
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
BKTree_AllocateNode(char *Word)
{
    bk_tree_node *Result = (bk_tree_node *)calloc(1, sizeof(bk_tree_node));

    u64 WordLength = StringLength(Word);
    CopyMemory(Word, WordLength * sizeof(char), Result->Word);

    return Result;
}

function bk_tree_node *
BKTree_Create(char *RootWord)
{
    return BKTree_AllocateNode(RootWord);
}

function bk_tree_node *
BKTree_Insert(bk_tree_node *Tree, char *Word)
{
    bk_tree_node *Result = BKTree_AllocateNode(Word);

    bk_tree_node *Node = Tree;
    b32 Complete = false;

    while (!Complete)
    {
        u32 Distance = CalculateLevenshteinDistance(Node->Word, Result->Word);
        if (Node->Children[Distance - 1])
        {
            Node = Node->Children[Distance - 1];
        }
        else
        {
            Node->Children[Distance - 1] = Result;
            Complete = true;
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
    printf("Word: %s\n", Node->Word);

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

function b32
LoadTextFile(char *Path, buffer *Buffer)
{
    b32 Result = true;

    FILE *File = fopen(Path, "rt");
    if (File)
    {
        fseek(File, 0, SEEK_END);
        u64 Length = ftell(File);

        *Buffer = AllocateBuffer(Length * sizeof(char));

        fseek(File, 0, SEEK_SET);
        fread(Buffer->Data, 1, Length, File);

        fclose(File);
    }
    else
    {
        Result = false;
    }

    return Result;
}

s32 main()
{
    buffer Test = { };
    if (LoadTextFile("data/text.txt", &Test))
    {
        printf("%s\n", Test.Data);
        DeallocateBuffer(Test);
    }
    else
    {
        printf("Could not open file!\n");
    }

    char *Words[] = { "hell", "help", "fall", "felt", "fell", "small", "melt" };

    keyword_list List = KeywordList_Create();
    for (u64 Index = 0;
         Index < ArrayCount(Words);
         ++Index)
    {
        keyword_list_node *Node = KeywordList_AllocateNode(Words[Index]);
        KeywordList_Insert(&List, Node);
    }

    KeywordList_Visualize(&List);
    KeywordList_Destroy(&List);

#if 0
    bk_tree_node *Tree = BKTree_Create(Words[0]);

    for (u64 Index = 1;
         Index < ArrayCount(Words);
         ++Index)
    {
        BKTree_Insert(Tree, Words[Index]);
    }

    BKTree_Visualize(Tree);

    BKTree_Destroy(Tree);
#endif

    return 0;
}
