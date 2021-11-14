#ifndef ISE_INTERFACE_H
#define ISE_INTERFACE_H

#include "ise_bk_tree.h"

/*

  NOTE(philip): These types and function prototypes are used for the interface the assignment requires. This
  essentially acts as a wrapper around the keyword_list and bk_tree implementations.

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

error_code create_entry(char *Word, entry *Entry);
error_code destroy_entry(entry *Entry);

typedef keyword_list entry_list;

error_code create_entry_list(entry_list *List);
error_code add_entry(entry_list *List, entry *Entry);
entry *get_first(entry_list *List);
entry *get_next(entry_list *List, entry *Entry);
u32 get_number_entries(entry_list *List);
error_code destroy_entry_list(entry_list *List);

typedef bk_tree index;

error_code build_entry_index(entry_list *List, match_type MatchType, index *Index);
error_code lookup_entry_index(char *Word, index *Index, u32 Threshold, entry_list *Result);
error_code destroy_entry_index(index *Index);

#endif
