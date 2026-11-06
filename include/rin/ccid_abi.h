/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_CCID_ABI_H
#define RIN_SDK_CCID_ABI_H

#include <stdint.h>

#define RIN_DEVICE_CONTROL_CCID UINT32_C(2)
#define RIN_DEVICE_CONTROL_CCID_GET_INFO UINT32_C(3)
#define RIN_CCID_DEVICE_INFO_VERSION_V1 UINT32_C(1)
#define RIN_CCID_CONTROL_VERSION_V1 UINT32_C(1)
#define RIN_CCID_CONTROL_MAX_TIMEOUT_MS UINT32_C(30000)
#define RIN_CCID_CONTROL_MAX_MESSAGE_BYTES UINT32_C(65554)

#define RIN_CCID_CONTROL_POWER_ON UINT32_C(1)
#define RIN_CCID_CONTROL_POWER_OFF UINT32_C(2)
#define RIN_CCID_CONTROL_GET_SLOT_STATUS UINT32_C(3)
#define RIN_CCID_CONTROL_MECHANICAL UINT32_C(4)
#define RIN_CCID_CONTROL_XFR_BLOCK UINT32_C(5)
#define RIN_CCID_CONTROL_POLL_SLOT_CHANGE UINT32_C(6)

#define RIN_CCID_CAP_POWER_ON UINT32_C(0x00000001)
#define RIN_CCID_CAP_POWER_OFF UINT32_C(0x00000002)
#define RIN_CCID_CAP_GET_SLOT_STATUS UINT32_C(0x00000004)
#define RIN_CCID_CAP_MECHANICAL UINT32_C(0x00000008)
#define RIN_CCID_CAP_XFR_BLOCK UINT32_C(0x00000010)
#define RIN_CCID_CAP_POLL_SLOT_CHANGE UINT32_C(0x00000020)
#define RIN_CCID_CAPABILITY_MASK UINT32_C(0x0000003f)

typedef struct RinCcidDeviceInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_id;
    uint64_t generation;
    uint32_t root_port_index;
    uint32_t root_port_generation;
    uint32_t max_message_length;
    uint32_t capabilities;
    uint8_t max_slot_index;
    uint8_t reserved0[7];
    uint64_t reserved[2];
} RinCcidDeviceInfoV1;

/* Input header followed by data_length request bytes. Only XFR_BLOCK accepts
 * request bytes. parameter is the voltage selector for POWER_ON, mechanical
 * function for MECHANICAL, or block-waiting integer for XFR_BLOCK. */
typedef struct RinCcidControlRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t timeout_ms;
    uint32_t slot;
    uint32_t parameter;
    uint32_t data_length;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinCcidControlRequestV1;

/* Output header followed by ATR/APDU response bytes. Poll-slot-change puts
 * the low 32 slot presence/change bits in these fields. */
typedef struct RinCcidControlResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t data_length;
    uint8_t command_status;
    uint8_t error;
    uint8_t chain_parameter;
    uint8_t reserved0;
    uint32_t card_inserted_mask;
    uint32_t card_changed_mask;
    uint32_t reserved[3];
} RinCcidControlResponseV1;

#endif /* RIN_SDK_CCID_ABI_H */
