#include "acutest.h"

#include "ise_base.h"
#include "ise_keyword.h"
#include "ise_query.h"

#include "ise_base.cpp"
#include "ise_keyword.cpp"
#include "ise_query.cpp"

function void
Test_IsExactMatch(void)
{
    b32 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = StringCompare(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = StringCompare(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }

    // NOTE(philip): Non-empty strings. Different words. Same length.
    {
        A = "hello";
        B = "world";

        Result = StringCompare(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words. Different length.
    {
        A = "000x00010f001";
        B = "rgjahrg";

        Result = StringCompare(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Max string length.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = StringCompare(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }
}

function void
Test_CalculateHammingDistance(void)
{
    u64 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = HammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = HammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = HammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = HammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = HammingDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }
}

function void
Test_CalculateEditDistance(void)
{
    u64 Result = 0;
    char *A = 0;
    char *B = 0;

    // NOTE(philip): Empty strings.
    {
        A = "";
        B = "";

        Result = EditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Same words.
    {
        A = "hello";
        B = "hello";

        Result = EditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 0);
    }

    // NOTE(philip): Non-empty strings. Different words.
    {
        A = "hello";
        B = "world";

        Result = EditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 4);
    }

    // NOTE(philip): Non-empty strings. Completely different words.
    {
        A = "1234567890";
        B = "qwertyuiop";

        Result = EditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 10);
    }

    // NOTE(philip): Test some regular cases, since this function is more complex than the other keyword matching
    // functions.
    {
        A = "kitten";
        B = "sitten";

        Result = EditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sitten";
        B = "sittin";

        Result = EditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "sittin";
        B = "sitting";

        Result = EditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);

        A = "bravo";
        B = "raven";

        Result = EditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 3);
    }

    // NOTE(philip): Big string.
    {
        A = "0000000000000000000000000000001";
        B = "0000000000000000000000000000002";

        Result = EditDistance(A, strlen(A), B, strlen(B));
        TEST_CHECK(Result == 1);
    }
}

function void
KeywordTable(void)
{
    // NOTE(philip): In depth test of table creation, destruction, insertion and duplicate insertion.
    {
        keyword_table Table;
        InitializeKeywordTable(&Table, 2);

        TEST_CHECK(Table.Slots != 0);
        TEST_CHECK(Table.SlotCount == 3);
        TEST_CHECK(Table.KeywordCount == 0);

        keyword *Keyword = InsertIntoKeywordTable(&Table, "hello");

        TEST_CHECK(Keyword != 0);

        TEST_CHECK(Table.KeywordCount == 1);
        TEST_CHECK(Table.Slots[0] == 0);
        TEST_CHECK(Table.Slots[1] == 0);
        TEST_CHECK(Table.Slots[2] != 0);
        TEST_CHECK(Table.Slots[2]->Next == 0);
        TEST_CHECK(strcmp(Table.Slots[2]->Data.Word, "hello") == 0);

        Keyword = InsertIntoKeywordTable(&Table, "hello");

        TEST_CHECK(Keyword != 0);
        TEST_CHECK(strcmp(Keyword->Word, "hello") == 0);

        DestroyKeywordTable(&Table);

        TEST_CHECK(Table.Slots == 0);
        TEST_CHECK(Table.SlotCount == 0);
        TEST_CHECK(Table.KeywordCount == 0);
    }

    // NOTE(philip): Multiple keyword insertions with rehashing. Search.
    {
        keyword_table Table;
        InitializeKeywordTable(&Table, 2);

        keyword *HelloKeyword = InsertIntoKeywordTable(&Table, "hello");
        keyword *WorldKeyword = InsertIntoKeywordTable(&Table, "world");
        keyword *BazingaKeyword = InsertIntoKeywordTable(&Table, "bazinga");

        TEST_CHECK(Table.SlotCount == 6);
        TEST_CHECK(Table.KeywordCount == 3);
        TEST_CHECK(Table.Slots[0] == 0);

        TEST_CHECK(Table.Slots[3] != 0);
        TEST_CHECK(&Table.Slots[3]->Data == BazingaKeyword);
        TEST_CHECK(Table.Slots[3]->Next != 0);
        TEST_CHECK(&Table.Slots[3]->Next->Data == WorldKeyword);
        TEST_CHECK(&Table.Slots[5]->Data == HelloKeyword);

        DestroyKeywordTable(&Table);

    }
}

