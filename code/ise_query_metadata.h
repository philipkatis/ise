#ifndef ISE_QUERY_TREE_H
#define ISE_QUERY_TREE_H

#include "ise_base.h"

struct query_metadata
{
    u32 ID;
    u32 Padding;
};

struct node;

struct query_metadata_tree
{
    node *Root;
    u64 Count;
};

query_metadata *FindOrInsertQueryMetadata(query_metadata_tree *Tree, u32 ID);
void RemoveQueryMetadata(query_metadata_tree *Tree, u32 ID);

#endif
