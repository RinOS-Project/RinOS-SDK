/* SPDX-License-Identifier: MIT */
/* Public raw-frame ABI for the RinAWDL user-space transport adapter. */
#ifndef RIN_SDK_NET_WIFI_AWDL_TRANSPORT_H
#define RIN_SDK_NET_WIFI_AWDL_TRANSPORT_H

#include <stdint.h>
#include <rin/syscall_abi.h>

#define RIN_WIFI_AWDL_TRANSPORT_VERSION UINT32_C(1)
#define RIN_WIFI_AWDL_TRANSPORT_MAX_FRAME_SIZE UINT32_C(2304)

typedef enum RinWifiAwdlTransportOperationV1 {
    RIN_WIFI_AWDL_TRANSPORT_RX = 1,
    RIN_WIFI_AWDL_TRANSPORT_TX = 2,
    RIN_WIFI_AWDL_TRANSPORT_AUTHORIZE_AND_PROGRAM_CHANNEL = 3
} RinWifiAwdlTransportOperationV1;

/* The caller obtains interface identity and generations from the network
 * interface snapshot ABI. RX timestamps are kernel monotonic dequeue times,
 * not fabricated radio or TSF timestamps. The request requires the network
 * administration capability. */
typedef struct RinWifiAwdlTransportRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t flags;
    uint64_t interface_id;
    uint64_t device_generation;
    uint64_t frame_buffer;
    uint64_t rx_dequeue_time_us;
    uint32_t interface_generation;
    uint32_t frame_size;
    uint32_t frame_capacity;
    uint32_t reserved0;
    uint8_t band;
    uint8_t channel;
    uint16_t channel_width_mhz;
    uint32_t reserved1;
    uint64_t reserved[2];
} RinWifiAwdlTransportRequestV1;

static inline int rin_wifi_awdl_transport_request_v1_valid(
    const RinWifiAwdlTransportRequestV1* request)
{
    if (request == 0 || request->struct_size != sizeof(*request) ||
        request->version != RIN_WIFI_AWDL_TRANSPORT_VERSION ||
        request->flags != 0u || request->interface_id == 0u ||
        request->interface_generation == 0u ||
        request->device_generation == 0u || request->reserved0 != 0u ||
        request->reserved1 != 0u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u)
        return 0;
    switch (request->operation) {
    case RIN_WIFI_AWDL_TRANSPORT_TX:
        return request->frame_buffer != 0u && request->frame_size != 0u &&
               request->frame_size <= RIN_WIFI_AWDL_TRANSPORT_MAX_FRAME_SIZE &&
               request->frame_capacity == request->frame_size &&
               request->rx_dequeue_time_us == 0u && request->band == 0u &&
               request->channel == 0u && request->channel_width_mhz == 0u;
    case RIN_WIFI_AWDL_TRANSPORT_RX:
        return request->frame_buffer != 0u && request->frame_size == 0u &&
               request->frame_capacity ==
                   RIN_WIFI_AWDL_TRANSPORT_MAX_FRAME_SIZE &&
               request->rx_dequeue_time_us == 0u && request->band == 0u &&
               request->channel == 0u && request->channel_width_mhz == 0u;
    case RIN_WIFI_AWDL_TRANSPORT_AUTHORIZE_AND_PROGRAM_CHANNEL:
        return request->frame_buffer == 0u && request->frame_size == 0u &&
               request->frame_capacity == 0u &&
               request->rx_dequeue_time_us == 0u && request->band != 0u &&
               request->channel != 0u && request->channel_width_mhz != 0u;
    default:
        return 0;
    }
}

#if defined(__cplusplus)
static_assert(sizeof(RinWifiAwdlTransportRequestV1) == 88u,
              "Wi-Fi AWDL transport ABI drift");
#else
_Static_assert(sizeof(RinWifiAwdlTransportRequestV1) == 88u,
               "Wi-Fi AWDL transport ABI drift");
#endif

#endif /* RIN_SDK_NET_WIFI_AWDL_TRANSPORT_H */
