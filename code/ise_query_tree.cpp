#include "ise_query_tree.h"

#include <stdlib.h>

function query_tree_node *
AllocateNode(u32 ID, query_tree_node *Parent)
{
    query_tree_node *Node = (query_tree_node *)calloc(1, sizeof(query_tree_node));

    Node->Data.ID = ID;

    Node->Parent = Parent;
    Node->Height = 1;

    return Node;
}

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
    query_tree_node *OldRoot = Root;
    query_tree_node *NewRoot = OldRoot->Right;

    if (OldRoot->Parent)
    {
        if (OldRoot->Parent->Right == OldRoot)
        {
            OldRoot->Parent->Right = NewRoot;
        }
        else if (OldRoot->Parent->Left == OldRoot)
        {
            OldRoot->Parent->Left = NewRoot;
        }
    }

    NewRoot->Parent = OldRoot->Parent;
    OldRoot->Parent = NewRoot;

    query_tree_node *MovingChild = NewRoot->Left;

    if (MovingChild)
    {
        MovingChild->Parent = OldRoot;
    }

    OldRoot->Right = MovingChild;
    NewRoot->Left = OldRoot;

    UpdateNodeHeight(OldRoot);
    UpdateNodeHeight(NewRoot);

    return NewRoot;
}

function query_tree_node *
RotateRight(query_tree_node *Root)
{
    query_tree_node *OldRoot = Root;
    query_tree_node *NewRoot = OldRoot->Left;

    if (OldRoot->Parent)
    {
        if (OldRoot->Parent->Right == OldRoot)
        {
            OldRoot->Parent->Right = NewRoot;
        }
        else if (OldRoot->Parent->Left == OldRoot)
        {
            OldRoot->Parent->Left = NewRoot;
        }
    }

    NewRoot->Parent = OldRoot->Parent;
    OldRoot->Parent = NewRoot;

    query_tree_node *MovingChild = NewRoot->Right;

    if (MovingChild)
    {
        MovingChild->Parent = OldRoot;
    }

    OldRoot->Left = MovingChild;
    NewRoot->Right = OldRoot;

    UpdateNodeHeight(OldRoot);
    UpdateNodeHeight(NewRoot);

    return NewRoot;
}

query_tree_insert_result
QueryTree_Insert(query_tree *Tree, u32 ID)
{
    query_tree_insert_result Result = { };
    query_tree_node *NewNode = 0;

    if (Tree->Root)
    {
        query_tree_node *LastValidNode = 0;
        query_tree_node *CurrentNode = Tree->Root;

        while (CurrentNode)
        {
            LastValidNode = CurrentNode;

            if (CurrentNode->Data.ID > ID)
            {
                CurrentNode = CurrentNode->Left;
            }
            else if (CurrentNode->Data.ID < ID)
            {
                CurrentNode = CurrentNode->Right;
            }
            else
            {
                Result.Query = &CurrentNode->Data;
                Result.Exists = true;

                return Result;
            }
        }

        NewNode = AllocateNode(ID, LastValidNode);

        if (LastValidNode->Data.ID > ID)
        {
            LastValidNode->Left = NewNode;
        }
        else if (LastValidNode->Data.ID < ID)
        {
            LastValidNode->Right = NewNode;
        }

        CurrentNode = NewNode->Parent;
        query_tree_node *PotentialNewRoot = 0;

        while (CurrentNode)
        {
            PotentialNewRoot = CurrentNode;

            u64 LeftHeight = GetNodeHeight(CurrentNode->Left);
            u64 RightHeight = GetNodeHeight(CurrentNode->Right);

            CurrentNode->Height = Max(LeftHeight, RightHeight) + 1;
            s64 Balance = RightHeight - LeftHeight;

            if (Balance < -1)
            {
                if (CurrentNode->Left->Data.ID < ID)
                {
                    CurrentNode->Left = RotateLeft(CurrentNode->Left);
                }

                CurrentNode = RotateRight(CurrentNode);
            }
            else if (Balance > 1)
            {
                if (CurrentNode->Right->Data.ID > ID)
                {
                    CurrentNode->Right = RotateRight(CurrentNode->Right);
                }

                CurrentNode = RotateLeft(CurrentNode);
            }
            else
            {
                CurrentNode = CurrentNode->Parent;
            }
        }

        Tree->Root = PotentialNewRoot;
    }
    else
    {
        NewNode = AllocateNode(ID, 0);
        Tree->Root = NewNode;
    }

    ++Tree->Count;

    Result.Query = &NewNode->Data;
    return Result;
}

