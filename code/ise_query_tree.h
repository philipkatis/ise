#ifndef ISE_QUERY_TREE_H
#define ISE_QUERY_TREE_H

struct keyword;

struct __attribute__ ((__packed__)) query
{
    union
    {
        keyword *Keywords[MAX_KEYWORD_COUNT_PER_QUERY];
        b64 HasKeyword[MAX_KEYWORD_COUNT_PER_QUERY];
    };

    u32 ID;
    u8 PackedInfo;
    u8 Padding;
};

#define GetQueryKeywordCount(Query) ((Query)->PackedInfo >> 5)
#define GetQueryType(Query) (((Query)->PackedInfo >> 3) & 0x3)
#define GetQueryDistance(Query) ((Query)->PackedInfo & 0x7)

struct __attribute__ ((__packed__)) query_tree_node
{
    union
    {
        query_tree_node *Left;
        query_tree_node *Next;
    };

    union
    {
        query_tree_node *Right;
        query_tree_node *Previous;
    };

    query Data;
    u16 Height;
};

static_assert(sizeof(query_tree_node) == 64);

struct query_tree
{
    memory_arena Arena;
    query_tree_node *Root;
    query_tree_node *FreeNode;
};

struct query_tree_insert_result
{
    query *Query;
    b64 Exists;
};

function query_tree QueryTree_Create();
function query_tree_insert_result QueryTree_Insert(query_tree *Tree, u32 ID, u32 KeywordCount, u32 Type, u32 Distance);
function query *QueryTree_Find(query_tree *Tree, u32 ID);
function b32 QueryTree_Remove(query_tree *Tree, u32 ID);
function void QueryTree_Visualize(query_tree *Tree);
function void QueryTree_Destroy(query_tree *Tree);

#endif
