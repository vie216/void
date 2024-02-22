#include <stdlib.h>
#include <string.h>

#include "wstr.h"

void wputc(u32 ch, FILE *stream) {
  u8 *ptr = (u8 *) &ch;

  for (u8 i = 0; i < 4; ++i)
    if (ptr[i])
      putc(ptr[i], stream);
}

static u8 offset_table[3] = { 7, 5, 4 };

u32 wgetc(FILE *stream) {
  u8 ptr[4] = {0};

  ptr[0] = getc(stream);
  for (u32 i = 0; i < sizeof(offset_table); ++i) {
    if (!((ptr[0] >> offset_table[i]) & 1))
      break;
    ptr[i + 1] = getc(stream);
  }

  return *(u32 *) ptr;
}

u32 wreadc(Str *ustr) {
  u8 ptr[4] = {0};

  ptr[0] = ustr->ptr++[0];
  for (u32 i = 0; i < sizeof(offset_table); ++i) {
    if (ustr->len <= i + 1 || !((ptr[0] >> offset_table[i]) & 1))
      break;
    ptr[i + 1] = ustr->ptr++[0];
  }

  return *(u32 *) ptr;
}

WStr utow(Str ustr) {
  u32 wlen = 0;
  for (u32 i = 0; i < ustr.len; ++i)
    wlen += (ustr.ptr[i] >> 6) != 0b10;

  u32 *wstr = malloc(wlen * sizeof(u32));
  memset(wstr, 0, wlen * sizeof(u32));

  for (u32 i = 0; i < wlen; ++i)
    wstr[i] = wreadc(&ustr);

  return (WStr) {
    .ptr = (u32 *) wstr,
    .len = wlen,
  };
}
