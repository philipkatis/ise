#ifndef ISE_BK_TREE_H
#define ISE_BK_TREE_H

#include "ise.h"
#include "ise_keyword_list.h"

typedef u32 bk_tree_type;
enum
{
    BKTree_Type_None,
    BKTree_Type_Hamming,
    BKTree_Type_Edit
};

struct bk_tree_node
{
    keyword *Keyword;
    bk_tree_node *FirstChild;
    bk_tree_node *NextSibling;
    s32 DistanceFromParent;
};

struct bk_tree
{
    bk_tree_node *Root;
    bk_tree_type Type;
};

bk_tree BKTree_Create(bk_tree_type Type);
void BKTree_Insert(bk_tree *Tree, keyword *Keyword);
keyword_list BKTree_FindMatches(bk_tree *Tree, keyword *Keyword, s32 DistanceThreshold);
void BKTree_Destroy(bk_tree *Tree);

#if ISE_DEBUG
    void _BKTree_Visualize(bk_tree *Tree);
    #define BKTree_Visualize(Tree) _BKTree_Visualize((Tree))
#else
    #define BKTree_Visualize(Tree)
#endif

#endif
