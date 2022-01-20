//
// NOTE(philip): Creates a query tree.
//

function query_tree
QueryTree_Create()
{
    query_tree Tree = {0};
    InitializeMemoryArena(&Tree.Arena, MB(2));

    return Tree;
}

//
// NOTE(philip): Returns the height of a node in the tree, if it exists. Otherwise, returns zero.
//

function u64
GetNodeHeight(query_tree_node *Node)
{
    u64 Height = (Node ? Node->Height : 0);
    return Height;
}

//
// NOTE(philip): Updates the height of a node in the tree.
//

function void
UpdateNodeHeight(query_tree_node *Node)
{
    u64 LeftHeight = GetNodeHeight(Node->Left);
    u64 RightHeight = GetNodeHeight(Node->Right);

    Node->Height = Max(LeftHeight, RightHeight) + 1;
}

//
// NOTE(philip): Returns the balance of a node in the tree. Positive values mean the node is right-heavy, negative
// values mean the node is left-heavy. Zero means the node is perfectly balanced.
//

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

//
//  NOTE(philip): Rotates a subtree once counter-clockwise, updates the node links and heights.
//

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

//
// NOTE(philip): Rotates a subtree once clockwise, updates the node links and heights.
//

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

//
// NOTE(philip): Allocates a new node.
//

function query_tree_node *
AllocateNode(query_tree *Tree, u32 ID)
{
    query_tree_node *Node = 0;

    if (Tree->FreeNode)
    {
        // NOTE(philip): If there is a node in the free list, pop it and use it.
        Node = Tree->FreeNode;
        Tree->FreeNode = Node->Next;

        memset(Node, 0, sizeof(query_tree_node));
    }
    else
    {
        // NOTE(philip): if no node is available, allocate a new one from the arena.
        Node = PushStruct(&Tree->Arena, query_tree_node);
    }

    Node->Data.ID = ID;
    Node->Height = 1;

    return Node;
}

//
// NOTE(philip): Deallocates a node from the tree.
//

function void
DeallocateNode(query_tree *Tree, query_tree_node *Node)
{
    // NOTE(philip): Push the node onto the free list.
    Node->Next = Tree->FreeNode;
    Tree->FreeNode = Node;
}

//
// NOTE(philip): Recursively inserts a new node into a subtree, with proper balancing. Returns the new root of the
// subtree. The new query data location is sent back up the stack by the function parameter. If the query already
// exists in the subtree, the function parameter flag is set.
//

