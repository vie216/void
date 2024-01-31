#include <stdio.h>

#include "buffer.h"
#include "config.h"

Line *buffer_line(Buffer *buffer) {
  return buffer->items + buffer->row;
}

i32 buffer_visual_offset_between_lines(Buffer *buffer, Line *prev_line) {
  i32 offset = 0;

  for (u32 col = 0; col < buffer->col; ++col)
    if (buffer_line(buffer)->items[col] == '\t')
      offset -= TAB_WIDTH - 1;
  for (u32 col = 0; col < buffer->col && col < prev_line->len; ++col)
    if (prev_line->items[col] == '\t')
      offset += TAB_WIDTH - 1;

  return offset;
}

u32 buffer_visual_col(Buffer *buffer) {
  u32 visual_col = 0;

  for (u32 col = 0; col < buffer->col; ++col)
    if (buffer_line(buffer)->items[col] == '\t')
      visual_col += TAB_WIDTH;
    else
      visual_col += 1;

  return visual_col;
}

void buffer_insert(Buffer *buffer, char input) {
  DA_INSERT(*buffer_line(buffer), input, buffer->col);
  buffer->col++;
  buffer_line(buffer)->dirty = true;
}

void buffer_insert_new_line(Buffer *buffer) {
  u32 rest_len = buffer_line(buffer)->len - buffer->col;
  Line new_line = (Line) {
    .items = malloc(rest_len * sizeof(char)),
    .len = rest_len,
    .cap = rest_len,
  };

  buffer_line(buffer)->len = buffer->col;
  buffer->row++;
  DA_INSERT(*buffer, new_line, buffer->row);

  Line *line = buffer->items + buffer->row - 1;
  memmove(new_line.items,
          line->items + buffer->col,
          sizeof(char) * rest_len);

  buffer->col = 0;

  for (u32 i = buffer->row - 1; i < buffer->len; ++i)
    buffer->items[i].dirty = true;
}

void buffer_delete_before_cursor(Buffer *buffer) {
  if (buffer->col > 0) {
    buffer->col--;
    DA_REMOVE(*buffer_line(buffer), buffer->col);
    buffer_line(buffer)->dirty = true;
  } else if (buffer->row > 0) {
    Line *prev_line = buffer->items + buffer->row - 1;
    u32 growth_amount = buffer_line(buffer)->len;

    DA_RESERVE_SPACE(*prev_line, growth_amount);
    memmove(prev_line->items + prev_line->len,
            buffer_line(buffer)->items,
            sizeof(char) * growth_amount);
    prev_line->len += growth_amount;
    DA_REMOVE(*buffer, buffer->row);

    buffer->row--;
    buffer->col = prev_line->len;

    for (u32 i = buffer->row; i < buffer->len; ++i)
      buffer->items[i].dirty = true;
  }
}

void buffer_move_left(Buffer *buffer) {
  if (buffer->col > 0) {
    buffer->col--;
  } else if (buffer->row > 0) {
    buffer->row--;
    buffer->col = buffer_line(buffer)->len;
  }
}

void buffer_move_right(Buffer *buffer) {
  if (buffer->col < buffer_line(buffer)->len) {
    buffer->col++;
  } else if (buffer->row + 1 < buffer->len) {
    buffer->row++;
    buffer->col = 0;
  }
}

void buffer_move_up(Buffer *buffer) {
  if (buffer->row > 0) {
    Line *prev_line = buffer_line(buffer);

    buffer->row--;
    buffer->col += buffer_visual_offset_between_lines(buffer, prev_line);
    if (buffer->col > buffer_line(buffer)->len)
      buffer->col = buffer_line(buffer)->len;
  }
}

void buffer_move_down(Buffer *buffer) {
  if (buffer->row + 1 < buffer->len) {
    Line *prev_line = buffer_line(buffer);

    buffer->row++;
    buffer->col += buffer_visual_offset_between_lines(buffer, prev_line);
    if (buffer->col > buffer_line(buffer)->len)
      buffer->col = buffer_line(buffer)->len;
  }
}

void buffer_indent(Buffer *buffer) {
  if (HARD_TABS) {
    DA_INSERT(*buffer_line(buffer), '\t', 0);
    buffer->col++;
  } else {
    for (u32 i = 0; i < TAB_WIDTH; ++i)
      DA_INSERT(*buffer_line(buffer), ' ', 0);
    buffer->col += TAB_WIDTH;
  }

  buffer_line(buffer)->dirty = true;
}

void buffer_unindent(Buffer *buffer) {
  if (HARD_TABS) {
    if (buffer_line(buffer)->len > 0) {
      DA_REMOVE(*buffer_line(buffer), 0);
      buffer->col--;
    }
  } else {
    for (u32 i = 0; i < TAB_WIDTH; ++i) {
      if (buffer_line(buffer)->len == 0)
        break;

      DA_REMOVE(*buffer_line(buffer), 0);
      buffer->col--;
    }
  }

  buffer_line(buffer)->dirty = true;
}
