#include "ise_query_tree.h"
#include "ise_keyword_table.h"
#include "ise_bk_tree.h"

#define HAMMING_TREE_COUNT (MAX_KEYWORD_LENGTH - MIN_KEYWORD_LENGTH)
#define GetHammingTreeIndex(Length) (Length - MIN_KEYWORD_LENGTH)

struct application
{
    query_tree Queries;
    keyword_table Keywords;
    bk_tree HammingTrees[HAMMING_TREE_COUNT];
};

global application Application = { };

ErrorCode
InitializeIndex()
{
    // NOTE(philip): Initialize the keyword table.
    Application.Keywords = KeywordTable_Create(1024);

    // NOTE(philip): Initialize the hamming BK trees.
    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        // TODO(philip): Change the type.
        Application.HammingTrees[Index] = BKTree_Create(1);
    }

    return EC_SUCCESS;
}

ErrorCode
DestroyIndex()
{
    // NOTE(philip): Destroy the hamming BK trees.
    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        BKTree_Destroy(&Application.HammingTrees[Index]);
    }

    // NOTE(philip): Destroy the keyword table and the query tree.
    KeywordTable_Destroy(&Application.Keywords);
    QueryTree_Destroy(&Application.Queries);

    return EC_SUCCESS;
}

function void
InsertInBK(u32 Type, keyword *Keyword)
{
    switch (Type)
    {
        case 1:
        {
            u64 TreeIndex = GetHammingTreeIndex(Keyword->Length);
            BKTree_Insert(&Application.HammingTrees[TreeIndex], Keyword);
        } break;

        case 2:
        {
            // TODO(philip): Insert in levenshtein tree.
        } break;
    }
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

        if (KeywordInsert.Exists && ((Type == 1) || (Type == 2)))
        {
            // NOTE(philip): If the keyword is already in the BK tree, do not attempt to insert it again.
            b32 AlreadyInBK = false;

            // TODO(philp): Make an iterator for this. Or use flags.
            for (query_list_node *Node = Keyword->Queries.Head;
                 Node;
                 Node = Node->Next)
            {
                if (GetQueryType(Node->Query) == (u8)Type)
                {
                    AlreadyInBK = true;
                    break;
                }
            }

            if (!AlreadyInBK)
            {
                InsertInBK(Type, Keyword);
            }
        }
        else
        {
            InsertInBK(Type, Keyword);
        }
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
        u32 KeywordCount = GetQueryKeywordCount(Query);

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

function u32
CountAnsweredQueries_(query_tree_node *Node)
{
    b32 Answered = true;

    query *Query = &Node->Data;
    u32 KeywordCount = GetQueryKeywordCount(Query);

    for (u32 KeywordIndex = 0;
         KeywordIndex < KeywordCount;
         ++KeywordIndex)
    {
        if (!Query->HasKeywordFlags[KeywordIndex])
        {
            Answered = false;
            break;
        }
    }

    u32 Count = Answered;

    if (Node->Left)
    {
        Count += CountAnsweredQueries_(Node->Left);
    }

    if (Node->Right)
    {
        Count += CountAnsweredQueries_(Node->Right);
    }

    return Count;
}

function u32
CountAnsweredQueries(query_tree *Tree)
{
    u32 Result = 0;

    if (Tree->Root)
    {
        Result = CountAnsweredQueries_(Tree->Root);
    }

    return Result;
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
    keyword_table DocumentWords = KeywordTable_Create(WordCount);

    // NOTE(philip): Put the document words in the hash table to deduplicate them.
    {
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
    }

    // TODO(philip): This is currently a waste of memroy. If it becomes a problem, switch to a more specific
    // data structure.
    query_tree PossibleAnswers = { };

    for (keyword_iterator Iterator = IterateAllKeywords(&DocumentWords);
         IsValid(&Iterator);
         Advance(&Iterator))
    {
        keyword *DocumentWord = GetValue(&Iterator);

        // NOTE(philip): Exact macthing.
        {
            // NOTE(philip): Find the word in keyword table.
            keyword *FoundKeyword = KeywordTable_Find(&Application.Keywords, DocumentWord->Word);
            if (FoundKeyword)
            {
                // TODO(philip): Make an iterator for this.
                // NOTE(philip): If it exists go through the queries it is part of.
                for (query_list_node *Node = FoundKeyword->Queries.Head;
                     Node;
                     Node = Node->Next)
                {
                    // NOTE(philip): Stop by every query that requires exact matching.
                    query *Query = Node->Query;
                    if (GetQueryType(Query) == 0)
                    {
                        // NOTE(philip): Insert that query in the possible answer query tree.
                        u32 KeywordCount = GetQueryKeywordCount(Query);
                        query_tree_insert_result InsertResult = QueryTree_Insert(&PossibleAnswers, Query->ID,
                                                                                 KeywordCount, 0, 0);
                        query *PossibleAnswer = InsertResult.Query;

                        // TODO(philip): Redundant? In the exact matching case we just need to store a count?
                        if (PossibleAnswer)
                        {
                            // NOTE(philip): Go through all the keywords that the query has.
                            for (u32 KeywordIndex = 0;
                                 KeywordIndex < KeywordCount;
                                 ++KeywordIndex)
                            {
                                if (Query->Keywords[KeywordIndex] == FoundKeyword)
                                {
                                    // NOTE(philip): Find the index of the found query and set the flag.
                                    PossibleAnswer->HasKeywordFlags[KeywordIndex] = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        // TODO(philip): Run each word through the bk-trees to find approximate matches.
        {

        }

        {

        }
    }

    KeywordTable_Destroy(&DocumentWords);

    u32 AnswerCount = CountAnsweredQueries(&PossibleAnswers);
    printf("%d\n", AnswerCount);

    // TODO(philip): From the possible matches, find the ones that were actually match.

    QueryTree_Destroy(&PossibleAnswers);
#if 0
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
#endif

    return EC_SUCCESS;
}

ErrorCode
GetNextAvailRes(DocID *DocumentIDs, u32 *QueryCount, QueryID **QueryIDs)
{
    return EC_SUCCESS;
}
