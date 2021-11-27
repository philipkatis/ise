#include "ise.h"

ErrorCode InitializeIndex()
{
    return EC_SUCCESS;
}

ErrorCode DestroyIndex()
{
    return EC_SUCCESS;
}

ErrorCode StartQuery(QueryID ID, const char *String, MatchType MatchType, u32 MatchDistance)
{
    return EC_SUCCESS;
}

ErrorCode EndQuery(QueryID ID)
{
    return EC_SUCCESS;
}

ErrorCode MatchDocument(DocID ID, const char *String)
{
    return EC_SUCCESS;
}

ErrorCode GetNextAvailRes(DocID *DocumentIDs, u32 *QueryCount, QueryID **QueryIDs)
{
    return EC_SUCCESS;
}
