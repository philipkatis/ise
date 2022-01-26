//
// NOTE(philip): Document Answer Queue
//

function void
InitializeDocumentAnswerQueue(document_answer_queue *Queue)
{
    Queue->ReadIndex = 0;
    Queue->WriteIndex = 0;
    Queue->Count = 0;

    Queue->Mutex = Platform.CreateMutex();
    Queue->HasSpace = Platform.CreateConditionVariable();
    Queue->HasData = Platform.CreateConditionVariable();
}

function void
PushToDocumentAnswerQueue(document_answer_queue *Queue, u32 ID, u64 QueryCount, u32 *Queries)
{
    Platform.LockMutex(Queue->Mutex);

    while (Queue->Count == DOCUMENT_ANSWER_BUFFER_SIZE)
    {
        Platform.BlockOnConditionVariable(Queue->HasSpace, Queue->Mutex);
    }

    document_answer *Answer = Queue->Data + Queue->WriteIndex;
    Answer->ID = ID;
    Answer->QueryCount = QueryCount;
    Answer->Queries = Queries;

    Queue->WriteIndex = (((Queue->WriteIndex + 1) < DOCUMENT_ANSWER_BUFFER_SIZE) ? (Queue->WriteIndex + 1) : 0);
    ++Queue->Count;

    Platform.SignalConditionVariable(Queue->HasData);
    Platform.UnlockMutex(Queue->Mutex);
}

function document_answer
PopFromDocumentAnswerQueue(document_answer_queue *Queue)
{
    Platform.LockMutex(Queue->Mutex);

    while (Queue->Count == 0)
    {
        Platform.BlockOnConditionVariable(Queue->HasData, Queue->Mutex);
    }

    document_answer Answer = Queue->Data[Queue->ReadIndex];

    Queue->ReadIndex = (((Queue->ReadIndex + 1) < DOCUMENT_ANSWER_BUFFER_SIZE) ? (Queue->ReadIndex + 1) : 0);
    --Queue->Count;

    Platform.SignalConditionVariable(Queue->HasSpace);
    Platform.UnlockMutex(Queue->Mutex);

    return Answer;
}

function void
DestroyDocumentAnswerQueue(document_answer_queue *Queue)
{
    Platform.DestroyMutex(Queue->Mutex);
    Platform.DestroyConditionVariable(Queue->HasSpace);
    Platform.DestroyConditionVariable(Queue->HasData);

    Queue->ReadIndex = 0;
    Queue->WriteIndex = 0;
    Queue->Count = 0;

    Queue->Mutex = 0;
    Queue->HasSpace = 0;
    Queue->HasData = 0;
}

//
// NOTE(philip): Work Queue
//

function void
InitializeWorkQueue(work_queue *Queue)
{
    Queue->ReadIndex = 0;
    Queue->WriteIndex = 0;
    Queue->Count = 0;

    Queue->Mutex = Platform.CreateMutex();
    Queue->HasSpace = Platform.CreateConditionVariable();
    Queue->HasData = Platform.CreateConditionVariable();
}

function void
PushToWorkQueue(work_queue *Queue, work_type Type, u8 *Data)
{
    Platform.LockMutex(Queue->Mutex);

    while (Queue->Count == WORK_QUEUE_BUFFER_SIZE)
    {
        Platform.BlockOnConditionVariable(Queue->HasSpace, Queue->Mutex);
    }

    work *Work = Queue->Data + Queue->WriteIndex;
    Work->Type = Type;
    Work->Data = Data;

    Queue->WriteIndex = (((Queue->WriteIndex + 1) < WORK_QUEUE_BUFFER_SIZE) ? (Queue->WriteIndex + 1) : 0);
    ++Queue->Count;

    Platform.SignalConditionVariable(Queue->HasData);
    Platform.UnlockMutex(Queue->Mutex);
}

