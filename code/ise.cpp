#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ise.h"

#include "ise_base.cpp"
#include "ise_query_tree.cpp"
#include "ise_query_list.cpp"
#include "ise_keyword_table.cpp"
#include "ise_keyword_list.cpp"
#include "ise_bk_tree.cpp"
#include "ise_answer_stack.cpp"

#define HAMMING_TREE_COUNT (MAX_KEYWORD_LENGTH - MIN_KEYWORD_LENGTH)
#define GetHammingTreeIndex(Length) (Length - MIN_KEYWORD_LENGTH)

struct application
{
    query_tree Queries;
    keyword_table Keywords;

    bk_tree HammingTrees[HAMMING_TREE_COUNT];
    bk_tree EditTree;

    answer_stack Answers;
};

global application Application = { };

ErrorCode
InitializeIndex(void)
{
    Assert(false);

    // NOTE(philip): Initialize the keyword table.
    Application.Keywords = KeywordTable_Create(1024);

    // NOTE(philip): Initialize the hamming BK trees.
    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        Application.HammingTrees[Index] = BKTree_Create(BKTree_Type_Hamming);
    }

    // NOTE(philip): Initialize the edit BK tree.
    Application.EditTree = BKTree_Create(BKTree_Type_Edit);

    return EC_SUCCESS;
}

ErrorCode
DestroyIndex(void)
{
    // NOTE(philip): Destroy the possible answer stack.
    AnswerStack_Destroy(&Application.Answers);

    // NOTE(philip): Destroy the edit tree.
    BKTree_Destroy(&Application.EditTree);

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
            BKTree_Insert(&Application.EditTree, Keyword);
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

        if (KeywordInsert.Exists && ((Type == 1) || (Type == 2)))
        {
            // NOTE(philip): If the keyword is already in the BK tree, do not attempt to insert it again.
            b32 AlreadyInBK = false;

            // TODO(philip): If we choose to use the occupancy flags, use them here.
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

        // NOTE(philip): Insert the keyword into the query and the query into the keyword.
        Query->Keywords[WordIndex] = Keyword;
        QueryList_Insert(&Keyword->Queries, Query);
    }

    return EC_SUCCESS;
}

ErrorCode
EndQuery(QueryID ID)
{
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

            // TODO(philip): Currenty, keywords that were previously part of queryies are not removed. They remain
            // in the keyword table, as well as the BK trees. This is fine per the assignment, but a proper solution
            // would be to evict them. Hash table removal is easy. BK tree removal on the other hand would require
            // either a complete rebuild or a rebuild of the subtree after the keyword. Another solution is at
            // a certain moment (based on a load factor), rebuild the entire tree. But these would happen here.
        }

        // NOTE(philip): Remove the query from the tree.
        QueryTree_Remove(&Application.Queries, ID);
    }

    return EC_SUCCESS;
}

function b32
IsAnswer(query *Query)
{
    b32 Result = true;
    u32 KeywordCount = GetQueryKeywordCount(Query);

    for (u32 KeywordIndex = 0;
         KeywordIndex < KeywordCount;
         ++KeywordIndex)
    {
        if (!Query->HasKeywordFlags[KeywordIndex])
        {
            Result = false;
            break;
        }
    }

    return Result;
}

