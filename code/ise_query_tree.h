#ifndef ISE_QUERY_TREE_H
#define ISE_QUERY_TREE_H

#include "ise.h"

struct keyword_table_node;

struct query
{
    u32 ID;
    u16 Distance;
    u8 Type;
    u8 KeywordCount;

    keyword_table_node *Keywords[MAX_KEYWORD_COUNT_PER_QUERY];
};

struct query_tree_node
{
    query Data;

    query_tree_node *Parent;
    u64 Height;

    query_tree_node *Left;
    query_tree_node *Right;
};

struct query_tree
{
    query_tree_node *Root;
    u64 Count;
};

struct query_tree_insert_result
{
    query *Query;
    b32 Exists;
};

/*

  NOTE(philip): Finds the query with ID in the tree and returns it if it exists. Otherwise, craetes a new query and
  returns that.

*/

query_tree_insert_result QueryTree_Insert(query_tree *Tree, u32 ID);

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