function work
PopFromWorkQueue(work_queue *Queue)
{
    Platform.LockMutex(Queue->Mutex);

    while (Queue->Count == 0)
    {
        Platform.BlockOnConditionVariable(Queue->HasData, Queue->Mutex);
    }

    work Work = Queue->Data[Queue->ReadIndex];

    Queue->ReadIndex = (((Queue->ReadIndex + 1) < WORK_QUEUE_BUFFER_SIZE) ? (Queue->ReadIndex + 1) : 0);
    --Queue->Count;

    Platform.SignalConditionVariable(Queue->HasSpace);
    Platform.UnlockMutex(Queue->Mutex);

    return Work;
}

function void
DestroyWorkQueue(work_queue *Queue)
{
    Platform.DestroyMutex(Queue->Mutex);
    Platform.DestroyConditionVariable(Queue->HasSpace);
    Platform.DestroyConditionVariable(Queue->HasData);

    Queue->ReadIndex = 0;
    Queue->WriteIndex = 0;
    Queue->Count = 0;

    Queue->Mutex = 0;
    Queue->HasSpace = 0;
    Queue->HasData = 0;
}

function void
RegisterQuery(u32 ID, query_type Type, u64 DistanceThreshold, char *String)
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

    query *Query = InsertIntoQueryTree(&Queries, ID, WordCount, Type, DistanceThreshold);

    switch (Type)
    {
        case QueryType_Exact:
        {
            for (u64 Index = 0;
                 Index < WordCount;
                 ++Index)
            {
                keyword *Keyword = InsertIntoKeywordTable(&Keywords, Words[Index]);
                ++Keyword->InstanceCount;

                Query->Keywords[Index] = Keyword;
                InsertIntoQueryList(&Keyword->Queries, Query);
            }
        } break;

        case QueryType_Hamming:
        {
            for (u64 Index = 0;
                 Index < WordCount;
                 ++Index)
            {
                keyword *Keyword = InsertIntoKeywordTable(&Keywords, Words[Index]);

                if (!Keyword->HammingInstanceCount)
                {
                    keyword_tree *Tree = HammingTrees + GetHammingTreeIndex(Keyword);
                    InsertIntoKeywordTree(Tree, Keyword);
                }

                ++Keyword->InstanceCount;
                ++Keyword->HammingInstanceCount;

                Query->Keywords[Index] = Keyword;
                InsertIntoQueryList(&Keyword->Queries, Query);
            }
        } break;

        case QueryType_Edit:
        {
            for (u64 Index = 0;
                 Index < WordCount;
                 ++Index)
            {
                keyword *Keyword = InsertIntoKeywordTable(&Keywords, Words[Index]);

                if (!Keyword->EditInstanceCount)
                {
                    InsertIntoKeywordTree(&EditTree, Keyword);
                }

                ++Keyword->InstanceCount;
                ++Keyword->EditInstanceCount;

                Query->Keywords[Index] = Keyword;
                InsertIntoQueryList(&Keyword->Queries, Query);
            }
        } break;
    }
}

