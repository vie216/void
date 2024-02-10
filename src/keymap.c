#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#include "keymap.h"

#define ARG_BUFFER_CAP 3

u8 decode_arg_from_stdin(u32 *ch) {
  char buffer[ARG_BUFFER_CAP + 1] = {0};
  u8 index = 0;

  do {
    if (index >= ARG_BUFFER_CAP)
      return 0;
    buffer[index++] = *ch;
  } while (isdigit(*ch = getchar()));

  return atoi(buffer);
}

Input decode_single_input_from_stdin(u32 ch) {
  switch (ch) {
  case 17:   return (Input) { KEY_CHAR, 'q', true, false };
  case '\r': return (Input) { KEY_ENTER, 0, false, false };
  case 127:  return (Input) { KEY_BACKSPACE, 0, false, false };
  case 8:    return (Input) { KEY_BACKSPACE, 0, true, false };
  case 9:    return (Input) { KEY_TAB, 0, false, false };
  case 19:   return (Input) { KEY_CHAR, 's', true, false };
  default:   return (Input) { KEY_CHAR, ch, false, false };
  }
}

Input decode_input_from_stdin(u32 ch) {
  u8 ctrl = 0;
  u8 alt = 0;
  u8 arg0 = 0;
  u8 arg1 = 0;

  if (ch != '\033')
    return decode_single_input_from_stdin(ch);
  if (getchar() != '[')
    return (Input) {0};

  ch = getchar();
  if (isdigit(ch))
    arg0 = decode_arg_from_stdin(&ch);
  if (ch == ';' && isdigit(ch = getchar()))
    arg1 = decode_arg_from_stdin(&ch);

  (void) arg0;

  ctrl = arg1 == 5;
  alt = arg1 == 3;

  switch (ch) {
  case 'D': return (Input) { KEY_LEFT, 0, ctrl, alt };
  case 'C': return (Input) { KEY_RIGHT, 0, ctrl, alt };
  case 'A': return (Input) { KEY_UP, 0, ctrl, alt };
  case 'B': return (Input) { KEY_DOWN, 0, ctrl, alt };
  case '~':
  case 'P': return (Input) { KEY_DELETE, 0, ctrl, alt };
  case 'M': return (Input) { KEY_DELETE, 0, true, alt };
  case 'Z': return (Input) { KEY_BACKTAB, 0, ctrl, alt };
  default:  return (Input) {0};
  }
}
