#include <stdio.h>
#include <sys/ioctl.h>
#include <math.h>

#include "renderer.h"
#include "config.h"
#include "wstr.h"

void renderer_render_line(Renderer *renderer, Buffer *buffer,
                          u32 row, bool full_redraw) {
  LineInfo *info = renderer->line_infos + row;
  Line *line = buffer->items + row + renderer->scroll;
  u32 rcol = 0, bcol = 0;

  if (row + renderer->scroll >= buffer->len)
    goto end;

  while (bcol < line->len && rcol < renderer->cols) {
    u32 offset = row * renderer->cols + rcol;

    if (line->items[bcol] == WIDEN('\t')) {
      for (u32 i = 0; i < TAB_WIDTH &&
             i + offset < renderer->cap; ++i)
        renderer->buffer[i + offset] = WIDEN(' ');
      rcol += TAB_WIDTH;
    } else {
      if (line->items[bcol] != renderer->buffer[offset]) {
        info->dirty = true;
        renderer->buffer[offset] = line->items[bcol];
      }
      rcol++;
    }

    bcol++;
  }

 end:
  u32 offset = row * renderer->cols + rcol;
  if (full_redraw && rcol < renderer->cols)
    memset(renderer->buffer + offset,
           0, (renderer->cols - rcol) * sizeof(u32));
  else if (rcol < info->len)
    memset(renderer->buffer + offset,
           0, (info->len - rcol) * sizeof(u32));

  info->dirty |= rcol != info->len;
  info->len = rcol;
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
    if (renderer->cap != 0) {
      free(renderer->buffer);
      free(renderer->line_infos);
    }
    renderer->buffer = malloc(ws.ws_row * ws.ws_col * sizeof(u32));
    renderer->line_infos = malloc(ws.ws_row * sizeof(LineInfo));
    renderer->rows = ws.ws_row;
    renderer->cols = ws.ws_col;
    renderer->cap = renderer->rows * renderer->cols;

    memset(renderer->buffer, 0, renderer->cap * sizeof(u32));
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

  if (buffer->dirty || full_redraw)
    for (u32 row = 0; row + 1 < renderer->rows; ++row)
      renderer_render_line(renderer, buffer, row, full_redraw);

  fputs("\033[H", stdout);
  for (u32 row = 0; row + 1 < renderer->rows; ++row) {
    if (row != 0)
      putchar('\n');

    if (renderer->line_infos[row].dirty || full_redraw) {
      fputs("\033[K", stdout);
      for (u32 col = 0; col < renderer->cols; ++col) {
        u32 ch = renderer->buffer[row * renderer->cols + col];
        if (ch == 0)
          break;
        wputc(ch, stdout);
      }
    }

#ifdef NDEBUG
    renderer->line_infos[row].dirty = false;
#endif
  }

  buffer->dirty = false;

  renderer_render_status_bar(renderer, buffer);

#ifndef NDEBUG
  renderer_render_debug_info(renderer, full_redraw);
  for (u32 row = 0; row < renderer->rows; ++row)
    renderer->line_infos[row].dirty = false;
#endif

  printf("\033[%d;%dH",
         buffer->row - renderer->scroll + 1,
         buffer_visual_col(buffer) + 1);
  fflush(stdout);
}
