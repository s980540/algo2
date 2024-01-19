#include "ucode.h"
#include "print.h"
#include "file.h"

#include "nifc_cmd_bin.out"
#include "nops_op_bin.out"

typedef struct _ucode_header_info_t {
    const u8 *buf;
    u16 size;
    void (*info)(void);
} ucode_header_info_t;

typedef struct  _ucode_bin_header_t
{
    ucode_header_info_t ucode_bin_desc;
    ucode_header_info_t nifc_cmd_bin_ofst;
    ucode_header_info_t nifc_cmd_bin_size;
    ucode_header_info_t nops_op_bin_ofst;
    ucode_header_info_t nops_op_bin_size;
} ucode_bin_header_t;

typedef struct _nand_op_header_t {
    ucode_header_info_t nand_type_desc;
    ucode_header_info_t fc_gen;
    ucode_header_info_t date;
    ucode_header_info_t ver;
} nand_op_header_t;

void ucode_bin_header_desc(void);
void ucode_bin_header_nifc_cmd_bin_ofst(void);
void ucode_bin_header_nifc_cmd_bin_size(void);
void ucode_bin_header_nops_op_bin_ofst(void);
void ucode_bin_header_nops_op_bin_size(void);

void nand_op_header_nand_type(void);
void nand_op_header_fc_gen(void);
void nand_op_header_date(void);
void nand_op_header_ver(void);

u8 *m_ucode_bin = NULL;

ucode_bin_header_t ucode_bin_header = {
    .ucode_bin_desc = {0, 16, ucode_bin_header_desc},
    .nifc_cmd_bin_ofst = {0, 4, ucode_bin_header_nifc_cmd_bin_ofst},
    .nifc_cmd_bin_size = {0, 4, ucode_bin_header_nifc_cmd_bin_size},
    .nops_op_bin_ofst = {0, 4, ucode_bin_header_nops_op_bin_ofst},
    .nops_op_bin_size = {0, 4, ucode_bin_header_nops_op_bin_size},
};

nand_op_header_t nand_op_header = {
    .nand_type_desc = {&nops_op_bin[0], 16, nand_op_header_nand_type},
    .fc_gen = {&nops_op_bin[16], 4, nand_op_header_fc_gen},
    .date = {&nops_op_bin[20], 8, nand_op_header_date},
    .ver = {&nops_op_bin[28], 4, nand_op_header_ver}
};

void nand_op_header_nand_type(void)
{
    const ucode_header_info_t *info = &nand_op_header.nand_type_desc;
    int size = info->size;
    char *str = malloc(size);
    memset(str, 0, size);
    for (int i = 0, j = 0; i < size; i++) {
        if (info->buf[size - i - 1] == 0) {
            continue;
        }

        str[j++] = info->buf[size - i - 1];
    }
    printf("Nand Type     :  %s\n", str);
    free(str);
}

void nand_op_header_fc_gen(void)
{
    const ucode_header_info_t *info = &nand_op_header.fc_gen;
    printf("FC Generation :  GEN%d.%d\n", info->buf[0] >> 4, info->buf[0] & 0xF);
}

void nand_op_header_date(void)
{
    const ucode_header_info_t *info = &nand_op_header.date;
    printf("Generated On  :  20%02x-%02x-%02x %02x:%02x:%02x\n",
           info->buf[5],
           info->buf[4],
           info->buf[3],
           info->buf[2],
           info->buf[1],
           info->buf[0]);
}

void nand_op_header_ver(void)
{
    const ucode_header_info_t *info = &nand_op_header.ver;
    printf("Configuration Version : %d.%d.%d\n",
           info->buf[3] + info->buf[2],
           info->buf[1],
           info->buf[0]);
}

void ucode_bin_header_desc(void)
{
    const ucode_header_info_t *info = &ucode_bin_header.ucode_bin_desc;

    int size = info->size;
    char *str = malloc(size + 1);
    // memset(str, 0, size);
    // for (int i = 0, j = 0; i < size; i++) {
    //     if (info->buf[i] == 0)
    //         continue;

    //     str[j++] = info->buf[size - i - 1];
    // }

    memset(str, 0, size + 1);
    for (int i = 0, j = 0; i < size; i++) {
        if (info->buf[i] < ' ')
            continue;
        str[j++] = info->buf[i];
    }

    printf("Description   :  %s\n", str);
    free(str);
}

void ucode_bin_header_nifc_cmd_bin_ofst(void)
{
    const ucode_header_info_t *info = &ucode_bin_header.nifc_cmd_bin_ofst;
    u32 ofst
    = (u32)info->buf[0]
    + ((u32)info->buf[1] << 8)
    + ((u32)info->buf[2] << 16)
    + ((u32)info->buf[3] << 24);
    printf("nifc_cmd_ofst :  %d\n", ofst);
}

