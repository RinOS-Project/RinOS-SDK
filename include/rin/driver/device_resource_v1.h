/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_DEVICE_RESOURCE_V1_H
#define RIN_DRIVER_DEVICE_RESOURCE_V1_H

#include <stdint.h>

#include <rin/audio/midi.h>
#include <rin/capture.h>
#include <rin/location.h>
#include <rin/resource/capability.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_DEVICE_RESOURCE_ENDPOINT_VERSION 1u
#define RIN_DEVICE_RESOURCE_PROVIDER_NOT_SUPPORTED INT32_C(1)
#define RIN_DEVICE_RESOURCE_PROVIDER_PROTOCOL_ERROR INT32_C(2)
#define RIN_DEVICE_RESOURCE_PROVIDER_INVALID_ARGUMENT INT32_C(3)
#define RIN_DEVICE_RESOURCE_PROVIDER_WOULD_BLOCK INT32_C(4)

#define RIN_DEVICE_LOCATION_BACKEND_VERSION 1u
#define RIN_DEVICE_MIDI_BACKEND_VERSION 1u
#define RIN_DEVICE_CAPTURE_BACKEND_VERSION 2u

typedef enum RinDeviceResourceEndpointStatus {
    RIN_DEVICE_RESOURCE_ENDPOINT_OK = 0,
    RIN_DEVICE_RESOURCE_ENDPOINT_INVALID_ARGUMENT = -1,
    RIN_DEVICE_RESOURCE_ENDPOINT_DENIED = -2,
    RIN_DEVICE_RESOURCE_ENDPOINT_NOT_FOUND = -3,
    RIN_DEVICE_RESOURCE_ENDPOINT_LIMIT_EXCEEDED = -4,
    RIN_DEVICE_RESOURCE_ENDPOINT_BACKEND_FAILURE = -5,
    RIN_DEVICE_RESOURCE_ENDPOINT_BACKEND_PROTOCOL = -6,
    RIN_DEVICE_RESOURCE_ENDPOINT_NOT_SUPPORTED = -7,
    RIN_DEVICE_RESOURCE_ENDPOINT_BUSY = -8,
    RIN_DEVICE_RESOURCE_ENDPOINT_WOULD_BLOCK = -9
} RinDeviceResourceEndpointStatus;

typedef int32_t (*RinDeviceResourceProviderOpenFn)(
    void* context, const RinDeviceCapabilitySubjectV1* subject,
    uint32_t resource, uint32_t rights, uint64_t* object_cookie_out);
typedef int32_t (*RinDeviceResourceProviderCallFn)(
    void* context, uint64_t object_cookie,
    const RinDeviceCapabilitySubjectV1* subject, uint32_t resource,
    uint32_t required_right, uint32_t command, void* buffer,
    uint32_t buffer_size, uint32_t* transferred_out);
typedef int32_t (*RinDeviceResourceProviderReleaseFn)(
    void* context, uint32_t resource, uint64_t object_cookie);

/* Generic provider contract. The OS snapshots this table and owns subject
 * authentication, resource handles, rights checks and endpoint lifetime. */
typedef struct RinDeviceResourceProviderV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    void* context;
    RinDeviceResourceProviderOpenFn open;
    RinDeviceResourceProviderCallFn call;
    RinDeviceResourceProviderReleaseFn release;
    uint64_t reserved[4];
} RinDeviceResourceProviderV1;

typedef int32_t (*RinDeviceLocationBackendOpenFn)(
    void* context, const RinDeviceCapabilitySubjectV1* subject,
    uint64_t* session_cookie_out);
typedef int32_t (*RinDeviceLocationBackendGetFixFn)(
    void* context, uint64_t session_cookie,
    const RinDeviceCapabilitySubjectV1* subject, RinLocationFixV1* fix_out);
typedef int32_t (*RinDeviceLocationBackendReleaseFn)(
    void* context, uint64_t session_cookie);

/* Trusted location provider contract. OS-Core retains privacy permission and
 * session ownership; a driver supplies only the location transport. */
typedef struct RinDeviceLocationBackendV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    void* context;
    RinDeviceLocationBackendOpenFn open;
    RinDeviceLocationBackendGetFixFn get_fix;
    RinDeviceLocationBackendReleaseFn release;
    uint64_t reserved[4];
} RinDeviceLocationBackendV1;

