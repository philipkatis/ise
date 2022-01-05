#ifndef ISE_BK_TREE_H
#define ISE_BK_TREE_H

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

typedef s32 (*match_function_type)(char *A, u64 LengthA, char *B, u64 LengthB);

struct bk_tree
{
    bk_tree_node *Root;
    match_function_type MatchFunction;
};

function bk_tree BKTree_Create(bk_tree_type Type);
function void BKTree_Insert(bk_tree *Tree, keyword *Keyword);
function keyword_list BKTree_FindMatches(bk_tree *Tree, keyword *Keyword, s32 DistanceThreshold);
function void BKTree_Destroy(bk_tree *Tree);

#if ISE_DEBUG
    function void _BKTree_Visualize(bk_tree *Tree);
    #define BKTree_Visualize(Tree) _BKTree_Visualize((Tree))
#else
    #define BKTree_Visualize(Tree)
#endif

#endif
