#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "term.h"
#include "defs.h"
#include "buffer.h"
#include "config.h"

void render_line(Buffer *buffer, Line *line, u32 row) {
  printf("\033[K");
  if (row < buffer->len) {
    for (u32 col = 0; col < line->len; ++col)
      if (line->items[col] == '\t')
        fputs(TAB_STR, stdout);
      else
        putc(line->items[col], stdout);
  }
  line->dirty = false;
}

/* TODO: clear the whole screen */
void render(Buffer *buffer) {
  bool full_redraw = false;
  /* u32 width; /\* get it from ioctl *\/ */

  /* static u32 prev_width = 0; */

  fputs("\033[H", stdout);
  for (u32 row = 0; row < /* width */buffer->len; ++row) {
    Line *line = buffer->items + row;
    if (line->dirty || full_redraw)
      render_line(buffer, line, row);
    putc('\n', stdout);
  }

  printf("\033[%d;%dH",
         buffer->row + 1,
         buffer_visual_col(buffer) + 1);
  fflush(stdout);

  /* prev_width = width; */
}

int main(void) {
  term_init();
  fputs("\033[H\033[2J", stdout);

  Buffer buffer = {0};
  char input;

  DA_APPEND(buffer, ((Line) {0}));

  while ((input = getchar()) != 3 && input != EOF) {
    switch (input) {
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

    render(&buffer);
  }

  fputs("\033[H\033[2J", stdout);
  term_reset();
  return 0;
}
