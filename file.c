#include "file.h"

long file_bin_size(FILE *fp)
{
    long pos, size;

    // backup the current location of file position indicator
    pos = ftell(fp);
    // move the file position indicator to the end
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    // restore the location of file position indicator
    fseek(fp, pos, SEEK_SET);

    return size;
}

long file_bin_size2(const char *file_name)
{
    struct stat st;
    stat(file_name, &st);

    return st.st_size;
}