//
// NOTE(philip): String matching functions.
//

function s32
IsExactMatch(char *A, u64 LengthA, char *B, u64 LengthB)
{
    s32 Result = 1;

    if (LengthA == LengthB)
    {
        for (u64 Index = 0;
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

function s32
CalculateHammingDistance(char *A, u64 LengthA, char *B, u64 LengthB)
{
    s32 Result = 0;
    Assert(LengthA == LengthB);

    for (u64 Index = 0;
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

#if 1
// TODO(philip): Change the types. Too overkill.
// NOTE(philip): We know that each word will be stored in a 32 byte long buffer.
function s32
CalculateEditDistance(char *A, u64 LengthA, char *B, u64 LengthB)
{
    s32 Cache[32] =
    {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    };

    for (u64 IndexA = 0;
         IndexA < LengthA;
         ++IndexA)
    {
        s32 PreviousCost = IndexA + 1;

        for (u64 IndexB = 0;
             IndexB < LengthB;
             ++IndexB)
        {
            s32 Cost = Cache[IndexB];
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
#else

#include <emmintrin.h>

function s32
CalculateEditDistance(char *A, u64 LengthA, char *B, u64 LengthB)
{
    u8 Cache[32] =
    {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    };

    for (u8 IndexA = 0;
         IndexA < 32;
         ++IndexA)
    {
        u8 PreviousCost = IndexA + 1;

        for (u8 IndexB = 0;
             IndexB < 32;
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

        Cache[32] = PreviousCost;
    }

    return Cache[32];
}
#endif

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
