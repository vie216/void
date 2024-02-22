#ifndef DA_H
#define DA_H

#include <stdlib.h>
#include <string.h>

#include "defs.h"

#define DA_APPEND(da, element)                             \
  do {                                                     \
    da_reserve_space((void **) &(da).items, &(da).len,     \
                     &(da).cap, sizeof((da).items[0]), 1); \
    (da).items[(da).len++] = element;                      \
  } while (0)

#define DA_INSERT(da, element, index) DA_INSERT_REPEAT(da, element, 1, index)

#define DA_INSERT_REPEAT(da, element, amount, index)            \
  do {                                                          \
    u32 size = sizeof((da).items[0]);                           \
    da_reserve_space((void **) &(da).items, &(da).len,          \
                     &(da).cap, sizeof((da).items[0]), amount); \
    memcpy((da).items + (index) + amount,                       \
           (da).items + (index),                                \
           size * ((da).len - (index)));                        \
    (da).len += amount;                                         \
    for (u32 i = 0; i < amount; ++i)                            \
      (da).items[index + i] = element;                          \
  } while (0)

#define DA_REMOVE(da, index) DA_REMOVE_REPEAT(da, 1, index)

#define DA_REMOVE_REPEAT(da, amount, index)     \
  do {                                          \
    u32 size = sizeof((da).items[0]);           \
    if ((index) + (amount) <= (da).len) {       \
      if ((index) + (amount) != (da).len)       \
        memcpy((da).items + (index),            \
               (da).items + (index) + (amount), \
               size * ((da).len - (index)));    \
      (da).len -= amount;                       \
    }                                           \
  } while (0)

void da_reserve_space(void **items, u32 *len, u32 *cap, u32 size, u32 amount);

#endif // DA_H
