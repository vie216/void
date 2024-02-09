#ifndef DEFS_H
#define DEFS_H

#include <stdlib.h>
#include <string.h>

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))

#define DA_RESERVE_SPACE(da, amount)                       \
  do {                                                     \
    u32 size = sizeof((da).items[0]);                      \
    if ((da).cap < (da).len + amount) {                    \
      if ((da).cap != 0) {                                 \
        while ((da).cap < (da).len + amount)               \
          (da).cap *= 2;                                   \
        (da).items = realloc((da).items, size * (da).cap); \
      } else {                                             \
        (da).cap = 1;                                      \
        (da).items = malloc(size * (da).cap);              \
      }                                                    \
    }                                                      \
  } while (0)

#define DA_APPEND(da, element)        \
  do {                                \
    DA_RESERVE_SPACE(da, 1);          \
    (da).items[(da).len++] = element; \
  } while (0)

#define DA_INSERT(da, element, index) DA_INSERT_REPEAT(da, element, 1, index)

#define DA_INSERT_REPEAT(da, element, amount, index) \
  do {                                               \
    u32 size = sizeof((da).items[0]);                \
    DA_RESERVE_SPACE(da, amount);                    \
    memcpy((da).items + (index) + amount,            \
           (da).items + (index),                     \
           size * ((da).len - (index)));             \
    (da).len += amount;                              \
    for (u32 i = 0; i < amount; ++i)                 \
      (da).items[index + i] = element;               \
  } while (0)

#define DA_REMOVE(da, index) DA_REMOVE_REPEAT(da, 1, index)

#define DA_REMOVE_REPEAT(da, amount, index)       \
  do {                                            \
    u32 size = sizeof((da).items[0]);             \
      if ((da).len >= (amount)) {                 \
        if ((index) + (amount) != (da).len)       \
          memcpy((da).items + (index),            \
                 (da).items + (index) + (amount), \
                 size * ((da).len - (index)));    \
        (da).len -= amount;                       \
      }                                           \
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