function query_tree_node *
Insert(query_tree *Tree, query_tree_node *Root, u32 ID, query **Query, b64 *Exists)
{
    query_tree_node *NewRoot = Root;

    if (Root)
    {
        if (Root->Data.ID > ID)
        {
            Root->Left = Insert(Tree, Root->Left, ID, Query, Exists);
        }
        else if (Root->Data.ID < ID)
        {
            Root->Right = Insert(Tree, Root->Right, ID, Query, Exists);
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
        NewRoot = AllocateNode(Tree, ID);
        *Query = &NewRoot->Data;
    }

    return NewRoot;
}

function query_tree_insert_result
QueryTree_Insert(query_tree *Tree, u32 ID, u32 KeywordCount, u32 Type, u32 Distance)
{
    query_tree_insert_result Result = { };

    Tree->Root = Insert(Tree, Tree->Root, ID, &Result.Query, &Result.Exists);
    if (Result.Query && !Result.Exists)
    {
        Result.Query->PackedInfo = ((KeywordCount << 5) | (Type << 3) | Distance);
    }

    return Result;
}

//
// NOTE(philip): Recursively searched for a node in the subtree and returns it, if it exists.
//

function query *
Find(query_tree_node *Root, u32 ID)
{
    query *Query = 0;

    if (Root)
    {
        if (Root->Data.ID > ID)
        {
            Query = Find(Root->Left, ID);
        }
        else if (Root->Data.ID < ID)
        {
            Query = Find(Root->Right, ID);
        }
        else
        {
            Query = &Root->Data;
        }
    }

    return Query;
}

function query *
QueryTree_Find(query_tree *Tree, u32 ID)
{
    query *Query = Find(Tree->Root, ID);
    return Query;
}

// TODO(philip): Maybe, probably, should do something better.
// NOTE(philip): Due to the way node removal works in AVL trees, there are some cases where the memory address of
// the query data might change. This invalidates the links from keywords to queries. There is not an attractive way
// to fix this without switching away from recursive functions. So currently when the memory address changes we
// go through the keywords and update the broken links.

struct query_list_node;

function void
UpdateKeywordQueryLinks(query *Source, query *Destination)
{
    u32 KeywordCount = GetQueryKeywordCount(Source);
    for (u32 KeywordIndex = 0;
         KeywordIndex < KeywordCount;
         ++KeywordIndex)
    {
        keyword *Keyword = Source->Keywords[KeywordIndex];
        for (query_list_node *Node = Keyword->Queries.Head;
             Node;
             Node = Node->Next)
        {
            if (Node->Query == Source)
            {
                Node->Query = Destination;
                break;
            }
        }
    }
}

//
// NOTE(philip): Recursively removes a node from the subtree, if it exists, with proper balancing. Sets the function
// parameter flag if the node was removed.
//

function query_tree_node *
Remove(query_tree *Tree, query_tree_node *Root, u32 ID, b32 *Removed)
{
    query_tree_node *NewRoot = Root;

    if (Root)
    {
        b32 CheckForBalance = true;

        if (Root->Data.ID > ID)
        {
            Root->Left = Remove(Tree, Root->Left, ID, Removed);
        }
        else if (Root->Data.ID < ID)
        {
            Root->Right = Remove(Tree, Root->Right, ID, Removed);
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
                UpdateKeywordQueryLinks(&MovingChild->Data, &Root->Data);

                Root->Right = Remove(Tree, Root->Right, MovingChild->Data.ID, 0);
            }
            else
            {
                query_tree_node *Child = Root->Left ? Root->Left : Root->Right;

                if (Child)
                {
                    *Root = *Child;
                    UpdateKeywordQueryLinks(&Child->Data, &Root->Data);

                    DeallocateNode(Tree, Child);
                }
                else
                {
                    NewRoot = 0;
                    CheckForBalance = false;

                    DeallocateNode(Tree, Root);
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

function b32
QueryTree_Remove(query_tree *Tree, u32 ID)
{
    b32 Removed = false;
    Tree->Root = Remove(Tree, Tree->Root, ID, &Removed);

    return Removed;
}

//
// NOTE(philip): Returns whether a query has all of it's keywords found.
//

function b32
IsResultQuery(query *Query)
{
    b32 Result = true;

    u32 KeywordCount = GetQueryKeywordCount(Query);
    for (u32 Index = 0;
         Index < KeywordCount;
         ++Index)
    {
        if (!Query->HasKeyword[Index])
        {
            Result = false;
            break;
        }
    }

    return Result;
}

//
// NOTE(philip): Returns the number of queries in a subtree, for which all the keywords were found.
//

function u32
GetResultQueryCount(query_tree_node *Node)
{
    u32 Count = IsResultQuery(&Node->Data);

    if (Node->Left)
    {
        Count += GetResultQueryCount(Node->Left);
    }

    if (Node->Right)
    {
        Count += GetResultQueryCount(Node->Right);
    }

    return Count;
}

//
// NOTE(philip): Stores the result query IDs that have to an array for the specified subtree, and returns the
// next empty slot of that array. The array must be preallocated.
//

function u32 *
ResultQueryIDsToArray(query_tree_node *Node, u32 *IDs)
{
    query *Query = &Node->Data;
    if (IsResultQuery(Query))
    {
        *IDs = Query->ID;
        ++IDs;
    }

    if (Node->Left)
    {
        IDs = ResultQueryIDsToArray(Node->Left, IDs);
    }

    if (Node->Right)
    {
        IDs = ResultQueryIDsToArray(Node->Right, IDs);
    }

    return IDs;
}

//
// NOTE(philip): Generates and returns an answer structure, containing all the query IDs that have been answered.
//

function s32
CompareU32(const void *A, const void *B)
{
    s32 Result = (*(u32 *)A - *(u32 *)B);
    return Result;
}

function result
QueryTree_CompileResult(query_tree *Tree, u32 DocumentID)
{
    result Result = { };
    Result.DocumentID = DocumentID;

    if (Tree->Root)
    {
        Result.QueryIDCount = GetResultQueryCount(Tree->Root);
        if (Result.QueryIDCount)
        {
            Result.QueryIDs = (u32 *)calloc(1, Result.QueryIDCount * sizeof(u32));
            ResultQueryIDsToArray(Tree->Root, Result.QueryIDs);

            // NOTE(philip): Sort the array.
            qsort(Result.QueryIDs, Result.QueryIDCount, sizeof(u32), CompareU32);
        }
    }

    return Result;
}

function void
QueryTree_Destroy(query_tree *Tree)
{
    DestroyMemoryArena(&Tree->Arena);
    Tree->Root = 0;
    Tree->FreeNode = 0;
}
