/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_SERIAL_SERVICE_V1_H
#define RIN_DRIVER_SERIAL_SERVICE_V1_H

#include <stdint.h>

#include <rin/serial_abi.h>

#define RIN_SERIAL_SERVICE_V1_VERSION UINT32_C(0x00010000)

enum {
    RIN_SERIAL_SERVICE_CALLBACK_CONFIGURE = UINT32_C(1) << 0,
    RIN_SERIAL_SERVICE_CALLBACK_CANCEL = UINT32_C(1) << 1,
    RIN_SERIAL_SERVICE_CALLBACK_TRANSMIT = UINT32_C(1) << 2
};

typedef int (*RinSerialServiceGetInfoV1Fn)(
    void* context, RinSerialCapabilityV1 capability,
    RinSerialDeviceInfoV1* info_out);
typedef int (*RinSerialServiceGetCallbackMaskV1Fn)(
    void* context, RinSerialCapabilityV1 capability, uint32_t* mask_out);
typedef int (*RinSerialServiceOpenV1Fn)(
    void* context, RinSerialCapabilityV1 capability, uint64_t owner,
    uint32_t privileges);
typedef int (*RinSerialServiceCloseV1Fn)(
    void* context, RinSerialCapabilityV1 capability, uint64_t owner);
typedef int (*RinSerialServiceConfigureV1Fn)(
    void* context, RinSerialCapabilityV1 capability, uint64_t owner,
    const RinSerialConfigV1* config);
typedef int (*RinSerialServiceReadV1Fn)(
    void* context, RinSerialCapabilityV1 capability, uint64_t owner,
    uint8_t* output, uint32_t capacity, uint32_t* count_out);
typedef int (*RinSerialServiceWriteV1Fn)(
    void* context, RinSerialCapabilityV1 capability, uint64_t owner,
    const uint8_t* input, uint32_t length, uint32_t* count_out);
typedef int (*RinSerialServiceCancelV1Fn)(
    void* context, RinSerialCapabilityV1 capability, uint64_t owner);

/* A Kernel-provided serial service is scoped by the capability and owner
 * passed to each operation. context is opaque provider state and must not be
 * inspected by clients. The provider validates every capability and owner;
 * its callbacks and context remain valid until the Kernel revokes the service. */
typedef struct RinSerialServiceV1 {
    uint32_t version;
    uint32_t struct_size;
    void* context;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t context_padding;
#endif
    RinSerialServiceGetInfoV1Fn get_info;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_info_padding;
#endif
    RinSerialServiceGetCallbackMaskV1Fn get_callback_mask;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_callback_mask_padding;
#endif
    RinSerialServiceOpenV1Fn open;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t open_padding;
#endif
    RinSerialServiceCloseV1Fn close;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t close_padding;
#endif
    RinSerialServiceConfigureV1Fn configure;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t configure_padding;
#endif
    RinSerialServiceReadV1Fn read;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t read_padding;
#endif
    RinSerialServiceWriteV1Fn write;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t write_padding;
#endif
    RinSerialServiceCancelV1Fn cancel;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t cancel_padding;
#endif
    uint64_t reserved[2];
} RinSerialServiceV1;

static inline int rin_serial_service_v1_valid(
    const RinSerialServiceV1* service) {
    if (!service || service->version != RIN_SERIAL_SERVICE_V1_VERSION ||
        service->struct_size != sizeof(*service) || !service->context ||
        !service->get_info || !service->get_callback_mask || !service->open ||
        !service->close || !service->configure || !service->read ||
        !service->write || !service->cancel || service->reserved[0] != 0u ||
        service->reserved[1] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (service->context_padding != 0u || service->get_info_padding != 0u ||
        service->get_callback_mask_padding != 0u ||
        service->open_padding != 0u || service->close_padding != 0u ||
        service->configure_padding != 0u || service->read_padding != 0u ||
        service->write_padding != 0u || service->cancel_padding != 0u)
        return 0;
#endif
    return 1;
}

#if defined(__cplusplus)
static_assert(sizeof(RinSerialServiceV1) == 96u,
              "serial service ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinSerialServiceV1) == 96u,
               "serial service ABI drift");
#endif

#endif /* RIN_DRIVER_SERIAL_SERVICE_V1_H */
