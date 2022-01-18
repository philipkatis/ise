//
// NOTE(philip): Work Queue
//

function void
WorkQueue_Initialize(work_queue *Queue)
{
    Assert(Queue);

    pthread_mutex_init(&Queue->Mutex, 0);
    pthread_cond_init(&Queue->HasSpace, 0);
    pthread_cond_init(&Queue->IsEmpty, 0);
}

function void
WorkQueue_Push(work_queue *Queue, work_type Type, u8 *Data)
{
    pthread_mutex_lock(&Queue->Mutex);

    // NOTE(philip): If the queue is full, we block until there is space.
    while ((Queue->PendingWork + 1) == ISE_WORK_QUEUE_SIZE)
    {
        pthread_cond_wait(&Queue->HasSpace, &Queue->Mutex);
    }

    // NOTE(philip): Copy the work data into the queue.
    work *Work = Queue->Data + Queue->WriteIndex;
    Work->Type = Type;
    Work->Data = Data;

    // NOTE(philip): Update the queue indices.
    Queue->WriteIndex = (((Queue->WriteIndex + 1) < ISE_WORK_QUEUE_SIZE) ? (Queue->WriteIndex + 1) : 0);
    ++Queue->PendingWork;

    // NOTE(philip): There is definitelly work in the queue, so we signal all the consumer threads.
    pthread_cond_broadcast(&Queue->IsEmpty);

    pthread_mutex_unlock(&Queue->Mutex);
}

function work
WorkQueue_Pop(work_queue *Queue)
{
    pthread_mutex_lock(&Queue->Mutex);

    // NOTE(philip): If the queue is empty, we block until there is work.
    while (Queue->PendingWork == 0)
    {
        // TODO(philip): Rename mutex.
        pthread_cond_wait(&Queue->IsEmpty, &Queue->Mutex);
    }

    // NOTE(philip): Make a copy of the work.
    work Work = Queue->Data[Queue->ReadIndex];

    // NOTE(philip): Update the queue indices.
    Queue->ReadIndex = (((Queue->ReadIndex + 1) < ISE_WORK_QUEUE_SIZE) ? (Queue->ReadIndex + 1) : 0);
    --Queue->PendingWork;

    // NOTE(philip): There is definitelly space in the queue, so we signal the producer.
    pthread_cond_signal(&Queue->HasSpace);

    pthread_mutex_unlock(&Queue->Mutex);

    return Work;
}

function void
WorkQueue_Destroy(work_queue *Queue)
{
    Assert(Queue);

    pthread_mutex_destroy(&Queue->Mutex);
    pthread_cond_destroy(&Queue->HasSpace);
    pthread_cond_destroy(&Queue->IsEmpty);
}

//
// NOTE(philip): Thread Pool
//

#define HAMMING_TREE_COUNT (MAX_KEYWORD_LENGTH - MIN_KEYWORD_LENGTH)
#define GetHammingTreeIndex(Length) (Length - MIN_KEYWORD_LENGTH)

function b32
IsAnswer(query *Query)
{
    b32 Result = true;
    u32 KeywordCount = GetQueryKeywordCount(Query);

    for (u32 KeywordIndex = 0;
         KeywordIndex < KeywordCount;
         ++KeywordIndex)
    {
        if (!Query->HasKeyword[KeywordIndex])
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
                            PossibleAnswer->HasKeyword[KeywordIndex] = true;
                            break;
                        }
                    }
                }
            }
        }
    }
}

