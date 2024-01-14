#include "c_test.h"

#include "gov_moi_reip.h"

void c_test(void)
{
    printf("%s start >>>\n", __FUNCTION__);
    gov_moi_reip_main();
    printf("%s end <<<\n", __FUNCTION__);
}
