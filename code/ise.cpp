// TODO(philip): Clean up the includes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if ISE_MULTI_THREADED
#include <pthread.h>
#endif

#include "ise.h"
#include "ise_base.h"
#include "ise_query_tree.h"
#include "ise_query_list.h"
#include "ise_keyword_table.h"
#include "ise_keyword_list.h"
#include "ise_bk_tree.h"
#include "ise_result_queue.h"

#if ISE_MULTI_THREADED
#include "ise_thread_pool.h"
#endif

#include "ise_base.cpp"
#include "ise_query_tree.cpp"
#include "ise_query_list.cpp"
#include "ise_keyword_table.cpp"
#include "ise_keyword_list.cpp"
#include "ise_bk_tree.cpp"
#include "ise_result_queue.cpp"
#include "ise_ops.cpp"

#if ISE_MULTI_THREADED
#include "ise_thread_pool.cpp"
#endif

struct application
{
    query_tree Queries;
    keyword_table Keywords;

    bk_tree HammingTrees[HAMMING_TREE_COUNT];
    bk_tree EditTree;

    result_queue Results;

#if 0
    answer_stack Answers;
#endif
};

global application Application = { };

#if ISE_MULTI_THREADED
global thread_pool ThreadPool = { };
#endif

ErrorCode
InitializeIndex(void)
{
    // NOTE(philip): Initialize the keyword table.
    Application.Keywords = KeywordTable_Create(1024);

    // NOTE(philip): Initialize the hamming BK trees.
    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        Application.HammingTrees[Index] = BKTree_Create(BKTree_Type_Hamming);
    }

    // NOTE(philip): Initialize the edit BK tree.
    Application.EditTree = BKTree_Create(BKTree_Type_Edit);

    InitializeResultQueue(&Application.Results);

#if ISE_MULTI_THREADED
    // TODO(philip): Choose proper thread count.
    ThreadPool = ThreadPool_Create(4, &Application.Keywords, Application.HammingTrees, &Application.EditTree,
                                   &Application.Results);
#endif

    return EC_SUCCESS;
}

ErrorCode
DestroyIndex(void)
{
#if ISE_MULTI_THREADED
    ThreadPool_Destroy(&ThreadPool);
#endif

    DestroyResultQueue(&Application.Results);

#if 0
    // NOTE(philip): Destroy the possible answer stack.
    AnswerStack_Destroy(&Application.Answers);
#endif

    // NOTE(philip): Destroy the edit tree.
    BKTree_Destroy(&Application.EditTree);

    // NOTE(philip): Destroy the hamming BK trees.
    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        BKTree_Destroy(&Application.HammingTrees[Index]);
    }

    // NOTE(philip): Destroy the keyword table and the query tree.
    KeywordTable_Destroy(&Application.Keywords);
    QueryTree_Destroy(&Application.Queries);

    return EC_SUCCESS;
}

function void
InsertInBK(u32 Type, keyword *Keyword)
{
    switch (Type)
    {
        case 1:
        {
            u64 TreeIndex = GetHammingTreeIndex(Keyword->Length);
            BKTree_Insert(&Application.HammingTrees[TreeIndex], Keyword);
        } break;

        case 2:
        {
            BKTree_Insert(&Application.EditTree, Keyword);
        } break;
    }
}

