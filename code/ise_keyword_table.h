#ifndef ISE_KEYWORD_TABLE_H
#define ISE_KEYWORD_TABLE_H

#include "ise.h"
#include "ise_query_list.h"

struct __attribute__ ((__packed__)) keyword
{
    char Word[MAX_KEYWORD_LENGTH + 1];

    // TODO(philip): If this list becomes a bottleneck, switch to an array that doubles in size.
    query_list Queries;

    u32 Length;
    u32 Hash;

    // TODO(philip): We currently have no way if a keyword is in a BK tree, without looking through the query list.
    // I do not suspect this to become a problem, but if it does, use 2 bits in the padding to store that.
    u64 Padding;
};

struct __attribute__ ((__packed__)) keyword_table_node
{
    keyword Data;
    keyword_table_node *Next;
};

static_assert(sizeof(keyword_table_node) == 64);

struct keyword_table
{
    keyword_table_node **Buckets;
    u64 BucketCount;
    u64 ElementCount;
};

struct keyword_table_insert_result
{
    keyword *Keyword;
    b64 Exists;
};

keyword_table KeywordTable_Create(u64 InitialBucketCount);
keyword_table_insert_result KeywordTable_Insert(keyword_table *Table, char *Word);
keyword *KeywordTable_Find(keyword_table *Table, char *Word);
void KeywordTable_Destroy(keyword_table *Table);

#if ISE_DEBUG
    void KeywordTable_Visualize_(keyword_table *Table);
    #define KeywordTable_Visualize(Table) KeywordTable_Visualize_(Table);
#else
    #define KeywordTable_Visualize(Table)
#endif

struct keyword_iterator
{
    keyword_table_node *Node;
    keyword_table *Table;
    u32 BucketIndex;
};

keyword_iterator IterateAllKeywords(keyword_table *Table);
b32 IsValid(keyword_iterator *Iterator);
void Advance(keyword_iterator *Iterator);
keyword *GetValue(keyword_iterator *Iterator);

#endif
