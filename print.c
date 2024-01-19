#include "print.h"

void print_buf(const void *buf, u32 size, char *title, int mode)
{
    u32 i, addr;
    u32 j, l, p;

    if (title) {
        printf("%s\n", title);
    }

    if (size == 0) {
        printf("[%s][%d] SIZE is zero.\n\n", __FUNCTION__, __LINE__);
        return;
    }

    if (mode)
        addr = 0;
    else
        addr = (u32)buf;

    printf("0x%08X: ", addr);

    for (i = 0; i < size; i++) {
        if (i) {
            if (i % 16 == 0) {
                printf("   ");
                for (j = i - 16; j < i; j++) {
                    if (((u8 *)buf)[j] < 0x20 || ((u8 *)buf)[j] > 0x7E) {
                        printf(".");
                    }
                    else {
                        printf("%c", ((u8 *)buf)[j]);
                    }
                }
                printf("\n0x%08X: ", addr + i);
                p = 1;
            }
            else if (i % 8 == 0) {
                printf(" ");
            }
        }
        printf("%02X ", ((u8 *)buf)[i]);
        p = 0;
    }

    if (!p) {
        if (i & 0xF) {
            for (j = i; j < i + 16 - (i & 0xF); j++) {
                if (j % 8 == 0) {
                    printf(" ");
                }

                printf("   ");
            }
        }

        printf("   ");
        for (j = i - (i & 0xF); j < i; j++) {
            printf(".");
        }

        if (i & 0xF) {
            for (j = i; j < i + 16 - (i & 0xF); j++) {
                if (((u8 *)buf)[j] < 0x20 || ((u8 *)buf)[j] > 0x7E) {
                    printf(".");
                }
                else {
                    printf("%c", ((u8 *)buf)[j]);
                }
            }
        }
        else {
            for (j = i - 16; j < i; j++) {
                if (((u8 *)buf)[j] < 0x20 || ((u8 *)buf)[j] > 0x7E) {
                    printf(".");
                }
                else {
                    printf("%c", ((u8 *)buf)[j]);
                }
            }
        }
    }

    printf("\n\n");
}