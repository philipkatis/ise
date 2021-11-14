#ifndef ISE_INTERFACE_H
#define ISE_INTERFACE_H

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
