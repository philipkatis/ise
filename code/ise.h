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

/*

  NOTE(philip): A 'keyword' is a word from a static query. These words are stored somewhere in memory and this
  structure references them. The word storage is a separate layer abstracted away from the keyword (yikes).
  Furthermore, this structure is is used as a node for a linked list of keywords. These lists have several
  usecases throughout the appilcation.

*/

struct keyword
{
    char *Word;
    keyword *Next;
};

/*

  NOTE(philip): A 'keyword list' is a collection of keywords which usually share a common attribute. That attribute
  might not be keyword-specific but query-specific.

*/

struct keyword_list
{
    keyword *Head;
};

/*

  NOTE(philip): A 'bk_tree_node' is the building block of a BK-Tree. It stores a reference to a keyword, that the
  node represents.

*/

struct bk_tree_node
{
    keyword *Keyword;
    u64 DistanceFromParent;
    bk_tree_node *FirstChild;
    bk_tree_node *NextSibling;
};

/*

  NOTE(philip): A 'bk_tree' is an acceleration tree structure that stores references to keywords in a way that
  speeds up the search of similar keywords up to a certain threshold.

*/

struct bk_tree
{
    bk_tree_node *Root;
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
