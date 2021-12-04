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

function u64
GetNodeBalance(node *Node)
{
    u64 Balance = 0;

    if (Node)
    {
        u64 LeftHeight = GetNodeHeight(Node->Left);
        u64 RightHeight = GetNodeHeight(Node->Right);

        Balance = LeftHeight - RightHeight;
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
            s64 Balance = LeftHeight - RightHeight;

            if (Balance > 1)
            {
                if (CurrentNode->Left->Data.ID < ID)
                {
                    CurrentNode->Left = RotateTree(CurrentNode->Left, Direction_Left);
                }

                CurrentNode = RotateTree(CurrentNode, Direction_Right);
            }
            else if (Balance < -1)
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
                node *MovingChild = CurrentNode->Right;
                while (MovingChild->Right)
                {
                    MovingChild = MovingChild->Right;
                }

                CurrentNode->Data = MovingChild->Data;
                MovingChild->Parent->Right = 0;

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
        s64 Balance = LeftHeight - RightHeight;

        if (Balance > 1)
        {
            u64 LeftBalance = GetNodeBalance(CurrentNode->Left);
            if (LeftBalance < 0)
            {
                CurrentNode->Left = RotateTree(CurrentNode->Left, Direction_Left);
            }

            CurrentNode = RotateTree(CurrentNode, Direction_Right);
        }
        else if (Balance < -1)
        {
            u64 RightBalance = GetNodeBalance(CurrentNode->Left);
            if (RightBalance < 0)
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
Test(node *Node)
{
    printf("%d ", Node->Data.ID);

    if (Node->Left)
    {
        Test(Node->Left);
    }

    if (Node->Right)
    {
        Test(Node->Right);
    }
}

void
TestQueryMetadata(query_metadata_tree *Tree)
{
    if (Tree->Root)
    {
        Test(Tree->Root);
    }
}
