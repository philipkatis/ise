#ifndef ISE_H
#define ISE_H

/*

  NOTE(philip): These limits are based on the SIGMOD 2013 Programming Contest referenced by the
  project assignment. We assume string length limits, do not include the zero termination character.

*/

#define MAX_KEYWORD_LENGTH           31
#define MAX_KEYWORD_COUNT_PER_QUERY  5
#define MAX_QUERIES_PER_ENTRY        64

typedef u32 match_type;
enum
{
    MatchType_Exact       = 0,
    MatchType_Hamming     = 1,
    MatchType_Levenshtein = 2
};

// NOTE(philip): This is a function pointer for the matching functions.
typedef u32 (*match_function)(char *A, u64 LengthA, char *B, u64 LengthB);

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

*/

struct bk_tree_node
{
    keyword_list_node *Keyword;
    bk_tree_node *Children[MAX_KEYWORD_LENGTH - 1];
};

struct entry
{
    char Word[MAX_KEYWORD_LENGTH + 1];
    u64 Payload[MAX_QUERIES_PER_ENTRY];
    /*

    NOTE(Alex): The payload could potentially point to an array of numbers, the numbers of which will indicate
    the corresponding queries in which the word exists.

    */
};

struct entry_list_node
{
    entry *Entry;
    entry_list_node* Previous;
    entry_list_node* Next;
};

struct entry_list
{
    entry_list_node* Head;
    entry_list_node* Tail;
    u64 Count;
};

#endif
