#include <stdlib.h>
#include <stdio.h>

#include "ise_match.h"
#include "ise_keyword_list.h"
#include "ise_bk_tree.h"

#include "acutest.h"

function void
Test_IsExactMatch(void)
{
    u32 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }

    // NOTE(philip): Non-empty strings. Different words. Same length.
    {
        A = "hello";
        B = "world";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words. Different length.
    {
        A = "000x00010f001";
        B = "rgjahrg";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Max string length.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = IsExactMatch(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }
}

function void
Test_CalculateHammingDistance(void)
{
    u32 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = CalculateHammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }
}

function void
Test_CalculateLevenshteinDistance(void)
{
    u32 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Test some regular cases, since this function is more complex than the other keyword matching
    // functions.
    {
        A = "kitten";
        B = "sitten";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sitten";
        B = "sittin";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sittin";
        B = "sitting";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "bravo";
        B = "raven";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 3);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = CalculateLevenshteinDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }
}

function void
Test_KeywordList(void)
{
#define KEYWORD_COUNT 100
    char Words[KEYWORD_COUNT][MAX_KEYWORD_LENGTH + 1];

    // NOTE(philip): Initialize the word array.
    for (u64 Index = 0;
         Index < KEYWORD_COUNT;
         ++Index)
    {
        sprintf(Words[Index], "Word%llu", Index + 1);
    }

    keyword_list List = KeywordList_Create();

    // NOTE(philip): List creation.
    {
        TEST_CHECK(List.Head == 0);
        TEST_CHECK(List.Count == 0);
    }

    // NOTE(philip): Keyword insertion.
    {
        keyword *PreviousKeyword = 0;

        for (u64 Index = 0;
             Index < KEYWORD_COUNT;
             ++Index)
        {
            keyword *Keyword = KeywordList_Insert(&List, Words[Index]);
            TEST_CHECK(Keyword != 0);
            TEST_CHECK(List.Head == Keyword);
            TEST_CHECK(List.Count == Index + 1);

            if (Index > 0)
            {
                TEST_CHECK(PreviousKeyword == PreviousKeyword);
            }

            PreviousKeyword = Keyword;
        }
    }

    // NOTE(philip): Keyword search.
    {
        keyword *Keyword = KeywordList_Find(&List, "RandomWord");
        TEST_CHECK(Keyword == 0);

        Keyword = KeywordList_Find(&List, Words[0]);
        TEST_CHECK(Keyword != 0);
        TEST_CHECK(Keyword->Word == Words[0]);

        Keyword = KeywordList_Find(&List, Words[66]);
        TEST_CHECK(Keyword != 0);
        TEST_CHECK(Keyword->Word == Words[66]);

        Keyword = KeywordList_Find(&List, Words[99]);
        TEST_CHECK(Keyword != 0);
        TEST_CHECK(Keyword->Word == Words[99]);
    }

    // NOTE(philip): List count.
    {
        TEST_CHECK(List.Count == KEYWORD_COUNT);
    }
#undef KEYWORD_COUNT

    // NOTE(philip): List destruction.
    {
        KeywordList_Destroy(&List);
        TEST_CHECK(List.Head == 0);
        TEST_CHECK(List.Count == 0);
    }
}

