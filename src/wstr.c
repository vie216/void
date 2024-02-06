#include <stdlib.h>

#include "wstr.h"

void wputc(u32 ch, FILE *stream) {
  u8 *ptr = (u8 *) &ch;

  for (i8 i = 3; i >= 0; --i)
    if (ptr[i])
      putc(ptr[i], stream);
}

u32 wgetc(FILE *stream) {
  u32 ch = 0;
  u8 *ptr = (u8 *) &ch;

  ptr[0] = getc(stream);
  if (ptr[0] >> 7) {
    ch <<= 8;
    ptr[0] = getc(stream);
  }

  return ch;
}

WStr utow(Str ustr) {
  u32 wlen = 0;
  for (u32 i = 0; i < ustr.len; ++i)
    wlen += (ustr.ptr[i] >> 6) != 0b10;

  u8 *wstr = malloc(wlen * sizeof(u32));
  memset(wstr, 0, wlen * sizeof(u32));

  u32 char_base = -4;
  for (u32 i = 0, j = 0; i < ustr.len; ++i, --j) {
    if (ustr.ptr[i] >> 6 != 0b10) {
      char_base += 4;
      j = 3;
    }

    wstr[char_base + j] = ustr.ptr[i];
  }

  return (WStr) {
    .ptr = (u32 *) wstr,
    .len = wlen,
  };
}