void
QueryTree_Remove(query_tree *Tree, u32 ID)
{
    b32 Found = false;
    query_tree_node *ParentOfRemovedNode = 0;
    query_tree_node *CurrentNode = Tree->Root;

    while (CurrentNode)
    {
        if (CurrentNode->Data.ID > ID)
        {
            CurrentNode = CurrentNode->Left;
        }
        else if (CurrentNode->Data.ID < ID)
        {
            CurrentNode = CurrentNode->Right;
        }
        else
        {
            if (CurrentNode->Left && CurrentNode->Right)
            {
                query_tree_node *MovingChild = CurrentNode->Left;
                while (MovingChild->Right)
                {
                    MovingChild = MovingChild->Right;
                }

                query_tree_node *Parent = MovingChild->Parent;

                if (Parent)
                {
                    if (Parent->Left == MovingChild)
                    {
                        Parent->Left = 0;
                    }
                    else if (Parent->Right == MovingChild)
                    {
                        Parent->Right = 0;
                    }
                }

                CurrentNode->Data = MovingChild->Data;
                ParentOfRemovedNode = MovingChild->Parent;

                free(MovingChild);
            }
            else
            {
                query_tree_node *Child = CurrentNode->Left ? CurrentNode->Left : CurrentNode->Right;
                query_tree_node *Parent = CurrentNode->Parent;

                if (Child)
                {
                    *CurrentNode = *Child;
                    CurrentNode->Parent = Parent;

                    free(Child);
                }
                else
                {
                    if (Parent)
                    {
                        if (Parent->Left == CurrentNode)
                        {
                            Parent->Left = Child;
                        }
                        else if (Parent->Right == CurrentNode)
                        {
                            Parent->Right = Child;
                        }
                    }

                    free(CurrentNode);
                }

                ParentOfRemovedNode = Parent;
            }

            Found = true;
            break;
        }
    }

    if (!Found)
    {
        return;
    }

    CurrentNode = ParentOfRemovedNode;
    query_tree_node *PotentialNewRoot = 0;

    while (CurrentNode)
    {
        PotentialNewRoot = CurrentNode;

        u64 LeftHeight = GetNodeHeight(CurrentNode->Left);
        u64 RightHeight = GetNodeHeight(CurrentNode->Right);

        CurrentNode->Height = Max(LeftHeight, RightHeight) + 1;
        s64 Balance = RightHeight - LeftHeight;

        if (Balance < -1)
        {
            s64 LeftBalance = GetNodeBalance(CurrentNode->Left);
            if (LeftBalance >= 0)
            {
                CurrentNode->Left = RotateLeft(CurrentNode->Left);
            }

            CurrentNode = RotateRight(CurrentNode);
        }
        else if (Balance > 1)
        {
            s64 RightBalance = GetNodeBalance(CurrentNode->Right);
            if (RightBalance <= 0)
            {
                CurrentNode->Right = RotateRight(CurrentNode->Right);
            }

            CurrentNode = RotateLeft(CurrentNode);
        }
        else
        {
            CurrentNode = CurrentNode->Parent;
        }
    }

    Tree->Root = PotentialNewRoot;
    --Tree->Count;
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
    VisualizeQueryMetadataTree_(query_tree *Tree)
    {
        if (Tree->Root)
        {
            printf("Root:\n");
            VisualizeNode(Tree->Root);
        }
    }
#endif
