function u64
StringLength(char *String)
{
    u64 Result = 0;

    while (String[Result])
    {
        ++Result;
    }

    return Result;
}
