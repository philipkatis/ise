#include "ise_bk_tree.h"

// TODO(philip): Change the BK tree types.

function void
BKTree(void)
{
    char *Words[10] =
    {
        "halt", "oouc", "pops", "oops", "felt", "fell", "smel", "shel", "help", "hell"
    };

    keyword_table Keywords = KeywordTable_Create(64);

    for (u64 WordIndex = 0;
         WordIndex < ArrayCount(Words);
         ++WordIndex)
    {
        KeywordTable_Insert(&Keywords, Words[WordIndex]);
    }

    bk_tree Tree = BKTree_Create(1);

    TEST_CHECK(Tree.Root == 0);
    TEST_CHECK(Tree.Type == 1);

    for (keyword_iterator Iterator = IterateAllKeywords(&Keywords);
         IsValid(&Iterator);
         Advance(&Iterator))
    {
        keyword *Keyword = GetValue(&Iterator);
        BKTree_Insert(&Tree, Keyword);
    }

    {
        char *Solutions[4] =
        {
            Words[0], Words[4], Words[8], Words[9]
        };

        keyword_table SearchValues = KeywordTable_Create(1);
        keyword_table_insert_result InsertResult = KeywordTable_Insert(&SearchValues, "helt");

        keyword_list Matches = BKTree_FindMatches(&Tree, InsertResult.Keyword, 100);
        KeywordTable_Destroy(&SearchValues);

        for (u64 SolutionIndex = 0;
             SolutionIndex < ArrayCount(Solutions);
             ++SolutionIndex)
        {
            b32 Found = false;

            for (keyword_list_node *Node = Matches.Head;
                 Node;
                 Node = Node->Next)
            {
                if (strcmp(Node->Keyword->Word, Solutions[SolutionIndex]) == 0)
                {
                    Found = true;
                    break;
                }
            }

            TEST_CHECK(Found == true);
        }

        KeywordList_Destroy(&Matches);
    }

    {
        keyword_table SearchValues = KeywordTable_Create(1);
        keyword_table_insert_result InsertResult = KeywordTable_Insert(&SearchValues, "opsy");

        keyword_list Matches = BKTree_FindMatches(&Tree, InsertResult.Keyword, 2);
        KeywordTable_Destroy(&SearchValues);

        TEST_CHECK(Matches.Head == 0);

        KeywordList_Destroy(&Matches);
    }

    BKTree_Destroy(&Tree);
    TEST_CHECK(Tree.Root == 0);

    KeywordTable_Destroy(&Keywords);
}

#if 0
function void
Test_HammingBKTree(void)
{

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
