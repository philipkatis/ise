//
// NOTE(philip): Thread
//

function platform_thread
LinuxCreateThread(platform_thread_entry_point *EntryPoint, void *Arguments)
{
    pthread_t Thread;
    pthread_create(&Thread, 0, EntryPoint, Arguments);

    return (platform_thread)Thread;
}

function void
LinuxWaitForThread(platform_thread Thread)
{
    pthread_join((pthread_t)Thread, 0);
}

function void
LinuxDestroyThread(platform_thread Thread)
{
}

function platform_thread_id
LinuxGetThreadID(void)
{
    platform_thread_id ThreadID = gettid();
    return ThreadID;
}

//
// NOTE(philip): Mutex
//

function platform_mutex
LinuxCreateMutex(void)
{
    linux_mutex *Mutex = 0;

    Assert((LinuxState.MutexCount + 1) <= LINUX_MUTEX_STORAGE_SIZE);

    for (u64 Index = 0;
         Index < LINUX_MUTEX_STORAGE_SIZE;
         ++Index)
    {
        Mutex = LinuxState.MutexStorage + Index;

        if (!Mutex->Used)
        {
            Mutex->Used = true;
            pthread_mutex_init(&Mutex->Handle, 0);

            ++LinuxState.MutexCount;
            break;
        }
    }

    return Mutex;
}

function void
LinuxLockMutex(platform_mutex Mutex)
{
    linux_mutex *LinuxMutex = (linux_mutex *)Mutex;
    pthread_mutex_lock(&LinuxMutex->Handle);
}

function void
LinuxUnlockMutex(platform_mutex Mutex)
{
    linux_mutex *LinuxMutex = (linux_mutex *)Mutex;
    pthread_mutex_unlock(&LinuxMutex->Handle);
}

function void
LinuxDestroyMutex(platform_mutex Mutex)
{
    linux_mutex *LinuxMutex = (linux_mutex *)Mutex;
    pthread_mutex_destroy(&LinuxMutex->Handle);

    LinuxMutex->Used = false;
    --LinuxState.MutexCount;
}

//
// NOTE(philip): Condition Variable
//

function platform_condition_variable
LinuxCreateConditionVariable(void)
{
    linux_condition_variable *ConditionVariable = 0;

    Assert((LinuxState.ConditionVariableCount + 1) <= LINUX_CONDITION_VARIABLE_STORAGE_SIZE);

    for (u64 Index = 0;
         Index < LINUX_CONDITION_VARIABLE_STORAGE_SIZE;
         ++Index)
    {
        ConditionVariable = LinuxState.ConditionVariableStorage + Index;

        if (!ConditionVariable->Used)
        {
            ConditionVariable->Used = true;
            pthread_cond_init(&ConditionVariable->Handle, 0);

            ++LinuxState.ConditionVariableCount;
            break;
        }
    }

    return ConditionVariable;
}

function void
LinuxBlockOnConditionVariable(platform_condition_variable ConditionVariable, platform_mutex Mutex)
{
    linux_condition_variable *LinuxConditionVariable = (linux_condition_variable *)ConditionVariable;
    linux_mutex *LinuxMutex = (linux_mutex *)Mutex;

    pthread_cond_wait(&LinuxConditionVariable->Handle, &LinuxMutex->Handle);
}

function void
LinuxSignalConditionVariable(platform_condition_variable ConditionVariable)
{
    linux_condition_variable *LinuxConditionVariable = (linux_condition_variable *)ConditionVariable;
    pthread_cond_signal(&LinuxConditionVariable->Handle);
}

function void
LinuxDestroyConditionVariable(platform_condition_variable ConditionVariable)
{
    linux_condition_variable *LinuxConditionVariable = (linux_condition_variable *)ConditionVariable;
    pthread_cond_destroy(&LinuxConditionVariable->Handle);

    LinuxConditionVariable->Used = false;
    --LinuxState.ConditionVariableCount;
}

//
// NOTE(philip): Platform API
//

function void
InitializePlatform(void)
{
    // NOTE(philip): Thread
    Platform.CreateThread = LinuxCreateThread;
    Platform.WaitForThread = LinuxWaitForThread;
    Platform.DestroyThread = LinuxDestroyThread;
    Platform.GetThreadID = LinuxGetThreadID;

    // NOTE(philip): Mutex
    Platform.CreateMutex = LinuxCreateMutex;
    Platform.LockMutex = LinuxLockMutex;
    Platform.UnlockMutex = LinuxUnlockMutex;
    Platform.DestroyMutex = LinuxDestroyMutex;

    // NOTE(philip): Condition Variable
    Platform.CreateConditionVariable = LinuxCreateConditionVariable;
    Platform.BlockOnConditionVariable = LinuxBlockOnConditionVariable;
    Platform.SignalConditionVariable = LinuxSignalConditionVariable;
    Platform.DestroyConditionVariable = LinuxDestroyConditionVariable;
}
