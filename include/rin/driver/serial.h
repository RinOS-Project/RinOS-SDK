/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_SERIAL_H
#define RIN_DRIVER_SERIAL_H

#include <rin/driver/module_api_v3.h>

#define RIN_DRIVER_V3_SERIAL_INFO_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_SERIAL_CONFIGURATION_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_SERIAL_TRANSFER_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_SERIAL_COMPLETION_VERSION UINT32_C(0x00010000)

#define RIN_DRIVER_V3_SERIAL_PARITY_NONE UINT32_C(0)
#define RIN_DRIVER_V3_SERIAL_PARITY_ODD UINT32_C(1)
#define RIN_DRIVER_V3_SERIAL_PARITY_EVEN UINT32_C(2)
#define RIN_DRIVER_V3_SERIAL_PARITY_MARK UINT32_C(3)
#define RIN_DRIVER_V3_SERIAL_PARITY_SPACE UINT32_C(4)
#define RIN_DRIVER_V3_SERIAL_FLOW_NONE UINT32_C(0)
#define RIN_DRIVER_V3_SERIAL_FLOW_RTS_CTS UINT32_C(1)
#define RIN_DRIVER_V3_SERIAL_FLOW_XON_XOFF UINT32_C(2)
#define RIN_DRIVER_V3_SERIAL_TRANSFER_TRANSMIT UINT32_C(1)
#define RIN_DRIVER_V3_SERIAL_TRANSFER_RECEIVE UINT32_C(2)
#define RIN_DRIVER_V3_SERIAL_TRANSFER_FLAG_MASK UINT32_C(0)
#define RIN_DRIVER_V3_SERIAL_LINES_DTR UINT32_C(0x00000001)
#define RIN_DRIVER_V3_SERIAL_LINES_RTS UINT32_C(0x00000002)
#define RIN_DRIVER_V3_SERIAL_LINES_MASK \
    (RIN_DRIVER_V3_SERIAL_LINES_DTR | RIN_DRIVER_V3_SERIAL_LINES_RTS)
#define RIN_DRIVER_V3_SERIAL_MAX_TRANSFER UINT32_C(1048576)

typedef struct RinDriverV3SerialInfo {
    uint32_t struct_size;
    uint32_t version;
    uint32_t minimum_baud;
    uint32_t maximum_baud;
    uint32_t data_bits_mask;
    uint32_t stop_bits_mask;
    uint32_t parity_mask;
    uint32_t flow_control_mask;
    uint32_t modem_inputs;
    uint32_t modem_outputs;
    uint32_t pending_tx_bytes;
    uint32_t pending_rx_bytes;
    uint64_t reserved[2];
} RinDriverV3SerialInfo;

