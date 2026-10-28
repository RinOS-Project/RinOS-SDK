/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_USB_UVC_CAPTURE_OWNER_V1_H
#define RIN_DRIVER_USB_UVC_CAPTURE_OWNER_V1_H

#include <stdint.h>

#include <rin/capture.h>
#include <rin/driver/capture/shared_queue_v1.h>
#include <rin/driver/device_resource_v1.h>
#include <rin/driver/usb/uvc_capture_v1.h>
#include <rinruntime/rin_capture_transport.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_UVC_CAPTURE_OWNER_VERSION UINT32_C(1)
#define RIN_UVC_CAPTURE_MAX_DEVICES UINT32_C(4)
#define RIN_UVC_CAPTURE_HANDLE_SLOT_BITS UINT32_C(8)
#define RIN_UVC_CAPTURE_HANDLE_GENERATION_MAX \
    (UINT64_MAX >> RIN_UVC_CAPTURE_HANDLE_SLOT_BITS)
#define RIN_UVC_CAPTURE_MAX_DESCRIPTOR_BYTES UINT32_C(4096)
#define RIN_UVC_CAPTURE_MAX_PACKET_BYTES RIN_CAPTURE_MAX_CHUNK_BYTES
#define RIN_UVC_CAPTURE_OWNER_STORAGE_BYTES UINT32_C(4096)

/* Shared storage for the OS-Core UVC capture owner. Callers must zero the
 * storage before initialization and must not inspect or copy it while live.
 * The implementation owns the layout; this fixed-size opaque record lets
 * Kernel and Driver code share the service without importing private headers.
 */
typedef union RinUvcCaptureOwner {
    uint64_t alignment;
    uint8_t opaque[RIN_UVC_CAPTURE_OWNER_STORAGE_BYTES];
} RinUvcCaptureOwner;

typedef struct RinUvcCaptureAttachV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_cookie;
    uint64_t device_generation;
    const uint8_t* descriptors;
    uint32_t descriptor_size;
    uint8_t format_index;
    uint8_t frame_index;
    uint16_t endpoint_address;
    uint32_t frame_interval_100ns;
    uint8_t* frame_storage;
    uint32_t frame_storage_size;
    uint8_t* packet_storage;
    uint32_t packet_storage_size;
    void* context;
    RinUvcCaptureStartFn start;
    RinUvcCapturePullPacketFn pull_packet;
    RinUvcCaptureStopFn stop;
    RinCaptureSharedQueueV1* shared_queue;
    uint64_t reserved[4];
} RinUvcCaptureAttachV1;

/* Packet/frame storage remains caller-owned until exact detach succeeds.
 * USB enumeration, DMA permission, and capture privacy remain separate
 * OS-Core services. */
/* Returns 1 only after initialization has completed; otherwise returns 0. */
int32_t rin_uvc_capture_owner_is_initialized(
    const RinUvcCaptureOwner* owner);
int32_t rin_uvc_capture_owner_init(RinUvcCaptureOwner* owner);
int32_t rin_uvc_capture_owner_is_initialized(
    const RinUvcCaptureOwner* owner);
int32_t rin_uvc_capture_owner_attach(
    RinUvcCaptureOwner* owner, const RinUvcCaptureAttachV1* attach);
int32_t rin_uvc_capture_owner_detach(
    RinUvcCaptureOwner* owner, uint64_t device_cookie,
    uint64_t device_generation);
int32_t rin_uvc_capture_owner_profile(
    RinUvcCaptureOwner* owner, uint64_t device_cookie,
    uint64_t device_generation, RinCaptureTransportProfileV1* profile_out);
int32_t rin_uvc_capture_owner_pump(
    RinUvcCaptureOwner* owner, uint64_t transport_cookie,
    uint32_t packet_budget, uint32_t* packets_processed_out);
int32_t rin_uvc_capture_owner_make_transport(
    RinUvcCaptureOwner* owner, RinCaptureTransportV1* transport_out);

#if defined(__cplusplus)
static_assert(sizeof(RinUvcCaptureOwner) == RIN_UVC_CAPTURE_OWNER_STORAGE_BYTES,
              "UVC capture owner storage ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUvcCaptureOwner) == RIN_UVC_CAPTURE_OWNER_STORAGE_BYTES,
               "UVC capture owner storage ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_USB_UVC_CAPTURE_OWNER_V1_H */
