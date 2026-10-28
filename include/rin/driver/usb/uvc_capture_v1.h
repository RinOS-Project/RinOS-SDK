/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_USB_UVC_CAPTURE_V1_H
#define RIN_DRIVER_USB_UVC_CAPTURE_V1_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_DRIVER_UVC_CAPTURE_CALLBACKS_V1_VERSION UINT32_C(0x00010000)

/* OS-Core calls start/stop for the exact admitted device generation. A pull
 * returns one complete isochronous payload packet and its arrival timestamp;
 * packet storage is borrowed for the call only. */
typedef int32_t (*RinUvcCaptureStartFn)(
    void* context, uint64_t device_cookie, uint64_t device_generation,
    uint64_t* stream_cookie_out);
typedef int32_t (*RinUvcCapturePullPacketFn)(
    void* context, uint64_t stream_cookie, void* packet_out,
    uint32_t packet_capacity, uint32_t* packet_size_out,
    uint64_t* occurred_ns_out);
typedef int32_t (*RinUvcCaptureStopFn)(
    void* context, uint64_t stream_cookie);

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_USB_UVC_CAPTURE_V1_H */
