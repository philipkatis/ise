#ifndef ISE_QUERY_TREE_H
#define ISE_QUERY_TREE_H

#include "ise_base.h"

struct query_metadata
{
    u32 ID;
    u32 Padding;
};

struct query_metadata_tree_node
{
    query_metadata Data;

    query_metadata_tree_node *Parent;
    u64 Height;

    query_metadata_tree_node *Left;
    query_metadata_tree_node *Right;
};

struct query_metadata_tree
{
    query_metadata_tree_node *Root;
    u64 Count;
};

query_metadata *FindOrInsertQueryMetadata(query_metadata_tree *Tree, u32 ID);
void RemoveQueryMetadata(query_metadata_tree *Tree, u32 ID);
void DestroyQueryMetadataTree(query_metadata_tree *Tree);

#if ISE_DEBUG
    void VisualizeQueryMetadataTree_(query_metadata_tree *Tree);
    #define VisualizeQueryMetadataTree(Tree) VisualizeQueryMetadataTree_(Tree)
#else
    #define VisualizeQueryMetadataTree(Tree)
#endif

#endif
