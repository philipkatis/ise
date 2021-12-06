#ifndef ISE_KEYWORD_TABLE_H
#define ISE_KEYWORD_TABLE_H

#include "ise.h"

struct keyword
{
    char Word[MAX_KEYWORD_LENGTH + 1];
};

struct keyword_table_node
{
    keyword Data;
    keyword_table_node *Next;
};

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
void KeywordTable_Destroy(keyword_table *Table);

#if ISE_DEBUG
    void KeywordTable_Visualize_(keyword_table *Table);
    #define KeywordTable_Visualize(Table) KeywordTable_Visualize_(Table);
#else
    #define KeywordTable_Visualize(Table)
#endif

#endif
