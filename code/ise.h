#ifndef ISE_H
#define ISE_H

//
// NOTE(philip): ISE Interface
//

#define MAX_DOCUMENT_LENGTH (1 << 22)

typedef unsigned int QueryID;
typedef unsigned int DocID;

enum MatchType
{
    MT_EXACT_MATCH,
    MT_HAMMING_DIST,
    MT_EDIT_DIST
};

enum ErrorCode
{
    EC_SUCCESS,
    EC_NO_AVAIL_RES,
    EC_FAIL
};

ErrorCode InitializeIndex(void);
ErrorCode DestroyIndex(void);

ErrorCode StartQuery(QueryID ID, const char *String, MatchType Type, unsigned int DistanceThreshold);
ErrorCode EndQuery(QueryID ID);

ErrorCode MatchDocument(DocID ID, const char *String);
ErrorCode GetNextAvailRes(DocID *DocumentID, unsigned int *QueryCount, QueryID **Queries);

#endif
