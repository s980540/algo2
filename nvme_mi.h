#ifndef NVME_MI_H
#define NVME_MI_H

#include "global.h"

#include "list.h"

#define EN_NVME_MI

#define OPT_NVME_MI_DEBUG_TRACE         (1)
#define CTRL_META_ELEMENT_TYPE_MAX      (6)

#define HOST_METADATA_DS_SIZE           (4096)
#define HOST_METADATA_DS_HEAD_SIZE      (2)
#define HOST_METADATA_BUF_MAX           (HOST_METADATA_DS_SIZE - HOST_METADATA_DS_HEAD_SIZE)
#define HOST_META_ELE_HEAD_SIZE         (4)

struct nvme_common_cmd
{
    byte opcode;
    byte flags;
    word command_id;
    dword nsid;
    dword cdw2[2];
    qword metadata;
    qword prp1;
    qword prp2;
    dword cdw10[6];
};

struct nvme_features_cmd
{
    byte opcode;
    byte flags;
    word command_id;
    dword nsid;
    qword rsvd2[2];
    qword prp1;
    qword prp2;
    dword feat_id;
    dword dword11;
    dword rsvd12[4];
};

typedef struct _nvme_mi_msg_req_t
{
    dword cdw0;
    dword nsid;
    qword rsvd1;
    qword mptr;
    qword prp1;
    qword prp2;
    dword nmh;              // cdw10
    byte opc;               // cdw11
    byte rsvd2[3];
    dword nmd0;             // cdw12
    dword nmd1;             // cdw13
    dword cdw14;            // cdw14
    dword cdw15;            // cdw15
} __attribute__((packed)) nvme_mi_msg_req_t;

struct nvme_command
{
    union
    {
        struct nvme_common_cmd common;
        #ifdef EN_NVME_MI
        struct _nvme_mi_msg_req_t nvme_mi_cmd;
        #endif
        struct nvme_features_cmd features;
    } bits;
};

typedef union _fw_admin_command_dw0
{
    struct
    {
        dword rsvd1 : 10;
        dword func_id : 4;
        dword rsvd2 : 2;
        dword sqid : 5;
        dword abort : 1;
        dword rsvd3 : 2;
        dword opcode : 8;
    } bits;

    dword config;
} fw_admin_command_dw0;

typedef struct _fw_admin_command
{
    fw_admin_command_dw0 fw_admin_cmd_dw0;
    struct nvme_command nvme_cmd;
} fw_admin_command;

// Generic Error Response
typedef struct _nvme_mi_err_resp_generic_t
{
    dword status : 8;       // bit[07:00] Response Message Status
    dword rsvd : 24;        // bit[31:08] Reserved
} nvme_mi_err_resp_generic_t;

// Invalid Parameter Error Response
typedef struct _nvme_mi_err_resp_invalid_param_t
{
    dword status : 8;       // Bit[07:00] Response Message Status
    /** Bit[10:08]
     * Least-significant bit in the least-significant byte of the Request Message of the parameter
     * that contained the error. Valid values are 0 to 7.
     **/
    dword lsbit : 3;
    dword rsvd : 5;         // Bit[15:11]
    /** Bit[31:16]
     * Least-significant byte of the Request Message of the parameter that contained the error.
     * If the error is beyond byte 65,535, then the value 65,535 is reported in this field.
     **/
    dword lsbyte : 16;
} nvme_mi_err_resp_invalid_param_t;

// Tunneled NVMe Management Response and Status
typedef struct _tunneled_nvme_management_response_t
{
    dword tstat : 8;        // bit[07:00] Tunneled Status (TSTAT)
    dword tnmresp : 24;     // bit[31:08] Tunneled NVMe Management Response (TNMRESP)
} tunneled_nvme_management_response_t;

// Controller Health Status Poll – NVMe Management Response
typedef struct _tunneled_nvme_managemant_response_chsp_t
{
    dword status : 8;       // bit[07:00] Response Message Status
    dword rsvd : 16;        // bit[23:08] Reserved
    dword rent : 8;         // bit[31:24] Response Entries (RENT)
} tunneled_nvme_managemant_response_chsp_t;

