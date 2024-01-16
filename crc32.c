#include "global.h"

int crc32_main(char *data)
{
#include "crctable.out"
    const uint32_t CRC_INIT = 0xffffffffL;
    const uint32_t XO_ROT   = 0xffffffffL;
    uint32_t crc = CRC_INIT;

    while (*data) {
        crc = crctable[(crc ^ *data++) & 0xFFL] ^ (crc >> 8);
    }
    crc = crc ^ XO_ROT;

    printf("Input: %s\n", data);
    printf("CRC: 0x%08x\n", crc);

    return 0;
}
