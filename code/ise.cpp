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

s32 main()
{
    b32 Result = IsExactMatch("hello", "world");
    printf("%d\n", Result);

    Result = IsExactMatch("different", "length");
    printf("%d\n", Result);

    Result = IsExactMatch("exact", "exact");
    printf("%d\n", Result);

    return 0;
}
