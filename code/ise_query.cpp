//
// NOTE(phlip): Query
//

function void
UpdateQueryLocation(query *Source, query *Destination)
{
    u64 KeywordCount = GetKeywordCount(Source);

    for (u32 Index = 0;
         Index < KeywordCount;
         ++Index)
    {
        keyword *Keyword = Source->Keywords[Index];

        for (query_list_node *Node = Keyword->Queries.Head;
             Node;
             Node = Node->Next)
        {
            if (Node->Data == Source)
            {
                Node->Data = Destination;
                break;
            }
        }
    }
}

//
// NOTE(philip): Query List
//

function void
InsertIntoQueryList(query_list *List, query *Data)
{
    query_list_node *Node = (query_list_node *)calloc(1, sizeof(query_list_node));
    Node->Data = Data;

    Node->Next = List->Head;
    List->Head = Node;
}

function void
RemoveFromQueryList(query_list *List, query *Query)
{
    query_list_node *Previous = 0;

    for (query_list_node *Node = List->Head;
         Node;
         Node = Node->Next)
    {
        if (Node->Data == Query)
        {
            if (Previous)
            {
                Previous->Next = Node->Next;
            }
            else
            {
                List->Head = Node->Next;
            }

            free(Node);
            break;
        }

        Previous = Node;
    }
}

//
// NOTE(philip): Query AVL-Tree
//

function void
InitializeQueryTree(query_tree *Tree)
{
    Tree->Root = 0;
}

function u64
GetSubtreeHeight(query_tree_node *Subtree)
{
    u64 Height = (Subtree ? Subtree->Height : 0);
    return Height;
}

function void
UpdateSubtreeHeight(query_tree_node *Subtree)
{
    u64 LeftHeight = GetSubtreeHeight(Subtree->Left);
    u64 RightHeight = GetSubtreeHeight(Subtree->Right);

    Subtree->Height = Max(LeftHeight, RightHeight) + 1;
}

function s64
GetSubtreeBalance(query_tree_node *Subtree)
{
    s64 Balance = 0;

    if (Subtree)
    {
        u64 LeftHeight = GetSubtreeHeight(Subtree->Left);
        u64 RightHeight = GetSubtreeHeight(Subtree->Right);

        Balance = (s64)(RightHeight - LeftHeight);
    }

    return Balance;
}

function query_tree_node *
RotateSubtreeLeft(query_tree_node *Subtree)
{
    query_tree_node *NewSubtree = Subtree->Right;
    Subtree->Right = NewSubtree->Left;
    NewSubtree->Left = Subtree;

    UpdateSubtreeHeight(Subtree);
    UpdateSubtreeHeight(NewSubtree);

    return NewSubtree;
}

function query_tree_node *
RotateSubtreeRight(query_tree_node *Subtree)
{
    query_tree_node *NewSubtree = Subtree->Left;
    Subtree->Left = NewSubtree->Right;
    NewSubtree->Right = Subtree;

    UpdateSubtreeHeight(Subtree);
    UpdateSubtreeHeight(NewSubtree);

    return NewSubtree;
}

function query_tree_node *
InsertIntoSubtree(query_tree_node *Subtree, u32 ID, query **Query, b32 *Exists)
{
    query_tree_node *NewSubtree = Subtree;

    if (Subtree)
    {
        if (Subtree->Data.ID > ID)
        {
            Subtree->Left = InsertIntoSubtree(Subtree->Left, ID, Query, Exists);
        }
        else if (Subtree->Data.ID < ID)
        {
            Subtree->Right = InsertIntoSubtree(Subtree->Right, ID, Query, Exists);
        }
        else
        {
            *Query = &Subtree->Data;
            *Exists = true;
        }

        if (!(*Exists))
        {
            u64 LeftHeight = GetSubtreeHeight(Subtree->Left);
            u64 RightHeight = GetSubtreeHeight(Subtree->Right);
            Subtree->Height = Max(LeftHeight, RightHeight) + 1;

            s64 Balance = (s64)(RightHeight - LeftHeight);
            if (Balance < -1)
            {
                if (Subtree->Left->Data.ID < ID)
                {
                    Subtree->Left = RotateSubtreeLeft(Subtree->Left);
                }

                NewSubtree = RotateSubtreeRight(Subtree);
            }
            else if (Balance > 1)
            {
                if (Subtree->Right->Data.ID > ID)
                {
                    Subtree->Right = RotateSubtreeRight(Subtree->Right);
                }

                NewSubtree = RotateSubtreeLeft(Subtree);
            }
        }
    }
    else
    {
        NewSubtree = (query_tree_node *)calloc(1, sizeof(query_tree_node));
        NewSubtree->Data.ID = ID;
        NewSubtree->Height = 1;

        *Query = &NewSubtree->Data;
    }

    return NewSubtree;
}

