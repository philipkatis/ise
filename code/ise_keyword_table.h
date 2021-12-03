#ifndef ISE_KEYWORD_TABLE_H
#define ISE_KEYWORD_TABLE_H

#include "ise_query_list.h"

#define KEYWORD_TABLE_BUCKET_COUNT 4096

struct keyword_table_node
{
    char Word[MAX_KEYWORD_LENGTH + 1];
    u64 Hash;
    query_list Queries;
    keyword_table_node *Next;
};

struct keyword_table
{
    keyword_table_node *Buckets[KEYWORD_TABLE_BUCKET_COUNT];
};

keyword_table_node *KeywordTable_Find(keyword_table *Table, char *Word);
keyword_table_node *KeywordTable_Insert(keyword_table *Table, char *Word);
void KeywordTable_Destroy(keyword_table *Table);

#if ISE_DEBUG
    void KeywordTable_Visualize_(keyword_table *Table);
    #define KeywordTable_Visualize(Table) KeywordTable_Visualize_((Table))
#else
    #define KeywordTable_Visualize(Table)
#endif

#endif
