function void
ZeroMemory(void *Memory, u64 Size)
{
    u8 *Byte = (u8 *)Memory;
    u8 *End  = (u8 *)Memory + Size;

    while (Byte != End)
    {
        *Byte = 0;
        ++Byte;
    }
}

function void
CopyMemory(void *Source, u64 Size, void *Destination)
{
    u8 *SourceByte      = (u8 *)Source;
    u8 *End             = (u8 *)Source + Size;
    u8 *DestinationByte = (u8 *)Destination;

    while (SourceByte != End)
    {
        *DestinationByte = *SourceByte;

        ++SourceByte;
        ++DestinationByte;
    }
}

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
