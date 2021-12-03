#ifndef ISE_QUERY_LIST_H
#define ISE_QUERY_LIST_H

#include "ise.h"

struct query
{
    u32 ID;
    u16 Type;
    u16 Distance;

    query *Next;
};

struct query_list
{
    query *Head;
};

query *QueryList_Find(query_list *List, u32 ID);
query *QueryList_Insert(query_list *List, u32 ID, u16 Type, u16 Distance);
void QueryList_Remove(query_list *List, u32 ID);
void QueryList_Destroy(query_list *List);

#endif