typedef struct RinDriverV3SerialConfiguration {
    uint32_t struct_size;
    uint32_t version;
    uint32_t baud_rate;
    uint32_t data_bits;
    uint32_t stop_bits;
    uint32_t parity;
    uint32_t flow_control;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverV3SerialConfiguration;

typedef struct RinDriverV3SerialTransfer {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t flags;
    uint64_t request_id;
    uint64_t deadline_ns;
    uint64_t memory_lease;
    uint64_t offset;
    uint32_t length;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverV3SerialTransfer;

typedef struct RinDriverV3SerialCompletion {
    uint32_t struct_size;
    uint32_t version;
    uint64_t request_id;
    int32_t result;
    uint32_t transferred;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverV3SerialCompletion;

typedef RinDriverV3Result (*RinDriverV3SerialGetInfoFn)(
    uint64_t driver_context, RinDriverV3SerialInfo* output);
typedef RinDriverV3Result (*RinDriverV3SerialConfigureFn)(
    uint64_t driver_context,
    const RinDriverV3SerialConfiguration* configuration);
/* submit copies the transfer record before returning and retains only its
 * named memory lease. A completion is terminal for the submitted request. */
typedef RinDriverV3Result (*RinDriverV3SerialSubmitFn)(
    uint64_t driver_context, const RinDriverV3SerialTransfer* transfer);
typedef RinDriverV3Result (*RinDriverV3SerialPollFn)(
    uint64_t driver_context, RinDriverV3SerialCompletion* completions,
    uint32_t capacity, uint32_t* completion_count);
typedef RinDriverV3Result (*RinDriverV3SerialCancelFn)(
    uint64_t driver_context, uint64_t request_id, uint64_t deadline_ns);
typedef RinDriverV3Result (*RinDriverV3SerialSetLinesFn)(
    uint64_t driver_context, uint32_t outputs, uint32_t* inputs);

typedef struct RinDriverV3SerialOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3SerialGetInfoFn get_info;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_info_padding;
#endif
    RinDriverV3SerialConfigureFn configure;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t configure_padding;
#endif
    RinDriverV3SerialSubmitFn submit;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t submit_padding;
#endif
    RinDriverV3SerialPollFn poll;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_padding;
#endif
    RinDriverV3SerialCancelFn cancel;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t cancel_padding;
#endif
    RinDriverV3SerialSetLinesFn set_lines;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t set_lines_padding;
#endif
    RinDriverV3DeviceRemoveFn remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t remove_padding;
#endif
    uint64_t reserved[2];
} RinDriverV3SerialOps;

static inline int rin_driver_v3_serial_info_valid(
    const RinDriverV3SerialInfo* info) {
    return info && info->struct_size >= sizeof(*info) &&
           info->version == RIN_DRIVER_V3_SERIAL_INFO_VERSION &&
           info->minimum_baud != 0u &&
           info->minimum_baud <= info->maximum_baud &&
           (info->data_bits_mask & ~UINT32_C(0x000003e0)) == 0u &&
           (info->stop_bits_mask & ~UINT32_C(0x00000006)) == 0u &&
           (info->parity_mask & ~UINT32_C(0x0000001f)) == 0u &&
           (info->flow_control_mask & ~UINT32_C(0x00000007)) == 0u &&
           (info->modem_inputs & ~UINT32_C(0x0000003f)) == 0u &&
           (info->modem_outputs & ~RIN_DRIVER_V3_SERIAL_LINES_MASK) == 0u &&
           info->reserved[0] == 0u && info->reserved[1] == 0u;
}

static inline int rin_driver_v3_serial_configuration_valid(
    const RinDriverV3SerialConfiguration* configuration,
    const RinDriverV3SerialInfo* info) {
    uint32_t data_bit;
    uint32_t stop_bit;
    uint32_t parity_bit;
    uint32_t flow_bit;
    if (!configuration || !rin_driver_v3_serial_info_valid(info) ||
        configuration->struct_size < sizeof(*configuration) ||
        configuration->version !=
            RIN_DRIVER_V3_SERIAL_CONFIGURATION_VERSION ||
        configuration->baud_rate < info->minimum_baud ||
        configuration->baud_rate > info->maximum_baud ||
        configuration->reserved0 != 0u ||
        configuration->reserved[0] != 0u ||
        configuration->reserved[1] != 0u)
        return 0;
    data_bit = configuration->data_bits < 32u
        ? UINT32_C(1) << configuration->data_bits : 0u;
    stop_bit = configuration->stop_bits < 32u
        ? UINT32_C(1) << configuration->stop_bits : 0u;
    parity_bit = configuration->parity < 32u
        ? UINT32_C(1) << configuration->parity : 0u;
    flow_bit = configuration->flow_control < 32u
        ? UINT32_C(1) << configuration->flow_control : 0u;
    return (info->data_bits_mask & data_bit) != 0u &&
           (info->stop_bits_mask & stop_bit) != 0u &&
           (info->parity_mask & parity_bit) != 0u &&
           (info->flow_control_mask & flow_bit) != 0u;
}

static inline int rin_driver_v3_serial_transfer_valid(
    const RinDriverV3SerialTransfer* transfer) {
    return transfer && transfer->struct_size >= sizeof(*transfer) &&
           transfer->version == RIN_DRIVER_V3_SERIAL_TRANSFER_VERSION &&
           (transfer->operation == RIN_DRIVER_V3_SERIAL_TRANSFER_TRANSMIT ||
            transfer->operation == RIN_DRIVER_V3_SERIAL_TRANSFER_RECEIVE) &&
           (transfer->flags & ~RIN_DRIVER_V3_SERIAL_TRANSFER_FLAG_MASK) == 0u &&
           transfer->request_id != 0u && transfer->deadline_ns != 0u &&
           transfer->memory_lease != 0u && transfer->length != 0u &&
           transfer->length <= RIN_DRIVER_V3_SERIAL_MAX_TRANSFER &&
           transfer->offset <= UINT64_MAX - transfer->length &&
           transfer->reserved0 == 0u && transfer->reserved[0] == 0u &&
           transfer->reserved[1] == 0u;
}

static inline int rin_driver_v3_serial_completion_valid(
    const RinDriverV3SerialCompletion* completion,
    const RinDriverV3SerialTransfer* transfer) {
    return completion && rin_driver_v3_serial_transfer_valid(transfer) &&
           completion->struct_size >= sizeof(*completion) &&
           completion->version == RIN_DRIVER_V3_SERIAL_COMPLETION_VERSION &&
           completion->request_id == transfer->request_id &&
           completion->result <= RIN_DRIVER_V3_OK &&
           completion->result >= RIN_DRIVER_V3_UNSUPPORTED &&
           completion->transferred <= transfer->length &&
           completion->flags == 0u && completion->reserved0 == 0u &&
           completion->reserved[0] == 0u && completion->reserved[1] == 0u &&
           (completion->result == RIN_DRIVER_V3_OK ||
            completion->transferred == 0u);
}

static inline int rin_driver_v3_serial_ops_valid(
    const RinDriverV3SerialOps* ops) {
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_OPS_VERSION || !ops->get_info ||
        !ops->configure || !ops->submit || !ops->poll || !ops->cancel ||
        !ops->set_lines || !ops->remove || ops->reserved[0] != 0u ||
        ops->reserved[1] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->get_info_padding != 0u || ops->configure_padding != 0u ||
        ops->submit_padding != 0u || ops->poll_padding != 0u ||
        ops->cancel_padding != 0u || ops->set_lines_padding != 0u ||
        ops->remove_padding != 0u)
        return 0;
#endif
    return 1;
}

#if defined(__cplusplus)
static_assert(sizeof(RinDriverV3SerialInfo) == 64u,
              "NDRV v3 serial info ABI drift");
static_assert(sizeof(RinDriverV3SerialConfiguration) == 48u,
              "NDRV v3 serial configuration ABI drift");
static_assert(sizeof(RinDriverV3SerialTransfer) == 72u,
              "NDRV v3 serial transfer ABI drift");
static_assert(sizeof(RinDriverV3SerialCompletion) == 48u,
              "NDRV v3 serial completion ABI drift");
static_assert(sizeof(RinDriverV3SerialOps) == 80u,
              "NDRV v3 serial operation ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverV3SerialInfo) == 64u,
               "NDRV v3 serial info ABI drift");
_Static_assert(sizeof(RinDriverV3SerialConfiguration) == 48u,
               "NDRV v3 serial configuration ABI drift");
_Static_assert(sizeof(RinDriverV3SerialTransfer) == 72u,
               "NDRV v3 serial transfer ABI drift");
_Static_assert(sizeof(RinDriverV3SerialCompletion) == 48u,
               "NDRV v3 serial completion ABI drift");
_Static_assert(sizeof(RinDriverV3SerialOps) == 80u,
               "NDRV v3 serial operation ABI drift");
#endif

#endif /* RIN_DRIVER_SERIAL_H */
