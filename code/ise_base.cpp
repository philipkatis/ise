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

function s32
CalculateEditDistance(char *A, u64 LengthA, char *B, u64 LengthB)
{
    #define CACHE_MATRIX_SIZE (MAX_KEYWORD_LENGTH + 1)
    s32 CacheMatrix[CACHE_MATRIX_SIZE * CACHE_MATRIX_SIZE];

    memset(CacheMatrix, 0, CACHE_MATRIX_SIZE * CACHE_MATRIX_SIZE * sizeof(s32));

    for (u64 Index = 1;
         Index <= LengthA;
         ++Index)
    {
        CacheMatrix[CACHE_MATRIX_SIZE * 0 + Index] = Index;
    }

    for (u64 Index = 1;
         Index <= LengthB;
         ++Index)
    {
        CacheMatrix[CACHE_MATRIX_SIZE * Index + 0] = Index;
    }

    for (u64 IndexB = 1;
         IndexB <= LengthB;
         ++IndexB)
    {
        for (u64 IndexA = 1;
             IndexA <= LengthA;
             ++IndexA)
        {
            s32 DeletionDistance  = CacheMatrix[CACHE_MATRIX_SIZE * IndexB + (IndexA - 1)] + 1;
            s32 InsertionDistance = CacheMatrix[CACHE_MATRIX_SIZE * (IndexB - 1) + IndexA] + 1;

            b32 RequiresSubstitution = (A[IndexA - 1] != B[IndexB - 1]);
            s32 SubstitutionDistance = CacheMatrix[CACHE_MATRIX_SIZE * (IndexB - 1) + (IndexA - 1)] +
                RequiresSubstitution;

            s32 *Distance = &CacheMatrix[CACHE_MATRIX_SIZE * IndexB + IndexA];
            *Distance = Min(Min(DeletionDistance, InsertionDistance), SubstitutionDistance);
        }
    }

    return CacheMatrix[CACHE_MATRIX_SIZE * LengthB + LengthA];
    #undef CACHE_MATRIX_SIZE
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
