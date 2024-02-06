#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))

#define DA_RESERVE_SPACE(da, amount)                                \
  do {                                                              \
    u32 size = sizeof((da).items[0]);                               \
    if ((da).cap < (da).len + amount) {                             \
      if ((da).cap != 0) {                                          \
        while ((da).cap < (da).len + amount)                        \
          (da).cap *= 2;                                            \
        (da).items = realloc((da).items, size * (da).cap);          \
      } else {                                                      \
        (da).cap = 1;                                               \
        (da).items = malloc(size * (da).cap);                       \
      }                                                             \
    }                                                               \
  } while (0)

#define DA_APPEND(da, element)        \
  do {                                \
    DA_RESERVE_SPACE(da, 1);          \
    (da).items[(da).len++] = element; \
  } while (0)

#define DA_INSERT(da, element, index)      \
  do {                                     \
    u32 size = sizeof((da).items[0]);      \
    DA_RESERVE_SPACE(da, 1);               \
    memcpy((da).items + (index) + 1,       \
           (da).items + (index),           \
           size * ((da).len++ - (index))); \
    (da).items[index] = element;           \
  } while (0)

#define DA_REMOVE(da, index)                   \
  do {                                         \
    u32 size = sizeof((da).items[0]);          \
    if ((da).len > 0) {                        \
      if ((index) != (da).len - 1)             \
        memcpy((da).items + (index),           \
               (da).items + (index) + 1,       \
               size * ((da).len - (index)));   \
      (da).len--;                              \
    }                                          \
  } while (0)

typedef signed char    i8;
typedef unsigned char  u8;
typedef signed short   i16;
typedef unsigned short u16;
typedef signed int     i32;
typedef unsigned int   u32;
typedef signed long    i64;
typedef unsigned long  u64;

typedef float f32;

#endif // DEFS_H
