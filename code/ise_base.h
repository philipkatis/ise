#ifndef ISE_BASE_H
#define ISE_BASE_H

//
// NOTE(philip): Base keyword definitions.
//

#define function               static
#define global                 static
#define local_persistant       static

//
// NOTE(philip): Base type definitions and size checks.
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
// TODO(philip): Assert macro.
//

#define Assert(Condition)

//
// NOTE(philip): Utility macros.
//

#define Min(A, B) (((A) < (B)) ? (A) : (B))
#define Max(A, B) (((A) > (B)) ? (A) : (B))

#define ArrayCount(Array) (sizeof((Array)) / sizeof(*(Array)))

#endif
