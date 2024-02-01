#include <stdio.h>

#include "fs.h"

char *read_file(char *path, u32 *len) {
  FILE *file = fopen(path, "r");
  if (!file)
    return NULL;
  fseek(file, 0, SEEK_END);
  *len = ftell(file);
  char *content = malloc(*len);
  fseek(file, 0, SEEK_SET);
  fread(content, 1, *len, file);
  fclose(file);

  return content;
}

void write_file(char *path, char *content) {
  FILE *file = fopen(path, "w");
  fputs(content, file);
  fclose(file);
}
