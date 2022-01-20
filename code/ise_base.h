#ifndef ISE_BASE_H
#define ISE_BASE_H

//
// NOTE(philip): Base Keywords
//

#define function               static
#define global                 static
#define local_persistant       static

//
// NOTE(philip): Base Types
//

typedef unsigned char          u8;
typedef unsigned short         u16;
typedef unsigned int           u32;
typedef unsigned long long     u64;

typedef signed char            s8;
typedef signed short           s16;
typedef signed int             s32;
typedef signed long long       s64;

typedef float                  f32;
typedef double                 f64;

static_assert(sizeof(u8)  == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);

static_assert(sizeof(s8)  == 1);
static_assert(sizeof(s16) == 2);
static_assert(sizeof(s32) == 4);
static_assert(sizeof(s64) == 8);

static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

typedef s8                     b8;
typedef s32                    b32;
typedef s64                    b64;

//
// NOTE(philip): Base Macros
//

#define Assert(Condition) if (!(Condition)) { *(int *)0 = 0; }

#define Min(A, B) (((A) < (B)) ? (A) : (B))
#define Max(A, B) (((A) > (B)) ? (A) : (B))

#define ArrayCount(Array) (sizeof((Array)) / sizeof(*(Array)))

#define KB(X) (X * 1024)
#define MB(X) (X * 1024 * 1024)
#define GB(X) (X * 1024 * 1024 * 1024)

//
// NOTE(philip): Memory
//

struct memory_block
{
    memory_block *Previous;

    u8 *Base;
    u64 Size;
    u64 Used;
};

struct memory_arena
{
    u64 BlockSize;
    memory_block *CurrentBlock;
};

//
// NOTE(philip): Other
//

typedef u32 match_type;
enum
{
    MatchType_Exact   = 0,
    MatchType_Hamming = 1,
    MatchType_Edit    = 2
};

#endif
