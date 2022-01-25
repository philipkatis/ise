#ifndef ISE_BASE_H
#define ISE_BASE_H

//
// NOTE(philip): Platform and Configuration Setup
//

#ifndef ISE_LINUX
    #define ISE_LINUX 0
#endif

#ifndef ISE_DEBUG
    #define ISE_DEBUG 0
#endif

//
// NOTE(philip): Base Keywords
//

#define global   static
#define function static

//
// NOTE(philip): Base Types
//

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

typedef float               f32;
typedef double              f64;

typedef s8                  b8;
typedef s32                 b32;
typedef s64                 b64;

//
// NOTE(philip): Base Macros
//

#if ISE_DEBUG
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
            *(int *)0 = 0; \
        }
#else
    #define Assert(Condition)
#endif

#define Min(A, B) (((A) < (B)) ? (A) : (B))
#define Max(A, B) (((A) > (B)) ? (A) : (B))

#define ArrayCount(Array) (sizeof((Array)) / sizeof((Array)[0]))

#endif
