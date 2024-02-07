#ifndef FS_H
#define FS_H

#include "defs.h"

u8 *read_file(char *path, u32 *len);
void write_file(char *path, char *content);

#endif // FS_H
