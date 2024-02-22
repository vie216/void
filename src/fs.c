#include <stdio.h>
#include <stdlib.h>

#include "fs.h"

u8 *read_file(char *path, u32 *len) {
  FILE *file = fopen(path, "r");
  if (!file)
    return NULL;
  fseek(file, 0, SEEK_END);
  *len = ftell(file);
  u8 *content = malloc(*len);
  fseek(file, 0, SEEK_SET);
  fread(content, 1, *len, file);
  fclose(file);

  return content;
}
