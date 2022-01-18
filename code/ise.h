#ifndef ISE_H
#define ISE_H

//
// NOTE(philip): Constants
//

#define MIN_KEYWORD_LENGTH              4
#define MAX_KEYWORD_LENGTH              31
#define MAX_KEYWORD_COUNT_PER_QUERY     5
#define MAX_DISTANCE_THRESHOLD          3
#define MAX_DOCUMENT_LENGTH             (1 << 22)

//
// NOTE(philip): ISE Library Interface
//

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

ErrorCode InitializeIndex();
ErrorCode DestroyIndex();

ErrorCode StartQuery(QueryID ID, const char *String, MatchType Type, unsigned int Distance);
ErrorCode EndQuery(QueryID ID);

ErrorCode MatchDocument(DocID ID, const char *String);
ErrorCode GetNextAvailRes(DocID *DocumentID, unsigned int *QueryCount, QueryID **QueryIDs);

#endif