// TODO(philip): Move this.
function void *
ThreadFn(void *Arguments)
{
    thread_pool_memory *Memory = (thread_pool_memory *)Arguments;

    b32 Running = true;
    while (Running)
    {
        work Work = WorkQueue_Pop(&Memory->Queue);

        switch (Work.Type)
        {
            case WorkType_MatchDocument:
            {
                u32 ID = *(u32 *)Work.Data;
                char *Document = (char *)(Work.Data + sizeof(u32));

                u64 WordCount = 1;

                // NOTE(philip): Count the number of works in the document and tokenize them.
                {
                    for (char *Character = Document;
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

                // TODO(philip): This is currently a waste of memory. If it becomes a problem, switch to a more
                // specific data structure.
                keyword_table DocumentWords = KeywordTable_Create(WordCount);

                // NOTE(philip): Store the document words in the keyword table, effectively deduplicating them.
                {
                    char *Word = Document;

                    for (u64 Index = 0;
                         Index < WordCount;
                         ++Index)
                    {
                        KeywordTable_Insert(&DocumentWords, Word);

                        while (*Word)
                        {
                            ++Word;
                        }

                        if (Index < WordCount - 1)
                        {
                            ++Word;
                        }
                    }
                }

                // NOTE(philip): Free the work data.
                free(Work.Data);

                // TODO(philip): This is currently a waste of memory. If it becomes a problem, switch to a more
                // specific data structure.
                query_tree PossibleAnswers = { };

                for (keyword_iterator Iterator = IterateAllKeywords(&DocumentWords);
                     IsValid(&Iterator);
                     Advance(&Iterator))
                {
                    keyword *DocumentWord = GetValue(&Iterator);

                    // NOTE(philip): Exact matching. Look for the keyword in the keyword table.
                    {
                        keyword *FoundKeyword = KeywordTable_Find(Memory->Keywords, DocumentWord->Word);
                        LookForMatchingQueries(&PossibleAnswers, 0, 0, FoundKeyword);
                    }

                    // NOTE(philip): Approximate matching.
                    {
                        // NOTE(philip): Find the hamming tree we want to search.
                        bk_tree *HammingTree = Memory->HammingTrees + GetHammingTreeIndex(DocumentWord->Length);

                        // NOTE(philip): Go through all the thresholds.
                        for (u32 Threshold = 1;
                             Threshold <= MAX_DISTANCE_THRESHOLD;
                             ++Threshold)
                        {
                            // NOTE(philip): Hamming tree.
                            {
                                keyword_list FoundKeywords = BKTree_FindMatches(HammingTree, DocumentWord,
                                                                                Threshold);

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
                                keyword_list FoundKeywords = BKTree_FindMatches(Memory->EditTree, DocumentWord,
                                                                                Threshold);

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
                AnswerStack_Push(Memory->Answers, Answer);
            } break;

            case WorkType_Exit:
            {
                Running = false;
            } break;

            default:
            {
                Assert(false);
            } break;
        }
    }

    return 0;
}

function thread_pool
ThreadPool_Create(u32 ThreadCount, keyword_table *Keywords, bk_tree *HammingTrees, bk_tree *EditTree,
                  answer_stack *Answers)
{
    thread_pool Pool = { };
    Pool.ThreadCount = ThreadCount;
    Pool.Threads = (pthread_t *)calloc(1, Pool.ThreadCount * sizeof(pthread_t));
    Pool.Memory = (thread_pool_memory *)calloc(1, sizeof(thread_pool_memory));

    WorkQueue_Initialize(&Pool.Memory->Queue);
    Pool.Memory->Keywords = Keywords;
    Pool.Memory->HammingTrees = HammingTrees;
    Pool.Memory->EditTree = EditTree;
    Pool.Memory->Answers = Answers;

    for (u32 Index = 0;
         Index < Pool.ThreadCount;
         ++Index)
    {
        pthread_create(Pool.Threads + Index, 0, ThreadFn, Pool.Memory);
    }

    return Pool;
}

function void
ThreadPool_Destroy(thread_pool *Pool)
{
    Assert(Pool);

    // NOTE(philip): Submit one work unit for each thread, informing them to stop execution.
    for (u32 Index = 0;
         Index < Pool->ThreadCount;
         ++Index)
    {
        WorkQueue_Push(&Pool->Memory->Queue, WorkType_Exit, 0);
    }

    // NOTE(philip): Wait for each thread to exit.
    for (u32 Index = 0;
         Index < Pool->ThreadCount;
         ++Index)
    {
        pthread_join(Pool->Threads[Index], 0);
    }

    WorkQueue_Destroy(&Pool->Memory->Queue);

    free(Pool->Threads);
    free(Pool->Memory);

    Pool->ThreadCount = 0;
    Pool->Threads = 0;
    Pool->Memory = 0;
}
