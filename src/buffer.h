#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>

#include "defs.h"
#include "config.h"

typedef struct {
  u32  *items;
  u32   len, cap;
} Line;

typedef struct {
  Line *items;
  u32   len, cap;
  u32   row, col;
  u32   persist_col;
  bool  dirty;
  char *file_path;
} Buffer;

Line *buffer_line(Buffer *buffer);
u32   buffer_visual_col(Buffer *buffer);
i32   buffer_tab_offset_from_current_line(Buffer *buffer, Line *line);
void  buffer_merge_line_down(Buffer *buffer, u32 index);
void  buffer_insert(Buffer *buffer, u32 input);
void  buffer_insert_new_line(Buffer *buffer);
void  buffer_delete_before_cursor(Buffer *buffer);
void  buffer_delete_at_cursor(Buffer *buffer);
void  buffer_indent(Buffer *buffer);
void  buffer_unindent(Buffer *buffer);
#if SMART_INDENT
void  buffer_smart_indent(Buffer *buffer);
#endif
#if REMOVE_TRAILING_WS
void  buffer_remove_trailing_whitespace(Buffer *buffer);
#endif
void  buffer_move_left(Buffer *buffer);
void  buffer_move_right(Buffer *buffer);
void  buffer_move_up(Buffer *buffer);
void  buffer_move_down(Buffer *buffer);
void  buffer_move_left_word(Buffer *buffer, bool delete);
void  buffer_move_right_word(Buffer *buffer, bool delete);
void  buffer_move_up_paragraph(Buffer *buffer);
void  buffer_move_down_paragraph(Buffer *buffer);
void  buffer_move_to_line_start(Buffer *buffer);
void  buffer_move_to_line_end(Buffer *buffer);
void  buffer_move_to_buffer_start(Buffer *buffer);
void  buffer_move_to_buffer_end(Buffer *buffer);
void  buffer_read_file(Buffer *buffer, char *path);
void  buffer_write_file(Buffer *buffer);

#endif // BUFFER_H