typedef int32_t (*RinDeviceMidiBackendOpenFn)(
    void* context, const RinDeviceCapabilitySubjectV1* subject,
    uint32_t rights, uint64_t* session_cookie_out);
typedef int32_t (*RinDeviceMidiBackendReceiveFn)(
    void* context, uint64_t session_cookie,
    const RinDeviceCapabilitySubjectV1* subject, RinMidiPacketV1* packet_out);
typedef int32_t (*RinDeviceMidiBackendSendFn)(
    void* context, uint64_t session_cookie,
    const RinDeviceCapabilitySubjectV1* subject,
    const RinMidiPacketV1* packet);
typedef int32_t (*RinDeviceMidiBackendReleaseFn)(
    void* context, uint64_t session_cookie);

/* Trusted MIDI provider contract; device access stays mediated by OS-Core. */
typedef struct RinDeviceMidiBackendV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    void* context;
    RinDeviceMidiBackendOpenFn open;
    RinDeviceMidiBackendReceiveFn receive;
    RinDeviceMidiBackendSendFn send;
    RinDeviceMidiBackendReleaseFn release;
    uint64_t reserved[4];
} RinDeviceMidiBackendV1;

typedef int32_t (*RinDeviceCaptureBackendOpenFn)(
    void* context, const RinDeviceCapabilitySubjectV1* subject,
    uint32_t resource, uint64_t* session_cookie_out);
typedef int32_t (*RinDeviceCaptureBackendReadFn)(
    void* context, uint64_t session_cookie,
    const RinDeviceCapabilitySubjectV1* subject, uint32_t resource,
    void* buffer, uint32_t buffer_capacity, uint32_t* transferred_out);
typedef int32_t (*RinDeviceCaptureBackendReleaseFn)(
    void* context, uint32_t resource, uint64_t session_cookie);
typedef int32_t (*RinDeviceCaptureBackendGetProfileFn)(
    void* context, uint64_t session_cookie,
    const RinDeviceCapabilitySubjectV1* subject, uint32_t resource,
    RinCaptureTransportProfileV1* profile_out);

/* Trusted capture provider contract. A read callback writes one header and
 * its payload into the zeroed buffer supplied by the OS service. */
typedef struct RinDeviceCaptureBackendV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    void* context;
    RinDeviceCaptureBackendOpenFn open;
    RinDeviceCaptureBackendReadFn read_chunk;
    RinDeviceCaptureBackendReleaseFn release;
    RinDeviceCaptureBackendGetProfileFn get_profile;
    uint64_t reserved[4];
} RinDeviceCaptureBackendV1;

#if defined(__cplusplus)
static_assert(sizeof(RinDeviceResourceEndpointStatus) == 4u,
              "device resource status ABI drift");
static_assert(sizeof(RinDeviceResourceProviderV1) ==
                  (sizeof(void*) == 8u ? 72u : 56u),
              "device resource provider ABI drift");
static_assert(sizeof(RinDeviceLocationBackendV1) ==
                  (sizeof(void*) == 8u ? 72u : 56u),
              "location backend ABI drift");
static_assert(sizeof(RinDeviceMidiBackendV1) ==
                  (sizeof(void*) == 8u ? 80u : 60u),
              "MIDI backend ABI drift");
static_assert(sizeof(RinDeviceCaptureBackendV1) ==
                  (sizeof(void*) == 8u ? 80u : 60u),
              "capture backend ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDeviceResourceEndpointStatus) == 4u,
               "device resource status ABI drift");
_Static_assert(sizeof(RinDeviceResourceProviderV1) ==
                   (sizeof(void*) == 8u ? 72u : 56u),
               "device resource provider ABI drift");
_Static_assert(sizeof(RinDeviceLocationBackendV1) ==
                   (sizeof(void*) == 8u ? 72u : 56u),
               "location backend ABI drift");
_Static_assert(sizeof(RinDeviceMidiBackendV1) ==
                   (sizeof(void*) == 8u ? 80u : 60u),
               "MIDI backend ABI drift");
_Static_assert(sizeof(RinDeviceCaptureBackendV1) ==
                   (sizeof(void*) == 8u ? 80u : 60u),
               "capture backend ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_DEVICE_RESOURCE_V1_H */
