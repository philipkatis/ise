#ifndef ISE_PLATFORM_H
#define ISE_PLATFORM_H

//
// NOTE(philip): Platform Types
//

typedef void *platform_handle;

typedef platform_handle platform_thread;
typedef platform_handle platform_mutex;
typedef platform_handle platform_condition_variable;

typedef void *platform_thread_entry_point(void *Arguments);

//
// NOTE(philip): Platform Functions
//

// NOTE(philip): Thread
typedef platform_thread platform_create_thread(platform_thread_entry_point *EntryPoint, void *Arguments);
typedef void platform_wait_for_thread(platform_thread Thread);
typedef void platform_destroy_thread(platform_thread Thread);

// NOTE(philip): Mutex
typedef platform_mutex platform_create_mutex(void);
typedef void platform_lock_mutex(platform_mutex Mutex);
typedef void platform_unlock_mutex(platform_mutex Mutex);
typedef void platform_destroy_mutex(platform_mutex Mutex);

// NOTE(philip): Condition Variable
typedef platform_condition_variable platform_create_condition_variable(void);
typedef void platform_block_on_condition_variable(platform_condition_variable ConditionVariable, platform_mutex Mutex);
typedef void platform_signal_condition_variable(platform_condition_variable ConditionVariable);
typedef void platform_destroy_condition_variable(platform_condition_variable ConditionVariable);

//
// NOTE(philip): Platform API
//

struct platform_api
{
    // NOTE(philip): Thread
    platform_create_thread *CreateThread;
    platform_wait_for_thread *WaitForThread;
    platform_destroy_thread *DestroyThread;

    // NOTE(philip): Mutex
    platform_create_mutex *CreateMutex;
    platform_lock_mutex *LockMutex;
    platform_unlock_mutex *UnlockMutex;
    platform_destroy_mutex *DestroyMutex;

    // NOTE(philip): Condition Variable
    platform_create_condition_variable *CreateConditionVariable;
    platform_block_on_condition_variable *BlockOnConditionVariable;
    platform_signal_condition_variable *SignalConditionVariable;
    platform_destroy_condition_variable *DestroyConditionVariable;
};

global platform_api Platform = { };

function void InitializePlatform(void);

#if ISE_LINUX
    #include "ise_linux.h"
#else
    #error Unsupported platform!
#endif

#endif
