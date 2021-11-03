#include <stdlib.h>
#include <stdio.h>

#include "ise_base.h"
#include "ise_parser.h"
#include "ise.h"

#include "ise_base.cpp"
#include "ise_match.cpp"
#include "ise_parser.cpp"

/*

  TODO(philip): Passing around strings with their length is a common thing that happens. We should probalby add a
  string structure to group them together.

  TODO(philip): Currently the Assert() macro is pressent in all builds. It should be stripped from release builds.

*/

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
        u32 Distance = CalculateLevenshteinDistance(Node->Keyword->Word, StringLength(Node->Keyword->Word),
                                                    Keyword->Word, StringLength(Keyword->Word));
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
        u32 Distance = MatchFunction(Word, StringLength(Word), Keyword->Word, StringLength(Keyword->Word));

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
Entry_Allocate(char* Word)
{
    entry* Result = (entry*)calloc(1, sizeof(entry));

    u64 WordLength = StringLength(Word);
    CopyMemory(Word, WordLength * sizeof(char), Result->Word);

    return Result;
}

function void
Entry_Destroy(entry *Entry)
{
    free(Entry);
}

function entry *
Entry_Create(char *Word)
{
    entry *Entry = Entry_Allocate(Word);

    /*

    NOTE(Alex): If the said payload is actually implemented, we need to search for queries in which the word
    exists and insert their ID numbers in the matrix.

    */

    return Entry;
}

function entry_list_node*
EntryList_AllocateNode(entry* Entry)
{
    entry_list_node* Result = (entry_list_node*)calloc(1, sizeof(entry_list_node));
    Result->Entry = Entry;
    Result->Previous = NULL;
    Result->Next = NULL;

    return Result;
}

function void
EntryList_DeallocateNode(entry_list_node* Node)
{
    free(Node);
}

function entry_list
EntryList_Create()
{
    entry_list Result = { };
    return Result;
}

function void
EntryList_AddEntry(entry_list* List, entry_list_node* Node)
{
    if (List->Head == 0)
    {
        List->Head = Node;
        List->Tail = Node;
    }
    else
    {
        entry_list_node* Next = List->Head;
        Next->Previous = Node;

        Node->Next = Next;
        List->Head = Node;
    }

    ++List->Count;
}

function void
EntryList_Visualize(entry_list* List)
{
    entry_list_node* Node = List->Head;
    u64 Index = 0;

    while (Node)
    {
        printf("Index: %llu, Word: %s\n", Index, Node->Entry->Word);
        Node = Node->Next;
        Index++;
    }
}

function entry_list_node*
EntryList_GetFirst(entry_list* List)
{
    return List->Head;
}

function entry_list_node*
EntryList_GetNext(entry_list_node* Node)
{
    return Node->Next;
}

function u32
EntryList_GetNumberEntries(entry_list* List)
{
    return List->Count;
}

function void
EntryList_Destroy(entry_list* List)
{
    entry_list_node* Node = List->Head;
    while (Node)
    {
        entry_list_node* Next = Node->Next;
        EntryList_DeallocateNode(Node);
        Node = Next;
    }

    List->Head = 0;
    List->Tail = 0;
    List->Count = 0;
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