ErrorCode
StartQuery(QueryID ID, const char *String, MatchType Type, u32 Distance)
{
    Assert(Type >= 0 && Type <= 2);
    Assert(Distance >= 0 && Distance <= MAX_DISTANCE_THRESHOLD);

    char *Words[MAX_KEYWORD_COUNT_PER_QUERY];
    u32 WordCount = 0;

    // NOTE(philip): Count and split the words from the input.
    {
        Words[WordCount] = (char*)String;
        ++WordCount;

        for (char *Character = (char *)String;
             *Character;
             ++Character)
        {
            if (*Character == ' ')
            {
                *Character = 0;
                Words[WordCount] = Character + 1;
                ++WordCount;
            }
        }
    }

    Assert(WordCount > 0 && WordCount <= MAX_KEYWORD_COUNT_PER_QUERY);

    // TODO(philip): This is kind of gross.
    keyword_table QueryWords = KeywordTable_Create(WordCount);

    for (u32 WordIndex = 0;
         WordIndex < WordCount;
         ++WordIndex)
    {
        KeywordTable_Insert(&QueryWords, Words[WordIndex]);
    }

    // NOTE(philip): Insert the query into the tree.
    query_tree_insert_result QueryInsert = QueryTree_Insert(&Application.Queries, ID, QueryWords.ElementCount,
                                                            Type, Distance);
    if (QueryInsert.Exists)
    {
        return EC_FAIL;
    }

    query *Query = QueryInsert.Query;

    for (keyword_iterator Iterator = IterateAllKeywords(&QueryWords);
         IsValid(&Iterator);
         Advance(&Iterator))
    {
        keyword *Word = GetValue(&Iterator);

        // NOTE(philip): Insert the keyword into the keyword hash table.
        keyword_table_insert_result KeywordInsert = KeywordTable_Insert(&Application.Keywords, Word->Word);
        keyword *Keyword = KeywordInsert.Keyword;

        if (KeywordInsert.Exists && ((Type == 1) || (Type == 2)))
        {
            // NOTE(philip): If the keyword is already in the BK tree, do not attempt to insert it again.
            b32 AlreadyInBK = false;

            // TODO(philip): If we choose to use the occupancy flags, use them here.
            for (query_list_node *Node = Keyword->Queries.Head;
                 Node;
                 Node = Node->Next)
            {
                if (GetQueryType(Node->Query) == (u8)Type)
                {
                    AlreadyInBK = true;
                    break;
                }
            }

            if (!AlreadyInBK)
            {
                InsertInBK(Type, Keyword);
            }
        }
        else
        {
            InsertInBK(Type, Keyword);
        }

        // NOTE(philip): Insert the keyword into the query and the query into the keyword.
        Query->Keywords[GetIndex(&Iterator)] = Keyword;
        QueryList_Insert(&Keyword->Queries, Query);
    }

    KeywordTable_Destroy(&QueryWords);

    return EC_SUCCESS;
}

ErrorCode
EndQuery(QueryID ID)
{
    query *Query = QueryTree_Find(&Application.Queries, ID);
    if (Query)
    {
        u32 KeywordCount = GetQueryKeywordCount(Query);

        for (u32 KeywordIndex = 0;
             KeywordIndex < KeywordCount;
             ++KeywordIndex)
        {
            // NOTE(philip): Remove the query from the keyword.
            keyword *Keyword = Query->Keywords[KeywordIndex];
            QueryList_Remove(&Keyword->Queries, Query);

            // TODO(philip): Currenty, keywords that were previously part of queryies are not removed. They remain
            // in the keyword table, as well as the BK trees. This is fine per the assignment, but a proper solution
            // would be to evict them. Hash table removal is easy. BK tree removal on the other hand would require
            // either a complete rebuild or a rebuild of the subtree after the keyword. Another solution is at
            // a certain moment (based on a load factor), rebuild the entire tree. But these would happen here.
        }

        // NOTE(philip): Remove the query from the tree.
        QueryTree_Remove(&Application.Queries, ID);
    }

    return EC_SUCCESS;
}

ErrorCode
MatchDocument(DocID ID, const char *String)
{
#if ISE_MULTI_THREADED
    // TODO(philip): Waste of memory.
    u8 *WorkData = (u8 *)calloc(1, sizeof(u32) + ((MAX_DOCUMENT_LENGTH + 1) * sizeof(char)));

    memcpy(WorkData, &ID, sizeof(u32));
    memcpy(WorkData + sizeof(u32), String, MAX_DOCUMENT_LENGTH * sizeof(char));

    // TODO(philip): Better API.
    PushWork(&ThreadPool.Memory->Queue, WorkType_MatchDocument, WorkData);
#else
    FindDocumentAnswer(&Application.Results, &Application.Keywords, Application.HammingTrees, &Application.EditTree,
                       ID, (char *)String);
#endif

    return EC_SUCCESS;
}

function s32
CompareQueryIDs(const void *A, const void *B)
{
    return (*(s32 *)A - *(s32 *)B);
}

ErrorCode
GetNextAvailRes(DocID *DocumentID, u32 *QueryCount, QueryID **QueryIDs)
{
    // TODO(philip): How do we return no available result?

    result Result = PopResult(&Application.Results);
    *DocumentID = Result.DocumentID;
    *QueryCount = Result.QueryIDCount;
    *QueryIDs = Result.QueryIDs;

#if 0
    if (Application.Answers.Count)
    {
        answer Answer = AnswerStack_Pop(&Application.Answers);
        qsort(Answer.QueryIDs, Answer.QueryIDCount, sizeof(u32), CompareQueryIDs);

        *DocumentID = Answer.DocumentID;
        *QueryCount = Answer.QueryIDCount;
        *QueryIDs = Answer.QueryIDs;

        Result = EC_SUCCESS;
    }
    else
    {
        Result = EC_NO_AVAIL_RES;
    }
#endif

    return EC_SUCCESS;
}
