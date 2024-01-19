#include "c_test.h"

#include "gov_moi_reip.h"

#include "ucode.h"

void c_test(void)
{
    printf("%s start >>>\n", __FUNCTION__);
    gov_moi_reip_main();
    printf("%s end <<<\n", __FUNCTION__);
}

void c_test_print_buf(void)
{
    // u16 g_nifc_cmd_bin_size = sizeof(nifc_cmd_bin);
    // u16 g_nops_op_bin_size = sizeof(nops_op_bin);

    // print_buf(nifc_cmd_bin, sizeof(nifc_cmd_bin), "nifc_cmd_bin", 1);
    // print_buf(nops_op_bin, sizeof(nops_op_bin), "nops_op_bin", 1);

    ucode_print_nand_op_header_info_raw();
    ucode_print_nand_op_header_info();
}