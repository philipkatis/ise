#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ise_base.h"
#include "ise_platform.h"
#include "ise_query.h"
#include "ise_keyword.h"
#include "ise_work.h"
#include "ise.h"

#include "ise_base.cpp"
#include "ise_platform.cpp"
#include "ise_query.cpp"
#include "ise_keyword.cpp"
#include "ise_work.cpp"

ErrorCode
InitializeIndex(void)
{
    InitializePlatform();

    InitializeWorkQueue(&WorkQueue);

    for (u64 Index = 0;
         Index < THREAD_COUNT;
         ++Index)
    {
        Threads[Index] = Platform.CreateThread(WorkerThreadEntryPoint, 0);
    }

    InitializeQueryTree(&Queries);
    InitializeKeywordTable(&Keywords, 1024);

    InitializeKeywordTreeNodeStack(&KeywordTreeNodeStack);
    InitializeKeywordTreeMatchStack(&KeywordTreeMatchStack);

    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        InitializeKeywordTree(HammingTrees + Index, KeywordTreeType_Hamming);
    }

    InitializeKeywordTree(&EditTree, KeywordTreeType_Edit);
    InitializeDocumentAnswerQueue(&DocumentAnswers);

    return EC_SUCCESS;
}

ErrorCode
DestroyIndex(void)
{
    for (u64 Index = 0;
         Index < THREAD_COUNT;
         ++Index)
    {
        PushToWorkQueue(&WorkQueue, WorkType_Exit, 0);
    }

    for (u64 Index = 0;
         Index < THREAD_COUNT;
         ++Index)
    {
        Platform.WaitForThread(Threads[Index]);
        Platform.DestroyThread(Threads[Index]);
    }

    DestroyWorkQueue(&WorkQueue);
    DestroyDocumentAnswerQueue(&DocumentAnswers);

    DestroyKeywordTree(&EditTree);

    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        DestroyKeywordTree(HammingTrees + Index);
    }

    DestroyKeywordTreeNodeStack(&KeywordTreeNodeStack);
    DestroyKeywordTreeMatchStack(&KeywordTreeMatchStack);

    DestroyKeywordTable(&Keywords);
    DestroyQueryTree(&Queries);

    return EC_SUCCESS;
}

ErrorCode
StartQuery(QueryID ID, const char *String, MatchType Type, u32 Distance)
{
    RegisterQuery(ID, Type, Distance, (char *)String);
    return EC_SUCCESS;
}

ErrorCode
EndQuery(QueryID ID)
{
    UnregisterQuery(ID);
    return EC_SUCCESS;
}

ErrorCode
MatchDocument(DocID ID, const char *String)
{
    u64 StringLength = strlen(String);
    u64 DataSize = (sizeof(u32) + ((StringLength + 1) * sizeof(char)));

    u8 *Data = (u8 *)calloc(1, DataSize);

    *(u32 *)Data = ID;
    memcpy(Data + sizeof(u32), String, StringLength * sizeof(char));

    PushToWorkQueue(&WorkQueue, WorkType_GenerateDocumentAnswers, Data);

    return EC_SUCCESS;
}

ErrorCode
GetNextAvailRes(DocID *DocumentID, u32 *QueryCount, QueryID **Queries)
{
    FetchDocumentAnswer(DocumentID, QueryCount, Queries);
    return EC_SUCCESS;
}
