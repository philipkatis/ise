#include "ise_query_tree.h"

#include <stdlib.h>

function u64
GetNodeHeight(query_tree_node *Node)
{
    u64 Height = (Node ? Node->Height : 0);
    return Height;
}

function void
UpdateNodeHeight(query_tree_node *Node)
{
    u64 LeftHeight = GetNodeHeight(Node->Left);
    u64 RightHeight = GetNodeHeight(Node->Right);

    Node->Height = Max(LeftHeight, RightHeight) + 1;
}

function s64
GetNodeBalance(query_tree_node *Node)
{
    s64 Balance = 0;

    if (Node)
    {
        u64 LeftHeight = GetNodeHeight(Node->Left);
        u64 RightHeight = GetNodeHeight(Node->Right);

        Balance = RightHeight - LeftHeight;
    }

    return Balance;
}

function query_tree_node *
RotateLeft(query_tree_node *Root)
{
    query_tree_node *NewRoot = Root->Right;
    Root->Right = NewRoot->Left;
    NewRoot->Left = Root;

    UpdateNodeHeight(Root);
    UpdateNodeHeight(NewRoot);

    return NewRoot;
}

function query_tree_node *
RotateRight(query_tree_node *Root)
{
    query_tree_node *NewRoot = Root->Left;
    Root->Left = NewRoot->Right;
    NewRoot->Right = Root;

    UpdateNodeHeight(Root);
    UpdateNodeHeight(NewRoot);

    return NewRoot;
}

function query_tree_node *
Insert(query_tree_node *Root, u32 ID, query **Query, b32 *Exists)
{
    query_tree_node *NewRoot = Root;

    if (Root)
    {
        if (Root->Data.ID > ID)
        {
            Root->Left = Insert(Root->Left, ID, Query, Exists);
        }
        else if (Root->Data.ID < ID)
        {
            Root->Right = Insert(Root->Right, ID, Query, Exists);
        }
        else
        {
            *Query = &NewRoot->Data;
            *Exists = true;
        }

        if (!(*Exists))
        {
            u64 LeftHeight = GetNodeHeight(Root->Left);
            u64 RightHeight = GetNodeHeight(Root->Right);

            Root->Height = Max(LeftHeight, RightHeight) + 1;
            s64 Balance = RightHeight - LeftHeight;

            if (Balance < -1)
            {
                if (Root->Left->Data.ID < ID)
                {
                    Root->Left = RotateLeft(Root->Left);
                }

                NewRoot = RotateRight(Root);
            }
            else if (Balance > 1)
            {
                if (Root->Right->Data.ID > ID)
                {
                    Root->Right = RotateRight(Root->Right);
                }

                NewRoot = RotateLeft(Root);
            }
        }
    }
    else
    {
        NewRoot = (query_tree_node *)calloc(1, sizeof(query_tree_node));
        NewRoot->Data.ID = ID;
        NewRoot->Height = 1;

        *Query = &NewRoot->Data;
    }

    return NewRoot;
}

query_tree_insert_result
QueryTree_Insert(query_tree *Tree, u32 ID)
{
    query_tree_insert_result Result = { };

    Tree->Root = Insert(Tree->Root, ID, &Result.Query, &Result.Exists);
    if (Result.Query && !Result.Exists)
    {
        ++Tree->Count;
    }

    return Result;
}

function query_tree_node *
Remove(query_tree_node *Root, u32 ID, b32 *Removed)
{
    query_tree_node *NewRoot = Root;

    if (Root)
    {
        b32 CheckForBalance = true;

        if (Root->Data.ID > ID)
        {
            Root->Left = Remove(Root->Left, ID, Removed);
        }
        else if (Root->Data.ID < ID)
        {
            Root->Right = Remove(Root->Right, ID, Removed);
        }
        else
        {
            if (Root->Left && Root->Right)
            {
                query_tree_node *MovingChild = Root->Right;
                while (MovingChild->Left)
                {
                    MovingChild = MovingChild->Left;
                }

                Root->Data = MovingChild->Data;
                Root->Right = Remove(Root->Right, MovingChild->Data.ID, 0);
            }
            else
            {
                query_tree_node *Child = Root->Left ? Root->Left : Root->Right;

                if (Child)
                {
                    *Root = *Child;
                    free(Child);
                }
                else
                {
                    NewRoot = 0;
                    CheckForBalance = false;

                    free(Root);
                }
            }

            if (Removed)
            {
                *Removed = true;
            }
        }

        if (CheckForBalance)
        {
            u64 LeftHeight = GetNodeHeight(Root->Left);
            u64 RightHeight = GetNodeHeight(Root->Right);

            Root->Height = Max(LeftHeight, RightHeight) + 1;
            s64 Balance = RightHeight - LeftHeight;

            if (Balance < -1)
            {
                s64 LeftBalance = GetNodeBalance(Root->Left);
                if (LeftBalance >= 0)
                {
                    Root->Left = RotateLeft(Root->Left);
                }

                NewRoot = RotateRight(Root);
            }
            else if (Balance > 1)
            {
                s64 RightBalance = GetNodeBalance(NewRoot->Right);
                if (RightBalance <= 0)
                {
                    Root->Right = RotateRight(NewRoot->Right);
                }

                NewRoot = RotateLeft(Root);
            }
        }
    }

    return NewRoot;
}

void
QueryTree_Remove(query_tree *Tree, u32 ID)
{
    b32 Removed = false;
    Tree->Root = Remove(Tree->Root, ID, &Removed);

    if (Removed)
    {
        --Tree->Count;
    }
}

function void
DestroyNode(query_tree_node *Node)
{
    if (Node->Left)
    {
        DestroyNode(Node->Left);
    }

    if (Node->Right)
    {
        DestroyNode(Node->Right);
    }

    free(Node);
}

void
QueryTree_Destroy(query_tree *Tree)
{
    if (Tree->Root)
    {
        DestroyNode(Tree->Root);
    }

    Tree->Root = 0;
    Tree->Count = 0;
}

#if ISE_DEBUG
    function void
    PrintTabs(u64 Count)
    {
        for (u64 Index = 0;
             Index < Count;
             ++Index)
        {
            printf("    ");
        }
    }

    function void
    VisualizeNode(query_tree_node *Node, u64 Depth = 0)
    {
        query *Data = &Node->Data;

        PrintTabs(Depth);
        printf("{\n");

        PrintTabs(Depth + 1);
        printf("Height: %llu\n", GetNodeHeight(Node));

        PrintTabs(Depth + 1);
        printf("Balance: %lld\n\n", GetNodeBalance(Node));

        PrintTabs(Depth + 1);
        printf("Data:\n");

        PrintTabs(Depth + 1);
        printf("{\n");

        PrintTabs(Depth + 2);
        printf("ID: %d\n", Data->ID);

        PrintTabs(Depth + 1);
        printf("}\n");

        if (Node->Left)
        {
            printf("\n");

            PrintTabs(Depth + 1);
            printf("Left Child:\n");

            VisualizeNode(Node->Left, Depth + 1);
        }

        if (Node->Right)
        {
            printf("\n");

            PrintTabs(Depth + 1);
            printf("Right Child:\n");

            VisualizeNode(Node->Right, Depth + 1);
        }

        PrintTabs(Depth);
        printf("}\n");
    }

    void
    QueryTree_Visualize_(query_tree *Tree)
    {
        if (Tree->Root)
        {
            printf("Root:\n");
            VisualizeNode(Tree->Root);
        }
    }
#endif
