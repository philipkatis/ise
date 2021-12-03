#include "ise.h"
#include "ise_keyword_table.h"

#include <ctype.h>

global keyword_table KeywordTable = { };

ErrorCode
InitializeIndex()
{
    return EC_SUCCESS;
}

ErrorCode
DestroyIndex()
{
    // KeywordTable_Visualize(&KeywordTable);

    KeywordTable_Destroy(&KeywordTable);
    return EC_SUCCESS;
}

ErrorCode
StartQuery(QueryID ID, const char *String, MatchType MatchType, u32 MatchDistance)
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
        keyword_table_node *Keyword = KeywordTable_Insert(&KeywordTable, Keywords[Index]);

        query *Query = QueryList_Find(&Keyword->Queries, ID);
        if (!Query)
        {
            Query = QueryList_Insert(&Keyword->Queries, ID, KeywordCount, (u8)MatchType, (u16)MatchDistance);
        }
    }

    return EC_SUCCESS;
}

ErrorCode
EndQuery(QueryID ID)
{
    for (u64 BucketIndex = 0;
         BucketIndex < KEYWORD_TABLE_BUCKET_COUNT;
         ++BucketIndex)
    {
        if (KeywordTable.Buckets[BucketIndex])
        {
            for (keyword_table_node *Node = KeywordTable.Buckets[BucketIndex];
                 Node;
                 Node = Node->Next)
            {
                QueryList_Remove(&Node->Queries, ID);
            }
        }
    }

    return EC_SUCCESS;
}

ErrorCode
MatchDocument(DocID ID, const char *String)
{
    u64 WordCount = 1;

    for (char *Character = (char *)String;
         *Character;
         ++Character)
    {
        if (*Character == ' ')
        {
            *Character = 0;
            ++WordCount;
        }
    }

    keyword_table DocumentWords = { };
    char *Word = (char *)String;

    for (u64 WordIndex = 0;
         WordIndex < WordCount;
         ++WordIndex)
    {
        KeywordTable_Insert(&DocumentWords, Word);

        while (*Word)
        {
            ++Word;
        }

        if (WordIndex < WordCount - 1)
        {
            ++Word;
        }
    }

    query_list AnsweredQueries = { };

    for (u64 BucketIndex = 0;
         BucketIndex < KEYWORD_TABLE_BUCKET_COUNT;
         ++BucketIndex)
    {
        for (keyword_table_node *Node = DocumentWords.Buckets[BucketIndex];
             Node;
             Node = Node->Next)
        {
            keyword_table_node *Keyword = KeywordTable_Find(&KeywordTable, Node->Word, Node->Hash);
            if (Keyword)
            {
                for (query *Query = Keyword->Queries.Head;
                     Query;
                     Query = Query->Next)
                {
                    if (Query->Type == 0)
                    {
                        query *ResultingQuery = QueryList_Find(&AnsweredQueries, Query->ID);
                        if (!ResultingQuery)
                        {
                            ResultingQuery = QueryList_Insert(&AnsweredQueries, Query->ID, Query->WordCount, Query->Type, 1);
                        }
                        else
                        {
                            ++ResultingQuery->WordsFound;
                        }
                    }
                }
            }
        }
    }

    for (query *Query = AnsweredQueries.Head;
         Query;
         Query = Query->Next)
    {
        if (Query->WordCount == Query->WordsFound)
        {
//            printf("ID: %d, WordCount: %d, Type: %d, Words Found: %d\n", Query->ID, Query->WordCount, Query->Type, Query->WordsFound);
        }
    }

    KeywordTable_Destroy(&DocumentWords);
    QueryList_Destroy(&AnsweredQueries);

    return EC_SUCCESS;
}

ErrorCode
GetNextAvailRes(DocID *DocumentIDs, u32 *QueryCount, QueryID **QueryIDs)
{
    return EC_SUCCESS;
}
