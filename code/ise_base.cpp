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

function b32
IsWhitespace(char Character)
{
    b32 Result = ((Character == ' ') || (Character == '\t') || (Character == '\r') || (Character == '\n'));
    return Result;
}

function b32
IsInteger(char *String)
{
    b32 Result = true;

    for (char *Character = String;
         *Character;
         ++Character)
    {
        if ((*Character < '0') || (*Character > '9'))
        {
            Result = false;
            break;
        }
    }

    return Result;
}

function buffer
AllocateBuffer(u64 Size)
{
    buffer Result = { };
    Result.Size = Size;
    Result.Data = (u8 *)calloc(1, Size);

    return Result;
}

function void
DeallocateBuffer(buffer Buffer)
{
    free(Buffer.Data);

    Buffer.Size = 0;
    Buffer.Data = 0;
}
