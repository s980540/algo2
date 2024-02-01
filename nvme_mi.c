#include "nvme_mi.h"

#include "utility.h"
#include "crc32.h"

fw_admin_command admin_cmd __attribute__((aligned(4)));

nrcqe_dw0_t g_nrcqe_dw0;

dword m_swap_buf_addr;
dword m_ctrl_meta_buf_addr;
dword m_ns_meta_buf_addr;
ctrl_meta_t *m_ctrl_meta;

dword g_buf_tbl[5];

void init_ctrl_meta(void)
{
    INIT_LIST_HEAD(&m_ctrl_meta->valid);
    INIT_LIST_HEAD(&m_ctrl_meta->empty);

    for (int i = 0; i < CTRL_META_ELEMENT_TYPE_MAX; i++)
    {
        list_add_tail(&m_ctrl_meta->record[i].entry, &m_ctrl_meta->empty);
    }

    printf("list_is_empty(&m_ctrl_meta->valid): %d\n", list_is_empty(&m_ctrl_meta->valid));
    printf("list_is_empty(&m_ctrl_meta->empty): %d\n", list_is_empty(&m_ctrl_meta->empty));
}

void nvme_mi_set_metadata_addr(void)
{
    if (!m_swap_buf_addr)
    {
        m_swap_buf_addr = g_buf_tbl[1];
        m_ctrl_meta_buf_addr = g_buf_tbl[2];
        m_ns_meta_buf_addr = g_buf_tbl[3];
        m_ctrl_meta = (void *)g_buf_tbl[4];
        memset(m_ctrl_meta, 0, sizeof(*m_ctrl_meta));
        init_ctrl_meta();
    }
}

char rand_char(void)
{
    char c;
    while (1)
    {
        c = rand() & 0xFF;
        if (c >= 0x20 && c <= 0x7E)
            return c;
    }
}

byte is_et_valid(byte et)
{
    if ((et >= operating_system_controller_name)
     && (et <= pre_boot_driver_version))
    {
        return 1;
    }

    return 0;
}

word insert_md(host_metadata_t *host_metadata, word offset, byte et, word elen)
{
    int i = offset;
    elen = elen - 4;
    host_metadata->buf[i++] = et;
    host_metadata->buf[i++] = 0;
    host_metadata->buf[i++] = elen & 0xFF;
    host_metadata->buf[i++] = elen >> 8;

    while (elen)
    {
        if (elen == 1)
        {
            host_metadata->buf[i++] = 0;
            break;
        }
        host_metadata->buf[i++] = rand_char();
        elen--;
    }
#if (OPT_NVME_MI_DEBUG_TRACE)
    printf("(%4d,%4d,%4d)(%d,%d,%4d): 0x%08x,%s\n",
        offset, i - offset, i,
        host_metadata->buf[offset],
        host_metadata->buf[offset + 1],
        (host_metadata->buf[offset + 3] << 8) + host_metadata->buf[offset + 2],
        crc32(&host_metadata->buf[offset], i - offset),
        &host_metadata->buf[offset + 4]);
#endif
    host_metadata->desc_cnt++;

    return i;
}

int nvme_mi_transfer_data(dword *transfer_addr, dword transfer_size, byte is_read)
{
    host_metadata_t *host_metadata = (void *)g_buf_tbl[0];
    memset(host_metadata, 0, sizeof(*host_metadata));
    admin_cmd.nvme_cmd.bits.features.dword11 = 1 << 13;

    word offset, length;

    offset = 0;
    while (1) {
        if ((host_metadata->desc_cnt == CTRL_META_ELEMENT_TYPE_MAX)
        // if ((host_metadata->desc_cnt == 255)
         || (offset >= 4090))
            break;

        length = 4 + (rand() % 32) + 1;
        // length = 4 + (rand() % 4094) + 1;
        length = (offset + length > 4094) ? (4094 - offset) : length;

        offset = insert_md(
            host_metadata,
            offset,
            (rand() % CTRL_META_ELEMENT_TYPE_MAX) + 1,
            length);
    }
#if (OPT_NVME_MI_DEBUG_TRACE)
    printf("[insert_md]\n");
#endif

    meta_ele_desc_t *md;
    byte desc_cnt = 0;

    offset = 0;
    while (1) {
        if ((desc_cnt == host_metadata->desc_cnt) || (offset >= 4090))
            break;

        md = (meta_ele_desc_t *)&(host_metadata->buf[offset]);
        length = (md->elen[1] << 8) + md->elen[0];
        desc_cnt++;
        // Update the offset to the next entry.
        offset += (sizeof(*md) + length);

        if (!is_et_valid(md->et) || (length == 0))
            continue;

        printf("(%4d,%4d,%4d)(%d,%d,%4d): 0x%08x,%s\n",
            ((void *)md - (void *)host_metadata->buf),
            offset - (word)((void *)md - (void *)host_metadata->buf),
            offset,
            md->et,
            md->er,
            length,
            crc32(
                &host_metadata->buf[(void *)md - (void *)host_metadata->buf],
                offset - (word)((void *)md - (void *)host_metadata->buf)),
            &host_metadata->buf[((void *)md - (void *)host_metadata->buf) + 4]);
    }
#if (OPT_NVME_MI_DEBUG_TRACE)
    printf("[%s]\n", __FUNCTION__);
#endif

    return 0;
}

