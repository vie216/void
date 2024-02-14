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
} Editor;

Line *editor_line(Editor *editor);
u32   editor_visual_col(Editor *editor);
i32   editor_tab_offset_from_current_line(Editor *editor, Line *line);
void  editor_merge_line_down(Editor *editor, u32 index);
void  editor_insert(Editor *editor, u32 input);
void  editor_insert_new_line(Editor *editor);
void  editor_delete_before_cursor(Editor *editor);
void  editor_delete_at_cursor(Editor *editor);
void  editor_indent(Editor *editor);
void  editor_unindent(Editor *editor);
#if SMART_INDENT
void  editor_smart_indent(Editor *editor);
#endif
#if REMOVE_TRAILING_WS
void  editor_remove_trailing_whitespace(Editor *editor);
#endif
void  editor_move_left(Editor *editor);
void  editor_move_right(Editor *editor);
void  editor_move_up(Editor *editor);
void  editor_move_down(Editor *editor);
void  editor_move_left_word(Editor *editor, bool delete);
void  editor_move_right_word(Editor *editor, bool delete);
void  editor_move_up_paragraph(Editor *editor);
void  editor_move_down_paragraph(Editor *editor);
void  editor_move_to_line_start(Editor *editor);
void  editor_move_to_line_end(Editor *editor);
void  editor_move_to_buffer_start(Editor *editor);
void  editor_move_to_buffer_end(Editor *editor);
void  editor_read_file(Editor *editor, char *path);
void  editor_write_file(Editor *editor);

#endif // BUFFER_H
