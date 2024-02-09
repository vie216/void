/* Functions for working with utf-8 and wide strings
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

/* Print wide character to stream as uft-8 */
void wputc(u32 ch, FILE *stream);
/* Get utf-8 character from stream as wide */
u32 wgetc(FILE *stream);
/* Get utf-8 character from string as wide */
u32 wreadc(Str *ustr);
/* Convert utf-8 string to wide string */
WStr utow(Str str);

#endif // WSTR_H
