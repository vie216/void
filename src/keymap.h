#ifndef KEYMAP_H
#define KEYMAP_H

#include "defs.h"

typedef enum {
  KEY_LEFT      = 1 << 0,
  KEY_RIGHT     = 1 << 1,
  KEY_UP        = 1 << 2,
  KEY_DOWN      = 1 << 3,
  KEY_Q         = 1 << 4,
  KEY_ENTER     = 1 << 5,
  KEY_BACKSPACE = 1 << 6,
  KEY_DELETE    = 1 << 7,
  KEY_TAB       = 1 << 8,
  KEY_BACKTAB   = 1 << 9,
  KEY_S         = 1 << 10,
} Key;

typedef struct {
  Key key;
  u8  ctrl;
  u8  alt;
} Input;

u8 decode_arg_from_stdin(u8 *ch);
Input decode_single_input_from_stdin(u8 ch);
Input decode_input_from_stdin(u8 ch);

#endif // KEYMAP_H
