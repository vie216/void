#ifndef KEYMAP_H
#define KEYMAP_H

#include "defs.h"

typedef enum {
  KEY_NONE      = 0,
  KEY_CHAR      = 1 << 0,
  KEY_LEFT      = 1 << 1,
  KEY_RIGHT     = 1 << 2,
  KEY_UP        = 1 << 3,
  KEY_DOWN      = 1 << 4,
  KEY_ENTER     = 1 << 5,
  KEY_BACKSPACE = 1 << 6,
  KEY_DELETE    = 1 << 7,
  KEY_TAB       = 1 << 8,
  KEY_BACKTAB   = 1 << 9,
} Key;

typedef struct {
  Key key;
  u32 ch;
  u8  ctrl;
  u8  alt;
} Input;

u8 decode_arg_from_stdin(u32 *ch);
Input decode_single_input_from_stdin(u32 ch);
Input decode_input_from_stdin(u32 ch);

#endif // KEYMAP_H
