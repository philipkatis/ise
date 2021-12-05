#ifndef ISE_QUERY_TREE_H
#define ISE_QUERY_TREE_H

#include "ise_base.h"

struct query
{
    u32 ID;
    u32 Padding;
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

query *QueryTree_FindOrInsert(query_tree *Tree, u32 ID);
void QueryTree_RemoveIfExists(query_tree *Tree, u32 ID);
void QueryTree_Destroy(query_tree *Tree);

#if ISE_DEBUG
    void QueryTree_Visualize_(query_tree *Tree);
    #define QueryTree_Visualize(Tree) QueryTree_Visualize_(Tree)
#else
    #define QueryTree_Visualize(Tree)
#endif

#endif
