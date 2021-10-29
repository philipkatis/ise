/*

  NOTE(philip): Since all the data that the application needs to process come from a file, we read the entire
  file contents and store them in a contiguous buffer. That buffer is then directly modified to separate the
  different data from one another, and then pointers into the buffer are used to reference each of them.

  TODO(philip): We should investigate the performance characteristics of this approach. While this requires the
  least number of string copies (which is an expensive operation), there are two other problems that I can think
  of right now.

  1) Cache misses. Since the actual data is separated from the BK-tree (the structure that we will need to
  traverse the most), the keyword strings will be loaded/unloaded to/from the cache all the time.

  2) Data duplication. We can certainly use separate data-structures to cull duplicate keywords from being
  processed, but the data will always be loaded in memory.

  NOTE(philip): Right now we assume that there will stricly be a single command per line.

*/

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


// TODO(philip): This function should return something containing the query data. And that should be passed into
// a separate function to add it to the data-structures. That ensures a separation layer between the parser and the
// data-structures, which should be agnostic to the way data is coming in.
function void
ParseQueryEntry(parse_context *Context)
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
        return;
    }

    if (!IsInteger(Tokens[0]))
    {
        printf("[Warning]: Found query with non-integer ID! Skipping... (Line: %llu)\n", Context->LineNumber);
        return;
    }

    if (!IsInteger(Tokens[1]))
    {
        printf("[Warning]: Found query with non-integer match type! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    if (!IsInteger(Tokens[2]))
    {
        printf("[Warning]: Found query with non-integer match distance! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    if (!IsInteger(Tokens[3]))
    {
        printf("[Warning]: Found query with non-integer keyword count! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
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
        return;
    }

    u32 MinMatchDistance = ((MatchType == 0) ? 0 : 1);
    u32 MaxMatchDistance = ((MatchType == 0) ? 0 : MAX_KEYWORD_LENGTH);

    if (MatchDistance < MinMatchDistance || MatchDistance > MaxMatchDistance)
    {
        printf("[Warning]: Found query with out-of-bounds match distance! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    if (KeywordCount <= 0 || KeywordCount > MAX_KEYWORD_COUNT_PER_QUERY)
    {
        printf("[Warning]: Found query with out-of-bounds keyword count! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    if (KeywordCount != TokenCount - 4)
    {
        printf("[Warning]: Found query with different keyword count and real keyword count! Skipping... (Line: %llu)\n",
               Context->LineNumber);
        return;
    }

    printf("Query %llu:\n{\n", QueryID);

    for (u32 Index = 4;
         Index < TokenCount;
         ++Index)
    {
        printf("    %s\n", Tokens[Index]);
    }

    printf("}\n\n");

#undef MAX_TOKEN_COUNT
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

            ParseQueryEntry(&Context);
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