#define KEYWORD_TREE_MATCH_STORAGE_SIZE 1024

global keyword_tree_match KeywordTreeMatches[KEYWORD_TREE_MATCH_STORAGE_SIZE];
global u64 KeywordTreeMatchCount = 0;

function void
KeywordTree(void)
{
    // NOTE(philip): Hamming Tree.
    {
        char *Words[10] =
        {
            "halt", "oouc", "pops", "oops", "felt", "fell", "smel", "shel", "help", "hell"
        };

        keyword_table Keywords;
        InitializeKeywordTable(&Keywords, ArrayCount(Words));

        for (u64 Index = 0;
             Index < ArrayCount(Words);
             ++Index)
        {
            InsertIntoKeywordTable(&Keywords, Words[Index]);
        }

        keyword_tree Tree;
        InitializeKeywordTree(&Tree, KeywordTreeType_Hamming);

        TEST_CHECK(Tree.Root == 0);

        for (keyword_table_iterator Iterator = IterateKeywordTable(&Keywords);
             IsValid(&Iterator);
             Advance(&Iterator))
        {
            keyword *Keyword = GetValue(&Iterator);
            InsertIntoKeywordTree(&Tree, Keyword);
        }

        {
            char *Solutions[4] =
            {
                Words[0], Words[4], Words[8], Words[9]
            };

            keyword_table SearchValues;
            InitializeKeywordTable(&SearchValues, 1);

            keyword *Keyword = InsertIntoKeywordTable(&SearchValues, "helt");
            FindMatchesInKeywordTree(&Tree, Keyword, &KeywordTreeMatchCount, KEYWORD_TREE_MATCH_STORAGE_SIZE,
                                     KeywordTreeMatches);

            DestroyKeywordTable(&SearchValues);

            for (u64 SolutionIndex = 0;
                 SolutionIndex < ArrayCount(Solutions);
                 ++SolutionIndex)
            {
                b32 Found = false;

                for (u64 Index = 0;
                     Index < KeywordTreeMatchCount;
                     ++Index)
                {
                    keyword_tree_match *Match = KeywordTreeMatches + Index;
                    if (strcmp(Match->Keyword->Word, Solutions[SolutionIndex]) == 0)
                    {
                        Found = true;
                        break;
                    }
                }

                TEST_CHECK(Found == true);
            }
        }

        {
            keyword_table SearchValues;
            InitializeKeywordTable(&SearchValues, 1);

            keyword *Keyword = InsertIntoKeywordTable(&SearchValues, "opsy");
            FindMatchesInKeywordTree(&Tree, Keyword, &KeywordTreeMatchCount, KEYWORD_TREE_MATCH_STORAGE_SIZE,
                                     KeywordTreeMatches);

            DestroyKeywordTable(&SearchValues);
        }

        DestroyKeywordTree(&Tree);
        TEST_CHECK(Tree.Root == 0);

        DestroyKeywordTable(&Keywords);
    }

    // NOTE(philip): Edit Tree.
    {
        char *Words[10] =
        {
            "halt", "oouch", "pop", "oops", "felt", "fell", "smell", "shell", "help", "hell"
        };

        keyword_table Keywords;
        InitializeKeywordTable(&Keywords, ArrayCount(Words));

        for (u64 Index = 0;
             Index < ArrayCount(Words);
             ++Index)
        {
            InsertIntoKeywordTable(&Keywords, Words[Index]);
        }

        keyword_tree Tree;
        InitializeKeywordTree(&Tree, KeywordTreeType_Edit);

        TEST_CHECK(Tree.Root == 0);

        for (keyword_table_iterator Iterator = IterateKeywordTable(&Keywords);
             IsValid(&Iterator);
             Advance(&Iterator))
        {
            keyword *Keyword = GetValue(&Iterator);
            InsertIntoKeywordTree(&Tree, Keyword);
        }

        {
            char *Solutions[6] =
            {
                Words[0], Words[4], Words[5], Words[7], Words[8], Words[9]
            };

            keyword_table SearchValues;
            InitializeKeywordTable(&SearchValues, 1);

            keyword *Keyword = InsertIntoKeywordTable(&SearchValues, "helt");
            FindMatchesInKeywordTree(&Tree, Keyword, &KeywordTreeMatchCount, KEYWORD_TREE_MATCH_STORAGE_SIZE,
                                     KeywordTreeMatches);

            DestroyKeywordTable(&SearchValues);

            for (u64 SolutionIndex = 0;
                 SolutionIndex < ArrayCount(Solutions);
                 ++SolutionIndex)
            {
                b32 Found = false;

                for (u64 Index = 0;
                     Index < KeywordTreeMatchCount;
                     ++Index)
                {
                    keyword_tree_match *Match = KeywordTreeMatches + Index;
                    if (strcmp(Match->Keyword->Word, Solutions[SolutionIndex]) == 0)
                    {
                        Found = true;
                        break;
                    }
                }

                TEST_CHECK(Found == true);
            }
        }

        {
            char *Solutions[2] =
            {
                Words[2], Words[3]
            };

            keyword_table SearchValues;
            InitializeKeywordTable(&SearchValues, 1);

            keyword *Keyword = InsertIntoKeywordTable(&SearchValues, "ops");
            FindMatchesInKeywordTree(&Tree, Keyword, &KeywordTreeMatchCount, KEYWORD_TREE_MATCH_STORAGE_SIZE,
                                     KeywordTreeMatches);

            DestroyKeywordTable(&SearchValues);

            for (u64 SolutionIndex = 0;
                 SolutionIndex < ArrayCount(Solutions);
                 ++SolutionIndex)
            {
                b32 Found = false;

                for (u64 Index = 0;
                     Index < KeywordTreeMatchCount;
                     ++Index)
                {
                    keyword_tree_match *Match = KeywordTreeMatches + Index;
                    if (strcmp(Match->Keyword->Word, Solutions[SolutionIndex]) == 0)
                    {
                        Found = true;
                        break;
                    }
                }

                TEST_CHECK(Found == true);
            }
        }

        DestroyKeywordTree(&Tree);
        DestroyKeywordTable(&Keywords);
    }
}