void ucode_bin_header_nifc_cmd_bin_size(void)
{
    const ucode_header_info_t *info = &ucode_bin_header.nifc_cmd_bin_size;
    u32 size
    = (u32)info->buf[0]
    + ((u32)info->buf[1] << 8)
    + ((u32)info->buf[2] << 16)
    + ((u32)info->buf[3] << 24);
    printf("nifc_cmd_size :  %d\n", size);
}

void ucode_bin_header_nops_op_bin_ofst(void)
{
    const ucode_header_info_t *info = &ucode_bin_header.nops_op_bin_ofst;
    u32 ofst
    = (u32)info->buf[0]
    + ((u32)info->buf[1] << 8)
    + ((u32)info->buf[2] << 16)
    + ((u32)info->buf[3] << 24);
    printf("nops_op_ofst  :  %d\n", ofst);
}

void ucode_bin_header_nops_op_bin_size(void)
{
    const ucode_header_info_t *info = &ucode_bin_header.nops_op_bin_size;
    u32 size
    = (u32)info->buf[0]
    + ((u32)info->buf[1] << 8)
    + ((u32)info->buf[2] << 16)
    + ((u32)info->buf[3] << 24);
    printf("nops_op_size  :  %d\n", size);
}

void ucode_print_ucode_bin_header_info_raw(void)
{
    print_buf(ucode_bin_header.ucode_bin_desc.buf, ucode_bin_header.ucode_bin_desc.size, "ucode_bin_desc", 1);
    print_buf(ucode_bin_header.nifc_cmd_bin_ofst.buf, ucode_bin_header.nifc_cmd_bin_ofst.size, "nifc_cmd_bin_ofst", 1);
    print_buf(ucode_bin_header.nifc_cmd_bin_size.buf, ucode_bin_header.nifc_cmd_bin_size.size, "nifc_cmd_bin_size", 1);
    print_buf(ucode_bin_header.nops_op_bin_ofst.buf, ucode_bin_header.nops_op_bin_ofst.size, "nops_op_bin_ofst", 1);
    print_buf(ucode_bin_header.nops_op_bin_size.buf, ucode_bin_header.nops_op_bin_size.size, "nops_op_bin_size", 1);
}

void ucode_print_nand_op_header_info_raw(void)
{
    print_buf(nand_op_header.nand_type_desc.buf, nand_op_header.nand_type_desc.size, "nand_type_desc", 1);
    print_buf(nand_op_header.fc_gen.buf, nand_op_header.fc_gen.size, "fc_gen", 1);
    print_buf(nand_op_header.date.buf, nand_op_header.date.size, "date", 1);
    print_buf(nand_op_header.ver.buf, nand_op_header.ver.size, "ver", 1);
}

void ucode_print_bin_header_info(void)
{
    ucode_bin_header.nifc_cmd_bin_ofst.info();
    ucode_bin_header.nifc_cmd_bin_size.info();
    ucode_bin_header.nops_op_bin_ofst.info();
    ucode_bin_header.nops_op_bin_size.info();
}

void ucode_print_nand_op_header_info(void)
{
    nand_op_header.nand_type_desc.info();
    nand_op_header.fc_gen.info();
    nand_op_header.date.info();
    nand_op_header.ver.info();
}

int ucode_bin_to_array(void)
{
    int ret = 0;
    FILE *fp;
    const char * file_name = "KIOXIA_BICS5.bin";
    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        printf("fopen \"%s\" fail\n", file_name);
        return -1;
    }

    fseek(fp, 0, SEEK_SET);

    long bin_size = file_bin_size(fp);
    printf("bin_size: (%d, %x)\n", bin_size, bin_size);

    m_ucode_bin = malloc(bin_size);
    if (m_ucode_bin == NULL) {
        ret = -1;
        goto exit;
    }

    for (int i = 0; i < bin_size; i++) {
        int c = fgetc(fp);
        if (c == EOF) {
            ret = ferror(fp);
            printf("fgetc fail (%d)\n", ret);
        }
        m_ucode_bin[i] = (char)c;
    }

    print_buf(m_ucode_bin, bin_size, "ucode_bin", 1);

    ucode_bin_header.ucode_bin_desc.buf = &m_ucode_bin[0];
    ucode_bin_header.nifc_cmd_bin_ofst.buf = &m_ucode_bin[16];
    ucode_bin_header.nifc_cmd_bin_size.buf = &m_ucode_bin[20];
    ucode_bin_header.nops_op_bin_ofst.buf = &m_ucode_bin[24];
    ucode_bin_header.nops_op_bin_size.buf = &m_ucode_bin[28];

    // ucode_print_ucode_bin_header_info_raw();
    ucode_bin_header.ucode_bin_desc.info();
    ucode_bin_header.nifc_cmd_bin_ofst.info();
    ucode_bin_header.nifc_cmd_bin_size.info();
    ucode_bin_header.nops_op_bin_ofst.info();
    ucode_bin_header.nops_op_bin_size.info();

exit:
    if (m_ucode_bin != NULL) {
        free(m_ucode_bin);
        m_ucode_bin = NULL;
    }
    fclose(fp);
    return ret;
}
