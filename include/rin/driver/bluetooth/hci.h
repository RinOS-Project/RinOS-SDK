/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_BLUETOOTH_HCI_H
#define RIN_DRIVER_BLUETOOTH_HCI_H

#include <rin/driver/module_api_v3.h>

#define RIN_DRIVER_V3_BLUETOOTH_HCI_INFO_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_PACKET_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_COMPLETION_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_COMMAND UINT32_C(1)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_ACL UINT32_C(2)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_SCO UINT32_C(3)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_ISO UINT32_C(4)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_EVENT UINT32_C(5)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_MAX_PACKET UINT32_C(1048576)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_INFO_FLAG_ADDRESS_VALID UINT16_C(0x0001)
#define RIN_DRIVER_V3_BLUETOOTH_HCI_INFO_FLAG_MASK \
    RIN_DRIVER_V3_BLUETOOTH_HCI_INFO_FLAG_ADDRESS_VALID

typedef struct RinDriverV3BluetoothHciInfo {
    uint32_t struct_size;
    uint32_t version;
    uint16_t hci_version;
    uint16_t hci_revision;
    uint16_t manufacturer;
    uint16_t flags;
    uint32_t max_command_payload;
    uint32_t max_event_payload;
    uint32_t max_acl_payload;
    uint32_t max_sco_payload;
    uint32_t max_iso_payload;
    uint8_t address[6];
    uint16_t reserved0;
    uint32_t reserved1;
    uint64_t reserved[2];
} RinDriverV3BluetoothHciInfo;

typedef struct RinDriverV3BluetoothHciPacket {
    uint32_t struct_size;
    uint32_t version;
    uint32_t packet_type;
    uint32_t flags;
    uint64_t request_id;
    uint64_t deadline_ns;
    uint64_t memory_lease;
    uint64_t offset;
    uint32_t length;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverV3BluetoothHciPacket;

typedef struct RinDriverV3BluetoothHciCompletion {
    uint32_t struct_size;
    uint32_t version;
    uint64_t request_id;
    int32_t result;
    uint32_t packet_type;
    uint32_t transferred;
    uint32_t flags;
    uint64_t reserved[2];
} RinDriverV3BluetoothHciCompletion;

typedef RinDriverV3Result (*RinDriverV3BluetoothHciGetInfoFn)(
    uint64_t driver_context, RinDriverV3BluetoothHciInfo* output);
typedef RinDriverV3Result (*RinDriverV3BluetoothHciResetFn)(
    uint64_t driver_context, uint64_t deadline_ns);
/* submit copies the record before returning and retains only the packet's
 * memory lease. poll_packet copies one complete HCI payload into caller-owned
 * transient memory and re-arms its receive buffer before returning. */
typedef RinDriverV3Result (*RinDriverV3BluetoothHciSubmitFn)(
    uint64_t driver_context, const RinDriverV3BluetoothHciPacket* packet);
typedef RinDriverV3Result (*RinDriverV3BluetoothHciPollCompletionsFn)(
    uint64_t driver_context,
    RinDriverV3BluetoothHciCompletion* completions, uint32_t capacity,
    uint32_t* completion_count);
typedef RinDriverV3Result (*RinDriverV3BluetoothHciPollPacketFn)(
    uint64_t driver_context, uint32_t* packet_type, void* output,
    uint32_t capacity, uint32_t* packet_length);
typedef RinDriverV3Result (*RinDriverV3BluetoothHciCancelFn)(
    uint64_t driver_context, uint64_t request_id, uint64_t deadline_ns);

typedef struct RinDriverV3BluetoothHciOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3BluetoothHciGetInfoFn get_info;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_info_padding;
#endif
    RinDriverV3BluetoothHciResetFn reset;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t reset_padding;
#endif
    RinDriverV3BluetoothHciSubmitFn submit;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t submit_padding;
#endif
    RinDriverV3BluetoothHciPollCompletionsFn poll_completions;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_completions_padding;
#endif
    RinDriverV3BluetoothHciPollPacketFn poll_packet;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_packet_padding;
#endif
    RinDriverV3BluetoothHciCancelFn cancel;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t cancel_padding;
#endif
    RinDriverV3DeviceRemoveFn remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t remove_padding;
#endif
    uint64_t reserved[2];
} RinDriverV3BluetoothHciOps;

static inline int rin_driver_v3_bluetooth_hci_info_valid(
    const RinDriverV3BluetoothHciInfo* info) {
    uint8_t address_nonzero = 0u;
    if (!info || info->struct_size < sizeof(*info) ||
        info->version != RIN_DRIVER_V3_BLUETOOTH_HCI_INFO_VERSION ||
        info->hci_version == 0u || info->max_command_payload < 3u ||
        info->max_event_payload < 2u ||
        info->max_acl_payload < 4u || info->reserved0 != 0u ||
        info->reserved1 != 0u ||
        (info->flags & ~RIN_DRIVER_V3_BLUETOOTH_HCI_INFO_FLAG_MASK) != 0u ||
        info->reserved[0] != 0u || info->reserved[1] != 0u)
        return 0;
    for (uint32_t index = 0u; index < sizeof(info->address); ++index)
        address_nonzero |= info->address[index];
    if ((info->flags & RIN_DRIVER_V3_BLUETOOTH_HCI_INFO_FLAG_ADDRESS_VALID) ==
        0u)
        return address_nonzero == 0u;
    return address_nonzero != 0u && (info->address[0] & 1u) == 0u;
}

