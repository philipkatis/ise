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
