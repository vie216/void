#include <stdio.h>
#include <stdlib.h>

#include "term.h"
#include "defs.h"
#include "buffer.h"
#include "config.h"
#include "renderer.h"
#include "wstr.h"

int main(i32 argc, char **argv) {
  term_init();
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
  while ((input = wgetc(stdin)) != 17 && input != (u32) EOF) {
    switch (input) {
    case 19: { /* Ctrl+S */
      if (file_path)
        buffer_write_file(&buffer, file_path);
    } break;

    case '\r': { /* Enter */
      buffer_insert_new_line(&buffer);
    } break;

    case 127: { /* Backspace */
      buffer_delete_before_cursor(&buffer);
    } break;

    case 8: { /* Ctrl+Backspace */
      buffer_move_left_word(&buffer, true);
    } break;

    case 27: { /* Escape code */
      if (getchar() != 91)
        break;

      input = getchar();
      if (input == 'D') { /* Left arrow */
        buffer_move_left(&buffer);
      } else if (input == 'C') { /* Right arrow */
        buffer_move_right(&buffer);
      } else if (input == 'A') { /* Up arrow */
        buffer_move_up(&buffer);
      } else if (input == 'B') { /* Down arrow */
        buffer_move_down(&buffer);
      } else if (input == 'Z') { /* Shift+Tab */
        buffer_unindent(&buffer);
      } else if (input == 'P') { /* Delete */
        buffer_delete_at_cursor(&buffer);
      } else if (input == '1') {
        if (getchar() != ';')
          break;
        char modifier = getchar();
        if (modifier != '5' && modifier != '3')
          break;

        input = getchar();
        if (input == 'D' && modifier == '5') /* Ctrl+Left arrow */
          buffer_move_left_word(&buffer, false);
        else if (input == 'D' && modifier == '3') /* Alt+Left arrow */
          buffer_move_to_line_start(&buffer);
        else if (input == 'C' && modifier == '5') /* Ctrl+Right arrow */
          buffer_move_right_word(&buffer, false);
        else if (input == 'C' && modifier == '3') /* Alt+Right arrow */
          buffer_move_to_line_end(&buffer);
        else if (input == 'A' && modifier == '5') /* Ctrl+Up arrow */
          buffer_move_up_paragraph(&buffer);
        else if (input == 'A' && modifier == '3') /* Alt+Up arrow */
          buffer_move_to_buffer_start(&buffer);
        else if (input == 'B' && modifier == '5') /* Ctrl+Down arrow */
          buffer_move_down_paragraph(&buffer);
        else if (input == 'B' && modifier == '3') /* Alt+Down arrow */
          buffer_move_to_buffer_end(&buffer);
      } else if (input == 'M') { /* Ctrl+Delete */
        buffer_move_right_word(&buffer, true);
      }
    } break;

    case '\t': { /* Tab */
      buffer_indent(&buffer);
    } break;

    default: {
      if ((input >= 32 && input <= 127) || input > 0x7F)
        buffer_insert(&buffer, input);
    }
    }

    renderer_render_buffer(&renderer, &buffer);
  }

  fputs("\033[H\033[2J", stdout);
  term_reset();
  return 0;
}
