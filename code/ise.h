#ifndef ISE_H
#define ISE_H

/*

  NOTE(philip): These limits are based on the SIGMOD 2013 Programming Contest referenced by the
  project assignment. We assume string length limits, do not include the zero termination character.

*/

#define MAX_KEYWORD_LENGTH           31
#define MAX_KEYWORD_COUNT_PER_QUERY  5

typedef u32 match_type;
enum
{
    MatchType_Exact       = 0,
    MatchType_Hamming     = 1,
    MatchType_Levenshtein = 2
};

struct query_data
{
    char *Keywords[MAX_KEYWORD_COUNT_PER_QUERY];

    // TODO(philip): Maybe these sizes are not right.
    u64 QueryID;
    match_type MatchType;
    u32 MatchDistance;
    u32 KeywordCount;
};

struct keyword_list_node
{
    // NOTE(philip): This now stores a pointer to the word in the file contents. Still not sure whether this is
    // the right approach.
    char *Word;

    u64 QueryID;

    // TODO(philip): These are properties of the query and certainly should not be in here. Maybe we need an
    // additional "query metadata" structure to store them. And then the query ID can be replaced by a pointer.
    match_type MatchType;
    u32 MatchDistance;

    keyword_list_node *Previous;
    keyword_list_node *Next;
};

struct keyword_list
{
    keyword_list_node *Head;
    keyword_list_node *Tail;
    u64 Count;
};

/*

  NOTE(philip): The max Levenshtein distance between two keywords cannot exceed the length
  of the bigger one. A Levenshtein distance of zero, means the two keywords are the same and
  thus cannot be in the tree at the same time.

  NOTE(philip): The tree no longer stores the word. Instead, it just stores a pointer to the list that stores
  the word.

  TODO(philip): There are several problems with this approach. Firstly, storing 30 8-byte pointers
  is not an attractive option. A better alternative would be to store all the tree nodes in a
  single continuous chunck of memory and using a 32-bit index into it. This reduces the size of this
  structure from 272 bytes to 160 bytes. The 32-bit index can access up to 4 billion entries. Plenty.
  Furthermore, we can improve this further with a custom allocator, cutting down on expensive OS
  allocations. Another benefit to this approach is deallocation. Since we are (probably) expected to
  deallocate the memory "properly", this can speed things up since we don't have to use a recursive
  function that visits each node.

*/

struct bk_tree_node
{
    keyword_list_node *Keyword;
    bk_tree_node *Children[MAX_KEYWORD_LENGTH - 1];
};

struct entry
{
    char Word[MAX_KEYWORD_LENGTH + 1];
    u64 payload;
    /*

    NOTE(Alex): I guess the payload needs to point to some queries however we get them or wherever we store them.

    */
};

#endif
