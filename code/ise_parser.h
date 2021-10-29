#ifndef ISE_PARSER_H
#define ISE_PARSER_H

// NOTE(philip): This struct contains the context while parsing a command file, throughout multiple function calls.
struct parse_context
{
    char *Pointer;
    u64 LineNumber;
};

#endif
