#ifndef ISE_PARSER_H
#define ISE_PARSER_H

#define MAX_KEYWORD_COUNT_PER_QUERY  5

struct buffer
{
    u64 Size;
    u8 *Data;
};

struct query_data
{
    char *Keywords[MAX_KEYWORD_COUNT_PER_QUERY];

    // TODO(philip): Maybe these sizes are not right.
    u64 QueryID;
    match_type MatchType;
    u32 MatchDistance;
    u32 KeywordCount;
};

struct parse_context
{
    char *Pointer;
    u64 LineNumber;
};

#endif
