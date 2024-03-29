#include "da.h"

void da_reserve_space(void **items, u32 *len, u32 *cap, u32 size, u32 amount) {
  if (*cap < *len + amount) {
    if (*cap != 0) {
      while (*cap < *len + amount)
        *cap *= 2;
      *items = realloc(*items, size * *cap);
    } else {
      *cap = amount;
      *items = malloc(size * *cap);
    }
  }
}
