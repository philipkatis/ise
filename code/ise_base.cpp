function b32
StringCompare(char *StringA, u64 LengthA, char *StringB, u64 LengthB)
{
    b32 Equal = true;

    if (LengthA == LengthB)
    {
        for (u64 Index = 0;
             Index < LengthA;
             ++Index)
        {
            if (StringA[Index] != StringB[Index])
            {
                Equal = false;
                break;
            }
        }
    }
    else
    {
        Equal = false;
    }

    return Equal;
}

// NOTE(philip): DBJ2 String Hashing
function u32
StringHash(char *String)
{
    u32 Hash = 5381;

    for (char *Character = String;
         *Character;
         ++Character)
    {
        Hash = ((Hash << 5) + Hash) + *Character;
    }

    return Hash;
}

function u64
HammingDistance(char *StringA, u64 LengthA, char *StringB, u64 LengthB)
{
    u64 Distance = 0;

    for (u64 Index = 0;
         Index < LengthA;
         ++Index)
    {
        if (StringA[Index] != StringB[Index])
        {
            ++Distance;
        }
    }

    return Distance;
}

function u64
EditDistance(char *StringA, u64 LengthA, char *StringB, u64 LengthB)
{
    u64 Cache[32] =
    {
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    };

    for (u64 IndexA = 0;
         IndexA < LengthA;
         ++IndexA)
    {
        u64 PreviousCost = IndexA + 1;

        for (u64 IndexB = 0;
             IndexB < LengthB;
             ++IndexB)
        {
            u64 Cost = Cache[IndexB];
            if (StringA[IndexA] != StringB[IndexB])
            {
                Cost = Min(Min(Cost, PreviousCost), Cache[IndexB + 1]) + 1;
            }

            Cache[IndexB] = PreviousCost;
            PreviousCost = Cost;
        }

        Cache[LengthB] = PreviousCost;
    }

    u64 Distance = Cache[LengthB];
    return Distance;
}
