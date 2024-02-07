/* Functions for working with wide strings
   Information about utf-8 format was taken from
   https://www.cprogramming.com/tutorial/unicode.html */

#ifndef WSTR_H
#define WSTR_H

#include <stdio.h>

#include "defs.h"

typedef struct {
  u32 *ptr;
  u32  len;
} WStr;

typedef struct {
  u8 *ptr;
  u32 len;
} Str;

/* Print wide character to stream */
void wputc(u32 ch, FILE *stream);
/* Get wide character from stream */
u32 wgetc(FILE *stream);
/* Convert utf-8 string to wide string */
WStr utow(Str str);

#define WIDEN(ch) ((u32) (ch) << 24)

#endif // WSTR_H
