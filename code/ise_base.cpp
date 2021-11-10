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

function b32
LoadTextFile(char *Path, buffer *Buffer)
{
    b32 Result = true;

    FILE *File = fopen(Path, "rt");
    if (File)
    {
        fseek(File, 0, SEEK_END);
        u64 Length = ftell(File);

        *Buffer = AllocateBuffer(Length * sizeof(char));

        fseek(File, 0, SEEK_SET);
        fread(Buffer->Data, 1, Length, File);

        fclose(File);
    }
    else
    {
        Result = false;
    }

    return Result;
}
