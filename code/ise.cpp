#include <stdio.h>

#include "ise_base.h"

#include "ise_base.cpp"

function b32
IsExactMatch(char *A, char *B)
{
    b32 Result = true;

    u64 LengthA = StringLength(A);
    u64 LengthB = StringLength(B);

    if (LengthA == LengthB)
    {
        for (u64 Index = 0;
             Index < LengthA;
             ++Index)
        {
            if (A[Index] != B[Index])
            {
                Result = false;
                break;
            }
        }
    }
    else
    {
        Result = false;
    }

    return Result;
}

// TODO(philip): What if we give this strings of different lengths??? Is that ever a case??? If so, does this
// function handle it??? For now a special value of -1 is returned.
function s32
CalculateHammingDistance(char *A, char *B)
{
    s32 Result = 0;

    u64 LengthA = StringLength(A);
    u64 LengthB = StringLength(B);

    if (LengthA == LengthB)
    {
        for (u64 Index = 0;
             Index < LengthA;
             ++Index)
        {
            if (A[Index] != B[Index])
            {
                ++Result;
            }
        }
    }
    else
    {
        Result = -1;
    }

    return Result;
}

s32 main()
{
    b32 Result = CalculateHammingDistance("karolin", "kathrin");
    printf("%d\n", Result);

    Result = CalculateHammingDistance("karolin", "kerstin");
    printf("%d\n", Result);

    Result = CalculateHammingDistance("kathrin", "kerstin");
    printf("%d\n", Result);

    Result = CalculateHammingDistance("0000", "1111");
    printf("%d\n", Result);

    Result = CalculateHammingDistance("different", "length");
    printf("%d\n", Result);

    Result = CalculateHammingDistance("exact", "exact");
    printf("%d\n", Result);

    return 0;
}
