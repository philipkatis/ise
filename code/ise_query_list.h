#ifndef ISE_QUERY_LIST_H
#define ISE_QUERY_LIST_H

struct query;

struct query_list_node
{
    query *Query;
    query_list_node *Next;
};

struct query_list
{
    query_list_node *Head;
};

void QueryList_Insert(query_list *List, query *Query);
void QueryList_Remove(query_list *List, query *Query);
void QueryList_Destroy(query_list *List);

#endif
