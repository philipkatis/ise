#ifndef ISE_H
#define ISE_H

#include "ise_base.h"

#define MAX_KEYWORD_LENGTH 31

typedef u32 QueryID;
typedef u32 DocID;

typedef u32 MatchType;
enum
{
    MT_EXACT_MATCH,
    MT_HAMMING_DIST,
    MT_EDIT_DIST
};

typedef u32 ErrorCode;
enum
{
    EC_SUCCESS,
    EC_NO_AVAIL_RES,
    EC_FAIL
};

ErrorCode InitializeIndex();
ErrorCode DestroyIndex();

ErrorCode StartQuery(QueryID ID, const char *String, MatchType MatchType, u32 MatchDistance);
ErrorCode EndQuery(QueryID ID);

ErrorCode MatchDocument(DocID ID, const char *String);
ErrorCode GetNextAvailRes(DocID *DocumentIDs, u32 *QueryCount, QueryID **QueryIDs);

#endif
