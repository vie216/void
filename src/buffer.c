#include <stdio.h>
#include <ctype.h>

#include "buffer.h"
#include "config.h"
#include "fs.h"

Line *buffer_line(Buffer *buffer) {
  return buffer->items + buffer->row;
}

i32 buffer_tab_offset_from_current_line(Buffer *buffer, Line *prev_line) {
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
  buffer->persist_col = buffer->col;
  buffer->dirty = true;
}

void buffer_insert_new_line(Buffer *buffer) {
  u32 rest_len = buffer_line(buffer)->len - buffer->col;
  Line new_line = (Line) {
    .items = malloc(rest_len),
    .len = rest_len,
    .cap = rest_len,
  };

  buffer_line(buffer)->len = buffer->col;
  buffer->row++;
  DA_INSERT(*buffer, new_line, buffer->row);

  Line *line = buffer->items + buffer->row - 1;
  memmove(new_line.items,
          line->items + buffer->col,
          rest_len);

  buffer->col = 0;
  buffer->persist_col = 0;
  buffer->dirty = true;
}

void buffer_merge_line_down(Buffer *buffer, u32 index) {
  Line *line0 = buffer->items + index - 1;
  Line *line1 = buffer->items + index;
  u32 growth_amount = line1->len;

  DA_RESERVE_SPACE(*line0, growth_amount);
  memmove(line0->items + line0->len,
          line1->items,
          growth_amount);
  free(line1->items);
  DA_REMOVE(*buffer, index);

  line0->len += growth_amount;
}

void buffer_delete_before_cursor(Buffer *buffer) {
  if (buffer->col > 0) {
    buffer->col--;
    DA_REMOVE(*buffer_line(buffer), buffer->col);
  } else if (buffer->row > 0) {
    buffer->col = buffer->items[buffer->row - 1].len;
    buffer_merge_line_down(buffer, buffer->row);
    buffer->row--;
  }

  buffer->persist_col = buffer->col;
  buffer->dirty = true;
}

void buffer_delete_at_cursor(Buffer *buffer) {
  if (buffer->col < buffer_line(buffer)->len)
    DA_REMOVE(*buffer_line(buffer), buffer->col);
  else if (buffer->row + 1 < buffer->len)
    buffer_merge_line_down(buffer, buffer->row + 1);

  buffer->dirty = true;
}

void buffer_move_left(Buffer *buffer) {
  if (buffer->col > 0) {
    buffer->col--;
  } else if (buffer->row > 0) {
    buffer->row--;
    buffer->col = buffer_line(buffer)->len;
  }

  buffer->persist_col = buffer->col;
}

void buffer_move_right(Buffer *buffer) {
  if (buffer->col < buffer_line(buffer)->len) {
    buffer->col++;
  } else if (buffer->row + 1 < buffer->len) {
    buffer->row++;
    buffer->col = 0;
  }

  buffer->persist_col = buffer->col;
}

void buffer_move_up(Buffer *buffer) {
  if (buffer->row > 0) {
    Line *prev_line = buffer_line(buffer);

    buffer->row--;
    buffer->col += buffer_tab_offset_from_current_line(buffer, prev_line);
    if (buffer->persist_col > buffer_line(buffer)->len)
      buffer->col = buffer_line(buffer)->len;
    else
      buffer->col = buffer->persist_col;
  }
}

void buffer_move_down(Buffer *buffer) {
  if (buffer->row + 1 < buffer->len) {
    Line *prev_line = buffer_line(buffer);

    buffer->row++;
    buffer->col += buffer_tab_offset_from_current_line(buffer, prev_line);
    if (buffer->persist_col > buffer_line(buffer)->len)
      buffer->col = buffer_line(buffer)->len;
    else
      buffer->col = buffer->persist_col;
  }
}

void buffer_move_left_word(Buffer *buffer, bool delete) {
  bool found_word = false;

  while (buffer->col > 0) {
    bool alnum = isalnum(buffer_line(buffer)->items[buffer->col - 1]);
    if (!alnum && found_word)
      break;

    found_word |= alnum;
    if (delete)
      buffer_delete_before_cursor(buffer);
    else
      buffer_move_left(buffer);
  }
}

void buffer_move_right_word(Buffer *buffer, bool delete) {
  bool found_word = false;

  while (buffer->col < buffer_line(buffer)->len) {
    bool alnum = isalnum(buffer_line(buffer)->items[buffer->col]);
    if (!alnum && found_word)
      break;

    found_word |= alnum;
    if (delete)
      buffer_delete_at_cursor(buffer);
    else
      buffer_move_right(buffer);
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

  buffer->persist_col = buffer->col;
  buffer->dirty = true;
}

void buffer_unindent(Buffer *buffer) {
  Line *line = buffer_line(buffer);

  if (HARD_TABS) {
    if (line->len > 0 && line->items[0] == '\t') {
      DA_REMOVE(*line, 0);
      buffer->col--;
    }
  } else {
    for (u32 i = 0; i < TAB_WIDTH; ++i) {
      if (line->len == 0 || line->items[0] != ' ')
        break;

      DA_REMOVE(*line, 0);
      buffer->col--;
    }
  }

  buffer->persist_col = buffer->col;
  buffer->dirty = true;
}

void buffer_read_file(Buffer *buffer, char *path) {
  u32   file_len = 0;
  char *file_content = read_file(path, &file_len);
  u32   line_start = 0;

  for (u32 i = 0; i <= file_len; ++i) {
    if (i == file_len || file_content[i] == '\n') {
      u32 line_len = i - line_start;
      Line line = (Line) {
        .items = malloc(line_len),
        .len = line_len,
        .cap = line_len,
      };

      DA_APPEND(*buffer, line);
      memmove(line.items,
              file_content + line_start,
              line_len);
      line_start = i + 1;
    }
  }

  buffer->dirty = true;
}

void buffer_write_file(Buffer *buffer, char *path) {
  u32 content_len = buffer->len;
  for (u32 row = 0; row < buffer->len; ++row)
    content_len += buffer->items[row].len;

  char *content = malloc(content_len);
  u32 offset = 0;
  for (u32 row = 0; row < buffer->len; ++row) {
    if (row != 0)
      content[offset++] = '\n';

    Line *line = buffer->items + row;
    memmove(content + offset, line->items, line->len);
    offset += line->len;
  }
  content[content_len - 1] = '\0';

  FILE *file = fopen(path, "w");
  fputs(content, file);
  fclose(file);
  free(content);
}
