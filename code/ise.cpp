#include "ise_query_tree.h"
#include "ise_keyword_table.h"

struct application
{
    query_tree Queries;
    keyword_table Keywords;
};

global application Application = { };

ErrorCode
InitializeIndex()
{
    Application.Keywords = KeywordTable_Create(1024);
    return EC_SUCCESS;
}

ErrorCode
DestroyIndex()
{
    KeywordTable_Destroy(&Application.Keywords);
    QueryTree_Destroy(&Application.Queries);

    return EC_SUCCESS;
}

ErrorCode
StartQuery(QueryID ID, const char *String, MatchType Type, u32 Distance)
{
    Assert(Type >= 0 && Type <= 2);
    Assert(Distance >= 0 && Distance <= MAX_DISTANCE_THRESHOLD);

    char *Words[MAX_KEYWORD_COUNT_PER_QUERY];
    u32 WordCount = 0;

    // NOTE(philip): Count and split the words from the input.
    {
        Words[WordCount] = (char*)String;
        ++WordCount;

        for (char *Character = (char *)String;
             *Character;
             ++Character)
        {
            if (*Character == ' ')
            {
                *Character = 0;
                Words[WordCount] = Character + 1;
                ++WordCount;
            }
        }
    }

    Assert(WordCount > 0 && WordCount <= MAX_KEYWORD_COUNT_PER_QUERY);

    // NOTE(philip): Insert the query into the tree.
    query_tree_insert_result QueryInsert = QueryTree_Insert(&Application.Queries, ID, WordCount, Type,
                                                            Distance);
    if (QueryInsert.Exists)
    {
        return EC_FAIL;
    }

    query *Query = QueryInsert.Query;

    for (u32 WordIndex = 0;
         WordIndex < WordCount;
         ++WordIndex)
    {
        // NOTE(philip): Insert the keyword into the keyword hash table.
        keyword_table_insert_result KeywordInsert = KeywordTable_Insert(&Application.Keywords, Words[WordIndex]);
        keyword *Keyword = KeywordInsert.Keyword;

        // NOTE(philip): Insert the keyword into the query and the query into the keyword.
        Query->Keywords[WordIndex] = Keyword;
        QueryList_Insert(&Keyword->Queries, Query);

        // TODO(philip): Add keyword to BK-trees.
    }

    return EC_SUCCESS;
}

ErrorCode
EndQuery(QueryID ID)
{
    // TODO(philip): Add unit testing for this.
    query *Query = QueryTree_Find(&Application.Queries, ID);
    if (Query)
    {
        u32 KeywordCount = GetKeywordCount(Query);

        for (u32 KeywordIndex = 0;
             KeywordIndex < KeywordCount;
             ++KeywordIndex)
        {
            // NOTE(philip): Remove the query from the keyword.
            keyword *Keyword = Query->Keywords[KeywordIndex];
            QueryList_Remove(&Keyword->Queries, Query);

            // TODO(philip): Remove keyword from tree. Remove keyword if it is in no queries.
        }

        // NOTE(philip): Remove the query from the tree.
        QueryTree_Remove(&Application.Queries, ID);
    }

    return EC_SUCCESS;
}

ErrorCode
MatchDocument(DocID ID, const char *String)
{
    u64 WordCount = 1;

    // NOTE(philip): Count and split the words from the input.
    {
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
    }

    // TODO(philip): This is currently a waste of memory. If it becomes a problem, switch to a more specific
    // data structure.
    keyword_table Words = KeywordTable_Create(WordCount);

    // NOTE(philip): Put the document words in the hash table to deduplicate them.
    {
        char *Word = (char *)String;

        for (u64 WordIndex = 0;
             WordIndex < WordCount;
             ++WordIndex)
        {
            KeywordTable_Insert(&Words, Word);

            while (*Word)
            {
                ++Word;
            }

            if (WordIndex < WordCount - 1)
            {
                ++Word;
            }
        }
    }

    for (keyword_iterator Iterator = IterateAllKeywords(&Words);
         IsValid(&Iterator);
         Advance(&Iterator))
    {
        keyword *Keyword = GetValue(&Iterator);
        printf("%s\n", Keyword->Word);
    }

    Assert(false);

#if 0
    query_list AnsweredQueries = { };

    for (u64 BucketIndex = 0;
         BucketIndex < KEYWORD_TABLE_BUCKET_COUNT;
         ++BucketIndex)
    {
        for (keyword_table_node *Node = DocumentWords.Buckets[BucketIndex];
             Node;
             Node = Node->Next)
        {
            // NOTE(philip): Exact Matching.
            {
                keyword_table_node *Keyword = KeywordTable_Find(&Application.KeywordTable, Node->Word, Node->Hash);
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

            // NOTE(philip): Hamming Matching.
            {
                u64 WordLength = strlen(Node->Word);
                u64 TreeIndex = HammingTreeIndex(WordLength);

                for (u32 Distance = 1;
                     Distance <= 4;
                     ++Distance)
                {
                    BKTree_FindMatches(&Application.HammingTrees[TreeIndex], Node->Word, Distance);

                    // TODO(philip): Do stuff.
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

    QueryList_Destroy(&AnsweredQueries);
#endif

    KeywordTable_Destroy(&Words);

    return EC_SUCCESS;
}

ErrorCode
GetNextAvailRes(DocID *DocumentIDs, u32 *QueryCount, QueryID **QueryIDs)
{
    return EC_SUCCESS;
}
