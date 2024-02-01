#include "utility.h"

void *aligned_alloc(size_t size, u32 align)
{
    size_t addr;
    void *ptr, *aligned_ptr;

    ptr = malloc(size + align + sizeof(size_t));
    if (ptr != NULL) {
        addr        = (size_t)ptr + align + sizeof(size_t);
        aligned_ptr = (void *)(addr - (addr % align));
        *((size_t *)aligned_ptr - 1) = (size_t)ptr;
        // // debug
        // printf("%4d %8x %8x %8x %8x\n",
        //     (size >= KB ? size / KB : size), align, ptr, aligned_ptr, *((size_t *)aligned_ptr - 1));
    } else
        return NULL;

    return aligned_ptr;
}