function void
Test_HammingBKTree(void)
{
    char *Words[10] =
    {
        "halt", "oouc", "pops", "oops", "felt", "fell", "smel", "shel", "help", "hell"
    };

    keyword_list List = KeywordList_Create();

    for (u64 Index = 0;
         Index < 10;
         ++Index)
    {
        KeywordList_Insert(&List, Words[Index]);
    }

    bk_tree Tree = BKTree_Create(MatchType_Hamming);

    // NOTE(philip): BK-Tree creation.
    {
        TEST_CHECK(Tree.Root == 0);
        TEST_CHECK(Tree.MatchFunction == CalculateHammingDistance);
    }

    // NOTE(philip): BK-Tree insertion.
    {
        for (keyword *Keyword = List.Head;
             Keyword;
             Keyword = Keyword->Next)
        {
            bk_tree_node *Node = BKTree_Insert(&Tree, Keyword);
            TEST_CHECK(Node != 0);
            TEST_CHECK(Node->Keyword == Keyword);
            TEST_CHECK(Node->FirstChild == 0);
        }
    }

    // NOTE(philip): BK-Tree search.
    {
        keyword_list Matches = BKTree_FindMatches(&Tree, "helt", 1);
        TEST_CHECK(Matches.Count == 4);

        keyword *Match = Matches.Head;
        TEST_CHECK(Match->Word == Words[0]);
        Match = Match->Next;

        TEST_CHECK(Match->Word == Words[4]);
        Match = Match->Next;

        TEST_CHECK(Match->Word == Words[8]);
        Match = Match->Next;

        TEST_CHECK(Match->Word == Words[9]);
        Match = Match->Next;

        TEST_CHECK(Match == 0);

        KeywordList_Destroy(&Matches);
    }

    // NOTE(philip): BK-Tree search.
    {
        keyword_list Matches = BKTree_FindMatches(&Tree, "opsy", 2);
        TEST_CHECK(Matches.Count == 0);

        keyword *Match = Matches.Head;
        TEST_CHECK(Match == 0);

        KeywordList_Destroy(&Matches);
    }

    // NOTE(philip): BK-Tree destruction.
    {
        BKTree_Destroy(&Tree);
        TEST_CHECK(Tree.Root == 0);
    }

    KeywordList_Destroy(&List);
}

function void
Test_LevenshteinBKTree(void)
{
    char *Words[10] =
    {
        "halt", "oouch", "pop", "oops", "felt", "fell", "smell", "shell", "help", "hell"
    };

    keyword_list List = KeywordList_Create();

    for (u64 Index = 0;
         Index < 10;
         ++Index)
    {
        KeywordList_Insert(&List, Words[Index]);
    }

    bk_tree Tree = BKTree_Create(MatchType_Levenshtein);

    // NOTE(philip): BK-Tree creation.
    {
        TEST_CHECK(Tree.Root == 0);
        TEST_CHECK(Tree.MatchFunction == CalculateLevenshteinDistance);
    }

    // NOTE(philip): BK-Tree insertion.
    {
        for (keyword *Keyword = List.Head;
             Keyword;
             Keyword = Keyword->Next)
        {
            bk_tree_node *Node = BKTree_Insert(&Tree, Keyword);
            TEST_CHECK(Node != 0);
            TEST_CHECK(Node->Keyword == Keyword);
            TEST_CHECK(Node->FirstChild == 0);
        }
    }

    // NOTE(philip): BK-Tree search.
    {
        keyword_list Matches = BKTree_FindMatches(&Tree, "helt", 2);
        TEST_CHECK(Matches.Count == 6);

        keyword *Match = Matches.Head;
        TEST_CHECK(Match->Word == Words[0]);
        Match = Match->Next;

        TEST_CHECK(Match->Word == Words[4]);
        Match = Match->Next;

        TEST_CHECK(Match->Word == Words[5]);
        Match = Match->Next;

        TEST_CHECK(Match->Word == Words[7]);
        Match = Match->Next;

        TEST_CHECK(Match->Word == Words[8]);
        Match = Match->Next;

        TEST_CHECK(Match->Word == Words[9]);
        Match = Match->Next;

        TEST_CHECK(Match == 0);

        KeywordList_Destroy(&Matches);
    }

    // NOTE(philip): BK-Tree search.
    {
        keyword_list Matches = BKTree_FindMatches(&Tree, "ops", 2);
        TEST_CHECK(Matches.Count == 2);

        keyword *Match = Matches.Head;
        TEST_CHECK(Match->Word == Words[2]);
        Match = Match->Next;

        TEST_CHECK(Match->Word == Words[3]);
        Match = Match->Next;

        TEST_CHECK(Match == 0);

        KeywordList_Destroy(&Matches);
    }

    // NOTE(philip): BK-Tree destruction.
    {
        BKTree_Destroy(&Tree);
        TEST_CHECK(Tree.Root == 0);
    }

    KeywordList_Destroy(&List);
}

TEST_LIST =
{
    { "Exact Keyword Matching",           Test_IsExactMatch                 },
    { "Hamming Distance Calculation",     Test_CalculateHammingDistance     },
    { "Levenshtein Distance Calculation", Test_CalculateLevenshteinDistance },
    { "Keyword List",                     Test_KeywordList                  },
    { "Hamming BK-Tree",                  Test_HammingBKTree                },
    { "Levenshtein BK-Tree",              Test_LevenshteinBKTree            },
    { 0, 0 }
};
