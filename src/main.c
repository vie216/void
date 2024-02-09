#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "term.h"
#include "defs.h"
#include "buffer.h"
#include "config.h"
#include "renderer.h"
#include "wstr.h"
#include "keymap.h"

#define NONE(key_) ((decoded.key & (key_)) && !decoded.ctrl && !decoded.alt)
#define CTRL(key_) ((decoded.key & (key_)) &&  decoded.ctrl && !decoded.alt)
#define ALT(key_)  ((decoded.key & (key_)) && !decoded.ctrl &&  decoded.alt)

bool process_input(Buffer *buffer, char *file_path, u32 input) {
  if ((input >= 32 && input <= 126) || input > 127) {
    buffer_insert(buffer, input);
  } else {
    Input decoded = decode_input_from_stdin(input);
    if (decoded.key == KEY_NONE)
      return true;

    if (KB_QUIT)
      return false;
    else if (KB_SAVE)
      /* Not using `if` here because I just don't want to
         put braces everywhere in this `if-else` chain
         (ambiguous `else` or inconsistent code style otherwise) */
      file_path ? buffer_write_file(buffer, file_path) : 0;
    /*               Editing                */
    else if (KB_NEW_LINE)
      buffer_insert_new_line(buffer);
    else if (KB_DEL_PREV)
      buffer_delete_before_cursor(buffer);
    else if (KB_DEL_PREV_WORD)
      buffer_move_left_word(buffer, true);
    else if (KB_DEL_NEXT)
      buffer_delete_at_cursor(buffer);
    else if (KB_DEL_NEXT_WORD)
      buffer_move_right_word(buffer, true);
    else if (KB_INDENT)
      buffer_indent(buffer);
    else if (KB_UNINDENT)
      buffer_unindent(buffer);
    /*              Navigation              */
    else if (KB_LEFT)
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
  }

  return true;
}

int main(i32 argc, char **argv) {
  term_init();
  setlocale(LC_ALL, "");
  fputs("\033[H\033[2J", stdout);

  Buffer buffer = {0};
  Renderer renderer = {0};
  char *file_path = NULL;

  if (argc > 1) {
    file_path = argv[1];
    buffer_read_file(&buffer, file_path);
  } else {
    DA_APPEND(buffer, ((Line) {0}));
  }

  u32 input;
  renderer_render_buffer(&renderer, &buffer);
  while ((input = wgetc(stdin)) != (u32) EOF) {
    if (!process_input(&buffer, file_path, input))
      break;
    renderer_render_buffer(&renderer, &buffer);
  }

  fputs("\033[H\033[2J", stdout);
  term_reset();
  return 0;
}
