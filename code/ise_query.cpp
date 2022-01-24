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
                Subtree->Right = RemoveFromSubtree(Subtree->Right, MovingChild->Data.ID);
            }
            else
            {
                query_tree_node *Child = (Subtree->Left ? Subtree->Left : Subtree->Right);

                if (Child)
                {
                    *Subtree = *Child;
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
