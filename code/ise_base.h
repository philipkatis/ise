#ifndef ISE_BASE_H
#define ISE_BASE_H

#ifndef ISE_DEBUG
    #define ISE_DEBUG 0
#endif

#define global   static
#define local    static
#define function static

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

typedef s8                  b8;
typedef s32                 b32;
typedef s64                 b64;

typedef float               f32;
typedef double              f64;

static_assert(sizeof(u8)  == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);

static_assert(sizeof(s8)  == 1);
static_assert(sizeof(s16) == 2);
static_assert(sizeof(s32) == 4);
static_assert(sizeof(s64) == 8);

static_assert(sizeof(b8)  == 1);
static_assert(sizeof(b32) == 4);
static_assert(sizeof(b64) == 8);

static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

#if ISE_DEBUG
    #define DebugBreak() __builtin_trap()
    #define Assert(Condition) \
        if (!(Condition)) \
        { \
            printf("\n"); \
            printf("*** Assertion Failed ***\n"); \
            printf("\n"); \
            printf("  File: %s\n", __FILE__); \
            printf("  Line: %d\n", __LINE__); \
            printf("  Condition: %s\n", #Condition); \
            printf("\n"); \
            printf("************************\n"); \
            printf("\n"); \
            DebugBreak(); \
        }
#else
    #define DebugBreak()
    #define Assert(Condition)
#endif

#define Min(A, B) (((A) < (B)) ? (A) : (B))
#define Max(A, B) (((A) > (B)) ? (A) : (B))

#define ArrayCount(Array) (sizeof((Array)) / sizeof(*(Array)))

#endif
