#ifndef ISE_LINUX_H
#define ISE_LINUX_H

#include <unistd.h>
#include <pthread.h>

//
// NOTE(philip): Linux Constants
//

#define LINUX_MUTEX_STORAGE_SIZE                  64
#define LINUX_CONDITION_VARIABLE_STORAGE_SIZE     64

//
// NOTE(philip): Linux Structures
//

struct linux_mutex
{
    b32 Used;
    pthread_mutex_t Handle;
};

struct linux_condition_variable
{
    b32 Used;
    pthread_cond_t Handle;
};

struct linux_state
{
    linux_mutex MutexStorage[LINUX_MUTEX_STORAGE_SIZE];
    u64 MutexCount;

    linux_condition_variable ConditionVariableStorage[LINUX_CONDITION_VARIABLE_STORAGE_SIZE];
    u64 ConditionVariableCount;
};

global linux_state LinuxState = { };

#endif
