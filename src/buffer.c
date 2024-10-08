#include <stdio.h>
#include <wctype.h>

#include "buffer.h"
#include "wstr.h"
#include "da.h"

Line *buffer_line(Buffer *buffer) {
  return buffer->items + buffer->row;
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

i32 buffer_tab_offset_from_current_line(Buffer *buffer, Line *line) {
  i32 current_offset = 0;
  i32 prev_offset = 0;

  for (u32 col = 0; col < buffer->persist_col && col < line->len; ++col)
    if (line->items[col] == '\t')
      prev_offset += TAB_WIDTH - 1;
  for (u32 col = 0; col + current_offset < buffer->persist_col + prev_offset; ++col)
    if (buffer_line(buffer)->items[col] == '\t')
      current_offset += TAB_WIDTH - 1;

  return prev_offset - current_offset;
}

void buffer_merge_line_down(Buffer *buffer, u32 index) {
  Line *line0 = buffer->items + index - 1;
  Line *line1 = buffer->items + index;

  da_reserve_space((void **) &line0->items, &line0->len,
                   &line0->cap, sizeof(u32), line1->len);
  memmove(line0->items + line0->len,
          line1->items,
          line1->len * sizeof(u32));
  line0->len += line1->len;

  free(line1->items);
  DA_REMOVE(*buffer, index);
}

void buffer_insert(Buffer *buffer, u32 input) {
  DA_INSERT(*buffer_line(buffer), input, buffer->col);
  buffer->col++;
  buffer->persist_col = buffer->col;
  buffer->dirty = true;
}

void buffer_insert_new_line(Buffer *buffer) {
  u32 rest_len = buffer_line(buffer)->len - buffer->col;
  Line new_line = (Line) {
    .items = malloc(rest_len * sizeof(u32)),
    .len = rest_len,
    .cap = rest_len,
  };

  buffer_line(buffer)->len = buffer->col;
  buffer->row++;
  DA_INSERT(*buffer, new_line, buffer->row);

  Line *line = buffer->items + buffer->row - 1;
  memmove(new_line.items,
          line->items + buffer->col,
          rest_len * sizeof(u32));

  buffer->col = 0;
#if SMART_INDENT
  buffer_smart_indent(buffer);
#endif

	buffer->persist_col = buffer->col;
	buffer->dirty = true;
}

void buffer_delete_before_cursor(Buffer *buffer) {
  if (buffer->col > 0) {
    buffer->col--;
    DA_REMOVE(*buffer_line(buffer), buffer->col);
    buffer->persist_col = buffer->col;
    buffer->dirty = true;
  } else if (buffer->row > 0) {
    buffer->col = buffer->items[buffer->row - 1].len;
    buffer_merge_line_down(buffer, buffer->row);
    buffer->row--;
    buffer->persist_col = buffer->col;
    buffer->dirty = true;
  }
}

void buffer_delete_at_cursor(Buffer *buffer) {
  if (buffer->col < buffer_line(buffer)->len) {
    DA_REMOVE(*buffer_line(buffer), buffer->col);
    buffer->dirty = true;
  } else if (buffer->row + 1 < buffer->len) {
    buffer_merge_line_down(buffer, buffer->row + 1);
    buffer->dirty = true;
  }
}

void buffer_indent(Buffer *buffer) {
  Line *line = buffer_line(buffer);
  u32 ident_char;

  if (HARD_TABS) {
    DA_INSERT(*line, '\t', 0);
    buffer->col += 1;
    ident_char = '\t';
  } else {
    DA_INSERT_REPEAT(*line, ' ', TAB_WIDTH, 0);
    buffer->col += TAB_WIDTH;
    ident_char = ' ';
  }

  while (buffer->col < line->len && line->items[buffer->col] == ident_char)
      buffer->col++;

  buffer->persist_col = buffer->col;
  buffer->dirty = true;
}

void buffer_unindent(Buffer *buffer) {
  Line *line = buffer_line(buffer);

  if (HARD_TABS) {
    if (line->len > 0 && line->items[0] == '\t') {
      DA_REMOVE(*line, 0);
      if (buffer->col > 0)
        buffer->col--;
    }
  } else {
    u32 i = 0;
    while (i < TAB_WIDTH &&
           i < line->len &&
           line->items[i] == ' ')
      i++;
    DA_REMOVE_REPEAT(*line, i, 0);
    if (i < buffer->col)
      buffer->col = buffer->col - i;
    else
      buffer->col = 0;
  }

  buffer->persist_col = buffer->col;
  buffer->dirty = true;
}

#if SMART_INDENT
void buffer_smart_indent(Buffer *buffer) {
  if (buffer->row == 0)
    return;

  Line *prev_line = buffer->items + buffer->row - 1;
  u32 ident_char;

  if (HARD_TABS)
    ident_char = '\t';
  else
    ident_char = ' ';

  for (u32 i = 0; i < prev_line->len &&
         prev_line->items[i] == ident_char; ++i) {
    DA_INSERT(*buffer_line(buffer), ident_char, 0);
    buffer->col++;
  }
}
#endif

#if REMOVE_TRAILING_WS
void buffer_remove_trailing_whitespace(Buffer *buffer) {
  for (u32 i = 0; i < buffer->len; ++i) {
    Line *line = buffer->items + i;
    u32 ws_len = 0;

    while (ws_len < line->len) {
      if (!iswspace(line->items[line->len - ws_len - 1]))
        break;
      ws_len++;
    }

    line->len -= ws_len;
  }

  if (buffer->col  > buffer->items[buffer->row].len) {
    buffer->col = buffer->items[buffer->row].len;
    buffer->persist_col = buffer->col;
    buffer->dirty = true;
  }
}
#endif

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
    i32 offset = buffer_tab_offset_from_current_line(buffer, prev_line);
    if ((i32) buffer->persist_col + offset >= 0)
      buffer->persist_col += offset;

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
    i32 offset = buffer_tab_offset_from_current_line(buffer, prev_line);
    if ((i32) buffer->persist_col + offset >= 0)
      buffer->persist_col += offset;

    if (buffer->persist_col > buffer_line(buffer)->len)
      buffer->col = buffer_line(buffer)->len;
    else
      buffer->col = buffer->persist_col;
  }
}

