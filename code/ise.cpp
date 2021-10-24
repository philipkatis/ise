#include <stdlib.h>
#include <stdio.h>

#include "ise_base.h"
#include "ise.h"

#include "ise_base.cpp"

/*

  TODO(philip): Implement an Assert macro and use it to catch very wrong outputs from the keyword matching
  functions.

  NOTE(philip): There are many ways to optimize the keyword matching functions. If there's a need to in the
  future, we should investigate whether the use of intrinsics (SSE) is allowed.

  TODO(philip): For deduplication, a keyword hash table per document could be used. This way, each keyword can
  be checked against ones with identical hash values. If they are identical, we can cull them.

  TODO(philip): Based on the assignment it looks like in order for a document to answer a specific query, all the
  keywords that make up the query must have similar keywords inside the docuemnt. So there needs to be some sort
  of relation between all the query words. Here are some ideas of how this can be achieved. 1) Store the data that
  make up a query separately from the acceleration structure. This will require a lot of fetching of data from RAM,
  thus making it slower. 2) Make the BK Tree nodes act as a linked list that contains the query keywords. This
  will probably require additional logic to ensure keywords are checked only once.

  TODO(philip): Investigate how we choose what keyword matching function is used each time.

  TODO(philip): There is not information in the assignment about duplicate keywords in the queries, only in the
  documents. Is that something that needs to be addressed?

  TODO(philip): How does the entry_list work?

  TODO(philip): Replace calloc() and free() with custom function that can be instrumented later on.

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

s32 main()
{
    char *Words[] = { "hell", "help", "fall", "felt", "fell", "small", "melt" };
    bk_tree_node *Tree = BKTree_Create(Words[0]);

    for (u64 Index = 1;
         Index < ArrayCount(Words);
         ++Index)
    {
        BKTree_Insert(Tree, Words[Index]);
    }

    BKTree_Visualize(Tree);

    BKTree_Destroy(Tree);

    return 0;
}
