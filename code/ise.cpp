#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ise_query_tree.h"

#include "ise_keyword.h"

#define KEYWORD_TREE_MATCH_STORAGE_SIZE 1024

#include "ise_base.cpp"
#include "ise_keyword.cpp"

struct document_answer
{
    u32 DocumentID;
    u32 *Queries;
    u64 QueryCount;
};

#define DOCUMENT_ANSWER_STORAGE_SIZE 1024

global document_answer DocumentAnswers[DOCUMENT_ANSWER_STORAGE_SIZE];
global u64 DocumentAnswerCount = 0;

global keyword_tree_match KeywordTreeMatches[KEYWORD_TREE_MATCH_STORAGE_SIZE];
global u64 KeywordTreeMatchCount = 0;

function void
PushDocumentAnswer(u32 DocumentID, u64 QueryCount, u32 *Queries)
{
    Assert((DocumentAnswerCount + 1) <= DOCUMENT_ANSWER_STORAGE_SIZE);

    document_answer *Answer = DocumentAnswers + DocumentAnswerCount++;
    Answer->DocumentID = DocumentID;
    Answer->QueryCount = QueryCount;
    Answer->Queries = Queries;
}

function document_answer
PopDocumentAnswer(void)
{
    Assert(DocumentAnswerCount > 0);

    document_answer Answer = DocumentAnswers[--DocumentAnswerCount];
    return Answer;
}

#define HAMMING_TREE_COUNT (MAX_KEYWORD_LENGTH - MIN_KEYWORD_LENGTH)
#define GetHammingTreeIndex(Length) (Length - MIN_KEYWORD_LENGTH)

struct application
{
    query_tree Queries;
    keyword_table Keywords;

    keyword_tree HammingTrees[HAMMING_TREE_COUNT];
    keyword_tree EditTree;
};

global application Application = { };

ErrorCode
InitializeIndex(void)
{
    InitializeKeywordTable(&Application.Keywords, 1024);

    // NOTE(philip): Initialize the hamming BK trees.
    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        InitializeKeywordTree(Application.HammingTrees + Index, KeywordTreeType_Hamming);
    }

    InitializeKeywordTree(&Application.EditTree, KeywordTreeType_Edit);

    return EC_SUCCESS;
}

ErrorCode
DestroyIndex(void)
{
    DestroyKeywordTree(&Application.EditTree);

    // NOTE(philip): Destroy the hamming BK trees.
    for (u64 Index = 0;
         Index < HAMMING_TREE_COUNT;
         ++Index)
    {
        DestroyKeywordTree(Application.HammingTrees + Index);
    }

    DestroyKeywordTable(&Application.Keywords);

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

    switch (Type)
    {
        case 0:
        {
            for (u32 WordIndex = 0;
                 WordIndex < WordCount;
                 ++WordIndex)
            {
                keyword *Keyword = InsertIntoKeywordTable(&Application.Keywords, Words[WordIndex]);
                Query->Keywords[WordIndex] = Keyword;
            }
        } break;

        case 1:
        {
            for (u32 WordIndex = 0;
                 WordIndex < WordCount;
                 ++WordIndex)
            {
                keyword *Keyword = InsertIntoKeywordTable(&Application.Keywords, Words[WordIndex]);

                if (!Keyword->IsInHammingTree)
                {
                    keyword_tree *Tree = Application.HammingTrees + GetHammingTreeIndex(Keyword->Length);
                    InsertIntoKeywordTree(Tree, Keyword);

                    Keyword->IsInHammingTree = true;
                }

                Query->Keywords[WordIndex] = Keyword;
            }
        } break;

        case 2:
        {
            for (u32 WordIndex = 0;
                 WordIndex < WordCount;
                 ++WordIndex)
            {
                keyword *Keyword = InsertIntoKeywordTable(&Application.Keywords, Words[WordIndex]);

                if (!Keyword->IsInEditTree)
                {
                    InsertIntoKeywordTree(&Application.EditTree, Keyword);
                    Keyword->IsInEditTree = true;
                }

                Query->Keywords[WordIndex] = Keyword;
            }
        } break;
    }

    return EC_SUCCESS;
}

