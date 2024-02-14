#include <stdio.h>
#include <wctype.h>

#include "editor.h"
#include "fs.h"
#include "wstr.h"

Line *editor_line(Editor *editor) {
  return editor->items + editor->row;
}

u32 editor_visual_col(Editor *editor) {
  u32 visual_col = 0;

  for (u32 col = 0; col < editor->col; ++col)
    if (editor_line(editor)->items[col] == '\t')
      visual_col += TAB_WIDTH;
    else
      visual_col += 1;

  return visual_col;
}

i32 editor_tab_offset_from_current_line(Editor *editor, Line *line) {
  i32 current_offset = 0;
  i32 prev_offset = 0;

  for (u32 col = 0; col < editor->persist_col && col < line->len; ++col)
    if (line->items[col] == '\t')
      prev_offset += TAB_WIDTH - 1;
  for (u32 col = 0; col + current_offset < editor->persist_col + prev_offset; ++col)
    if (editor_line(editor)->items[col] == '\t')
      current_offset += TAB_WIDTH - 1;

  return prev_offset - current_offset;
}

void editor_merge_line_down(Editor *editor, u32 index) {
  Line *line0 = editor->items + index - 1;
  Line *line1 = editor->items + index;

  da_reserve_space((void **) &line0->items, &line0->len,
                   &line0->cap, sizeof(u32), line1->len);
  memmove(line0->items + line0->len,
          line1->items,
          line1->len * sizeof(u32));
  line0->len += line1->len;

  free(line1->items);
  DA_REMOVE(*editor, index);
}

void editor_insert(Editor *editor, u32 input) {
  DA_INSERT(*editor_line(editor), input, editor->col);
  editor->col++;
  editor->persist_col = editor->col;
  editor->dirty = true;
}

void editor_insert_new_line(Editor *editor) {
  u32 rest_len = editor_line(editor)->len - editor->col;
  Line new_line = (Line) {
    .items = malloc(rest_len * sizeof(u32)),
    .len = rest_len,
    .cap = rest_len,
  };

  editor_line(editor)->len = editor->col;
  editor->row++;
  DA_INSERT(*editor, new_line, editor->row);

  Line *line = editor->items + editor->row - 1;
  memmove(new_line.items,
          line->items + editor->col,
          rest_len * sizeof(u32));

  editor->col = 0;
#if SMART_INDENT
  editor_smart_indent(editor);
#endif

	editor->persist_col = editor->col;
	editor->dirty = true;
}

void editor_delete_before_cursor(Editor *editor) {
  if (editor->col > 0) {
    editor->col--;
    DA_REMOVE(*editor_line(editor), editor->col);
    editor->persist_col = editor->col;
    editor->dirty = true;
  } else if (editor->row > 0) {
    editor->col = editor->items[editor->row - 1].len;
    editor_merge_line_down(editor, editor->row);
    editor->row--;
    editor->persist_col = editor->col;
    editor->dirty = true;
  }
}

void editor_delete_at_cursor(Editor *editor) {
  if (editor->col < editor_line(editor)->len) {
    DA_REMOVE(*editor_line(editor), editor->col);
    editor->dirty = true;
  } else if (editor->row + 1 < editor->len) {
    editor_merge_line_down(editor, editor->row + 1);
    editor->dirty = true;
  }
}

void editor_indent(Editor *editor) {
  Line *line = editor_line(editor);
  u32 ident_char;

  if (HARD_TABS) {
    DA_INSERT(*line, '\t', 0);
    editor->col += 1;
    ident_char = '\t';
  } else {
    DA_INSERT_REPEAT(*line, ' ', TAB_WIDTH, 0);
    editor->col += TAB_WIDTH;
    ident_char = ' ';
  }

  while (editor->col < line->len && line->items[editor->col] == ident_char)
      editor->col++;

  editor->persist_col = editor->col;
  editor->dirty = true;
}

void editor_unindent(Editor *editor) {
  Line *line = editor_line(editor);

  if (HARD_TABS) {
    if (line->len > 0 && line->items[0] == '\t') {
      DA_REMOVE(*line, 0);
      if (editor->col > 0)
        editor->col--;
    }
  } else {
    u32 i = 0;
    while (i < TAB_WIDTH &&
           i < line->len &&
           line->items[i] == ' ')
      i++;
    DA_REMOVE_REPEAT(*line, i, 0);
    if (i < editor->col)
      editor->col = editor->col - i;
    else
      editor->col = 0;
  }

  editor->persist_col = editor->col;
  editor->dirty = true;
}

#if SMART_INDENT
void editor_smart_indent(Editor *editor) {
  if (editor->row == 0)
    return;

  Line *prev_line = editor->items + editor->row - 1;
  u32 ident_char;

  if (HARD_TABS)
    ident_char = '\t';
  else
    ident_char = ' ';

  for (u32 i = 0; i < prev_line->len &&
         prev_line->items[i] == ident_char; ++i) {
    DA_INSERT(*editor_line(editor), ident_char, 0);
    editor->col++;
  }
}
#endif

