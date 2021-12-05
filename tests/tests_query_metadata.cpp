#include "ise_query_metadata.h"

function u32 *
ValidateNodePreOrder(query_metadata_tree_node *Node, u32 *Solutions)
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
ValidateTreePreOrder(query_metadata_tree *Tree, u32 *Solutions, u32 SolutionCount)
{
    TEST_CHECK(Tree->Count == SolutionCount);

    if (Tree->Root)
    {
        ValidateNodePreOrder(Tree->Root, Solutions);
    }
}

function void
QueryMetadata(void)
{
    // NOTE(philip): In depth test of proper links between nodes and tree rotation for rebalancing.
    {
        query_metadata_tree Tree = { };

        TEST_CHECK(Tree.Root == 0);
        TEST_CHECK(Tree.Count == 0);

        FindOrInsertQueryMetadata(&Tree, 100);
        FindOrInsertQueryMetadata(&Tree, 25);
        FindOrInsertQueryMetadata(&Tree, 50);

        TEST_CHECK(Tree.Count == 3);

        query_metadata_tree_node *Root = Tree.Root;
        TEST_CHECK(Root != 0);
        TEST_CHECK(Root->Data.ID == 50);
        TEST_CHECK(Root->Parent == 0);

        query_metadata_tree_node *Left = Root->Left;
        TEST_CHECK(Left != 0);
        TEST_CHECK(Left->Data.ID == 25);
        TEST_CHECK(Left->Parent == Root);
        TEST_CHECK(Left->Left == 0);
        TEST_CHECK(Left->Right == 0);

        query_metadata_tree_node *Right = Root->Right;
        TEST_CHECK(Right != 0);
        TEST_CHECK(Right->Data.ID == 100);
        TEST_CHECK(Right->Parent == Root);
        TEST_CHECK(Right->Left == 0);
        TEST_CHECK(Right->Right == 0);

        u32 Solutions[3] = { 50, 25, 100 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryMetadataTree(&Tree);

        TEST_CHECK(Tree.Root == 0);
        TEST_CHECK(Tree.Count == 0);
    }

    // NOTE(philip): High level test of multiple insertions with rebalancing.
    {
        query_metadata_tree Tree = { };

        FindOrInsertQueryMetadata(&Tree, 50);
        FindOrInsertQueryMetadata(&Tree, 10);
        FindOrInsertQueryMetadata(&Tree, 100);
        FindOrInsertQueryMetadata(&Tree, 5);
        FindOrInsertQueryMetadata(&Tree, 15);
        FindOrInsertQueryMetadata(&Tree, 25);

        u32 Solutions[6] = { 15, 10, 5, 50, 25, 100 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryMetadataTree(&Tree);
    }

    // NOTE(philip): High level test of even more insertions.
    {
        query_metadata_tree Tree = { };

        FindOrInsertQueryMetadata(&Tree, 50);
        FindOrInsertQueryMetadata(&Tree, 10);
        FindOrInsertQueryMetadata(&Tree, 100);
        FindOrInsertQueryMetadata(&Tree, 5);
        FindOrInsertQueryMetadata(&Tree, 15);
        FindOrInsertQueryMetadata(&Tree, 75);
        FindOrInsertQueryMetadata(&Tree, 150);
        FindOrInsertQueryMetadata(&Tree, 0);
        FindOrInsertQueryMetadata(&Tree, 12);
        FindOrInsertQueryMetadata(&Tree, 20);
        FindOrInsertQueryMetadata(&Tree, 25);

        u32 Solutions[11] = { 15, 10, 5, 0, 12, 50, 20, 25, 100, 75, 150 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryMetadataTree(&Tree);
    }

    // NOTE(philip): In depth test of proper links between nodes and rebalancing after node removal.
    {
        query_metadata_tree Tree = { };

        FindOrInsertQueryMetadata(&Tree, 10);
        FindOrInsertQueryMetadata(&Tree, 5);
        FindOrInsertQueryMetadata(&Tree, 20);
        FindOrInsertQueryMetadata(&Tree, 25);

        RemoveQueryMetadata(&Tree, 5);

        TEST_CHECK(Tree.Count == 3);

        query_metadata_tree_node *Root = Tree.Root;
        TEST_CHECK(Root != 0);
        TEST_CHECK(Root->Data.ID == 20);
        TEST_CHECK(Root->Parent == 0);

        query_metadata_tree_node *Left = Root->Left;
        TEST_CHECK(Left != 0);
        TEST_CHECK(Left->Data.ID == 10);
        TEST_CHECK(Left->Parent == Root);
        TEST_CHECK(Left->Left == 0);
        TEST_CHECK(Left->Right == 0);

        query_metadata_tree_node *Right = Root->Right;
        TEST_CHECK(Right != 0);
        TEST_CHECK(Right->Data.ID == 25);
        TEST_CHECK(Right->Parent == Root);
        TEST_CHECK(Right->Left == 0);
        TEST_CHECK(Right->Right == 0);

        u32 Solutions[3] = { 20, 10, 25 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryMetadataTree(&Tree);
    }

    // NOTE(philip): High level test of a node removal with two children.
    {
        query_metadata_tree Tree = { };

        FindOrInsertQueryMetadata(&Tree, 10);
        FindOrInsertQueryMetadata(&Tree, 5);
        FindOrInsertQueryMetadata(&Tree, 20);
        FindOrInsertQueryMetadata(&Tree, 25);

        RemoveQueryMetadata(&Tree, 10);

        u32 Solutions[3] = { 20, 5, 25 };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryMetadataTree(&Tree);
    }

    // NOTE(philip): High level test of node removal with multiple rotations.
    {
        query_metadata_tree Tree = { };

        FindOrInsertQueryMetadata(&Tree, 50);

        FindOrInsertQueryMetadata(&Tree, 25);
        FindOrInsertQueryMetadata(&Tree, 75);

        FindOrInsertQueryMetadata(&Tree, 15);
        FindOrInsertQueryMetadata(&Tree, 35);
        FindOrInsertQueryMetadata(&Tree, 65);
        FindOrInsertQueryMetadata(&Tree, 85);

        FindOrInsertQueryMetadata(&Tree, 40);
        FindOrInsertQueryMetadata(&Tree, 60);
        FindOrInsertQueryMetadata(&Tree, 80);
        FindOrInsertQueryMetadata(&Tree, 90);
        FindOrInsertQueryMetadata(&Tree, 100);

        RemoveQueryMetadata(&Tree, 15);

        u32 Solutions[11] = { 75, 50, 35, 25, 40, 65, 60, 85, 80, 90, 100  };
        ValidateTreePreOrder(&Tree, Solutions, ArrayCount(Solutions));

        DestroyQueryMetadataTree(&Tree);
    }
}
