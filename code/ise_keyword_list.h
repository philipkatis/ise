#ifndef ISE_KEYWORD_LIST_H
#define ISE_KEYWORD_LIST_H

struct keyword_list_node
{
    keyword *Keyword;
    keyword_list_node *Next;
};

struct keyword_list
{
    keyword_list_node *Head;
};

function void KeywordList_Insert(keyword_list *List, keyword *Keyword);
function void KeywordList_Destroy(keyword_list *List);

#endif
