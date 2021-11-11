#ifndef ISE_H
#define ISE_H

/*

  NOTE(philip): These limits are based on the SIGMOD 2013 Programming Contest referenced by the
  project assignment. We assume string length limits, do not include the zero termination character.

*/

#define MAX_KEYWORD_LENGTH 31

function u32 IsExactMatch(char *A, u64 LengthA, char *B, u64 LengthB);
function u32 CalculateHammingDistance(char *A, u64 LengthA, char *B, u64 LengthB);
function u32 CalculateLevenshteinDistance(char *A, u64 LengthA, char *B, u64 LengthB);

#if 0
#define MAX_KEYWORD_COUNT_PER_QUERY  5
#define MAX_QUERIES_PER_ENTRY        64

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
#endif

/*

  NOTE(philip): These are the different types of keyword matching that we support.

*/

typedef u32 match_type;
enum
{
    MatchType_Exact       = 0,
    MatchType_Hamming     = 1,
    MatchType_Levenshtein = 2
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
    u64 Count;
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
    match_type MatchType;
};

/*

  NOTE(philip): These types and function prototypes are used for the interface the assignment requires.

*/

typedef u32 error_code;
enum
{
    ErrorCode_Success           = 0,
    ErrorCode_InvalidParameters = 1,
    ErrorCode_FailedAllocation  = 2,
    ErrorCode_DuplicateEntry    = 3
};

typedef keyword *entry;

function error_code create_entry(char *Word, entry *Entry);
function error_code destroy_entry(entry *Entry);

typedef keyword_list entry_list;

function error_code create_entry_list(entry_list *List);
function error_code add_entry(entry_list *List, entry *Entry);
function entry *get_first(entry_list *List);
function entry *get_next(entry_list *List, entry *Entry);
function u32 get_number_entries(entry_list *List);
function error_code destroy_entry_list(entry_list *List);

typedef bk_tree index;

function error_code build_entry_index(entry_list *List, match_type MatchType, index *Index);
function error_code lookup_entry_index(char *Word, index *Index, u32 Threshold, entry_list *Result);
function error_code destroy_entry_index(index *Index);

#endif
