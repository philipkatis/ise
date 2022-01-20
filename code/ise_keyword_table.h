#ifndef ISE_KEYWORD_TABLE_H
#define ISE_KEYWORD_TABLE_H

struct __attribute__ ((__packed__)) keyword
{
    char Word[MAX_KEYWORD_LENGTH + 1];

    // TODO(philip): If this list becomes a bottleneck, switch to an array that doubles in size.
    query_list Queries;

    u8 Length;
    u8 Padding01;
    u16 Padding02;

    u32 Hash;

    // TODO(philip): We currently have no way if a keyword is in a BK tree, without looking through the query list.
    // I do not suspect this to become a problem, but if it does, use 2 bits in the padding to store that.
    u64 Padding03;
};

struct __attribute__ ((__packed__)) keyword_table_node
{
    keyword Data;
    keyword_table_node *Next;
};

static_assert(sizeof(keyword_table_node) == 64);

struct keyword_table
{
    memory_arena Arena;
    keyword_table_node **Buckets;
    u64 BucketCount;
    u64 ElementCount;
};

struct keyword_table_insert_result
{
    keyword *Keyword;
    b64 Exists;
};

struct keyword_iterator
{
    keyword_table_node *Node;
    keyword_table *Table;
    u32 BucketIndex;
    u32 Index;
};

#endif
