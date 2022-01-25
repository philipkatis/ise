#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ise_base.h"
#include "ise_query.h"
#include "ise_keyword.h"
#include "ise_work.h"
#include "ise.h"

#include "ise_base.cpp"
#include "ise_query.cpp"
#include "ise_keyword.cpp"
#include "ise_work.cpp"

ErrorCode
InitializeIndex(void)
{
    InitializeGlobalContext();
    return EC_SUCCESS;
}

ErrorCode
DestroyIndex(void)
{
    DestroyGlobalContext();
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
    GenerateDocumentAnswers(ID, (char *)String);
    return EC_SUCCESS;
}

ErrorCode
GetNextAvailRes(DocID *DocumentID, u32 *QueryCount, QueryID **Queries)
{
    FetchDocumentAnswer(DocumentID, QueryCount, Queries);
    return EC_SUCCESS;
}
