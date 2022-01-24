#ifndef ISE_KEYWORD_H
#define ISE_KEYWORD_H

//
// NOTE(philip): Keyword Hash Table
//

#define MAX_KEYWORD_LENGTH 31

struct __attribute__ ((__packed__)) keyword
{
    char Word[MAX_KEYWORD_LENGTH + 1];

    query_list Queries;
    u32 Length;
    u32 Hash;

    b32 IsInHammingTree;
    b32 IsInEditTree;
};

struct __attribute__ ((__packed__)) keyword_table_node
{
    keyword Data;
    keyword_table_node *Next;
};

struct keyword_table
{
    keyword_table_node **Slots;
    u64 SlotCount;
    u64 KeywordCount;
};

struct keyword_table_iterator
{
    keyword_table *Table;
    keyword_table_node *Node;
    u64 SlotIndex;
};

//
// NOTE(philip): Keyword BK-Tree
//

typedef u32 keyword_tree_type;
enum
{
    KeywordTreeType_Hamming = 0,
    KeywordTreeType_Edit    = 1
};

struct __attribute__ ((__packed__)) keyword_tree_node
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

#define MAX_DISTANCE_THRESHOLD 3

struct keyword_tree_match
{
    keyword* Keyword;
    u64 Distance;
};

struct keyword_tree
{
    keyword_tree_node *Root;
    keyword_tree_calculate_distance *CalculateDistance;
    keyword_tree_candidate_array Candidates;
};

#endif
