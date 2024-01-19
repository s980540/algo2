#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <sys/stat.h>

long file_bin_size(FILE *fp);
long file_bin_size2(const char *file_name);



#endif // ~ FILE_H