#ifndef ISE_KEYWORD_LIST_H
#define ISE_KEYWORD_LIST_H

struct keyword;

struct keyword_list_node
{
    keyword *Keyword;
    keyword_list_node *Next;
};

struct keyword_list
{
    keyword_list_node *Head;
};

void KeywordList_Insert(keyword_list *List, keyword *Keyword);
void KeywordList_Destroy(keyword_list *List);

#endif
