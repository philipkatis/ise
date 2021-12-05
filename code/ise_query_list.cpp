#include "ise_query_list.h"

#if 0

#include <stdlib.h>

query *
QueryList_Find(query_list *List, u32 ID)
{
    query *Result = 0;

    for (query *Query = List->Head;
         Query;
         Query = Query->Next)
    {
        if (Query->ID == ID)
        {
            Result = Query;
            break;
        }
    }

    return Result;
}

query *
QueryList_Insert(query_list *List, u32 ID, u8 WordCount, u8 Type, u16 Distance)
{
    query *Query = (query *)calloc(1, sizeof(query));
    Query->ID = ID;
    Query->WordCount = WordCount;
    Query->Type = Type;
    Query->Distance = Distance;

    Query->Next = List->Head;
    List->Head = Query;

    return Query;
}

void
QueryList_Remove(query_list *List, u32 ID)
{
    query *Query = 0;
    query *Previous = 0;
    query *Next = 0;

    for (query *CurrentQuery = List->Head;
         CurrentQuery;
         CurrentQuery = CurrentQuery->Next)
    {
        Next = CurrentQuery->Next;

        if (CurrentQuery->ID == ID)
        {
            break;
        }

        Previous = CurrentQuery;
    }

    if (Query)
    {
        if (Previous)
        {
            Previous->Next = Next;
        }
        else
        {
            List->Head = Next;
        }

        free(Query);
    }
}

void
QueryList_Destroy(query_list *List)
{
    query *Query = List->Head;
    while (Query)
    {
        query *Next = Query->Next;
        free(Query);
        Query = Next;
    }

    List->Head = 0;
}

#endif
