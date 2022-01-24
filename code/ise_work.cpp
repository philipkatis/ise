#define GetHammingTreeIndex(Keyword) ((Keyword)->Length - MIN_KEYWORD_LENGTH)

function void
RegisterQuery(query_tree *Queries, keyword_table *Keywords, keyword_tree *HammingTrees, keyword_tree *EditTree,
              u32 ID, query_type Type, u64 DistanceThreshold, char *String)
{
    char *Words[MAX_KEYWORD_COUNT_PER_QUERY];
    u64 WordCount = 0;

    Words[WordCount++] = String;

    for (char *Character = String;
         *Character;
         ++Character)
    {
        if (*Character == ' ')
        {
            *Character = 0;
            Words[WordCount++] = Character + 1;
        }
    }

    query *Query = InsertIntoQueryTree(Queries, ID, WordCount, Type, DistanceThreshold);

    switch (Type)
    {
        case QueryType_Exact:
        {
            for (u64 Index = 0;
                 Index < WordCount;
                 ++Index)
            {
                keyword *Keyword = InsertIntoKeywordTable(Keywords, Words[Index]);
                ++Keyword->InstanceCount;

                Query->Keywords[Index] = Keyword;
            }
        } break;

        case QueryType_Hamming:
        {
            for (u64 Index = 0;
                 Index < WordCount;
                 ++Index)
            {
                keyword *Keyword = InsertIntoKeywordTable(Keywords, Words[Index]);

                if (!Keyword->HammingInstanceCount)
                {
                    keyword_tree *Tree = HammingTrees + GetHammingTreeIndex(Keyword);
                    InsertIntoKeywordTree(Tree, Keyword);
                }

                ++Keyword->InstanceCount;
                ++Keyword->HammingInstanceCount;

                Query->Keywords[Index] = Keyword;
            }
        } break;

        case QueryType_Edit:
        {
            for (u64 Index = 0;
                 Index < WordCount;
                 ++Index)
            {
                keyword *Keyword = InsertIntoKeywordTable(Keywords, Words[Index]);

                if (!Keyword->EditInstanceCount)
                {
                    InsertIntoKeywordTree(EditTree, Keyword);
                }

                ++Keyword->InstanceCount;
                ++Keyword->EditInstanceCount;

                Query->Keywords[Index] = Keyword;
            }
        } break;
    }
}

function void
UnregisterQuery(query_tree *Queries, keyword_tree *HammingTrees, keyword_tree *EditTree, u32 ID)
{
    query *Query = FindQueryInTree(Queries, ID);

    query_type Type = GetType(Query);
    u64 KeywordCount = GetKeywordCount(Query);

    switch (Type)
    {
        case QueryType_Exact:
        {
            for (u64 Index = 0;
                 Index < KeywordCount;
                 ++Index)
            {
                keyword *Keyword = Query->Keywords[Index];

                --Keyword->InstanceCount;

                if (!Keyword->InstanceCount)
                {
                    // TODO(philip): Remove from hash table.
                }
            }
        } break;

        case QueryType_Hamming:
        {
            for (u64 Index = 0;
                 Index < KeywordCount;
                 ++Index)
            {
                keyword *Keyword = Query->Keywords[Index];

                --Keyword->HammingInstanceCount;
                --Keyword->InstanceCount;

                if (!Keyword->HammingInstanceCount)
                {
                    keyword_tree *Tree = HammingTrees + GetHammingTreeIndex(Keyword);
                    RemoveFromKeywordTree(Tree, Keyword);
                }

                if (!Keyword->InstanceCount)
                {
                    // TODO(philip): Remove from hash table.
                }
            }
        } break;

        case QueryType_Edit:
        {
            for (u64 Index = 0;
                 Index < KeywordCount;
                 ++Index)
            {
                keyword *Keyword = Query->Keywords[Index];

                --Keyword->EditInstanceCount;
                --Keyword->InstanceCount;

                if (!Keyword->EditInstanceCount)
                {
                    RemoveFromKeywordTree(EditTree, Keyword);
                }

                if (!Keyword->InstanceCount)
                {
                    // TODO(philip): Remove from hash table.
                }
            }
        } break;
    }

    RemoveFromQueryTree(Queries, ID);
}

