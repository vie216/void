#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define TAB_WIDTH 2
#define HARD_TABS false

#define KB_QUIT           CTRL(KEY_Q)
#define KB_SAVE           CTRL(KEY_S)
/*               Editing                */
#define KB_NEW_LINE       NONE(KEY_ENTER)
#define KB_DEL_PREV       NONE(KEY_BACKSPACE)
#define KB_DEL_PREV_WORD  CTRL(KEY_BACKSPACE)
#define KB_DEL_NEXT       NONE(KEY_DELETE)
#define KB_DEL_NEXT_WORD  CTRL(KEY_DELETE)
#define KB_INDENT         NONE(KEY_TAB)
#define KB_UNINDENT       NONE(KEY_BACKTAB)
/*              Navigation              */
#define KB_LEFT           NONE(KEY_LEFT)
#define KB_RIGHT          NONE(KEY_RIGHT)
#define KB_UP             NONE(KEY_UP)
#define KB_DOWN           NONE(KEY_DOWN)
#define KB_LEFT_WORD      CTRL(KEY_LEFT)
#define KB_RIGHT_WORD     CTRL(KEY_RIGHT)
#define KB_UP_PARAGRAPH   CTRL(KEY_UP)
#define KB_DOWN_PARAGRAPH CTRL(KEY_DOWN)
#define KB_LINE_START     ALT(KEY_LEFT)
#define KB_LINE_END       ALT(KEY_RIGHT)
#define KB_FILE_START     ALT(KEY_UP)
#define KB_FILE_END       ALT(KEY_DOWN)

#endif // CONFIG_H