function query *
InsertIntoQueryTree(query_tree *Tree, u32 ID, u64 KeywordCount, query_type Type, u64 DistanceThreshold)
{
    query *Query = 0;
    b32 Exists = false;

    Tree->Root = InsertIntoSubtree(Tree->Root, ID, &Query, &Exists);

    if (Query->PackedInfo == 0)
    {
        Query->PackedInfo = ((KeywordCount << 5) | (Type << 3) | DistanceThreshold);
    }

    return Query;
}

function query *
FindQueryInSubtree(query_tree_node *Subtree, u32 ID)
{
    query *Query = 0;

    if (Subtree)
    {
        if (Subtree->Data.ID > ID)
        {
            Query = FindQueryInSubtree(Subtree->Left, ID);
        }
        else if (Subtree->Data.ID < ID)
        {
            Query = FindQueryInSubtree(Subtree->Right, ID);
        }
        else
        {
            Query = &Subtree->Data;
        }
    }

    return Query;
}

function query *
FindQueryInTree(query_tree *Tree, u32 ID)
{
    query *Query = FindQueryInSubtree(Tree->Root, ID);
    return Query;
}

function query_tree_node *
RemoveFromSubtree(query_tree_node *Subtree, u32 ID)
{
    query_tree_node *NewSubtree = Subtree;

    if (Subtree)
    {
        b32 CheckForBalance = true;

        if (Subtree->Data.ID > ID)
        {
            Subtree->Left = RemoveFromSubtree(Subtree->Left, ID);
        }
        else if (Subtree->Data.ID < ID)
        {
            Subtree->Right = RemoveFromSubtree(Subtree->Right, ID);
        }
        else
        {
            if (Subtree->Left && Subtree->Right)
            {
                query_tree_node *MovingChild = Subtree->Right;
                while (MovingChild->Left)
                {
                    MovingChild = MovingChild->Left;
                }

                Subtree->Data = MovingChild->Data;
                UpdateQueryLocation(&MovingChild->Data, &Subtree->Data);

                Subtree->Right = RemoveFromSubtree(Subtree->Right, MovingChild->Data.ID);
            }
            else
            {
                query_tree_node *Child = (Subtree->Left ? Subtree->Left : Subtree->Right);

                if (Child)
                {
                    *Subtree = *Child;
                    UpdateQueryLocation(&Child->Data, &Subtree->Data);

                    free(Child);
                }
                else
                {
                    NewSubtree = 0;
                    CheckForBalance = false;

                    free(Subtree);
                }
            }
        }

        if (CheckForBalance)
        {
            u64 LeftHeight = GetSubtreeHeight(Subtree->Left);
            u64 RightHeight = GetSubtreeHeight(Subtree->Right);
            Subtree->Height = Max(LeftHeight, RightHeight) + 1;

            s64 Balance = (s64)(RightHeight - LeftHeight);
            if (Balance < -1)
            {
                s64 LeftBalance = GetSubtreeBalance(Subtree->Left);
                if (LeftBalance >= 0)
                {
                    Subtree->Left = RotateSubtreeLeft(Subtree->Left);
                }

                NewSubtree = RotateSubtreeRight(Subtree);
            }
            else if (Balance > 1)
            {
                s64 RightBalance = GetSubtreeBalance(Subtree->Right);
                if (RightBalance <= 0)
                {
                    Subtree->Right = RotateSubtreeRight(Subtree->Right);
                }

                NewSubtree = RotateSubtreeLeft(Subtree);
            }
        }
    }

    return NewSubtree;
}

function void
RemoveFromQueryTree(query_tree *Tree, u32 ID)
{
    Tree->Root = RemoveFromSubtree(Tree->Root, ID);
}

function void
DestroySubtree(query_tree_node *Subtree)
{
    if (Subtree->Left)
    {
        DestroySubtree(Subtree->Left);
    }

    if (Subtree->Right)
    {
        DestroySubtree(Subtree->Right);
    }

    free(Subtree);
}

function void
DestroyQueryTree(query_tree *Tree)
{
    if (Tree->Root)
    {
        DestroySubtree(Tree->Root);
    }

    Tree->Root = 0;
}
