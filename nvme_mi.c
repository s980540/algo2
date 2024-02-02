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

byte et_is_valid(byte et)
{
    if ((et >= operating_system_controller_name)
     && (et <= pre_boot_driver_version))
    {
        return 1;
    }

    return 0;
}

word insert_md(host_metadata_t *host_metadata, word offset, byte et, word len)
{
    int i = offset;
    word elen = len - 4;
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
    printf("(buf_ofst: %4d, len: %4d)(et: %d, er: %d, elen: %4d): 0x%08x, %s\n",
        offset, i - offset,
        host_metadata->buf[offset],
        host_metadata->buf[offset + 1],
        (host_metadata->buf[offset + 3] << 8) + host_metadata->buf[offset + 2],
        crc32(&host_metadata->buf[offset], i - offset),
        &host_metadata->buf[offset + HOST_META_ELE_HEAD_SIZE]);
#endif
    host_metadata->desc_cnt++;

    return i;
}

int nvme_mi_transfer_data(
    dword *transfer_addr,
    dword transfer_size,
    byte is_read,
    byte is_delete)
{
    host_metadata_t *host_metadata = (void *)g_buf_tbl[0];
    memset(host_metadata, 0, sizeof(*host_metadata));
    admin_cmd.nvme_cmd.bits.features.dword11 = (is_delete ? delete_entry : update_entry) << 13;

    word buf_ofst, length;

#if (OPT_NVME_MI_DEBUG_TRACE)
    printf("\n[insert_md]\n");
#endif
    buf_ofst = 0;
    while (1) {
        if ((host_metadata->desc_cnt == CTRL_META_ELEMENT_TYPE_MAX)
         || (buf_ofst >= (HOST_METADATA_BUF_MAX - HOST_META_ELE_HEAD_SIZE)))  // 4090
            break;

        length = HOST_META_ELE_HEAD_SIZE + (is_delete ? 0 : (rand() % 32 + 1));
        // length = HOST_META_ELE_HEAD_SIZE + (rand() % 4094 + 1);
        length = (buf_ofst + length > HOST_METADATA_BUF_MAX) ? (HOST_METADATA_BUF_MAX - buf_ofst) : length; // 4094

        buf_ofst = insert_md(
            host_metadata,
            buf_ofst,
            rand() % CTRL_META_ELEMENT_TYPE_MAX + 1,
            length);
    }

#if (OPT_NVME_MI_DEBUG_TRACE)
    printf("\n[%s] host_metadata\n", __FUNCTION__);
#endif
    meta_ele_desc_t *md;
    byte desc_cnt = 0;
    word ptr_ofst = HOST_METADATA_DS_HEAD_SIZE;
    while (1) {
        if ((desc_cnt == host_metadata->desc_cnt)
         || (ptr_ofst >= (HOST_METADATA_DS_SIZE - HOST_META_ELE_HEAD_SIZE)))  // 4092
            break;

        md = (meta_ele_desc_t *)((void *)host_metadata + ptr_ofst);
        length = sizeof(*md) + (md->elen[1] << 8) + md->elen[0];
        desc_cnt++;
        // Update the ptr_ofst to the next entry.
        ptr_ofst += length;

        if (!et_is_valid(md->et))
            continue;

    #if (OPT_NVME_MI_DEBUG_TRACE)
        if (is_delete)
        {
            printf("(ptr_ofst: %4d, len: %4d)(et: %d, er: %d, elen: %4d): 0x%08x\n",
                (void *)md - (void *)host_metadata,
                ptr_ofst - (word)((void *)md - (void *)host_metadata),
                md->et, md->er, (md->elen[1] << 8) + md->elen[0],
                crc32((void *)md,
                    ptr_ofst - (word)((void *)md - (void *)host_metadata)));
        }
        else
        {
            printf("(ptr_ofst: %4d, len: %4d)(et: %d, er: %d, elen: %4d): 0x%08x, %s\n",
                (void *)md - (void *)host_metadata,
                ptr_ofst - (word)((void *)md - (void *)host_metadata),
                md->et, md->er, (md->elen[1] << 8) + md->elen[0],
                crc32((void *)md,
                    ptr_ofst - (word)((void *)md - (void *)host_metadata)),
                    (void *)md + HOST_META_ELE_HEAD_SIZE);
        }
    #endif
    }

    return 0;
}

