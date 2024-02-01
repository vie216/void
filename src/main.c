#include <stdio.h>
#include <stdlib.h>

#include "term.h"
#include "defs.h"
#include "buffer.h"
#include "config.h"
#include "renderer.h"

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

  char input;
  renderer_render_buffer(&renderer, &buffer);
  while ((input = getchar()) != 3 && input != EOF) {
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

    case 27: { /* Escape code */
      if (getchar() != 91)
        break;

      input = getchar();
      if (input == 68) /* Left arrow */
        buffer_move_left(&buffer);
      else if (input == 67) /* Right arrow */
        buffer_move_right(&buffer);
      else if (input == 65) /* Up arrow */
        buffer_move_up(&buffer);
      else if (input == 66) /* Down arrow */
        buffer_move_down(&buffer);
      else if (input == 90) /* Shift+Tab */
        buffer_unindent(&buffer);
    } break;

    case '\t': { /* Tab */
      buffer_indent(&buffer);
    } break;

    default: {
      if (input >= 32 && (u8) input <= 127)
        buffer_insert(&buffer, input);
    }
    }

    renderer_render_buffer(&renderer, &buffer);
  }

  fputs("\033[H\033[2J", stdout);
  term_reset();
  return 0;
}