function u32 *
ValidateNodePreOrder(query_tree_node *Node, u32 *Solutions)
{
    TEST_CHECK(Node->Data.ID == *Solutions);

    if (Node->Left)
    {
        Solutions = ValidateNodePreOrder(Node->Left, Solutions + 1);
    }

    if (Node->Right)
    {
        Solutions = ValidateNodePreOrder(Node->Right, Solutions + 1);
    }

    return Solutions;
}

function void
ValidateTreePreOrder(query_tree *Tree, u32 *Solutions, u32 SolutionCount)
{
    if (Tree->Root)
    {
        ValidateNodePreOrder(Tree->Root, Solutions);
    }
}

function void
QueryTree(void)
{
    // NOTE(philip): In depth test of proper links between nodes and tree rotation for rebalancing.
    {
        query_tree Tree = { };
        TEST_CHECK(Tree.Root == 0);

        InsertIntoQueryTree(&Tree, 100, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 25, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 50, 0, 0, 0);

        query_tree_node *Root = Tree.Root;
        TEST_CHECK(Root != 0);
        TEST_CHECK(Root->Data.ID == 50);

        query_tree_node *Left = Root->Left;
        TEST_CHECK(Left != 0);
        TEST_CHECK(Left->Data.ID == 25);
        TEST_CHECK(Left->Left == 0);
        TEST_CHECK(Left->Right == 0);

        query_tree_node *Right = Root->Right;
        TEST_CHECK(Right != 0);
        TEST_CHECK(Right->Data.ID == 100);
        TEST_CHECK(Right->Left == 0);
        TEST_CHECK(Right->Right == 0);

        u32 Solutions[3] = { 50, 25, 100 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryTree(&Tree);

        TEST_CHECK(Tree.Root == 0);
    }

    // NOTE(philip): High level test of multiple insertions with rebalancing. Duplicate query insertion. Search.
    {
        query_tree Tree = { };

        InsertIntoQueryTree(&Tree, 50, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 10, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 100, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 5, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 15, 0, 0, 0);

        query *Query = InsertIntoQueryTree(&Tree, 25, 0, 0, 0);
        TEST_CHECK(Query->ID == 25);

        Query = InsertIntoQueryTree(&Tree, 25, 0, 0, 0);
        TEST_CHECK(Query->ID == 25);

        u32 Solutions[6] = { 15, 10, 5, 50, 25, 100 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryTree(&Tree);
    }

    // NOTE(philip): High level test of even more insertions.
    {
        query_tree Tree = { };

        InsertIntoQueryTree(&Tree, 50, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 10, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 100, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 5, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 15, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 75, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 150, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 0, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 12, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 20, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 25, 0, 0, 0);

        u32 Solutions[11] = { 15, 10, 5, 0, 12, 50, 20, 25, 100, 75, 150 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryTree(&Tree);
    }

    // NOTE(philip): In depth test of proper links between nodes and rebalancing after node removal.
    {
        query_tree Tree = { };

        InsertIntoQueryTree(&Tree, 10, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 5, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 20, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 25, 0, 0, 0);

        RemoveFromQueryTree(&Tree, 5);

        query_tree_node *Root = Tree.Root;
        TEST_CHECK(Root != 0);
        TEST_CHECK(Root->Data.ID == 20);

        query_tree_node *Left = Root->Left;
        TEST_CHECK(Left != 0);
        TEST_CHECK(Left->Data.ID == 10);
        TEST_CHECK(Left->Left == 0);
        TEST_CHECK(Left->Right == 0);

        query_tree_node *Right = Root->Right;
        TEST_CHECK(Right != 0);
        TEST_CHECK(Right->Data.ID == 25);
        TEST_CHECK(Right->Left == 0);
        TEST_CHECK(Right->Right == 0);

        u32 Solutions[3] = { 20, 10, 25 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryTree(&Tree);
    }

    // NOTE(philip): High level test of a node removal with two children. Removal of query that does not exist.
    {
        query_tree Tree = { };

        InsertIntoQueryTree(&Tree, 10, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 5, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 20, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 25, 0, 0, 0);

        RemoveFromQueryTree(&Tree, 10);
        RemoveFromQueryTree(&Tree, 100);

        u32 Solutions[3] = { 20, 5, 25 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryTree(&Tree);
    }

    // NOTE(philip): High level test of node removal with multiple rotations.
    {
        query_tree Tree = { };

        InsertIntoQueryTree(&Tree, 50, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 25, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 75, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 15, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 35, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 65, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 85, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 40, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 60, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 80, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 90, 0, 0, 0);
        InsertIntoQueryTree(&Tree, 100, 0, 0, 0);

        RemoveFromQueryTree(&Tree, 15);

        u32 Solutions[11] = { 75, 50, 35, 25, 40, 65, 60, 85, 80, 90, 100  };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryTree(&Tree);
    }
}

TEST_LIST =
{
    { "Exact Keyword Matching",       Test_IsExactMatch             },
    { "Hamming Distance Calculation", Test_CalculateHammingDistance },
    { "Edit Distance Calculation",    Test_CalculateEditDistance    },
    { "Query Tree",                   QueryTree                     },
    { "Keyword Table",                KeywordTable                  },
    { "Keyword Tree",                 KeywordTree                   },
    { 0, 0 }
};
