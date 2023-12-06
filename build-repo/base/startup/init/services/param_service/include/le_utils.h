#ifndef LE_UTILS_H
#define LE_UTILS_H
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "beget_ext.h"

void SetNoBlock(int fd);
void SetCloseExec(int fd);
int MakeDirRecursive(char* dir, mode_t mode);

#endif // LE_UTILS_H
