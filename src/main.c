#include <stdio.h>
#include <locale.h>

#include "term.h"
#include "defs.h"
#include "buffer.h"
#include "renderer.h"
#include "wstr.h"
#include "keymap.h"
#include "da.h"

#define NONE(key_) ((decoded.key & (key_)) && !decoded.ctrl && !decoded.alt)
#define CTRL(key_) ((decoded.key & (key_)) &&  decoded.ctrl && !decoded.alt)
#define ALT(key_)  ((decoded.key & (key_)) && !decoded.ctrl &&  decoded.alt)
#define CHAR(ch_)      ((decoded.key & KEY_CHAR) && decoded.ch == ch_ && !decoded.ctrl && !decoded.alt)
#define CTRL_CHAR(ch_) ((decoded.key & KEY_CHAR) && decoded.ch == ch_ &&  decoded.ctrl && !decoded.alt)
#define ALT_CHAR(ch_)  ((decoded.key & KEY_CHAR) && decoded.ch == ch_ && !decoded.ctrl &&  decoded.alt)

#define ACTIONS_LIMIT 3

static u32 actions = 0;

bool process_input(Buffer *buffer, u32 input) {
  Input decoded = decode_input_from_stdin(input);
  if (decoded.key == KEY_NONE)
    return true;

  if (KB_QUIT)
    return false;

  if (KB_SAVE) {
    buffer_write_file(buffer);
    actions = 0;
  } else if (KB_NEW_LINE) {
    buffer_insert_new_line(buffer);
    actions++;
  } else if (KB_DEL_PREV) {
    buffer_delete_before_cursor(buffer);
    actions++;
  } else if (KB_DEL_PREV_WORD) {
    buffer_move_left_word(buffer, true);
  } else if (KB_DEL_NEXT) {
    buffer_delete_at_cursor(buffer);
    actions++;
  } else if (KB_DEL_NEXT_WORD) {
    buffer_move_right_word(buffer, true);
    actions++;
  } else if (KB_INDENT) {
    buffer_indent(buffer);
  } else if (KB_UNINDENT) {
    buffer_unindent(buffer);
    actions++;
  } else if (KB_LEFT)
    buffer_move_left(buffer);
  else if (KB_RIGHT)
    buffer_move_right(buffer);
  else if (KB_UP)
    buffer_move_up(buffer);
  else if (KB_DOWN)
    buffer_move_down(buffer);
  else if (KB_LEFT_WORD)
    buffer_move_left_word(buffer, false);
  else if (KB_RIGHT_WORD)
    buffer_move_right_word(buffer, false);
  else if (KB_UP_PARAGRAPH)
    buffer_move_up_paragraph(buffer);
  else if (KB_DOWN_PARAGRAPH)
    buffer_move_down_paragraph(buffer);
  else if (KB_LINE_START)
    buffer_move_to_line_start(buffer);
  else if (KB_LINE_END)
    buffer_move_to_line_end(buffer);
  else if (KB_FILE_START)
    buffer_move_to_buffer_start(buffer);
  else if (KB_FILE_END)
    buffer_move_to_buffer_end(buffer);
  else if (decoded.key == KEY_CHAR) {
    if ((input >= 32 && input <= 126) || input > 127) {
      buffer_insert(buffer, input);
      actions++;
    }
  }

  return true;
}

int main(i32 argc, char **argv) {
  setlocale(LC_ALL, "");
  term_init();
  fputs("\033[H\033[2J", stdout);

  Buffer buffer = {0};
  Renderer renderer = {0};

  buffer.message = "";

  if (argc > 1)
    buffer_read_file(&buffer, argv[1]);
  else
    DA_APPEND(buffer, ((Line) {0}));

  u32 input;
  renderer_render_buffer(&renderer, &buffer);
  while ((input = wgetc(stdin)) != (u32) EOF) {
    if (!process_input(&buffer, input))
      break;
    if (actions >= ACTIONS_LIMIT)
      buffer.message = "";
    renderer_render_buffer(&renderer, &buffer);
  }

  fputs("\033[H\033[2J", stdout);
  term_reset();
  return 0;
}
