#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "term.h"
#include "defs.h"
#include "buffer.h"
#include "config.h"

typedef struct {
  u32  len, height;
  u32  visual_row;
  bool dirty;
} LineInfo;

/* TODO: scrolling */
typedef struct {
  char     *buffer;
  LineInfo *line_infos;
  u32       rows, cols, cap;
  u32       visual_row;
} Renderer;

void renderer_render_line(Renderer *renderer, Line *line,
                          u32 _row, u32 *visual_row) {
  LineInfo *info = renderer->line_infos + _row;
  u32 height = 1;
  u32 col = 0;

  while (col < line->len && col < renderer->cap) {
    if (col != 0 && col % renderer->cols == 0)
      height++;

    if (line->items[col] == '\t') {
      if (renderer->cap - col > TAB_WIDTH)
        sprintf(renderer->buffer + _row * renderer->cols + col, TAB_STR);
      col += TAB_WIDTH;
    } else {
      if (line->items[col] != renderer->buffer[_row * renderer->cols + col]) {
        info->dirty = true;
        renderer->buffer[_row * renderer->cols + col] = line->items[col];
      }
      col++;
    }
  }

  if (line->len < info->len) {
    info->dirty = true;
    memset(renderer->buffer + _row * renderer->cols + line->len,
           ' ', info->len - line->len);
  }

  info->dirty = info->dirty ||
    height != info->height ||
    *visual_row != info->visual_row;
  info->len = line->len;
  info->height = height;
  info->visual_row = *visual_row;
  *visual_row += height;
}

#ifndef NDEBUG
void renderer_render_debug_info(Renderer *renderer, bool full_redraw) {
  printf("\033[%d;0H\033[K", renderer->rows);
  for (u32 row = 0; row < renderer->rows; ++row)
    if (renderer->line_infos[row].dirty || full_redraw)
      printf("%d ", row);
}
#endif

void renderer_render_buffer(Renderer *renderer, Buffer *buffer) {
  struct winsize ws;
  bool full_redraw = false;

  ioctl(1, TIOCGWINSZ, &ws);
  if (renderer->rows != ws.ws_row || renderer->cols != ws.ws_col) {
    renderer->buffer = realloc(renderer->buffer, ws.ws_row * ws.ws_col);
    renderer->line_infos = realloc(renderer->line_infos, ws.ws_row * sizeof(LineInfo));
    renderer->rows = ws.ws_row;
    renderer->cols = ws.ws_col;
    renderer->cap = renderer->rows * renderer->cols;

    memset(renderer->buffer, ' ', ws.ws_row * ws.ws_col);
    full_redraw = true;
  }

  u32 visual_row = 0;
  for (u32 row = 0; row < buffer->len && row < renderer->rows; ++row)
    renderer_render_line(renderer, buffer->items + row, row, &visual_row);

#ifndef NDEBUG
  renderer_render_debug_info(renderer, full_redraw);
#endif

  fputs("\033[H", stdout);
  for (u32 row = 0; row < renderer->rows; ++row) {
    if (row != 0)
      putc('\n', stdout);
    if (renderer->line_infos[row].dirty || full_redraw)
      for (u32 col = 0; col < renderer->cols; ++col)
        putc(renderer->buffer[row * renderer->cols + col], stdout);
    renderer->line_infos[row].dirty = false;
  }

  if (visual_row < renderer->visual_row) {
    for (u32 row = visual_row; row < renderer->visual_row; ++row) {
      if (row != visual_row)
        putc('\n', stdout);
      fputs("\033[K", stdout);
    }
  }

  renderer->visual_row = visual_row;

  printf("\033[%d;%dH",
         buffer->row + 1,
         buffer_visual_col(buffer) + 1);
  fflush(stdout);
}

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
