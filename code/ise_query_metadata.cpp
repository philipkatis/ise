#include "ise_query_metadata.h"

#include <stdlib.h>

struct node
{
    query_metadata Data;

    node *Parent;
    u64 Height;

    node *Left;
    node *Right;
};

function node *
AllocateNode(u32 ID, node *Parent)
{
    node *Node = (node *)calloc(1, sizeof(node));

    Node->Data.ID = ID;

    Node->Parent = Parent;
    Node->Height = 1;

    return Node;
}

function u64
GetNodeHeight(node *Node)
{
    u64 Height = (Node ? Node->Height : 0);
    return Height;
}

function void
UpdateNodeHeight(node *Node)
{
    u64 LeftHeight = GetNodeHeight(Node->Left);
    u64 RightHeight = GetNodeHeight(Node->Right);

    Node->Height = Max(LeftHeight, RightHeight) + 1;
}

function s64
GetNodeBalance(node *Node)
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

function node *
RotateTree(node *Root, direction Direction)
{
    node *OldRoot = Root;
    node *NewRoot = 0;

    switch (Direction)
    {
        case Direction_Left:
        {
            NewRoot = OldRoot->Right;
        } break;

        case Direction_Right:
        {
            NewRoot = OldRoot->Left;
        } break;
    }

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

    node *MovingChild = 0;

    switch (Direction)
    {
        case Direction_Left:
        {
            MovingChild = NewRoot->Left;

            OldRoot->Right = MovingChild;
            NewRoot->Left = OldRoot;
        } break;

        case Direction_Right:
        {
            MovingChild = NewRoot->Right;

            OldRoot->Left = MovingChild;
            NewRoot->Right = OldRoot;
        } break;
    }

    if (MovingChild)
    {
        MovingChild->Parent = OldRoot;
    }

    UpdateNodeHeight(OldRoot);
    UpdateNodeHeight(NewRoot);

    return NewRoot;
}

query_metadata *
FindOrInsertQueryMetadata(query_metadata_tree *Tree, u32 ID)
{
    node *NewNode = 0;

    if (Tree->Root)
    {
        node *LastValidNode = 0;
        node *CurrentNode = Tree->Root;

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
                return &CurrentNode->Data;
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
        node *PotentialNewRoot = 0;

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
                    CurrentNode->Left = RotateTree(CurrentNode->Left, Direction_Left);
                }

                CurrentNode = RotateTree(CurrentNode, Direction_Right);
            }
            else if (Balance > 1)
            {
                if (CurrentNode->Right->Data.ID > ID)
                {
                    CurrentNode->Right = RotateTree(CurrentNode->Right, Direction_Right);
                }

                CurrentNode = RotateTree(CurrentNode, Direction_Left);
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

    return &NewNode->Data;
}

void
RemoveQueryMetadata(query_metadata_tree *Tree, u32 ID)
{
    b32 Found = false;
    node *ParentOfRemovedNode = 0;
    node *CurrentNode = Tree->Root;

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
                node *MovingChild = CurrentNode->Left;
                while (MovingChild->Right)
                {
                    MovingChild = MovingChild->Right;
                }

                node *Parent = MovingChild->Parent;

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
                node *Child = CurrentNode->Left ? CurrentNode->Left : CurrentNode->Right;
                node *Parent = CurrentNode->Parent;

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
    node *PotentialNewRoot = 0;

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
                CurrentNode->Left = RotateTree(CurrentNode->Left, Direction_Left);
            }

            CurrentNode = RotateTree(CurrentNode, Direction_Right);
        }
        else if (Balance > 1)
        {
            s64 RightBalance = GetNodeBalance(CurrentNode->Left);
            if (RightBalance >= 0)
            {
                CurrentNode->Right = RotateTree(CurrentNode->Right, Direction_Right);
            }

            CurrentNode = RotateTree(CurrentNode, Direction_Left);
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
DestroyNode(node *Node)
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
DestroyQueryMetadataTree(query_metadata_tree *Tree)
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
    VisualizeNode(node *Node, u64 Depth = 0)
    {
        query_metadata *Data = &Node->Data;

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
    VisualizeQueryMetadataTree_(query_metadata_tree *Tree)
    {
        if (Tree->Root)
        {
            printf("Root:\n");
            VisualizeNode(Tree->Root);
        }
    }
#endif
