#include "c_test.h"

#include "gov_moi_reip.h"
#include "nvme_mi.h"

void c_test(void)
{
    printf("%s start >>>\n", __FUNCTION__);
    // gov_moi_reip_main();
    nvme_mi_init();
    nvme_mi_deinit();
    printf("%s end <<<\n", __FUNCTION__);
}
