#include <stdio.h>
#include <locale.h>

#include "term.h"
#include "defs.h"
#include "editor.h"
#include "renderer.h"
#include "wstr.h"
#include "keymap.h"

#define NONE(key_) ((decoded.key & (key_)) && !decoded.ctrl && !decoded.alt)
#define CTRL(key_) ((decoded.key & (key_)) &&  decoded.ctrl && !decoded.alt)
#define ALT(key_)  ((decoded.key & (key_)) && !decoded.ctrl &&  decoded.alt)
#define CHAR(ch_)      ((decoded.key & KEY_CHAR) && decoded.ch == ch_ && !decoded.ctrl && !decoded.alt)
#define CTRL_CHAR(ch_) ((decoded.key & KEY_CHAR) && decoded.ch == ch_ &&  decoded.ctrl && !decoded.alt)
#define ALT_CHAR(ch_)  ((decoded.key & KEY_CHAR) && decoded.ch == ch_ && !decoded.ctrl &&  decoded.alt)

bool process_input(Editor *editor, u32 input) {
  Input decoded = decode_input_from_stdin(input);
  if (decoded.key == KEY_NONE)
    return true;

  if (KB_QUIT)
    return false;

  /*     General      */
  if (KB_SAVE)
    editor_write_file(editor);
  /*     Editing      */
  else if (KB_NEW_LINE)
    editor_insert_new_line(editor);
  else if (KB_DEL_PREV)
    editor_delete_before_cursor(editor);
  else if (KB_DEL_PREV_WORD)
    editor_move_left_word(editor, true);
  else if (KB_DEL_NEXT)
    editor_delete_at_cursor(editor);
  else if (KB_DEL_NEXT_WORD)
    editor_move_right_word(editor, true);
  else if (KB_INDENT)
    editor_indent(editor);
  else if (KB_UNINDENT)
    editor_unindent(editor);
  /*    Navigation    */
  else if (KB_LEFT)
    editor_move_left(editor);
  else if (KB_RIGHT)
    editor_move_right(editor);
  else if (KB_UP)
    editor_move_up(editor);
  else if (KB_DOWN)
    editor_move_down(editor);
  else if (KB_LEFT_WORD)
    editor_move_left_word(editor, false);
  else if (KB_RIGHT_WORD)
    editor_move_right_word(editor, false);
  else if (KB_UP_PARAGRAPH)
    editor_move_up_paragraph(editor);
  else if (KB_DOWN_PARAGRAPH)
    editor_move_down_paragraph(editor);
  else if (KB_LINE_START)
    editor_move_to_line_start(editor);
  else if (KB_LINE_END)
    editor_move_to_line_end(editor);
  else if (KB_FILE_START)
    editor_move_to_buffer_start(editor);
  else if (KB_FILE_END)
    editor_move_to_buffer_end(editor);
  else if (decoded.key == KEY_CHAR)
    if ((input >= 32 && input <= 126) || input > 127)
      editor_insert(editor, input);

  return true;
}

int main(i32 argc, char **argv) {
  term_init();
  setlocale(LC_ALL, "");
  fputs("\033[H\033[2J", stdout);

  Editor editor = {0};
  Renderer renderer = {0};
  char *file_path = NULL;

  if (argc > 1) {
    file_path = argv[1];
    editor_read_file(&editor, file_path);
  } else {
    DA_APPEND(editor, ((Line) {0}));
  }

  u32 input;
  renderer_render_editor(&renderer, &editor);
  while ((input = wgetc(stdin)) != (u32) EOF) {
    if (!process_input(&editor, input))
      break;
    renderer_render_editor(&renderer, &editor);
  }

  fputs("\033[H\033[2J", stdout);
  term_reset();
  return 0;
}
