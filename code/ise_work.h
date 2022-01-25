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

#define HAMMING_TREE_COUNT (MAX_KEYWORD_LENGTH - MIN_KEYWORD_LENGTH)

struct context
{
    query_tree Queries;

    keyword_table Keywords;
    keyword_tree HammingTrees[HAMMING_TREE_COUNT];
    keyword_tree EditTree;

    document_answer_queue DocumentAnswers;
};

global context GlobalContext = { };

#endif