// NVMe-MI Receive – Completion Queue Entry Dword 0 (NRCQED0)
typedef union _nrcqe_dw0_t
{
    tunneled_nvme_management_response_t common;         // Tunneled NVMe Management Response and Status
    tunneled_nvme_managemant_response_chsp_t chsp;      // Controller Health Status Poll – NVMe Management Response
    nvme_mi_err_resp_generic_t generic_err;             // Generic Error Response
    nvme_mi_err_resp_invalid_param_t invalid_param;     // Invalid Parameter Error Response
    dword config;
} nrcqe_dw0_t;

typedef struct _host_metadata_t
{
    byte desc_cnt;      // Number of Metadata Element Descriptors
    byte rsvd;
    byte buf[4094];
} __attribute__((packed)) host_metadata_t;

typedef struct _meta_ele_desc_t
{
    byte et;            // Element Type : 5;
    byte er;            // Element Revision : 4
    byte elen[2];       // Element Length : 16
} __attribute__((packed)) meta_ele_desc_t;

// Metadata Element Records
typedef struct _meta_ele_record_t
{
    byte et;
    byte src;
    byte rsvd1[2];  // 1dw
    word offset;
    word length;    // 2dw
    list_t entry;   // 4dw
} __attribute__((packed)) meta_ele_record_t;

typedef struct _ctrl_meta_t
{
    meta_ele_record_t record[CTRL_META_ELEMENT_TYPE_MAX];
    list_t valid;
    list_t empty;
    word valid_cnt;
    short metadata_size;
} ctrl_meta_t;

typedef enum _controller_metedata_element_type_e
{
    operating_system_controller_name = 0x01,
    operating_system_driver_name = 0x02,
    operating_system_driver_cersion = 0x03,
    pre_boot_controller_name = 0x04,
    pre_boot_driver_name = 0x05,
    pre_boot_driver_version = 0x06,
} controller_metedata_element_type_e;

typedef enum _element_action_e
{
    update_entry = 0x0,
    delete_entry = 0x1,
} element_action_e;

typedef enum _nvme_mi_resp_status_e
{
    NVME_MI_RESP_SUCCESS = 0x00,
    NVME_MI_RESP_MPR = 0x01,
    NVME_MI_RESP_INTERNAL_ERR = 0x02,
    NVME_MI_RESP_INVALID_OPCODE = 0x03,
    NVME_MI_RESP_INVALID_PARAM = 0x04,
    NVME_MI_RESP_INVALID_CMD_SIZE = 0x05,
    NVME_MI_RESP_INVALID_INPUT_SIZE = 0x06,
    NVME_MI_RESP_ACCESS_DENIED = 0x07,
    /* 0x08 - 0x1f: reserved */
    NVME_MI_RESP_VPD_UPDATES_EXCEEDED = 0x20,
    NVME_MI_RESP_PCIE_INACCESSIBLE = 0x21,
    NVME_MI_RESP_MEB_SANITIZED = 0x22,
    NVME_MI_RESP_ENC_SERV_FAILURE = 0x23,
    NVME_MI_RESP_ENC_SERV_XFER_FAILURE = 0x24,
    NVME_MI_RESP_ENC_FAILURE = 0x25,
    NVME_MI_RESP_ENC_XFER_REFUSED = 0x26,
    NVME_MI_RESP_ENC_FUNC_UNSUP = 0x27,
    NVME_MI_RESP_ENC_SERV_UNAVAIL = 0x28,
    NVME_MI_RESP_ENC_DEGRADED = 0x29,
    NVME_MI_RESP_SANITIZE_IN_PROGRESS = 0x2a,
    /* 0x2b - 0xdf: reserved */
    /* 0xe0 - 0xff: vendor specific */
} nvme_mi_resp_status_e;

void nvme_mi_init(void);
void nvme_mi_deinit(void);

#endif // ~ NVME_MI_H