#include "ise_match.h"

#include "ise.h"

#include <string.h>

s32
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

s32
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

#define EDIT_DISTANCE_CACHE_MATRIX_SIZE (MAX_KEYWORD_LENGTH + 1)
global s32 EditDistanceCacheMatrix[EDIT_DISTANCE_CACHE_MATRIX_SIZE * EDIT_DISTANCE_CACHE_MATRIX_SIZE];

s32
CalculateEditDistance(char *A, u64 LengthA, char *B, u64 LengthB)
{
    memset(EditDistanceCacheMatrix,
           0, EDIT_DISTANCE_CACHE_MATRIX_SIZE * EDIT_DISTANCE_CACHE_MATRIX_SIZE * sizeof(u32));

    for (u64 Index = 1;
         Index <= LengthA;
         ++Index)
    {
        EditDistanceCacheMatrix[EDIT_DISTANCE_CACHE_MATRIX_SIZE * 0 + Index] = Index;
    }

    for (u64 Index = 1;
         Index <= LengthB;
         ++Index)
    {
        EditDistanceCacheMatrix[EDIT_DISTANCE_CACHE_MATRIX_SIZE * Index + 0] = Index;
    }

    for (u64 IndexB = 1;
         IndexB <= LengthB;
         ++IndexB)
    {
        for (u64 IndexA = 1;
             IndexA <= LengthA;
             ++IndexA)
        {
            s32 DeletionDistance  = EditDistanceCacheMatrix[EDIT_DISTANCE_CACHE_MATRIX_SIZE * IndexB +
                                                            (IndexA - 1)] + 1;
            s32 InsertionDistance = EditDistanceCacheMatrix[EDIT_DISTANCE_CACHE_MATRIX_SIZE * (IndexB - 1) +
                                                            IndexA] + 1;

            b32 RequiresSubstitution = (A[IndexA - 1] != B[IndexB - 1]);
            s32 SubstitutionDistance = EditDistanceCacheMatrix[EDIT_DISTANCE_CACHE_MATRIX_SIZE * (IndexB - 1) +
                (IndexA - 1)] + RequiresSubstitution;

            s32 *Distance = &EditDistanceCacheMatrix[EDIT_DISTANCE_CACHE_MATRIX_SIZE * IndexB + IndexA];
            *Distance = Min(Min(DeletionDistance, InsertionDistance), SubstitutionDistance);
        }
    }

    return EditDistanceCacheMatrix[EDIT_DISTANCE_CACHE_MATRIX_SIZE * LengthB + LengthA];
}

#undef EDIT_DISTANCE_CACHE_MATRIX_SIZE
