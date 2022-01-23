#ifndef ISE_KEYWORD_H
#define ISE_KEYWORD_H

typedef u32 keyword_tree_type;
enum
{
    KeywordTreeType_Hamming = 0,
    KeywordTreeType_Edit    = 1
};

struct keyword_tree_node
{
    keyword *Data;

    keyword_tree_node *NextSibling;
    keyword_tree_node *FirstChild;
    u64 DistanceFromParent;

    u64 Padding[4];
};

typedef u64 keyword_tree_calculate_distance(char *StringA, u64 LengthA, char *StringB, u64 LengthB);

struct keyword_tree_candidate_array
{
    u64 Capacity;
    u64 Count;
    keyword_tree_node **Data;
};

struct keyword_tree
{
    keyword_tree_node *Root;
    keyword_tree_calculate_distance *CalculateDistance;
    keyword_tree_candidate_array Candidates;
};

#endif
