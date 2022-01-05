#ifndef ISE_H
#define ISE_H

#include "ise_base.h"

#define MIN_KEYWORD_LENGTH          4
#define MAX_KEYWORD_LENGTH          31
#define MAX_KEYWORD_COUNT_PER_QUERY 5
#define MAX_DISTANCE_THRESHOLD      3
#define MAX_DOCUMENT_LENGTH         (1 << 22)

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

ErrorCode StartQuery(QueryID ID, const char *String, MatchType Type, u32 Distance);
ErrorCode EndQuery(QueryID ID);

ErrorCode MatchDocument(DocID ID, const char *String);
ErrorCode GetNextAvailRes(DocID *DocumentID, u32 *QueryCount, QueryID **QueryIDs);

#include "ise_query_tree.h"
#include "ise_query_list.h"
#include "ise_keyword_table.h"
#include "ise_keyword_list.h"
#include "ise_bk_tree.h"
#include "ise_answer_stack.h"

#endif
