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

bool process_input(Buffer *buffer, char *file_path, u32 input) {
  if ((input >= 32 && input <= 126) || input > 127) {
    buffer_insert(buffer, input);
  } else {
    Input decoded = decode_input_from_stdin(input);
    if (!decoded.key)
      return true;

    if (decoded.key == KEY_Q && decoded.ctrl)
      return false;
    else if (decoded.key == KEY_ENTER)
      buffer_insert_new_line(buffer);
    else if (decoded.key == KEY_BACKSPACE && decoded.ctrl)
      buffer_move_left_word(buffer, true);
    else if (decoded.key == KEY_BACKSPACE)
      buffer_delete_before_cursor(buffer);
    else if (decoded.key == KEY_DELETE && decoded.ctrl)
      buffer_move_right_word(buffer, true);
    else if (decoded.key == KEY_DELETE)
      buffer_delete_at_cursor(buffer);
    else if (decoded.key == KEY_TAB)
      buffer_indent(buffer);
    else if (decoded.key == KEY_BACKTAB)
      buffer_unindent(buffer);
    else if (decoded.key == KEY_S && decoded.ctrl)
      /* Not using `if` here because I just don't want to
         put braces everywhere in this `if-else` chain
         (ambiguous `else` or inconsistent code style otherwise) */
      file_path ? buffer_write_file(buffer, file_path) : 0;
    /* Ctrl+Arrows */
    else if (decoded.key == KEY_LEFT && decoded.ctrl)
      buffer_move_left_word(buffer, false);
    else if (decoded.key == KEY_RIGHT && decoded.ctrl)
      buffer_move_right_word(buffer, false);
    else if (decoded.key == KEY_UP && decoded.ctrl)
      buffer_move_up_paragraph(buffer);
    else if (decoded.key == KEY_DOWN && decoded.ctrl)
      buffer_move_down_paragraph(buffer);
    /* Alt+Arrows */
    else if (decoded.key == KEY_LEFT && decoded.alt)
      buffer_move_to_line_start(buffer);
    else if (decoded.key == KEY_RIGHT && decoded.alt)
      buffer_move_to_line_end(buffer);
    else if (decoded.key == KEY_UP && decoded.alt)
      buffer_move_to_buffer_start(buffer);
    else if (decoded.key == KEY_DOWN && decoded.alt)
      /* Arrows */
      buffer_move_to_buffer_end(buffer);
    else if (decoded.key == KEY_LEFT)
      buffer_move_left(buffer);
    else if (decoded.key == KEY_RIGHT)
      buffer_move_right(buffer);
    else if (decoded.key == KEY_UP)
      buffer_move_up(buffer);
    else if (decoded.key == KEY_DOWN)
      buffer_move_down(buffer);
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
