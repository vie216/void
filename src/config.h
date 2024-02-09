#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define TAB_WIDTH 2
#define HARD_TABS false

#define KB_QUIT           BIND_CTRL(KEY_Q)
#define KB_SAVE           BIND_CTRL(KEY_S)
/*               Editing                */
#define KB_NEW_LINE       BIND(KEY_ENTER)
#define KB_DEL_PREV       BIND(KEY_BACKSPACE)
#define KB_DEL_PREV_WORD  BIND_CTRL(KEY_BACKSPACE)
#define KB_DEL_NEXT       BIND(KEY_DELETE)
#define KB_DEL_NEXT_WORD  BIND_CTRL(KEY_DELETE)
#define KB_INDENT         BIND(KEY_TAB)
#define KB_UNINDENT       BIND(KEY_BACKTAB)
/*              Navigation              */
#define KB_LEFT           BIND(KEY_LEFT)
#define KB_RIGHT          BIND(KEY_RIGHT)
#define KB_UP             BIND(KEY_UP)
#define KB_DOWN           BIND(KEY_DOWN)
#define KB_LEFT_WORD      BIND_CTRL(KEY_LEFT)
#define KB_RIGHT_WORD     BIND_CTRL(KEY_RIGHT)
#define KB_UP_PARAGRAPH   BIND_CTRL(KEY_UP)
#define KB_DOWN_PARAGRAPH BIND_CTRL(KEY_DOWN)
#define KB_LINE_START     BIND_ALT(KEY_LEFT)
#define KB_LINE_END       BIND_ALT(KEY_RIGHT)
#define KB_FILE_START     BIND_ALT(KEY_UP)
#define KB_FILE_END       BIND_ALT(KEY_DOWN)

#endif // CONFIG_H
