#ifndef ISE_QUERY_TREE_H
#define ISE_QUERY_TREE_H

struct keyword;

struct __attribute__ ((__packed__)) query
{
    union
    {
        keyword *Keywords[MAX_KEYWORD_COUNT_PER_QUERY];
        b64 HasKeywordFlags[MAX_KEYWORD_COUNT_PER_QUERY];
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
    query_tree_node *Left;
    query_tree_node *Right;
    query Data;
    u16 Height;
};

static_assert(sizeof(query_tree_node) == 64);

struct query_tree
{
    query_tree_node *Root;
};

struct query_tree_insert_result
{
    query *Query;
    b64 Exists;
};

function query_tree_insert_result QueryTree_Insert(query_tree *Tree, u32 ID, u32 KeywordCount, u32 Type, u32 Distance);
function query *QueryTree_Find(query_tree *Tree, u32 ID);
function b32 QueryTree_Remove(query_tree *Tree, u32 ID);
function void QueryTree_Destroy(query_tree *Tree);

#if ISE_DEBUG
    function void QueryTree_Visualize_(query_tree *Tree);
    #define QueryTree_Visualize(Tree) QueryTree_Visualize_(Tree)
#else
    #define QueryTree_Visualize(Tree)
#endif

#endif
