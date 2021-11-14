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

function b32
EndOfCommand(parse_context *Context)
{
    b32 Result = (!(*Context->Pointer) || (*Context->Pointer == '\n'));
    return Result;
}

// NOTE(philip): This function is what modifies the file data, essentially replacing whitespace with 0, except
// new line characters. That replacement occurs at ParseCommandFile().
function void
NullifyWhitespace(parse_context *Context)
{
    while (!EndOfCommand(Context) && IsWhitespace(*Context->Pointer))
    {
        *Context->Pointer = 0;
        ++Context->Pointer;
    }
}

function b32
ParseQueryEntry(parse_context *Context, query_data *Data)
{
#define MAX_TOKEN_COUNT (4 + MAX_KEYWORD_COUNT_PER_QUERY)

    char *Tokens[MAX_TOKEN_COUNT];
    u32 TokenCount = 0;

    // NOTE(philip): Get rid of any whitespace before the first token.
    NullifyWhitespace(Context);

    if (!EndOfCommand(Context))
    {
        while (!EndOfCommand(Context) && (TokenCount <= MAX_TOKEN_COUNT))
        {
            ++TokenCount;
            Tokens[TokenCount - 1] = Context->Pointer;

            ++Context->Pointer;

            // NOTE(philip): Go over all the token characters.
            while (!EndOfCommand(Context) && !IsWhitespace(*Context->Pointer))
            {
                ++Context->Pointer;
            }

            // NOTE(philip): Get rid of any whitespace before the next token.
            NullifyWhitespace(Context);
        }

        // NOTE(philip): If there are more tokens that the maximum allowable number, skip them.
        if (TokenCount > MAX_TOKEN_COUNT)
        {
            while (!EndOfCommand(Context))
            {
                ++Context->Pointer;
            }
        }
    }

    // NOTE(philip): After parsing is complete, the last character should always be a new line character.
    Assert(*Context->Pointer == '\n');
    *Context->Pointer = 0;

    // NOTE(philip): Ensure that the data we read is all valid.

    if (TokenCount < 4 || TokenCount > MAX_TOKEN_COUNT)
    {
        printf("[Warning]: Incorrect argument count for query insertion! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return false;
    }

#undef MAX_TOKEN_COUNT

    if (!IsInteger(Tokens[0]))
    {
        printf("[Warning]: Found query with non-integer ID! Skipping... (Line: %llu)\n", Context->LineNumber);
        return false;
    }

    if (!IsInteger(Tokens[1]))
    {
        printf("[Warning]: Found query with non-integer match type! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return false;
    }

    if (!IsInteger(Tokens[2]))
    {
        printf("[Warning]: Found query with non-integer match distance! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return false;
    }

    if (!IsInteger(Tokens[3]))
    {
        printf("[Warning]: Found query with non-integer keyword count! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return false;
    }

    u64 QueryID = atoi(Tokens[0]);
    match_type MatchType = atoi(Tokens[1]);
    u32 MatchDistance = atoi(Tokens[2]);
    u32 KeywordCount = atoi(Tokens[3]);

    // TODO(philip): Check for duplicate IDs.

    if (MatchType > 2)
    {
        printf("[Warning]: Found query with out-of-bounds match type! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return false;
    }

    u32 MinMatchDistance = ((MatchType == 0) ? 0 : 1);
    u32 MaxMatchDistance = ((MatchType == 0) ? 0 : MAX_KEYWORD_LENGTH);

    if (MatchDistance < MinMatchDistance || MatchDistance > MaxMatchDistance)
    {
        printf("[Warning]: Found query with out-of-bounds match distance! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return false;
    }

    if (KeywordCount <= 0 || KeywordCount > MAX_KEYWORD_COUNT_PER_QUERY)
    {
        printf("[Warning]: Found query with out-of-bounds keyword count! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return false;
    }

    if (KeywordCount != TokenCount - 4)
    {
        printf("[Warning]: Found query with different keyword count and real keyword count! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return false;
    }

    // NOTE(philip): Copy the data to the output struct.

    for (u64 Index = 0;
         Index < KeywordCount;
         ++Index)
    {
        Data->Keywords[Index] = Tokens[4 + Index];
    }

    Data->QueryID       = QueryID;
    Data->MatchType     = MatchType;
    Data->MatchDistance = MatchDistance;
    Data->KeywordCount  = KeywordCount;

    return true;
}

function void
ParseCommandFile(buffer Contents)
{
    parse_context Context = { };
    Context.Pointer = (char *)Contents.Data;
    Context.LineNumber = 1;

    while (*Context.Pointer)
    {
        if (*Context.Pointer == 's')
        {
            // NOTE(philip): This command adds a new query to the active set and it has the following format:
            // s <QueryID> <MatchType> <MatchDistance> <Keyword Count> keyword1 ... ... ... ...

            // Nullify the command signature.
            *Context.Pointer = 0;
            ++Context.Pointer;

            query_data QueryData = { };
            if (ParseQueryEntry(&Context, &QueryData))
            {
                printf("Query %llu:\n{\n", QueryData.QueryID);
                printf("    Match Type: %d\n", QueryData.MatchType);
                printf("    Match Distance: %d\n", QueryData.MatchDistance);
                printf("    Keyword Count: %d\n", QueryData.KeywordCount);
                printf("    Keywords: {");

                for (u64 Index = 0;
                     Index < QueryData.KeywordCount;
                     ++Index)
                {
                    if (Index != 0)
                    {
                        printf(", ");
                    }

                    printf(" %s", QueryData.Keywords[Index]);
                }

                printf(" }\n}\n\n");
            }
        }
        else
        {
            printf("[Warning]: Skipping unknown command! (Command: '%c', Line: %llu)\n", *Context.Pointer,
                   Context.LineNumber);
        }

        // NOTE(philip): This should always be zero (previously the new line character). Advance to
        // the next command.
        ++Context.Pointer;

        ++Context.LineNumber;
    }
}
