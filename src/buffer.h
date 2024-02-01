#ifndef BUFFER_H
#define BUFFER_H

#include "defs.h"

typedef struct {
  char *items;
  u32   len, cap;
} Line;

typedef struct {
  Line *items;
  u32   len, cap;
  u32   row, col;
  bool  dirty;
} Buffer;

u32  buffer_visual_col(Buffer *buffer);
void buffer_insert(Buffer *buffer, char input);
void buffer_insert_new_line(Buffer *buffer);
void buffer_delete_before_cursor(Buffer *buffer);
void buffer_move_left(Buffer *buffer);
void buffer_move_right(Buffer *buffer);
void buffer_move_up(Buffer *buffer);
void buffer_move_down(Buffer *buffer);
void buffer_indent(Buffer *buffer);
void buffer_unindent(Buffer *buffer);
void buffer_process_input(Buffer *buffer, u8 input);
void buffer_read_file(Buffer *buffer, char *path);
void buffer_write_file(Buffer *buffer, char *path);

#endif // BUFFER_H
