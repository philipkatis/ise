#include <stdlib.h>
#include <stdio.h>

#include "ise_match.h"
#include "ise_keyword_list.h"
#include "ise_bk_tree.h"
#include "ise.h"

#include "acutest.h"

#include "tests_query_tree.cpp"
#include "tests_keyword_table.cpp"

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
Test_CalculateEditDistance(void)
{
    u32 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Test some regular cases, since this function is more complex than the other keyword matching
    // functions.
    {
        A = "kitten";
        B = "sitten";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sitten";
        B = "sittin";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sittin";
        B = "sitting";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "bravo";
        B = "raven";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 3);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = CalculateEditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }
}

#if TODO

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

    keyword_table Table = { };

    for (u64 Index = 0;
         Index < 10;
         ++Index)
    {
        if (!KeywordTable_Find(&Table, Words[Index]))
        {
            KeywordTable_Insert(&Table, Words[Index]);
        }
    }

    bk_tree Tree = BKTree_Create(BKTreeType_Hamming);

    // NOTE(philip): BK-Tree creation.
    {
        TEST_CHECK(Tree.Root == 0);
        TEST_CHECK(Tree.MatchFunction == CalculateHammingDistance);
    }

    // NOTE(philip): BK-Tree insertion.
    {
        // TODO(philip): Make an iterator for this.
        for (u64 BucketIndex = 0;
             BucketIndex < KEYWORD_TABLE_BUCKET_COUNT;
             ++BucketIndex)
        {
            if (Table.Buckets[BucketIndex])
            {
                for (keyword_table_node *Keyword = Table.Buckets[BucketIndex];
                     Keyword;
                     Keyword = Keyword->Next)
                {
                    bk_tree_node *Node = BKTree_Insert(&Tree, Keyword);
                    TEST_CHECK(Node != 0);
                    TEST_CHECK(Node->Keyword == Keyword);
                    TEST_CHECK(Node->FirstChild == 0);
                }
            }
        }
    }

    // NOTE(philip): BK-Tree search.
    {
        char *Solutions[4] =
        {
            Words[0], Words[4], Words[8], Words[9]
        };

        keyword_list Matches = BKTree_FindMatches(&Tree, "helt", 1);
        TEST_CHECK(Matches.Count == 4);

        for (u64 Index = 0;
             Index < 4;
             ++Index)
        {
            keyword* Match = KeywordList_Find(&Matches, Solutions[Index]);
            TEST_CHECK(Match != 0);
        }

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

    KeywordTable_Destroy(&Table);
}

function void
Test_EditBKTree(void)
{
    char *Words[10] =
    {
        "halt", "oouch", "pop", "oops", "felt", "fell", "smell", "shell", "help", "hell"
    };

    keyword_table Table = { };

    for (u64 Index = 0;
         Index < 10;
         ++Index)
    {
        if (!KeywordTable_Find(&Table, Words[Index]))
        {
            KeywordTable_Insert(&Table, Words[Index]);
        }
    }

    bk_tree Tree = BKTree_Create(BKTreeType_Edit);

    // NOTE(philip): BK-Tree creation.
    {
        TEST_CHECK(Tree.Root == 0);
        TEST_CHECK(Tree.MatchFunction == CalculateEditDistance);
    }

    // NOTE(philip): BK-Tree insertion.
    {
        // TODO(philip): Make an iterator for this.
        for (u64 BucketIndex = 0;
             BucketIndex < KEYWORD_TABLE_BUCKET_COUNT;
             ++BucketIndex)
        {
            if (Table.Buckets[BucketIndex])
            {
                for (keyword_table_node *Keyword = Table.Buckets[BucketIndex];
                     Keyword;
                     Keyword = Keyword->Next)
                {
                    bk_tree_node *Node = BKTree_Insert(&Tree, Keyword);
                    TEST_CHECK(Node != 0);
                    TEST_CHECK(Node->Keyword == Keyword);
                    TEST_CHECK(Node->FirstChild == 0);
                }
            }
        }
    }

    // NOTE(philip): BK-Tree search.
    {
        char *Solutions[6] =
        {
            Words[0], Words[4], Words[5], Words[7], Words[8], Words[9]
        };

        keyword_list Matches = BKTree_FindMatches(&Tree, "helt", 2);
        TEST_CHECK(Matches.Count == 6);

        for (u64 Index = 0;
             Index < 6;
             ++Index)
        {
            keyword* Match = KeywordList_Find(&Matches, Solutions[Index]);
            TEST_CHECK(Match != 0);
        }

        KeywordList_Destroy(&Matches);
    }

    // NOTE(philip): BK-Tree search.
    {
        char *Solutions[2] =
        {
            Words[2], Words[3]
        };

        keyword_list Matches = BKTree_FindMatches(&Tree, "ops", 2);
        TEST_CHECK(Matches.Count == 2);

        for (u64 Index = 0;
             Index < 2;
             ++Index)
        {
            keyword* Match = KeywordList_Find(&Matches, Solutions[Index]);
            TEST_CHECK(Match != 0);
        }

        KeywordList_Destroy(&Matches);
    }

    // NOTE(philip): BK-Tree destruction.
    {
        BKTree_Destroy(&Tree);
        TEST_CHECK(Tree.Root == 0);
    }

    KeywordTable_Destroy(&Table);
}

#endif

TEST_LIST =
{
    { "Query Tree",                   QueryTree                     },
    { "Keyword Table",                KeywordTable                  },

    { "Exact Keyword Matching",       Test_IsExactMatch             },
    { "Hamming Distance Calculation", Test_CalculateHammingDistance },
    { "Edit Distance Calculation",    Test_CalculateEditDistance    },

#if 0
    { "Keyword List",                 Test_KeywordList              },
    { "Hamming BK-Tree",              Test_HammingBKTree            },
    { "Edit BK-Tree",                 Test_EditBKTree               },
#endif

    { 0, 0 }
};
