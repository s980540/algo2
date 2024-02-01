#include "nvme_mi.h"

fw_admin_command admin_cmd __attribute__((aligned(4)));

nrcqe_dw0_t g_nrcqe_dw0;

dword m_swap_buf_addr;
dword m_ctrl_meta_buf_addr;
dword m_ns_meta_buf_addr;
ctrl_meta_t *m_ctrl_meta;

dword g_buf_tbl[5];

void nvme_mi_init(void)
{
    g_nrcqe_dw0.config = 0;
}

void nvme_mi_deinit(void)
{

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
    }
}

int nvme_mi_transfer_data(dword *transfer_addr, dword transfer_size, byte is_read)
{
    int ret;

    // #if (OPT_NVME_MI_DEBUG_TRACE)
    // DBGPRINT(FW_TRACE, "tran data[%d](0x%08x,0x%x)\r", is_read, *transfer_addr, transfer_size);
    // #endif

    // ret = nvme_cmd_transfer_data(
    //                   admin_cmd.nvme_cmd.bits.nvme_mi_cmd.prp1,
    //                   admin_cmd.nvme_cmd.bits.nvme_mi_cmd.prp2,
    //                   transfer_size,
    //                   transfer_addr,
    //                   is_read);

    // /**
    //  * If any errors are encountered transferring the Request/Response Data then the command is
    //  * completed with an error status code in the Status field as per the NVM Express Base
    //  * Specification and the Tunneled Status and Tunneled NVMe Management Response fields
    //  * shall be cleared to 0h.
    //  **/
    // if (ret)
    // {
    //     g_cq_sts = NVME_SC_DATA_XFER_ERROR;
    //     g_nrcqe_dw0.config = 0;
    // }
    // else
    // {
    //     g_cq_sts = NVME_SC_SUCCESS;
    //     g_nrcqe_dw0.config = 0;
    // }

    return ret;
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

    metadata_element_descriptor_t *md;
    short offset = 0, length;
    byte desc_cnt = 0;

    if (ea == update_entry)
    {
        while (1)
        {
            if ((desc_cnt == host_metadata->desc_cnt) || (offset >= 4090))
                break;

            md = (metadata_element_descriptor_t *)&host_metadata->buf[offset];
            length = (md->elen[1] << 8) + md->elen[0];
            desc_cnt++;
            // Update the offset to the next entry.
            offset = sizeof(*md) + length;

            if (!is_et_valid(md->et) || (length == 0))
                continue;

            metadata_element_record_t *record;
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
                    short diff;
                    diff = length - record->length;
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
                    record->offset = (dword)md;
                    record->length = length;
                    record->src = 1;
                }
            }
            // Creates a new record of metadata descriptor
            else
            {
                // Get an unused record from the empty list.
                record = list_first_entry(
                    &m_ctrl_meta->empty,
                    metadata_element_record_t,
                    entry);

                list_del(&record->entry);

                // Update the information of the metadata descriptor for the record.
                record->et = md->et;
                record->offset = (dword)md;
                record->length = length;
                record->src = 1;

                // And then, push the record to the valid list.
                list_add_tail(&record->entry, &m_ctrl_meta->valid);
            }
        }
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