function void
UnregisterQuery(u32 ID)
{
    query *Query = FindQueryInTree(&Queries, ID);

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

                RemoveFromQueryList(&Keyword->Queries, Query);

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

                RemoveFromQueryList(&Keyword->Queries, Query);

                --Keyword->HammingInstanceCount;
                --Keyword->InstanceCount;

                if (!Keyword->HammingInstanceCount)
                {
                    keyword_tree *Tree = HammingTrees + GetHammingTreeIndex(Keyword);
                    RemoveFromKeywordTree(Tree, &KeywordTreeNodeStack, Keyword);
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

                RemoveFromQueryList(&Keyword->Queries, Query);

                --Keyword->EditInstanceCount;
                --Keyword->InstanceCount;

                if (!Keyword->EditInstanceCount)
                {
                    RemoveFromKeywordTree(&EditTree, &KeywordTreeNodeStack, Keyword);
                }

                if (!Keyword->InstanceCount)
                {
                    // TODO(philip): Remove from hash table.
                }
            }
        } break;
    }

    RemoveFromQueryTree(&Queries, ID);
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
LookForMatchingQueries(query_tree *PossibleAnswers, keyword *Keyword, u32 Type, u32 Distance)
{
    for (query_list_node *Node = Keyword->Queries.Head;
         Node;
         Node = Node->Next)
    {
        query *Query = Node->Data;

        if ((GetType(Query) == Type) && (GetDistanceThreshold(Query) >= Distance))
        {
            u64 KeywordCount = GetKeywordCount(Query);
            query *PossibleAnswer = InsertIntoQueryTree(PossibleAnswers, Query->ID, KeywordCount, Type, 0);

            for (u64 Index = 0;
                 Index < KeywordCount;
                 ++Index)
            {
                if (Query->Keywords[Index] == Keyword)
                {
                    // NOTE(philip): Don't break here. Duplicate words.
                    PossibleAnswer->HasKeywords[Index] = true;
                }
            }
        }
    }
}

function s32
CompareQueryIDs(const void *A, const void *B)
{
    return (*(s32 *)A - *(s32 *)B);
}

function void
GenerateDocumentAnswers(keyword_tree_node_stack *NodeStack, keyword_tree_match_stack *MatchStack,
                        u32 ID, char *String)
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

        keyword *Keyword = FindKeywordInTable(&Keywords, Word);
        if (Keyword)
        {
            LookForMatchingQueries(&Matches, Keyword, 0, 0);
        }

        keyword_tree *Tree = HammingTrees + GetHammingTreeIndex(Word);
        FindMatchesInKeywordTree(Tree, NodeStack, MatchStack, Word);

        for (keyword_tree_match *Match = PopFromKeywordTreeMatchStack(MatchStack);
             Match;
             Match = PopFromKeywordTreeMatchStack(MatchStack))
        {
            LookForMatchingQueries(&Matches, Match->Keyword, 1, Match->Distance);
        }

        FindMatchesInKeywordTree(&EditTree, NodeStack, MatchStack, Word);

        for (keyword_tree_match *Match = PopFromKeywordTreeMatchStack(MatchStack);
             Match;
             Match = PopFromKeywordTreeMatchStack(MatchStack))
        {
            LookForMatchingQueries(&Matches, Match->Keyword, 2, Match->Distance);
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

    PushToDocumentAnswerQueue(&DocumentAnswers, ID, MachedQueryCount, MachedQueries);
    DestroyQueryTree(&Matches);
}

function void
FetchDocumentAnswer(u32 *ID, u32 *QueryCount, u32 **Queries)
{
    document_answer Answer = PopFromDocumentAnswerQueue(&DocumentAnswers);

    *ID = Answer.ID;
    *QueryCount = Answer.QueryCount;
    *Queries = Answer.Queries;
}

function void *
WorkerThreadEntryPoint(void *Arguments)
{
    keyword_tree_node_stack NodeStack;
    keyword_tree_match_stack MatchStack;

    InitializeKeywordTreeNodeStack(&NodeStack);
    InitializeKeywordTreeMatchStack(&MatchStack);

    b32 Running = true;
    while (Running)
    {
        work Work = PopFromWorkQueue(&WorkQueue);

        switch (Work.Type)
        {
            case WorkType_GenerateDocumentAnswers:
            {
                u32 ID = *(u32 *)Work.Data;
                char *String = (char *)(Work.Data + sizeof(u32));

                GenerateDocumentAnswers(&NodeStack, &MatchStack, ID, String);
            } break;

            case WorkType_Exit:
            {
                Running = false;
            } break;
        }

        free(Work.Data);
    }

    DestroyKeywordTreeMatchStack(&MatchStack);
    DestroyKeywordTreeNodeStack(&NodeStack);

    return 0;
}
