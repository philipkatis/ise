function u64
StringLength(char *String)
{
    u64 Length = 0;

    while (String[Length])
    {
        ++Length;
    }

    return Length;
}
