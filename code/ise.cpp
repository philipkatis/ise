#include "ise.h"
#include "ise_keyword_table.h"
#include "ise_bk_tree.h"

#include <string.h>
#include <ctype.h>

#define HAMMING_TREE_COUNT (MAX_KEYWORD_LENGTH - MIN_KEYWORD_LENGTH + 1)

struct application
{
    keyword_table KeywordTable;
    bk_tree HammingTrees[HAMMING_TREE_COUNT];
};

global application Application = { };

function u64
HammingTreeIndex(u64 Length)
{
    return Length - MIN_KEYWORD_LENGTH;
}

ErrorCode
InitializeIndex()
{
    for (u32 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        Application.HammingTrees[Index] = BKTree_Create(BKTreeType_Hamming);
    }

    return EC_SUCCESS;
}

ErrorCode
DestroyIndex()
{
    // KeywordTable_Visualize(&KeywordTable);

    KeywordTable_Destroy(&Application.KeywordTable);
    return EC_SUCCESS;
}

ErrorCode
StartQuery(QueryID ID, const char *String, MatchType Type, u32 Distance)
{
    char *Words[MAX_KEYWORDS_PER_QUERY];
    u32 WordCount = 0;

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

    for (u32 Index = 0;
         Index < WordCount;
         ++Index)
    {
        // TODO(philip): Maybe store the length in the keyword.
        u64 WordLength = strlen(Words[Index]);
        keyword_table_node *Keyword = KeywordTable_Insert(&Application.KeywordTable, Words[Index]);

        query *Query = QueryList_Find(&Keyword->Queries, ID);
        if (!Query)
        {
            Query = QueryList_Insert(&Keyword->Queries, ID, WordCount, (u8)Type, (u16)Distance);
        }

        BKTree_Insert(&Application.HammingTrees[HammingTreeIndex(WordLength)], Keyword);
    }

    return EC_SUCCESS;
}

ErrorCode
EndQuery(QueryID ID)
{
    // TODO(philip): Make an iterator for this.
    for (u64 BucketIndex = 0;
         BucketIndex < KEYWORD_TABLE_BUCKET_COUNT;
         ++BucketIndex)
    {
        if (Application.KeywordTable.Buckets[BucketIndex])
        {
            for (keyword_table_node *Node = Application.KeywordTable.Buckets[BucketIndex];
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