void nvme_mi_set_feat_controller_metadata(void)
{
    int ret;
    dword host_metadata_buf_addr = g_buf_tbl[0];
    host_metadata_t *host_metadata = (void *)host_metadata_buf_addr;
    byte ea = (admin_cmd.nvme_cmd.bits.features.dword11 >> 13) & 0x3;

    nvme_mi_set_metadata_addr();

    /**
     * The Host Metadata data structure is 4 KiB in size and contains
     * zero or more Metadata Element Descriptors.
    */
    // Transfer request data
    if (nvme_mi_transfer_data(&host_metadata_buf_addr, 4096, 0))
        return;

    meta_ele_desc_t *md;
    word offset = 0, length;
    byte desc_cnt = 0;

    if (ea == update_entry)
    {
        while (1)
        {
            if ((desc_cnt == host_metadata->desc_cnt) || (offset >= 4090))
                break;

            md = (meta_ele_desc_t *)&host_metadata->buf[offset];
            length = (md->elen[1] << 8) + md->elen[0];
            desc_cnt++;
            // Update the offset to the next metadata descriptor.
            offset += sizeof(*md) + length;

            if (!is_et_valid(md->et) || (length == 0))
                continue;

            meta_ele_record_t *record;
            if (!list_is_empty(&m_ctrl_meta->valid))
            {
                byte meta_exist = FALSE;
                list_for_each_entry(record, &m_ctrl_meta->valid, entry)
                {
                    /**
                     * If a Metadata Element Descriptor with the specified Element Type
                     * exists in Controller Metadata.
                    */
                    if (md->et == record->et)
                    {
                        meta_exist = TRUE;
                        break;
                    }
                }

                // Update the existing record of metadata descriptor
                if (meta_exist)
                {
                    short diff = 0;
                    diff = length - (record->length - sizeof(*md));
                    /**
                     * If host software attempts to add or update a Metadata Element that
                     * causes the stored Host Metadata data structure to grow larger than
                     * 4 KiB, the Controller shall abort the command with an Invalid
                     * Parameter Error Response.
                    */
                    if ((m_ctrl_meta->metadata_size + diff) > 4096)
                    {
                        g_nrcqe_dw0.invalid_param.status = NVME_MI_RESP_INVALID_PARAM;
                        g_nrcqe_dw0.invalid_param.lsbit = 0;
                        g_nrcqe_dw0.invalid_param.lsbyte = 2 + ((dword)md->elen - (dword)host_metadata);
                        goto send_cq_stage;
                    }

                    // Update the m_ctrl_meta->record
                    m_ctrl_meta->metadata_size += diff;
                    record->offset = (void *)md - (void *)host_metadata->buf;
                    record->length = sizeof(*md) + length;
                    record->src = 1;
                }
                else
                {
                    // Get an unused record from the empty list.
                    record = list_first_entry(
                        &m_ctrl_meta->empty,
                        meta_ele_record_t,
                        entry);

                    list_del(&record->entry);

                    // Update the information of the metadata descriptor for the record.
                    record->et = md->et;
                    record->offset = (void *)md - (void *)host_metadata->buf;
                    record->length = sizeof(*md) + length;
                    record->src = 1;
                    m_ctrl_meta->metadata_size += record->length;

                    // And then, push the record to the valid list.
                    list_add_tail(&record->entry, &m_ctrl_meta->valid);
                }
            }
            // Creates a new record of metadata descriptor
            else
            {
                // Get an unused record from the empty list.
                record = list_first_entry(
                    &m_ctrl_meta->empty,
                    meta_ele_record_t,
                    entry);

                list_del(&record->entry);

                // Update the information of the metadata descriptor for the record.
                record->et = md->et;
                record->offset = (void *)md - (void *)host_metadata->buf;
                record->length = sizeof(*md) + length;
                record->src = 1;
                m_ctrl_meta->metadata_size += record->length;

                // And then, push the record to the valid list.
                list_add_tail(&record->entry, &m_ctrl_meta->valid);
            }
        }
    #if (OPT_NVME_MI_DEBUG_TRACE)
        meta_ele_record_t *record = NULL;
        list_for_each_entry(record, &m_ctrl_meta->valid, entry)
        {
            printf("(%d,%4d,%4d): 0x%08x,%s\n",
                record->et,
                record->offset,
                record->length,
                crc32(&host_metadata->buf[record->offset], record->length),
                &host_metadata->buf[record->offset + 4]);
        }
    #endif
    }
    else if (ea == delete_entry)
    {

    }
    else
    {

    }

send_cq_stage:
    // if (g_task)
    // {
    //     admin_task.func = nvme_frontend_handle;
    // }
    return;
}

void nvme_mi_init(void)
{
    srand(time(0));
    g_nrcqe_dw0.config = 0;
    memset(&admin_cmd, 0, sizeof(admin_cmd));
    printf("sizeof(admin_cmd): %d\n", sizeof(admin_cmd));
    printf("sizeof(host_metadata_t): %d\n", sizeof(host_metadata_t));
    printf("sizeof(ctrl_meta_t): %d,%d\n", sizeof(*m_ctrl_meta), sizeof(ctrl_meta_t));
    printf("sizeof(meta_ele_record_t): %d\n", sizeof(meta_ele_record_t));
    g_buf_tbl[0] = (dword)aligned_alloc(4096, 4096);
    g_buf_tbl[1] = (dword)aligned_alloc(4096, 4096);
    g_buf_tbl[2] = (dword)aligned_alloc(4096, 4096);
    g_buf_tbl[3] = (dword)aligned_alloc(4096, 4096);
    g_buf_tbl[4] = (dword)aligned_alloc(4096, 4096);

    // nvme_mi_transfer_data(0, 0, 0);
    nvme_mi_set_feat_controller_metadata();
}

void nvme_mi_deinit(void)
{
    aligned_free((void *)g_buf_tbl[0]);
    aligned_free((void *)g_buf_tbl[1]);
    aligned_free((void *)g_buf_tbl[2]);
    aligned_free((void *)g_buf_tbl[3]);
    aligned_free((void *)g_buf_tbl[4]);
}
