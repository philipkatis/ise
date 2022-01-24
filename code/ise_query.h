#ifndef ISE_QUERY_H
#define ISE_QUERY_H

typedef u32 query_type;
enum
{
    QueryType_Exact   = 0,
    QueryType_Hamming = 1,
    QueryType_Edit    = 2
};

#define MAX_KEYWORD_COUNT_PER_QUERY 5

struct __attribute__ ((__packed__)) query
{
    union
    {
        struct keyword *Keywords[MAX_KEYWORD_COUNT_PER_QUERY];
        b64 HasKeywords[MAX_KEYWORD_COUNT_PER_QUERY];
    };

    u32 ID;
    u8 PackedInfo;
    u8 Padding;
};

#define GetKeywordCount(Query) ((Query)->PackedInfo >> 5)
#define GetType(Query) (((Query)->PackedInfo >> 3) & 0x3)
#define GetDistanceThreshold(Query) ((Query)->PackedInfo & 0x7)

struct __attribute__ ((__packed__)) query_tree_node
{
    query_tree_node *Left;
    query_tree_node *Right;

    query Data;
    u16 Height;
};

struct query_tree
{
    query_tree_node *Root;
};

#endif
