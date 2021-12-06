#ifndef ISE_QUERY_TREE_H
#define ISE_QUERY_TREE_H

#include "ise.h"

struct keyword;

struct __attribute__ ((__packed__)) query
{
    keyword *Keywords[MAX_KEYWORD_COUNT_PER_QUERY];
    u32 ID;
    u8 PackedInfo;
    u8 Padding;
};

#define GetKeywordCount(Query) (Query->PackedInfo >> 5)
#define GetType(Query) ((Query->PackedInfo << 3) >> 6)
#define GetDistance(Query) ((Query->PackedInfo << 5) >> 5)

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

    // TODO(philip): Will the count be useful in the end?
    u64 Count;
};

struct query_tree_insert_result
{
    query *Query;
    b64 Exists;
};

/*

  NOTE(philip): Inserts a new query into the tree, if it does not exist. If the query already exists, it returns it
  and sets the flag.

*/

query_tree_insert_result QueryTree_Insert(query_tree *Tree, u32 ID, u32 KeywordCount, u32 Type, u32 Distance);

/*

  NOTE(philip): Finds a query in the tree and returns it. If the query does not exist, it returns zero.

*/

query *QueryTree_Find(query_tree *Tree, u32 ID);

/*

  NOTE(philip): Removes a node from the tree and returns true, if it exists. If the query does not exist, it
  returns false.

*/

b32 QueryTree_Remove(query_tree *Tree, u32 ID);

/*

  NOTE(philip): Deallocates all the memory allocated by the tree.

*/

void QueryTree_Destroy(query_tree *Tree);

#if ISE_DEBUG
    void QueryTree_Visualize_(query_tree *Tree);
    #define QueryTree_Visualize(Tree) QueryTree_Visualize_(Tree)
#else
    #define QueryTree_Visualize(Tree)
#endif

#endif
