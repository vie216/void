#include <stdio.h>
#include <sys/ioctl.h>
#include <math.h>

#include "renderer.h"
#include "config.h"

void renderer_render_line(Renderer *renderer, Buffer *buffer, u32 _row) {
  if (_row + renderer->scroll >= buffer->len)
    return;

  LineInfo *info = renderer->line_infos + _row;
  Line *line = buffer->items + _row + renderer->scroll;
  u32 height = 1;
  u32 col = 0;

  while (col < line->len && col < renderer->cap) {
    u32 offset = _row * renderer->cols + col;

    if (col != 0 && col % renderer->cols == 0)
      height++;

    if (line->items[col] == '\t') {
      if (renderer->cap - col > TAB_WIDTH)
        sprintf(renderer->buffer + offset, TAB_STR);
      col += TAB_WIDTH;
    } else {
      if (line->items[col] != renderer->buffer[offset]) {
        info->dirty = true;
        renderer->buffer[offset] = line->items[col];
      }
      col++;
    }
  }

  if (line->len < info->len) {
    u32 offset = _row * renderer->cols + line->len;

    info->dirty = true;
    memset(renderer->buffer + offset,
           ' ', info->len - line->len);
  }

  info->dirty = info->dirty ||
    line->len != info->len ||
    height != info->height;
  info->len = line->len;
  info->height = height;
}

#ifndef NDEBUG
void renderer_render_debug_info(Renderer *renderer, bool full_redraw) {
  printf("\033[%d;1H\033[K", renderer->rows - 1);
  u32 col = 0;
  for (u32 row = 0; row < renderer->rows && col + 3 < renderer->cols; ++row)
    if (renderer->line_infos[row].dirty || full_redraw)
      col += printf("%d ", row);
}
#endif

#define UINT_LEN(num) (num < 10 ? 1 : (u32) (log10(num) + 1))

void renderer_render_status_bar(Renderer *renderer, Buffer *buffer) {
  u32 row = buffer->row + 1;
  u32 col = buffer->col + 1;
  printf("\033[%d;%dH\033[2K%d:%d", renderer->rows,
         renderer->cols - UINT_LEN(row) - UINT_LEN(col) - 2,
         row, col);
}

void renderer_render_buffer(Renderer *renderer, Buffer *buffer) {
  struct winsize ws;
  bool full_redraw = false;

  ioctl(1, TIOCGWINSZ, &ws);
  if (renderer->rows != ws.ws_row || renderer->cols != ws.ws_col) {
    if (renderer->cap == 0) {
      free(renderer->buffer);
      free(renderer->line_infos);
    }
    renderer->buffer = malloc(ws.ws_row * ws.ws_col);
    renderer->line_infos = malloc(ws.ws_row * sizeof(LineInfo));
    renderer->rows = ws.ws_row;
    renderer->cols = ws.ws_col;
    renderer->cap = renderer->rows * renderer->cols;

    memset(renderer->buffer, ' ', renderer->cap);
    memset(renderer->line_infos, 0, ws.ws_row * sizeof(LineInfo));
    full_redraw = true;
  }

  if (buffer->row < renderer->scroll) {
    renderer->scroll = buffer->row;
    full_redraw = true;
#ifdef NDEBUG
  } else if (buffer->row > renderer->scroll + renderer->rows - 2) {
    renderer->scroll = buffer->row - renderer->rows + 2;
#else
  } else if (buffer->row > renderer->scroll + renderer->rows - 3) {
    renderer->scroll = buffer->row - renderer->rows + 3;
#endif
    full_redraw = true;
  }

  fputs("\033[H", stdout);
  for (u32 row = 0; row + 1 < renderer->rows && row < buffer->len; ++row) {
    if (buffer->dirty || full_redraw)
      renderer_render_line(renderer, buffer, row);

    if (row != 0)
      putc('\n', stdout);

    if (renderer->line_infos[row].dirty || full_redraw)
      for (u32 col = 0; col < renderer->cols; ++col)
        putc(renderer->buffer[row * renderer->cols + col], stdout);

#ifdef NDEBUG
    renderer->line_infos[row].dirty = false;
#endif
  }

  if (buffer->len < renderer->prev_buffer_len)
    for (u32 row = buffer->len; row < renderer->prev_buffer_len; ++row)
      fputs("\n\033[K", stdout);

  buffer->dirty = false;
  renderer->prev_buffer_len = buffer->len;

  renderer_render_status_bar(renderer, buffer);

#ifndef NDEBUG
  renderer_render_debug_info(renderer, full_redraw);
  for (u32 row = 0; row < renderer->rows && row < buffer->len; ++row)
    renderer->line_infos[row].dirty = false;
#endif

  printf("\033[%d;%dH",
         buffer->row - renderer->scroll + 1,
         buffer_visual_col(buffer) + 1);
  fflush(stdout);
}