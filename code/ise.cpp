#include "ise.h"
#include "ise_keyword_table.h"

#include <ctype.h>

global keyword_table KeywordTable = { };

ErrorCode InitializeIndex()
{
    return EC_SUCCESS;
}

ErrorCode DestroyIndex()
{
    KeywordTable_Destroy(&KeywordTable);
    return EC_SUCCESS;
}

ErrorCode StartQuery(QueryID ID, const char *String, MatchType MatchType, u32 MatchDistance)
{
    char *Keywords[MAX_KEYWORDS_PER_QUERY];
    u32 KeywordCount = 0;

    Keywords[KeywordCount] = (char*)String;
    ++KeywordCount;

    for (char *Character = (char *)String;
         *Character;
         ++Character)
    {
        if (*Character == ' ')
        {
            *Character = 0;
            Keywords[KeywordCount] = Character + 1;
            ++KeywordCount;
        }
    }

    for (u32 Index = 0;
         Index < KeywordCount;
         ++Index)
    {
        KeywordTable_Insert(&KeywordTable, Keywords[Index]);
    }

    // KeywordTable_Visualize(&KeywordTable);

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