void buffer_move_left_word(Buffer *buffer, bool delete) {
  bool found_word = false;

  while (buffer->col > 0) {
    u32 ch = buffer_line(buffer)->items[buffer->col - 1];
    bool is_word = iswalnum(ch);
    if (found_word && !is_word)
      break;

    found_word |= is_word;
    if (delete)
      buffer_delete_before_cursor(buffer);
    else
      buffer_move_left(buffer);
  }
}

void buffer_move_right_word(Buffer *buffer, bool delete) {
  bool found_word = false;

  while (buffer->col < buffer_line(buffer)->len) {
    u32 ch = buffer_line(buffer)->items[buffer->col];
    bool is_word = iswalnum(ch);
    if (found_word && !is_word)
      break;

    found_word |= is_word;
    if (delete)
      buffer_delete_at_cursor(buffer);
    else
      buffer_move_right(buffer);
  }
}

void buffer_move_up_paragraph(Buffer *buffer) {
  bool found_paragraph = false;

  while (buffer->row > 0) {
    bool is_paragraph = buffer_line(buffer)->len != 0;
    if (!is_paragraph && found_paragraph)
      break;

    found_paragraph |= is_paragraph;
    buffer_move_up(buffer);
  }

  buffer_move_to_line_start(buffer);
}

void buffer_move_down_paragraph(Buffer *buffer) {
  bool found_paragraph = false;

  while (buffer->row + 1 < buffer->len) {
    bool is_paragraph = buffer_line(buffer)->len != 0;
    if (!is_paragraph && found_paragraph)
      break;

    found_paragraph |= is_paragraph;
    buffer_move_down(buffer);
  }

  buffer_move_to_line_start(buffer);
}

void buffer_move_to_line_start(Buffer *buffer) {
  buffer->col = 0;
  buffer->persist_col = buffer->col;
}

void buffer_move_to_line_end(Buffer *buffer) {
  buffer->col = buffer_line(buffer)->len;
  buffer->persist_col = buffer->col;
}

void buffer_move_to_buffer_start(Buffer *buffer) {
  buffer->row = 0;
  buffer_move_to_line_start(buffer);
}

void buffer_move_to_buffer_end(Buffer *buffer) {
  buffer->row = buffer->len - 1;
  buffer_move_to_line_end(buffer);
}

void buffer_read_file(Buffer *buffer, char *path) {
  u32 file_len;
  u8 *file_content;

  buffer->file_path = path;

  FILE *file = fopen(path, "r");
  if (!file) {
    DA_APPEND(*buffer, (Line) {0});
    buffer->message = "error opening file";
    return;
  }

  fseek(file, 0, SEEK_END);
  file_len = ftell(file);
  file_content = malloc(file_len);
  fseek(file, 0, SEEK_SET);
  fread(file_content, 1, file_len, file);
  fclose(file);

  u32 line_start = 0;
  for (u32 i = 0; i <= file_len; ++i) {
    if (file_content[i] == '\n' || i == file_len) {
      WStr wstr = utow((Str) {
          .ptr = file_content + line_start,
          .len = i - line_start,
        });
      Line line = (Line) {
        .items = wstr.ptr,
        .len = wstr.len,
        .cap = wstr.len,
      };

      DA_APPEND(*buffer, line);
      line_start = i + 1;
    }
  }

  free(file_content);
  buffer->dirty = true;
}

void buffer_write_file(Buffer *buffer) {
#if REMOVE_TRAILING_WS
  buffer_remove_trailing_whitespace(buffer);
#endif

  if (!buffer->file_path) {
    buffer->message = "no opened file";
    return;
  }

  FILE *file = fopen(buffer->file_path, "w");
  if (!file) {
    buffer->message = "error saving file";
    return;
  }

  for (u32 row = 0; row < buffer->len; ++row) {
    if (row != 0)
      putc('\n', file);

    Line *line = buffer->items + row;
    for (u32 col = 0; col < line->len; ++col)
      wputc(line->items[col], file);
  }
  fclose(file);

  buffer->message = "saved";
}
