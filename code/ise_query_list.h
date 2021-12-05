#ifndef ISE_QUERY_LIST_H
#define ISE_QUERY_LIST_H

#if 0

#include "ise.h"

struct query
{
    u32 ID;
    u8 WordCount;
    u8 Type;

    union
    {
        u16 Distance;
        u16 WordsFound;
    };

    query *Next;
};

struct query_list
{
    query *Head;
};

query *QueryList_Find(query_list *List, u32 ID);

// TODO(philip): Maybe change this to Allocate, since the data meaning is different each time.
query *QueryList_Insert(query_list *List, u32 ID, u8 WordCount, u8 Type, u16 Distance);
void QueryList_Remove(query_list *List, u32 ID);
void QueryList_Destroy(query_list *List);

#endif

#endif
