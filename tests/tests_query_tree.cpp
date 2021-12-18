#include "ise_query_tree.h"

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

        QueryTree_Insert(&Tree, 100, 0, 0, 0);
        QueryTree_Insert(&Tree, 25, 0, 0, 0);
        QueryTree_Insert(&Tree, 50, 0, 0, 0);

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

        QueryTree_Destroy(&Tree);

        TEST_CHECK(Tree.Root == 0);
    }

    // NOTE(philip): High level test of multiple insertions with rebalancing. Duplicate query insertion. Search.
    {
        query_tree Tree = { };

        QueryTree_Insert(&Tree, 50, 0, 0, 0);
        QueryTree_Insert(&Tree, 10, 0, 0, 0);
        QueryTree_Insert(&Tree, 100, 0, 0, 0);
        QueryTree_Insert(&Tree, 5, 0, 0, 0);
        QueryTree_Insert(&Tree, 15, 0, 0, 0);

        query_tree_insert_result InsertResult = QueryTree_Insert(&Tree, 25, 0, 0, 0);
        TEST_CHECK(InsertResult.Query->ID == 25);
        TEST_CHECK(InsertResult.Exists == false);

        InsertResult = QueryTree_Insert(&Tree, 25, 0, 0, 0);
        TEST_CHECK(InsertResult.Query->ID == 25);
        TEST_CHECK(InsertResult.Exists == true);

        query *Query = QueryTree_Find(&Tree, 20);
        TEST_CHECK(Query == 0);

        Query = QueryTree_Find(&Tree, 5);
        TEST_CHECK(Query != 0);

        u32 Solutions[6] = { 15, 10, 5, 50, 25, 100 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        QueryTree_Destroy(&Tree);
    }

    // NOTE(philip): High level test of even more insertions.
    {
        query_tree Tree = { };

        QueryTree_Insert(&Tree, 50, 0, 0, 0);
        QueryTree_Insert(&Tree, 10, 0, 0, 0);
        QueryTree_Insert(&Tree, 100, 0, 0, 0);
        QueryTree_Insert(&Tree, 5, 0, 0, 0);
        QueryTree_Insert(&Tree, 15, 0, 0, 0);
        QueryTree_Insert(&Tree, 75, 0, 0, 0);
        QueryTree_Insert(&Tree, 150, 0, 0, 0);
        QueryTree_Insert(&Tree, 0, 0, 0, 0);
        QueryTree_Insert(&Tree, 12, 0, 0, 0);
        QueryTree_Insert(&Tree, 20, 0, 0, 0);
        QueryTree_Insert(&Tree, 25, 0, 0, 0);

        u32 Solutions[11] = { 15, 10, 5, 0, 12, 50, 20, 25, 100, 75, 150 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        QueryTree_Destroy(&Tree);
    }

    // NOTE(philip): In depth test of proper links between nodes and rebalancing after node removal.
    {
        query_tree Tree = { };

        QueryTree_Insert(&Tree, 10, 0, 0, 0);
        QueryTree_Insert(&Tree, 5, 0, 0, 0);
        QueryTree_Insert(&Tree, 20, 0, 0, 0);
        QueryTree_Insert(&Tree, 25, 0, 0, 0);

        QueryTree_Remove(&Tree, 5);

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

        QueryTree_Destroy(&Tree);
    }

    // NOTE(philip): High level test of a node removal with two children. Removal of query that does not exist.
    {
        query_tree Tree = { };

        QueryTree_Insert(&Tree, 10, 0, 0, 0);
        QueryTree_Insert(&Tree, 5, 0, 0, 0);
        QueryTree_Insert(&Tree, 20, 0, 0, 0);
        QueryTree_Insert(&Tree, 25, 0, 0, 0);

        QueryTree_Remove(&Tree, 10);
        QueryTree_Remove(&Tree, 100);

        u32 Solutions[3] = { 20, 5, 25 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        QueryTree_Destroy(&Tree);
    }

    // NOTE(philip): High level test of node removal with multiple rotations.
    {
        query_tree Tree = { };

        QueryTree_Insert(&Tree, 50, 0, 0, 0);
        QueryTree_Insert(&Tree, 25, 0, 0, 0);
        QueryTree_Insert(&Tree, 75, 0, 0, 0);
        QueryTree_Insert(&Tree, 15, 0, 0, 0);
        QueryTree_Insert(&Tree, 35, 0, 0, 0);
        QueryTree_Insert(&Tree, 65, 0, 0, 0);
        QueryTree_Insert(&Tree, 85, 0, 0, 0);
        QueryTree_Insert(&Tree, 40, 0, 0, 0);
        QueryTree_Insert(&Tree, 60, 0, 0, 0);
        QueryTree_Insert(&Tree, 80, 0, 0, 0);
        QueryTree_Insert(&Tree, 90, 0, 0, 0);
        QueryTree_Insert(&Tree, 100, 0, 0, 0);

        QueryTree_Remove(&Tree, 15);

        u32 Solutions[11] = { 75, 50, 35, 25, 40, 65, 60, 85, 80, 90, 100  };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        QueryTree_Destroy(&Tree);
    }
}
