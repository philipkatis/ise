#ifndef ISE_BK_TREE_H
#define ISE_BK_TREE_H

#include "ise_match.h"
#include "ise_keyword_list.h"
#include "ise.h"

/*

  NOTE(philip): A 'bk_tree_node' is the building block of a BK-Tree. It stores a reference to a keyword, that the
  node represents.

*/

struct bk_tree_node
{
    keyword *Keyword;
    bk_tree_node *FirstChild;
    bk_tree_node *NextSibling;
    s32 DistanceFromParent;
};

/*

  NOTE(philip): A 'bk_tree' is an acceleration tree structure that stores references to keywords in a way that
  speeds up the search of similar keywords up to a certain threshold.

*/

struct bk_tree
{
    bk_tree_node *Root;
    match_function_type MatchFunction;
};

/*

  NOTE(philip): This function creates a BK-Tree with zero nodes.

*/

bk_tree BKTree_Create(MatchType MatchType);

/*

  NOTE(philip): This functions inserts a new keyword into the BK-Tree at the correct location using the edit
  distance calculation algorithm. In case the keyword we try to insert is already in the BK-Tree, this function will
  return a reference to it.

*/

bk_tree_node *BKTree_Insert(bk_tree *Tree, keyword *Keyword);

/*

  NOTE(philip): This function searches through the BK-Tree in an optimized manner, in order to find the keywords
  that match the specified word with the maximum threshold. It returns a keyword list containing the matches.

*/

keyword_list BKTree_FindMatches(bk_tree *Tree, char *Word, u32 DistanceThreshold);

/*

  NOTE(philip): This function destroys a BK-Tree, deallocating all the memory allocated by all it's nodes.
  This does not deallocate the memory of the keywords referenced by all the nodes.

*/

void BKTree_Destroy(bk_tree *Tree);

/*

  NOTE(philip): This function is used only for debugging. It is a tool for visualizing the contents of a BK-Tree,
  by dumping them into the command-line.

*/

#if ISE_DEBUG
    void _BKTree_Visualize(bk_tree *Tree);
    #define BKTree_Visualize(Tree) _BKTree_Visualize((Tree))
#else
    #define BKTree_Visualize(Tree)
#endif

#endif
