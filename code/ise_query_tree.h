#ifndef ISE_QUERY_TREE_H
#define ISE_QUERY_TREE_H

#include "ise.h"

struct keyword_table_node;

struct __attribute__ ((__packed__)) query
{
    u32 ID;
    u8 PackedInfo;
    keyword_table_node *Keywords[MAX_KEYWORD_COUNT_PER_QUERY];
};

struct __attribute__ ((__packed__)) query_tree_node
{
    query_tree_node *Left;
    query_tree_node *Right;

    query Data;

    u16 Height;
    u8 Padding;
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

  NOTE(philip): Finds the query with ID in the tree and returns it if it exists. Otherwise, craetes a new query and
  returns that.

*/

query_tree_insert_result QueryTree_Insert(query_tree *Tree, u32 ID, u32 KeywordCount, u32 Type, u32 Distance);

/*

  NOTE(philip): Finds the query with ID in the tree and removes it if it exists.

*/

void QueryTree_Remove(query_tree *Tree, u32 ID);

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
