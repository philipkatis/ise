//
// NOTE(philip): Memory
//

function void *
PlatformAllocateMemory(u64 Size)
{
    void *Memory = mmap(0, Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0);
    return Memory;
}

function void
PlatformFreeMemory(void *Memory, u64 Size)
{
    munmap(Memory, Size);
}

function memory_block *
AllocateMemoryBlock(u64 Size, memory_block *Previous = 0)
{
    u64 TotalSize = (Size + sizeof(memory_block));
    void *Memory = PlatformAllocateMemory(TotalSize);

    memory_block *Block = (memory_block *)Memory;
    Block->Previous = Previous;
    Block->Base = (u8 *)Memory + sizeof(memory_block);
    Block->Size = Size;
    Block->Used = 0;

    return Block;
}

function memory_block *
FreeMemoryBlock(memory_block *Block)
{
    memory_block *Previous = Block->Previous;

    void *Memory = Block->Base - sizeof(memory_block);
    u64 Size = Block->Size + sizeof(memory_block);

    PlatformFreeMemory(Memory, Size);

    return Previous;
}

function void
InitializeMemoryArena(memory_arena *Arena, u64 BlockSize)
{
    Arena->BlockSize = BlockSize;
    Arena->CurrentBlock = 0;
}

function void *
PushSize(memory_arena *Arena, u64 Size)
{
    if (!Arena->CurrentBlock || ((Arena->CurrentBlock->Used + Size) > Arena->CurrentBlock->Size))
    {
        Assert(Size <= Arena->BlockSize);

        memory_block *Block = AllocateMemoryBlock(Arena->BlockSize, Arena->CurrentBlock);
        Arena->CurrentBlock = Block;
    }

    void *Memory = Arena->CurrentBlock->Base + Arena->CurrentBlock->Used;
    Arena->CurrentBlock->Used += Size;

    return Memory;
}

#define PushStruct(Arena, Type) (Type *)PushSize(Arena, sizeof(Type))
#define PushArray(Arena, Type, Count) (Type *)PushSize(Arena, (Count) * sizeof(Type));

function void
DestroyMemoryArena(memory_arena *Arena)
{
    memory_block *Block = Arena->CurrentBlock;
    while (Block)
    {
        Block = FreeMemoryBlock(Block);
    }

    Arena->BlockSize = 0;
    Arena->CurrentBlock = 0;
}

//
// NOTE(philip): String matching functions.
//

function u8
IsExactMatch(char *A, u8 LengthA, char *B, u8 LengthB)
{
    u8 Result = 1;

    if (LengthA == LengthB)
    {
        for (u8 Index = 0;
             Index < LengthA;
             ++Index)
        {
            if (A[Index] != B[Index])
            {
                Result = 0;
                break;
            }
        }
    }
    else
    {
        Result = 0;
    }

    return Result;
}

function u8
HammingDistance(char *A, u8 LengthA, char *B, u8 LengthB)
{
    u8 Result = 0;
    Assert(LengthA == LengthB);

    for (u8 Index = 0;
         Index < LengthA;
         ++Index)
    {
        if (A[Index] != B[Index])
        {
            ++Result;
        }
    }

    return Result;
}

function u8
EditDistance(char *A, u8 LengthA, char *B, u8 LengthB)
{
    // NOTE(philip): We know that each word will be stored in a 32 byte long buffer.
    u8 Cache[32] =
    {
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    };

    for (u8 IndexA = 0;
         IndexA < LengthA;
         ++IndexA)
    {
        u8 PreviousCost = IndexA + 1;

        for (u8 IndexB = 0;
             IndexB < LengthB;
             ++IndexB)
        {
            u8 Cost = Cache[IndexB];
            if (A[IndexA] != B[IndexB])
            {
                Cost = Min(Min(PreviousCost, Cost), Cache[IndexB + 1]) + 1;
            }

            Cache[IndexB] = PreviousCost;
            PreviousCost = Cost;
        }

        Cache[LengthB] = PreviousCost;
    }

    return Cache[LengthB];
}

//
// NOTE(philip): Utility functions.
//

function void
PrintTabs(u64 Count)
{
    for (u64 Index = 0;
         Index < Count;
         ++Index)
    {
        printf("    ");
    }
}