function u32
CountAnsweredQueries_(query_tree_node *Node)
{
    u32 Count = IsAnswer(&Node->Data);

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

function u32
CompileAnsweredQueries_(query_tree_node *Node, u32 Index, u32 *Result)
{
    query *Query = &Node->Data;
    if (IsAnswer(Query))
    {
        Result[Index] = Query->ID;
        ++Index;
    }

    if (Node->Left)
    {
        Index = CompileAnsweredQueries_(Node->Left, Index, Result);
    }

    if (Node->Right)
    {
        Index = CompileAnsweredQueries_(Node->Right, Index, Result);
    }

    return Index;
}

function void
CompileAnsweredQueries(query_tree *Tree, u32 *Result)
{
    if (Tree->Root)
    {
        CompileAnsweredQueries_(Tree->Root, 0, Result);
    }
}

function void
LookForMatchingQueries(query_tree *PossibleAnswers, u32 Type, u32 Threshold, keyword *Keyword)
{
    if (Keyword)
    {
        // NOTE(philip): If it exists go through the queries it is part of.
        for (query_list_node *Node = Keyword->Queries.Head;
             Node;
             Node = Node->Next)
        {
            // NOTE(philip): Stop by every query that satisfies the search results.
            query *Query = Node->Query;
            if ((GetQueryType(Query) == Type) && (GetQueryDistance(Query) == Threshold))
            {
                // NOTE(philip): Insert that query in the possible answer query tree.
                u32 KeywordCount = GetQueryKeywordCount(Query);
                query_tree_insert_result InsertResult = QueryTree_Insert(PossibleAnswers, Query->ID, KeywordCount,
                                                                         Type, 0);
                query *PossibleAnswer = InsertResult.Query;

                if (PossibleAnswer)
                {
                    // NOTE(philip): Go through all the keywords that the query has.
                    for (u32 KeywordIndex = 0;
                         KeywordIndex < KeywordCount;
                         ++KeywordIndex)
                    {
                        if (Query->Keywords[KeywordIndex] == Keyword)
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

        // NOTE(philip): Exact matching.
        {
            // NOTE(philip): Find the word in keyword table.
            keyword *FoundKeyword = KeywordTable_Find(&Application.Keywords, DocumentWord->Word);
            LookForMatchingQueries(&PossibleAnswers, 0, 0, FoundKeyword);
        }

        // NOTE(philip): Approximate matching.
        {
            // NOTE(philip): Find the hamming tree we want to search.
            bk_tree *HammingTree = &Application.HammingTrees[GetHammingTreeIndex(DocumentWord->Length)];

            // NOTE(philip): Go through all the thresholds.
            for (u32 Threshold = 1;
                 Threshold <= MAX_DISTANCE_THRESHOLD;
                 ++Threshold)
            {
                // NOTE(philip): Hamming tree.
                {
                    // NOTE(philip): Find the matches.
                    keyword_list FoundKeywords = BKTree_FindMatches(HammingTree, DocumentWord, Threshold);

                    // NOTE(philip): Go through all the keywords and update the queries.
                    for (keyword_list_node *Node = FoundKeywords.Head;
                         Node;
                         Node = Node->Next)
                    {
                        keyword *FoundKeyword = Node->Keyword;
                        LookForMatchingQueries(&PossibleAnswers, 1, Threshold, FoundKeyword);
                    }

                    KeywordList_Destroy(&FoundKeywords);
                }

                // NOTE(philip): Edit tree.
                {
                    // NOTE(philip): Find the matches.
                    keyword_list FoundKeywords = BKTree_FindMatches(&Application.EditTree, DocumentWord, Threshold);

                    // NOTE(philip): Go through all the keywords and update the queries.
                    for (keyword_list_node *Node = FoundKeywords.Head;
                         Node;
                         Node = Node->Next)
                    {
                        keyword *FoundKeyword = Node->Keyword;
                        LookForMatchingQueries(&PossibleAnswers, 2, Threshold, FoundKeyword);
                    }

                    KeywordList_Destroy(&FoundKeywords);
                }
            }
        }
    }

    KeywordTable_Destroy(&DocumentWords);

    answer Answer = { };
    Answer.DocumentID = ID;

    // NOTE(philip): Compile the answered queries. This will be deallocated by the client.
    Answer.QueryIDCount = CountAnsweredQueries(&PossibleAnswers);
    if (Answer.QueryIDCount)
    {
        Answer.QueryIDs = (u32 *)calloc(1, Answer.QueryIDCount * sizeof(u32));
        CompileAnsweredQueries(&PossibleAnswers, Answer.QueryIDs);
    }

    QueryTree_Destroy(&PossibleAnswers);
    AnswerStack_Push(&Application.Answers, Answer);

    return EC_SUCCESS;
}

function s32
CompareQueryIDs(const void *A, const void *B)
{
    return (*(s32 *)A - *(s32 *)B);
}

ErrorCode
GetNextAvailRes(DocID *DocumentID, u32 *QueryCount, QueryID **QueryIDs)
{
    ErrorCode Result = EC_FAIL;

    if (Application.Answers.Count)
    {
        answer Answer = AnswerStack_Pop(&Application.Answers);
        qsort(Answer.QueryIDs, Answer.QueryIDCount, sizeof(u32), CompareQueryIDs);

        *DocumentID = Answer.DocumentID;
        *QueryCount = Answer.QueryIDCount;
        *QueryIDs = Answer.QueryIDs;

        Result = EC_SUCCESS;
    }
    else
    {
        Result = EC_NO_AVAIL_RES;
    }

    return Result;
}
