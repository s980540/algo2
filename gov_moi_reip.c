#include "gov_moi_reip.h"

#define STR_BUF_SIZE    (256)

char str_buf[STR_BUF_SIZE];
// char rlt_buf[STR_BUF_SIZE];

int gov_moi_reip_main(void)
{
    FILE *fp_r = NULL, *fp_w = NULL;
    const char fname_r[] = "2.csv";
    const char fname_w[] = "2w.csv";
    long fline = 0, pos;

    //
    fp_r = fopen(fname_r, "r");
    if (fp_r == NULL)
    {
        printf("fopen %s failed\n", fname_r);
        goto exit;
    }

    //
    fp_w = fopen(fname_w, "w");
    if (fp_w == NULL)
    {
        printf("fopen %s failed\n", fname_w);
        goto exit;
    }

    //
    fseek(fp_r, 0, SEEK_SET);
    fseek(fp_w, 0, SEEK_SET);

    //
    pos = ftell(fp_r);
    while (fgets(str_buf, STR_BUF_SIZE, fp_r) != NULL)
    {
        fline++;
    }
    fseek(fp_r, pos, SEEK_SET);

    //
    fline--;
    if (fgets(str_buf, STR_BUF_SIZE, fp_r) == NULL)
    {
        goto exit;
    }

    printf("%s", str_buf);
    #if 1
    fprintf(fp_w, "%s", str_buf);

    //
    while (fline--)
    {
        if (fgets(str_buf, STR_BUF_SIZE, fp_r) == NULL)
        {
            goto exit;
        }

        printf("%s", str_buf);

        char *p, *r;
        p = str_buf;
        for (int i = 0 ; i < 4; i++)
        {
            r = strchr(p, ',') + 1;
            r[-1] = 0;
            fprintf(fp_w, "%s,", p);
            p = r;
        }

        //
        // printf("%s", p);
        while (1)
        {
            r = strchr(p, ',');
            if (r == NULL)
            {
                fprintf(fp_w, "%s", p);
                break;
            }
            *r = 0;

            if (strcmp(p, "-") != 0)
            {
                char *yy, *mm, *dd;
                int y, m, d;
                yy = p;
                mm = strchr(yy, '/') + 1;
                mm[-1] = 0;
                dd = strchr(mm, '/') + 1;
                dd[-1] = 0;

                y = atoi(yy);
                m = atoi(mm);
                d = atoi(dd);

                // printf("%d/%d/%d,", (y <= 1911 ? y + 1911 : y), m, d);
                fprintf(fp_w, "%d/%d/%d,", (y <= 1911 ? y + 1911 : y), m, d);

            }
            else
            {
                // printf("%s,", p);
                fprintf(fp_w, "%s,", p);
            }

            p = r + 1;
        }
    }
    #endif
exit:
    fclose(fp_r);
    fclose(fp_w);

    return 0;
}