#ifndef ISE_KEYWORD_TABLE_H
#define ISE_KEYWORD_TABLE_H

#include "ise.h"
#include "ise_query_list.h"

// TODO(philip): If storing queries in a list becomes a problem, we could switch to an array that doubles in size
// when it runs out.

struct __attribute__ ((__packed__)) keyword
{
    char Word[MAX_KEYWORD_LENGTH + 1];
    query_list Queries;
    u32 Length;
    u32 Hash;
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

// TODO(philip): Do we care about it already existing? Maybe it might speed some things up but is that the common
// case?
struct keyword_table_insert_result
{
    keyword *Keyword;
    b64 Exists;
};

keyword_table KeywordTable_Create(u64 InitialBucketCount);
keyword_table_insert_result KeywordTable_Insert(keyword_table *Table, char *Word);
void KeywordTable_Destroy(keyword_table *Table);

#if ISE_DEBUG
    void KeywordTable_Visualize_(keyword_table *Table);
    #define KeywordTable_Visualize(Table) KeywordTable_Visualize_(Table);
#else
    #define KeywordTable_Visualize(Table)
#endif

#endif