void nvme_mi_set_feat_controller_metadata(byte is_delete)
{
    int ret;
    host_metadata_t *host_metadata;
    meta_ele_desc_t *md;
    meta_ele_record_t *record;
    word ptr_ofst, length;
    byte ea, desc_cnt, meta_exist;

    nvme_mi_set_metadata_addr();

    /**
     * The Host Metadata data structure is 4 KiB in size and contains
     * zero or more Metadata Element Descriptors.
    */
    // Transfer request data
    if (nvme_mi_transfer_data(&g_buf_tbl[0], HOST_METADATA_DS_SIZE, 0, is_delete))
        return;

    host_metadata = (void *)g_buf_tbl[0];
    ea = (admin_cmd.nvme_cmd.bits.features.dword11 >> 13) & 0x3;
    if (ea == update_entry)
    {
        ptr_ofst = HOST_METADATA_DS_HEAD_SIZE;
        length = 0;
        desc_cnt = 0;
        while (1)
        {
            if ((desc_cnt == host_metadata->desc_cnt)
             || (ptr_ofst >= (HOST_METADATA_DS_SIZE - HOST_META_ELE_HEAD_SIZE)))  // 4092
                break;

            md = (meta_ele_desc_t *)((void *)host_metadata + ptr_ofst);
            length = sizeof(*md) + (md->elen[1] << 8) + md->elen[0];
            desc_cnt++;
            // Update the offset to the next metadata descriptor.
            ptr_ofst += length;

            if (!et_is_valid(md->et) || (length == 0))
                continue;

            meta_exist = FALSE;
            if (!list_is_empty(&m_ctrl_meta->valid)) {
                list_for_each_entry(record, &m_ctrl_meta->valid, entry) {
                    /**
                     * If a Metadata Element Descriptor with the specified Element Type
                     * exists in Controller Metadata.
                    */
                    if (md->et == record->et) {
                        meta_exist = TRUE;
                        break;
                    }
                }
            }

            // Update the existing record of metadata descriptor
            if (meta_exist)
            {
                short diff = length - record->length;
                /**
                 * If host software attempts to add or update a Metadata Element that
                 * causes the stored Host Metadata data structure to grow larger than
                 * 4 KiB, the Controller shall abort the command with an Invalid
                 * Parameter Error Response.
                */
                if ((m_ctrl_meta->metadata_size + diff) > HOST_METADATA_BUF_MAX) { // 4094
                    g_nrcqe_dw0.invalid_param.status = NVME_MI_RESP_INVALID_PARAM;
                    g_nrcqe_dw0.invalid_param.lsbit = 0;
                    g_nrcqe_dw0.invalid_param.lsbyte = (void *)md->elen - (void *)host_metadata;
                    goto send_cq_stage;
                }

                // Update the information of the metadata descriptor for the record.
                record->offset = (void *)md - (void *)host_metadata;
                record->length = length;
                record->src = 1;
                m_ctrl_meta->metadata_size += diff;
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
                record->offset = (void *)md - (void *)host_metadata;
                record->length = length;
                record->src = 1;
                m_ctrl_meta->metadata_size += record->length;
                m_ctrl_meta->valid_cnt++;

                // And then, push the record to the valid list.
                list_add_tail(&record->entry, &m_ctrl_meta->valid);
            }
        } // ~ while (1)

        dword dst_addr, src_addr;
    #if (OPT_NVME_MI_DEBUG_TRACE)
        printf("\n[meta_record]\n");
        list_for_each_entry(record, &m_ctrl_meta->valid, entry) {
            src_addr = (record->src
                ? (dword)((void *)host_metadata + record->offset)
                : (dword)(m_ctrl_meta_buf_addr + record->offset));

            printf("(ptr_ofst: %4d, len: %4d)(et: %d, sr: %d, elen: %4d): 0x%08x, %s\n",
                record->offset, record->length, record->et, record->src, record->length - HOST_META_ELE_HEAD_SIZE,
                crc32(((void *)host_metadata + record->offset), record->length),
                (void*)src_addr + HOST_META_ELE_HEAD_SIZE);
        }
    #endif

        memset((void *)m_swap_buf_addr, 0, HOST_METADATA_DS_SIZE);
        // Set the number of metadata element descriptors
        *((byte *)m_swap_buf_addr) = m_ctrl_meta->valid_cnt;
        ptr_ofst = HOST_METADATA_DS_HEAD_SIZE;
        list_for_each_entry(record, &m_ctrl_meta->valid, entry) {
            dst_addr = m_swap_buf_addr + ptr_ofst;
            src_addr = record->src
                ? (dword)((void *)host_metadata + record->offset)
                : (dword)(m_ctrl_meta_buf_addr + record->offset);

            memcpy((void *)dst_addr, (void *)src_addr, record->length);

            record->offset = ptr_ofst;
            record->src = 0;
            ptr_ofst += record->length;
        }

        dword temp = m_swap_buf_addr;
        m_swap_buf_addr = m_ctrl_meta_buf_addr;
        m_ctrl_meta_buf_addr = temp;

    #if (OPT_NVME_MI_DEBUG_TRACE)
        printf("\n[m_ctrl_meta_buf_addr]\n");
        ptr_ofst = HOST_METADATA_DS_HEAD_SIZE;
        desc_cnt = 0;
        while (1) {
            if ((desc_cnt == m_ctrl_meta->valid_cnt)
             || (ptr_ofst >= (HOST_METADATA_DS_SIZE - HOST_META_ELE_HEAD_SIZE))) // 4092
                break;

            md = (meta_ele_desc_t *)(m_ctrl_meta_buf_addr + ptr_ofst);
            length = sizeof(*md) + (md->elen[1] << 8) + md->elen[0];
            desc_cnt++;
            // Update the offset to the next entry.
            ptr_ofst += length;

            if (!et_is_valid(md->et) || (length == 0))
                continue;

            printf("(ptr_ofst: %4d, len: %4d)(et: %d, er: %d, elen: %4d): 0x%08x, %s\n",
                ((dword)md - m_ctrl_meta_buf_addr),
                ptr_ofst - (word)((dword)md - m_ctrl_meta_buf_addr),
                md->et, md->er, (md->elen[1] << 8) + md->elen[0],
                crc32((void *)md, ptr_ofst - (word)((dword)md - m_ctrl_meta_buf_addr)),
                (void *)md + HOST_META_ELE_HEAD_SIZE);
        }

        printf("\n[new meta_record]\n");
        list_for_each_entry(record, &m_ctrl_meta->valid, entry) {
            printf("(ptr_ofst: %4d, len: %4d)(et: %d, sr: %d, elen: %4d): 0x%08x, %s\n",
                record->offset, record->length, record->et, record->src, record->length - HOST_META_ELE_HEAD_SIZE,
                crc32((void *)(m_ctrl_meta_buf_addr + record->offset), record->length),
                (void *)(m_ctrl_meta_buf_addr + record->offset + HOST_META_ELE_HEAD_SIZE));
        }
    #endif
    }
    else if (ea == delete_entry)
    {
        ptr_ofst = HOST_METADATA_DS_HEAD_SIZE;
        length = 0;
        desc_cnt = 0;
        while (1)
        {
            if ((desc_cnt == host_metadata->desc_cnt)
             || (ptr_ofst >= (HOST_METADATA_DS_SIZE - HOST_META_ELE_HEAD_SIZE))) // 4092
                break;

            md = (meta_ele_desc_t *)((void *)host_metadata + ptr_ofst);
            length = sizeof(*md) + (md->elen[1] << 8) + md->elen[0];
            desc_cnt++;
            // Update the offset to the next metedata descriptor.
            ptr_ofst += length;

            if (!et_is_valid(md->et))
                continue;

            meta_exist = FALSE;
            if (!list_is_empty(&m_ctrl_meta->valid)) {
                list_for_each_entry(record, &m_ctrl_meta->valid, entry) {
                    if (md->et == record->et) {
                        meta_exist = TRUE;
                        break;
                    }
                }
            }

            if (meta_exist)
            {
                // Delete the record from the valid list.
                list_del(&record->entry);
                m_ctrl_meta->metadata_size -= record->length;
                m_ctrl_meta->valid_cnt--;
                // Clear the content of the record.
                memset(record, 0, sizeof(*record));
                // Add the record back to the empty list.
                list_add_tail(&record->entry, &m_ctrl_meta->empty);
            }
        }   // ~ while (1)

        dword dst_addr, src_addr;
    #if (OPT_NVME_MI_DEBUG_TRACE)
        printf("\n[meta_record]\n");
        list_for_each_entry(record, &m_ctrl_meta->valid, entry) {
            src_addr = (record->src
                ? (dword)((void *)host_metadata + record->offset)
                : (dword)(m_ctrl_meta_buf_addr + record->offset));

            printf("(ptr_ofst: %4d, len: %4d)(et: %d, sr: %d, elen: %4d): 0x%08x, %s\n",
                record->offset, record->length, record->et, record->src, record->length - HOST_META_ELE_HEAD_SIZE,
                crc32((void *)src_addr, record->length),
                (void*)src_addr + HOST_META_ELE_HEAD_SIZE);
        }
    #endif

        memset((void *)m_swap_buf_addr, 0, HOST_METADATA_DS_SIZE);
        // Set the number of metadata element descriptors
        *((byte *)m_swap_buf_addr) = m_ctrl_meta->valid_cnt;
        ptr_ofst = HOST_METADATA_DS_HEAD_SIZE;
        list_for_each_entry(record, &m_ctrl_meta->valid, entry) {
            dst_addr = m_swap_buf_addr + ptr_ofst;
            src_addr = record->src
                ? (dword)((void *)host_metadata + record->offset)
                : (dword)(m_ctrl_meta_buf_addr + record->offset);

            memcpy((void *)dst_addr, (void *)src_addr, record->length);

            record->offset = ptr_ofst;
            record->src = 0;
            ptr_ofst += record->length;
        }

        dword temp = m_swap_buf_addr;
        m_swap_buf_addr = m_ctrl_meta_buf_addr;
        m_ctrl_meta_buf_addr = temp;

    #if (OPT_NVME_MI_DEBUG_TRACE)
        printf("\n[m_ctrl_meta_buf_addr]\n");
        ptr_ofst = HOST_METADATA_DS_HEAD_SIZE;
        desc_cnt = 0;
        while (1) {
            if ((desc_cnt == m_ctrl_meta->valid_cnt)
             || (ptr_ofst >= (HOST_METADATA_DS_SIZE - HOST_META_ELE_HEAD_SIZE))) // 4092
                break;

            md = (meta_ele_desc_t *)(m_ctrl_meta_buf_addr + ptr_ofst);
            length = sizeof(*md) + (md->elen[1] << 8) + md->elen[0];
            desc_cnt++;
            // Update the offset to the next entry.
            ptr_ofst += length;

            if (!et_is_valid(md->et) || (length == 0))
                continue;

            printf("(ptr_ofst: %4d, len: %4d)(et: %d, er: %d, elen: %4d): 0x%08x, %s\n",
                ((dword)md - m_ctrl_meta_buf_addr),
                ptr_ofst - (word)((dword)md - m_ctrl_meta_buf_addr),
                md->et, md->er, (md->elen[1] << 8) + md->elen[0],
                crc32((void *)md, ptr_ofst - (word)((dword)md - m_ctrl_meta_buf_addr)),
                (void *)md + HOST_META_ELE_HEAD_SIZE);
        }

        printf("\n[new meta_record]\n");
        list_for_each_entry(record, &m_ctrl_meta->valid, entry) {
            printf("(ptr_ofst: %4d, len: %4d)(et: %d, sr: %d, elen: %4d): 0x%08x, %s\n",
                record->offset, record->length, record->et, record->src, record->length - HOST_META_ELE_HEAD_SIZE,
                crc32((void *)(m_ctrl_meta_buf_addr + record->offset), record->length),
                (void *)(m_ctrl_meta_buf_addr + record->offset + HOST_META_ELE_HEAD_SIZE));
        }
    #endif
    }
    else
    {
        // NVME_SC_INVALID_FIELD
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
    nvme_mi_set_feat_controller_metadata(0);
    nvme_mi_set_feat_controller_metadata(1);
    // nvme_mi_set_feat_controller_metadata(0);
    // nvme_mi_set_feat_controller_metadata(0);
}

void nvme_mi_deinit(void)
{
    aligned_free((void *)g_buf_tbl[0]);
    aligned_free((void *)g_buf_tbl[1]);
    aligned_free((void *)g_buf_tbl[2]);
    aligned_free((void *)g_buf_tbl[3]);
    aligned_free((void *)g_buf_tbl[4]);
}
