#ifndef RENDERER_H
#define RENDERER_H

#include <stdbool.h>

#include "defs.h"
#include "editor.h"

typedef struct {
  u32  len;
  bool dirty;
} LineInfo;

typedef struct {
  u32      *buffer;
  LineInfo *line_infos;
  u32       rows, cols, cap;
  u32       scroll;
} Renderer;

void renderer_render_line(Renderer *renderer, Editor *editor,
                          u32 row, bool full_redraw);
#ifdef DEBUG
void renderer_render_debug_info(Renderer *renderer, bool full_redraw);
#endif
void renderer_render_status_bar(Renderer *renderer, Editor *editor);
void renderer_render_editor(Renderer *renderer, Editor *editor);

#endif // RENDERER_H