function b32
IsAnswer(query *Query)
{
    b32 Result = true;
    u32 KeywordCount = GetKeywordCount(Query);

    for (u32 KeywordIndex = 0;
         KeywordIndex < KeywordCount;
         ++KeywordIndex)
    {
        if (!Query->HasKeywords[KeywordIndex])
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

    if ((GetType(Query) == Type) && (GetDistanceThreshold(Query) >= Distance))
    {
        u64 KeywordCount = GetKeywordCount(Query);
        query *PossibleAnswer = 0;

        for (u64 Index = 0;
             Index < KeywordCount;
             ++Index)
        {
            if (Query->Keywords[Index] == Keyword)
            {
                if (!PossibleAnswer)
                {
                    PossibleAnswer = InsertIntoQueryTree(PossibleAnswers, Query->ID, KeywordCount, Type, 0);
                }

                PossibleAnswer->HasKeywords[Index] = true;

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

struct document_answer
{
    u32 DocumentID;
    u32 *Queries;
    u64 QueryCount;
};

#define DOCUMENT_ANSWER_STORAGE_SIZE 1024

global document_answer DocumentAnswers[DOCUMENT_ANSWER_STORAGE_SIZE];
global u64 DocumentAnswerCount = 0;

#define KEYWORD_TREE_MATCH_STORAGE_SIZE 1024

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

function void
GenerateDocumentAnswers(query_tree *Queries, keyword_table *Keywords, keyword_tree *HammingTrees,
                       keyword_tree *EditTree, u32 ID, char *String)
{
    u64 WordCount = 1;

    for (char *Character = String;
         *Character;
         ++Character)
    {
        if (*Character == ' ')
        {
            *Character = 0;
            ++WordCount;
        }
    }

    keyword_table Words;
    InitializeKeywordTable(&Words, WordCount);

    char *Word = String;
    for (u64 Index = 0;
         Index < WordCount;
         ++Index)
    {
        InsertIntoKeywordTable(&Words, Word);

        while (*Word)
        {
            ++Word;
        }

        if (Index < (WordCount - 1))
        {
            ++Word;
        }
    }

    query_tree Matches;
    InitializeQueryTree(&Matches);

    for (keyword_table_iterator Iterator = IterateKeywordTable(&Words);
         IsValid(&Iterator);
         Advance(&Iterator))
    {
        keyword *Word = GetValue(&Iterator);

        keyword *Keyword = FindKeywordInTable(Keywords, Word);
        if (Keyword)
        {
            LookForMatchingQueries(Queries, &Matches, 0, Keyword, 0);
        }

        keyword_tree *Tree = HammingTrees + GetHammingTreeIndex(Word);
        FindMatchesInKeywordTree(Tree, Word, &KeywordTreeMatchCount, KEYWORD_TREE_MATCH_STORAGE_SIZE,
                                 KeywordTreeMatches);

        for (u64 Index = 0;
             Index < KeywordTreeMatchCount;
             ++Index)
        {
            keyword_tree_match *Match = KeywordTreeMatches + Index;
            LookForMatchingQueries(Queries, &Matches, 1, Match->Keyword, Match->Distance);
        }

        FindMatchesInKeywordTree(EditTree, Word, &KeywordTreeMatchCount, KEYWORD_TREE_MATCH_STORAGE_SIZE,
                                 KeywordTreeMatches);

        for (u64 Index = 0;
             Index < KeywordTreeMatchCount;
             ++Index)
        {
            keyword_tree_match *Match = KeywordTreeMatches + Index;
            LookForMatchingQueries(Queries, &Matches, 2, Match->Keyword, Match->Distance);
        }
    }


    DestroyKeywordTable(&Words);

    u64 MachedQueryCount = CountAnsweredQueries(&Matches);
    u32 *MachedQueries = 0;

    if (MachedQueryCount)
    {
        MachedQueries = (u32 *)calloc(1, MachedQueryCount * sizeof(u32));
        CompileAnsweredQueries(&Matches, MachedQueries);

        qsort(MachedQueries, MachedQueryCount, sizeof(u32), CompareQueryIDs);
    }

    PushDocumentAnswer(ID, MachedQueryCount, MachedQueries);
    DestroyQueryTree(&Matches);
}