static inline int rin_driver_v3_bluetooth_hci_packet_valid(
    const RinDriverV3BluetoothHciPacket* packet,
    const RinDriverV3BluetoothHciInfo* info) {
    uint32_t packet_limit;
    uint32_t minimum;
    if (!packet || !rin_driver_v3_bluetooth_hci_info_valid(info) ||
        packet->struct_size < sizeof(*packet) ||
        packet->version != RIN_DRIVER_V3_BLUETOOTH_HCI_PACKET_VERSION ||
        packet->flags != 0u || packet->request_id == 0u ||
        packet->deadline_ns == 0u || packet->memory_lease == 0u ||
        packet->length == 0u ||
        packet->length > RIN_DRIVER_V3_BLUETOOTH_HCI_MAX_PACKET ||
        packet->offset > UINT64_MAX - packet->length ||
        packet->reserved0 != 0u || packet->reserved[0] != 0u ||
        packet->reserved[1] != 0u)
        return 0;
    switch (packet->packet_type) {
    case RIN_DRIVER_V3_BLUETOOTH_HCI_COMMAND:
        minimum = 3u;
        packet_limit = info->max_command_payload;
        break;
    case RIN_DRIVER_V3_BLUETOOTH_HCI_ACL:
        minimum = 4u;
        packet_limit = info->max_acl_payload;
        break;
    case RIN_DRIVER_V3_BLUETOOTH_HCI_SCO:
        minimum = 3u;
        packet_limit = info->max_sco_payload;
        break;
    case RIN_DRIVER_V3_BLUETOOTH_HCI_ISO:
        minimum = 4u;
        packet_limit = info->max_iso_payload;
        break;
    default:
        return 0;
    }
    return packet_limit >= minimum && packet->length >= minimum &&
           packet->length <= packet_limit;
}

static inline int rin_driver_v3_bluetooth_hci_completion_valid(
    const RinDriverV3BluetoothHciCompletion* completion,
    const RinDriverV3BluetoothHciPacket* packet) {
    return completion && packet &&
           completion->struct_size >= sizeof(*completion) &&
           completion->version ==
               RIN_DRIVER_V3_BLUETOOTH_HCI_COMPLETION_VERSION &&
           completion->request_id == packet->request_id &&
           completion->result <= RIN_DRIVER_V3_OK &&
           completion->result >= RIN_DRIVER_V3_UNSUPPORTED &&
           completion->packet_type == packet->packet_type &&
           completion->transferred <= packet->length &&
           completion->flags == 0u && completion->reserved[0] == 0u &&
           completion->reserved[1] == 0u &&
           (completion->result == RIN_DRIVER_V3_OK ||
            completion->transferred == 0u);
}

static inline int rin_driver_v3_bluetooth_hci_ops_valid(
    const RinDriverV3BluetoothHciOps* ops) {
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_OPS_VERSION || !ops->get_info ||
        !ops->reset || !ops->submit || !ops->poll_completions ||
        !ops->poll_packet || !ops->cancel || !ops->remove ||
        ops->reserved[0] != 0u || ops->reserved[1] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->get_info_padding != 0u || ops->reset_padding != 0u ||
        ops->submit_padding != 0u ||
        ops->poll_completions_padding != 0u ||
        ops->poll_packet_padding != 0u || ops->cancel_padding != 0u ||
        ops->remove_padding != 0u)
        return 0;
#endif
    return 1;
}

static inline int rin_driver_v3_bluetooth_hci_packet_output_valid(
    uint32_t packet_type, uint32_t packet_length,
    const RinDriverV3BluetoothHciInfo* info) {
    uint32_t minimum;
    uint32_t maximum;
    if (!rin_driver_v3_bluetooth_hci_info_valid(info)) return 0;
    switch (packet_type) {
    case RIN_DRIVER_V3_BLUETOOTH_HCI_COMMAND:
        minimum = 3u;
        maximum = info->max_command_payload;
        break;
    case RIN_DRIVER_V3_BLUETOOTH_HCI_ACL:
        minimum = 4u;
        maximum = info->max_acl_payload;
        break;
    case RIN_DRIVER_V3_BLUETOOTH_HCI_SCO:
        minimum = 3u;
        maximum = info->max_sco_payload;
        break;
    case RIN_DRIVER_V3_BLUETOOTH_HCI_ISO:
        minimum = 4u;
        maximum = info->max_iso_payload;
        break;
    case RIN_DRIVER_V3_BLUETOOTH_HCI_EVENT:
        minimum = 2u;
        maximum = info->max_event_payload;
        break;
    default:
        return 0;
    }
    return maximum >= minimum && packet_length >= minimum &&
           packet_length <= maximum;
}

#if defined(__cplusplus)
static_assert(sizeof(RinDriverV3BluetoothHciInfo) == 64u,
              "NDRV v3 Bluetooth HCI info ABI drift");
static_assert(sizeof(RinDriverV3BluetoothHciPacket) == 72u,
              "NDRV v3 Bluetooth HCI packet ABI drift");
static_assert(sizeof(RinDriverV3BluetoothHciCompletion) == 48u,
              "NDRV v3 Bluetooth HCI completion ABI drift");
static_assert(sizeof(RinDriverV3BluetoothHciOps) == 80u,
              "NDRV v3 Bluetooth HCI operation ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverV3BluetoothHciInfo) == 64u,
               "NDRV v3 Bluetooth HCI info ABI drift");
_Static_assert(sizeof(RinDriverV3BluetoothHciPacket) == 72u,
               "NDRV v3 Bluetooth HCI packet ABI drift");
_Static_assert(sizeof(RinDriverV3BluetoothHciCompletion) == 48u,
               "NDRV v3 Bluetooth HCI completion ABI drift");
_Static_assert(sizeof(RinDriverV3BluetoothHciOps) == 80u,
               "NDRV v3 Bluetooth HCI operation ABI drift");
#endif

#endif /* RIN_DRIVER_BLUETOOTH_HCI_H */
