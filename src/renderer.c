#include <stdio.h>
#include <sys/ioctl.h>

#include "renderer.h"
#include "config.h"

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
