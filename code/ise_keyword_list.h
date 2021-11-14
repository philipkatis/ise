#ifndef ISE_KEYWORD_LIST_H
#define ISE_KEYWORD_LIST_H

#include "ise_base.h"

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

  NOTE(philip): This function creates a keyword list with zero nodes.

*/

keyword_list KeywordList_Create();

/*

  NOTE(philip): This function allocates a new keyword and sets the data it stores.

*/

keyword *KeywordList_AllocateKeyword(char *Word);

/*

  NOTE(philip): This function deallocates a keyword.

*/

void KeywordList_DeallocateKeyword(keyword *Keyword);

/*

  NOTE(philip): This function creates a new keyword and inserts it the keyword list. The new word will be at the
  head of the list. This is done to improve insertion speed. This function does not ensure that there are no
  duplicates. It is up to the client to use KeywordList_Find() to implement that functionality.

*/

keyword *KeywordList_Insert(keyword_list *List, char *Word);

/*

  NOTE(philip): This function inserts a keyword to the keyword list. The new word will be at the head of the list.
  This is done to improve insertion speed. This function does not ensure that there are no duplicates. It is up to
  the client to use KeywordList_Find() to implement that functionality.

*/

keyword *KeywordList_Insert(keyword_list *List, keyword *Keyword);

/*

  NOTE(philip): This function finds a word in the keyword list and returns it. If the word could not be found, this
  function returns zero.

*/

keyword *KeywordList_Find(keyword_list *List, char *Word);

/*

  NOTE(philip): This function destroys a keyword list, deallocating all the memory allocated by all it's nodes.
  This does not deallocate the memory of the words referenced by all the nodes.

*/

void KeywordList_Destroy(keyword_list *List);

/*

  NOTE(philip): This function is used only for debugging. It is a tool for visualizing the contents of a keyword
  list, by dumping them into the command-line.

*/

#if ISE_DEBUG
    void _KeywordList_Visualize(keyword_list *List);
    #define KeywordList_Visualize(List) _KeywordList_Visualize((List))
#else
    #define KeywordList_Visualize(List)
#endif

#endif