ErrorCode
EndQuery(QueryID ID)
{
    query *Query = QueryTree_Find(&Application.Queries, ID);
    if (Query)
    {
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
LookForMatchingQueries_(query_tree_node *Node, query_tree *PossibleAnswers, u32 Type, keyword *Keyword, u32 Distance)
{
    query *Query = &Node->Data;

    if ((GetQueryType(Query) == Type) && (GetQueryDistance(Query) >= Distance))
    {
        u64 KeywordCount = GetQueryKeywordCount(Query);

        for (u64 Index = 0;
             Index < KeywordCount;
             ++Index)
        {
            if (Query->Keywords[Index] == Keyword)
            {
                query_tree_insert_result InsertResult = QueryTree_Insert(PossibleAnswers, Query->ID, KeywordCount,
                                                                         Type, 0);
                query *PossibleAnswer = InsertResult.Query;
                if (PossibleAnswer)
                {
                    PossibleAnswer->HasKeywordFlags[Index] = true;
                }

                // NOTE(philip): Don't break here. Duplicate words.
            }
        }
    }

    if (Node->Left)
    {
        LookForMatchingQueries_(Node->Left, PossibleAnswers, Type, Keyword, Distance);
    }

    if (Node->Right)
    {
        LookForMatchingQueries_(Node->Right, PossibleAnswers, Type, Keyword, Distance);
    }
}

function void
LookForMatchingQueries(query_tree *Queries, query_tree *PossibleAnswers, u32 Type, keyword *Keyword, u32 Distance)
{
    if (Queries->Root)
    {
        LookForMatchingQueries_(Queries->Root, PossibleAnswers, Type, Keyword, Distance);
    }
}

function s32
CompareQueryIDs(const void *A, const void *B)
{
    return (*(s32 *)A - *(s32 *)B);
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

    keyword_table DocumentWords;
    InitializeKeywordTable(&DocumentWords, WordCount);

    // NOTE(philip): Put the document words in the hash table to deduplicate them.
    {
        char *Word = (char *)String;

        for (u64 WordIndex = 0;
             WordIndex < WordCount;
             ++WordIndex)
        {
            InsertIntoKeywordTable(&DocumentWords, Word);

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

    for (keyword_table_iterator Iterator = IterateKeywordTable(&DocumentWords);
         IsValid(&Iterator);
         Advance(&Iterator))
    {
        keyword *DocumentWord = GetValue(&Iterator);

        keyword *Keyword = FindKeywordInTable(&Application.Keywords, DocumentWord);
        if (Keyword)
        {
            LookForMatchingQueries(&Application.Queries, &PossibleAnswers, 0, Keyword, 0);
        }

        keyword_tree *HammingTree = Application.HammingTrees + GetHammingTreeIndex(DocumentWord->Length);

        KeywordTreeMatchCount = 0;
        FindMatchesInKeywordTree(HammingTree, DocumentWord, &KeywordTreeMatchCount, KeywordTreeMatches);

        for (u64 Index = 0;
             Index < KeywordTreeMatchCount;
             ++Index)
        {
            keyword_tree_match *Match = KeywordTreeMatches + Index;
            LookForMatchingQueries(&Application.Queries, &PossibleAnswers, 1, Match->Keyword, Match->Distance);
        }

        KeywordTreeMatchCount = 0;
        FindMatchesInKeywordTree(&Application.EditTree, DocumentWord, &KeywordTreeMatchCount, KeywordTreeMatches);

        for (u64 Index = 0;
             Index < KeywordTreeMatchCount;
             ++Index)
        {
            keyword_tree_match *Match = KeywordTreeMatches + Index;
            LookForMatchingQueries(&Application.Queries, &PossibleAnswers, 2, Match->Keyword, Match->Distance);
        }
    }

    DestroyKeywordTable(&DocumentWords);

    {
        u64 QueryCount = CountAnsweredQueries(&PossibleAnswers);
        u32 *Queries = 0;

        if (QueryCount)
        {
            Queries = (u32 *)calloc(1, QueryCount * sizeof(u32));
            CompileAnsweredQueries(&PossibleAnswers, Queries);

            qsort(Queries, QueryCount, sizeof(u32), CompareQueryIDs);
        }

        PushDocumentAnswer(ID, QueryCount, Queries);
    }

    QueryTree_Destroy(&PossibleAnswers);

    return EC_SUCCESS;
}

ErrorCode
GetNextAvailRes(DocID *DocumentID, u32 *QueryCount, QueryID **Queries)
{
    document_answer Answer = PopDocumentAnswer();

    *DocumentID = Answer.DocumentID;
    *QueryCount = Answer.QueryCount;
    *Queries    = Answer.Queries;

    return EC_SUCCESS;
}
