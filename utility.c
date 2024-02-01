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
        // printf("[%s](%x,%x)\n", __FUNCTION__, ptr, aligned_ptr);
    } else
        return NULL;

    return aligned_ptr;
}

void aligned_free(void *aligned_ptr)
{
    // printf("[%s](%x,%x)\n", __FUNCTION__, *((size_t *)aligned_ptr - 1), aligned_ptr);
    free((void *)(*((size_t *)aligned_ptr - 1)));
}
