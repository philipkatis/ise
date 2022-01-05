#ifndef ISE_KEYWORD_TABLE_H
#define ISE_KEYWORD_TABLE_H

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

function keyword_table KeywordTable_Create(u64 InitialBucketCount);
function keyword_table_insert_result KeywordTable_Insert(keyword_table *Table, char *Word);
function keyword *KeywordTable_Find(keyword_table *Table, char *Word);
function void KeywordTable_Visualize(keyword_table *Table);
function void KeywordTable_Destroy(keyword_table *Table);

struct keyword_iterator
{
    keyword_table_node *Node;
    keyword_table *Table;
    u32 BucketIndex;
};

function keyword_iterator IterateAllKeywords(keyword_table *Table);
function b32 IsValid(keyword_iterator *Iterator);
function void Advance(keyword_iterator *Iterator);
function keyword *GetValue(keyword_iterator *Iterator);

#endif
