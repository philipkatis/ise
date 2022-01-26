#ifndef ISE_WORK_H
#define ISE_WORK_H

//
// NOTE(philip): Document Answer Queue
//

struct document_answer
{
    u32 ID;
    u64 QueryCount;
    u32 *Queries;
};

#define DOCUMENT_ANSWER_BUFFER_SIZE 1024

struct document_answer_queue
{
    document_answer Data[DOCUMENT_ANSWER_BUFFER_SIZE];

    u64 ReadIndex;
    u64 WriteIndex;
    u64 Count;

    platform_mutex Mutex;
    platform_condition_variable HasSpace;
    platform_condition_variable HasData;
};

//
// NOTE(philip): Work Queue
//

typedef u32 work_type;
enum
{
    WorkType_None                    = 0,
    WorkType_GenerateDocumentAnswers = 1,
    WorkType_Exit                    = 2
};

struct work
{
    work_type Type;
    u8 *Data;
};

#define WORK_QUEUE_BUFFER_SIZE 1024

struct work_queue
{
    work Data[WORK_QUEUE_BUFFER_SIZE];

    u64 ReadIndex;
    u64 WriteIndex;
    u64 Count;

    platform_mutex Mutex;
    platform_condition_variable HasSpace;
    platform_condition_variable HasData;
};

//
// NOTE(philip): Global Context
//

#define THREAD_COUNT 4
#define HAMMING_TREE_COUNT (MAX_KEYWORD_LENGTH - MIN_KEYWORD_LENGTH)
#define GetHammingTreeIndex(Keyword) ((Keyword)->Length - MIN_KEYWORD_LENGTH)

global platform_thread Threads[THREAD_COUNT];
global work_queue WorkQueue;

global query_tree Queries;
global keyword_table Keywords;

global keyword_tree_node_stack KeywordTreeNodeStack;
global keyword_tree_match_stack KeywordTreeMatchStack;
global keyword_tree HammingTrees[HAMMING_TREE_COUNT];
global keyword_tree EditTree;

global document_answer_queue DocumentAnswers;

#endif
