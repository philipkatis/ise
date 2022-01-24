#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ise_base.h"
#include "ise_keyword.h"
#include "ise_query.h"
#include "ise.h"

#include "ise_base.cpp"
#include "ise_keyword.cpp"
#include "ise_query.cpp"
#include "ise_work.cpp"

#define HAMMING_TREE_COUNT (MAX_KEYWORD_LENGTH - MIN_KEYWORD_LENGTH)

struct application
{
    query_tree Queries;
    keyword_table Keywords;

    keyword_tree HammingTrees[HAMMING_TREE_COUNT];
    keyword_tree EditTree;
};

global application Application = { };

ErrorCode
InitializeIndex(void)
{
    InitializeQueryTree(&Application.Queries);
    InitializeKeywordTable(&Application.Keywords, 1024);

    // NOTE(philip): Initialize the hamming BK trees.
    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        InitializeKeywordTree(Application.HammingTrees + Index, KeywordTreeType_Hamming);
    }

    InitializeKeywordTree(&Application.EditTree, KeywordTreeType_Edit);

    return EC_SUCCESS;
}

ErrorCode
DestroyIndex(void)
{
    DestroyKeywordTree(&Application.EditTree);

    // NOTE(philip): Destroy the hamming BK trees.
    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        DestroyKeywordTree(Application.HammingTrees + Index);
    }

    DestroyKeywordTable(&Application.Keywords);
    DestroyQueryTree(&Application.Queries);

    return EC_SUCCESS;
}

ErrorCode
StartQuery(QueryID ID, const char *String, MatchType Type, u32 Distance)
{
    RegisterQuery(&Application.Queries, &Application.Keywords, Application.HammingTrees, &Application.EditTree,
                  ID, Type, Distance, (char *)String);

    return EC_SUCCESS;
}

ErrorCode
EndQuery(QueryID ID)
{
    UnregisterQuery(&Application.Queries, Application.HammingTrees, &Application.EditTree, ID);
    return EC_SUCCESS;
}

ErrorCode
MatchDocument(DocID ID, const char *String)
{
    GenerateDocumentAnswers(&Application.Queries, &Application.Keywords, Application.HammingTrees,
                            &Application.EditTree, ID, (char *)String);

    return EC_SUCCESS;
}

ErrorCode
GetNextAvailRes(DocID *DocumentID, u32 *QueryCount, QueryID **Queries)
{
    document_answer Answer = PopDocumentAnswer();

    *DocumentID = Answer.DocumentID;
    *QueryCount = Answer.QueryCount;
    *Queries    = Answer.Queries;

    return EC_SUCCESS;
}