#if REMOVE_TRAILING_WS
void editor_remove_trailing_whitespace(Editor *editor) {
  for (u32 i = 0; i < editor->len; ++i) {
    Line *line = editor->items + i;
    u32 ws_len = 0;

    while (ws_len < line->len) {
      if (!iswspace(line->items[line->len - ws_len - 1]))
        break;
      ws_len++;
    }

    line->len -= ws_len;
  }

  if (editor->col  > editor->items[editor->row].len) {
    editor->col = editor->items[editor->row].len;
    editor->persist_col = editor->col;
    editor->dirty = true;
  }
}
#endif

void editor_move_left(Editor *editor) {
  if (editor->col > 0) {
    editor->col--;
  } else if (editor->row > 0) {
    editor->row--;
    editor->col = editor_line(editor)->len;
  }

  editor->persist_col = editor->col;
}

void editor_move_right(Editor *editor) {
  if (editor->col < editor_line(editor)->len) {
    editor->col++;
  } else if (editor->row + 1 < editor->len) {
    editor->row++;
    editor->col = 0;
  }

  editor->persist_col = editor->col;
}

void editor_move_up(Editor *editor) {
  if (editor->row > 0) {
    Line *prev_line = editor_line(editor);

    editor->row--;
    i32 offset = editor_tab_offset_from_current_line(editor, prev_line);
    if ((i32) editor->persist_col + offset >= 0)
      editor->persist_col += offset;

    if (editor->persist_col > editor_line(editor)->len)
      editor->col = editor_line(editor)->len;
    else
      editor->col = editor->persist_col;
  }
}

void editor_move_down(Editor *editor) {
  if (editor->row + 1 < editor->len) {
    Line *prev_line = editor_line(editor);

    editor->row++;
    i32 offset = editor_tab_offset_from_current_line(editor, prev_line);
    if ((i32) editor->persist_col + offset >= 0)
      editor->persist_col += offset;

    if (editor->persist_col > editor_line(editor)->len)
      editor->col = editor_line(editor)->len;
    else
      editor->col = editor->persist_col;
  }
}

void editor_move_left_word(Editor *editor, bool delete) {
  bool found_word = false;

  while (editor->col > 0) {
    u32 ch = editor_line(editor)->items[editor->col - 1];
    bool is_word = iswalnum(ch);
    if (!is_word && found_word)
      break;

    found_word |= is_word;
    if (delete)
      editor_delete_before_cursor(editor);
    else
      editor_move_left(editor);
  }
}

void editor_move_right_word(Editor *editor, bool delete) {
  bool found_word = false;

  while (editor->col < editor_line(editor)->len) {
    u32 ch = editor_line(editor)->items[editor->col];
    bool is_word = iswalnum(ch);
    if (!is_word && found_word)
      break;

    found_word |= is_word;
    if (delete)
      editor_delete_at_cursor(editor);
    else
      editor_move_right(editor);
  }
}

void editor_move_up_paragraph(Editor *editor) {
  bool found_paragraph = false;

  while (editor->row > 0) {
    bool is_paragraph = editor_line(editor)->len != 0;
    if (!is_paragraph && found_paragraph)
      break;

    found_paragraph |= is_paragraph;
    editor_move_up(editor);
  }

  editor_move_to_line_start(editor);
}

void editor_move_down_paragraph(Editor *editor) {
  bool found_paragraph = false;

  while (editor->row + 1 < editor->len) {
    bool is_paragraph = editor_line(editor)->len != 0;
    if (!is_paragraph && found_paragraph)
      break;

    found_paragraph |= is_paragraph;
    editor_move_down(editor);
  }

  editor_move_to_line_start(editor);
}

void editor_move_to_line_start(Editor *editor) {
  editor->col = 0;
  editor->persist_col = editor->col;
}

void editor_move_to_line_end(Editor *editor) {
  editor->col = editor_line(editor)->len;
  editor->persist_col = editor->col;
}

void editor_move_to_buffer_start(Editor *editor) {
  editor->row = 0;
  editor_move_to_line_start(editor);
}

void editor_move_to_buffer_end(Editor *editor) {
  editor->row = editor->len - 1;
  editor_move_to_line_end(editor);
}

void editor_read_file(Editor *editor, char *path) {
  u32 file_len = 0;
  u8 *file_content = read_file(path, &file_len);

  if (!file_content) {
    DA_APPEND(*editor, (Line) {0});
    return;
  }

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

      DA_APPEND(*editor, line);
      line_start = i + 1;
    }
  }

  free(file_content);
  editor->dirty = true;
}

void editor_write_file(Editor *editor, char *path) {
#if REMOVE_TRAILING_WS
  editor_remove_trailing_whitespace(editor);
#endif

  FILE *file = fopen(path, "w");
  for (u32 row = 0; row < editor->len; ++row) {
    if (row != 0)
      putc('\n', file);

    Line *line = editor->items + row;
    for (u32 col = 0; col < line->len; ++col)
      wputc(line->items[col], file);
  }
  fclose(file);
}
