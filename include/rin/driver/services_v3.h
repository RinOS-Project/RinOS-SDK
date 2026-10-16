/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_SERVICES_V3_H
#define RIN_DRIVER_SERVICES_V3_H

#include <stddef.h>
#include <stdint.h>

#include <rin/driver/module_api_v3.h>

#define RIN_DRIVER_SERVICES_V3_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_IOPORT_READ UINT32_C(0x00000001)
#define RIN_DRIVER_V3_IOPORT_WRITE UINT32_C(0x00000002)
#define RIN_DRIVER_V3_IOPORT_FLAG_MASK \
    (RIN_DRIVER_V3_IOPORT_READ | RIN_DRIVER_V3_IOPORT_WRITE)
#define RIN_DRIVER_V3_FIRMWARE_AUTHENTICATED UINT32_C(0x00000001)
#define RIN_DRIVER_V3_FIRMWARE_EMBEDDED UINT32_C(0x00000002)
#define RIN_DRIVER_V3_FIRMWARE_FLAG_MASK \
    (RIN_DRIVER_V3_FIRMWARE_AUTHENTICATED | RIN_DRIVER_V3_FIRMWARE_EMBEDDED)
#define RIN_DRIVER_V3_CHILD_HOTPLUGGABLE UINT32_C(0x00000001)
#define RIN_DRIVER_V3_CHILD_REMOVABLE UINT32_C(0x00000002)
#define RIN_DRIVER_V3_CHILD_FLAG_MASK \
    (RIN_DRIVER_V3_CHILD_HOTPLUGGABLE | RIN_DRIVER_V3_CHILD_REMOVABLE)
#define RIN_DRIVER_V3_CHILD_HARDWARE_ID_MAX 64u
#define RIN_DRIVER_V3_CHILD_NAME_MAX 32u
#define RIN_DRIVER_V3_FIRMWARE_NAME_MAX 128u
#define RIN_DRIVER_V3_FIRMWARE_MAX_READ UINT32_C(65536)
#define RIN_DRIVER_V3_IRQ_EVENT_MAX_BATCH 64u

typedef struct RinDriverV3IoPortLease {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3Lease lease;
    uint64_t generation;
    uint16_t base;
    uint16_t reserved0;
    uint32_t length;
    uint32_t flags;
    uint32_t reserved1;
    uint64_t reserved[2];
} RinDriverV3IoPortLease;

typedef struct RinDriverV3IrqEvent {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3Lease lease;
    /* Device generation captured when the interrupt was observed. */
    uint64_t generation;
    uint64_t timestamp_ns;
    uint32_t vector;
    uint32_t occurrences;
    uint64_t reserved[2];
} RinDriverV3IrqEvent;

typedef struct RinDriverV3FirmwareInfo {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3Lease lease;
    uint64_t generation;
    uint64_t length;
    uint32_t flags;
    uint32_t reserved0;
    uint8_t sha256[32];
    uint64_t reserved[2];
} RinDriverV3FirmwareInfo;

typedef struct RinDriverV3ChildDescriptor {
    uint32_t struct_size;
    uint32_t version;
    uint16_t bus_type;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_device_id;
    uint16_t reserved0;
    uint32_t class_code;
    uint32_t revision;
    uint64_t parent_device;
    uint32_t flags;
    uint32_t hardware_id_length;
    uint32_t name_length;
    uint32_t reserved1;
    char hardware_id[RIN_DRIVER_V3_CHILD_HARDWARE_ID_MAX];
    char name[RIN_DRIVER_V3_CHILD_NAME_MAX];
    uint64_t reserved[2];
} RinDriverV3ChildDescriptor;

typedef struct RinDriverV3ChildDevice {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3Lease lease;
    uint64_t device;
    uint64_t parent_device;
    uint64_t generation;
    uint16_t bus_type;
    uint16_t reserved0;
    uint32_t flags;
    uint64_t reserved[2];
} RinDriverV3ChildDevice;

typedef enum RinDriverV3PropertyKey {
    RIN_DRIVER_V3_PROPERTY_BUS_TYPE = 1,
    RIN_DRIVER_V3_PROPERTY_VENDOR_ID = 2,
    RIN_DRIVER_V3_PROPERTY_DEVICE_ID = 3,
    RIN_DRIVER_V3_PROPERTY_SUBSYSTEM_VENDOR_ID = 4,
    RIN_DRIVER_V3_PROPERTY_SUBSYSTEM_DEVICE_ID = 5,
    RIN_DRIVER_V3_PROPERTY_CLASS_CODE = 6,
    RIN_DRIVER_V3_PROPERTY_REVISION = 7,
    RIN_DRIVER_V3_PROPERTY_AVAILABLE_RESOURCES = 8,
    RIN_DRIVER_V3_PROPERTY_GENERATION = 9,
    RIN_DRIVER_V3_PROPERTY_BAR_LENGTH = 10
} RinDriverV3PropertyKey;

typedef enum RinDriverV3PropertyType {
    RIN_DRIVER_V3_PROPERTY_U32 = 1,
    RIN_DRIVER_V3_PROPERTY_U64 = 2
} RinDriverV3PropertyType;

typedef struct RinDriverV3Property {
    uint32_t struct_size;
    uint32_t version;
    uint32_t key;
    uint32_t index;
    uint32_t type;
    uint32_t reserved0;
    uint64_t value;
    uint64_t generation;
    uint64_t reserved[2];
} RinDriverV3Property;

typedef RinDriverV3Result (*RinDriverV3IoPortAcquireFn)(
    uint64_t api_context, uint64_t session, uint16_t base, uint32_t length,
    uint32_t flags, RinDriverV3IoPortLease* output);
typedef RinDriverV3Result (*RinDriverV3IoPortReadFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease lease,
    uint32_t offset, uint8_t width, uint32_t* value);
typedef RinDriverV3Result (*RinDriverV3IoPortWriteFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease lease,
    uint32_t offset, uint8_t width, uint32_t value);
typedef RinDriverV3Result (*RinDriverV3IoPortReleaseFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease lease,
    uint64_t generation);
typedef RinDriverV3Result (*RinDriverV3IrqPollFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease irq_lease,
    RinDriverV3IrqEvent* events, uint32_t capacity, uint32_t* event_count);
typedef RinDriverV3Result (*RinDriverV3FirmwareOpenFn)(
    uint64_t api_context, uint64_t session, const char* name,
    uint32_t name_length, RinDriverV3FirmwareInfo* output);
typedef RinDriverV3Result (*RinDriverV3FirmwareReadFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease firmware,
    uint64_t offset, void* output, uint32_t capacity, uint32_t* bytes_read);
typedef RinDriverV3Result (*RinDriverV3FirmwareReleaseFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease firmware,
    uint64_t generation);
typedef RinDriverV3Result (*RinDriverV3ChildPublishFn)(
    uint64_t api_context, uint64_t session,
    const RinDriverV3ChildDescriptor* descriptor,
    RinDriverV3ChildDevice* output);
typedef RinDriverV3Result (*RinDriverV3ChildRemoveFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease child,
    uint64_t generation);
typedef RinDriverV3Result (*RinDriverV3PropertyGetFn)(
    uint64_t api_context, uint64_t session, uint32_t key, uint32_t index,
    RinDriverV3Property* output);

typedef struct RinDriverServicesV3 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t api_context;
    uint64_t capabilities;
    RinDriverV3IoPortAcquireFn ioport_acquire;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t ioport_acquire_padding;
#endif
    RinDriverV3IoPortReadFn ioport_read;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t ioport_read_padding;
#endif
    RinDriverV3IoPortWriteFn ioport_write;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t ioport_write_padding;
#endif
    RinDriverV3IoPortReleaseFn ioport_release;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t ioport_release_padding;
#endif
    RinDriverV3IrqPollFn irq_poll;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t irq_poll_padding;
#endif
    RinDriverV3FirmwareOpenFn firmware_open;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t firmware_open_padding;
#endif
    RinDriverV3FirmwareReadFn firmware_read;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t firmware_read_padding;
#endif
    RinDriverV3FirmwareReleaseFn firmware_release;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t firmware_release_padding;
#endif
    RinDriverV3ChildPublishFn child_publish;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t child_publish_padding;
#endif
    RinDriverV3ChildRemoveFn child_remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t child_remove_padding;
#endif
    RinDriverV3PropertyGetFn property_get;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t property_get_padding;
#endif
    uint64_t reserved[4];
} RinDriverServicesV3;

static inline const RinDriverServicesV3* rin_driver_services_v3_get(
    const RinDriverApiV3* api) {
    const RinDriverServicesV3* services;
    uint64_t known = RIN_DRIVER_V3_CAP_IOPORT |
                     RIN_DRIVER_V3_CAP_IRQ_EVENTS |
                     RIN_DRIVER_V3_CAP_FIRMWARE |
                     RIN_DRIVER_V3_CAP_CHILD_DEVICE |
                     RIN_DRIVER_V3_CAP_DEVICE_PROPERTIES;
    if (!api || api->version != RIN_DRIVER_V3_ABI_VERSION ||
        api->struct_size < sizeof(*api) ||
        api->services_api_address == 0u ||
        api->services_api_address > (uint64_t)UINTPTR_MAX ||
        (api->services_api_address & (sizeof(uint32_t) - 1u)) != 0u)
        return NULL;
    services = (const RinDriverServicesV3*)(uintptr_t)api->services_api_address;
    if (services->struct_size < sizeof(*services) ||
        services->version != RIN_DRIVER_SERVICES_V3_VERSION ||
        (services->capabilities != 0u && services->api_context == 0u) ||
        (services->capabilities & ~known) != 0u ||
        (services->capabilities & ~api->capabilities) != 0u ||
        services->reserved[0] != 0u || services->reserved[1] != 0u ||
        services->reserved[2] != 0u || services->reserved[3] != 0u ||
        (((services->capabilities & RIN_DRIVER_V3_CAP_IOPORT) != 0u) &&
         (!services->ioport_acquire || !services->ioport_read ||
          !services->ioport_write || !services->ioport_release)) ||
        (((services->capabilities & RIN_DRIVER_V3_CAP_IRQ_EVENTS) != 0u) &&
         !services->irq_poll) ||
        (((services->capabilities & RIN_DRIVER_V3_CAP_FIRMWARE) != 0u) &&
         (!services->firmware_open || !services->firmware_read ||
          !services->firmware_release)) ||
        (((services->capabilities & RIN_DRIVER_V3_CAP_CHILD_DEVICE) != 0u) &&
         (!services->child_publish || !services->child_remove)) ||
        (((services->capabilities &
           RIN_DRIVER_V3_CAP_DEVICE_PROPERTIES) != 0u) &&
         !services->property_get))
        return NULL;
#if UINTPTR_MAX == UINT32_MAX
    if (services->ioport_acquire_padding != 0u ||
        services->ioport_read_padding != 0u ||
        services->ioport_write_padding != 0u ||
        services->ioport_release_padding != 0u ||
        services->irq_poll_padding != 0u ||
        services->firmware_open_padding != 0u ||
        services->firmware_read_padding != 0u ||
        services->firmware_release_padding != 0u ||
        services->child_publish_padding != 0u ||
        services->child_remove_padding != 0u ||
        services->property_get_padding != 0u)
        return NULL;
#endif
    return services;
}

static inline int rin_driver_services_v3_has(
    const RinDriverApiV3* api, const RinDriverServicesV3* services,
    uint64_t capability) {
    return api && services && capability != 0u &&
           (api->capabilities & capability) == capability &&
           (services->capabilities & capability) == capability;
}

static inline int rin_driver_services_v3_bytes_zero(
    const void* memory, uint32_t size) {
    const uint8_t* bytes = (const uint8_t*)memory;
    for (uint32_t index = 0u; index < size; ++index) {
        if (bytes[index] != 0u) return 0;
    }
    return 1;
}

static inline void rin_driver_services_v3_zero(void* memory, uint32_t size) {
    uint8_t* bytes = (uint8_t*)memory;
    for (uint32_t index = 0u; index < size; ++index) bytes[index] = 0u;
}

static inline int rin_driver_services_v3_result_valid(
    RinDriverV3Result result) {
    return result <= RIN_DRIVER_V3_OK &&
           result >= RIN_DRIVER_V3_UNSUPPORTED;
}

#if defined(__cplusplus)
static_assert(sizeof(RinDriverServicesV3) == 144u,
              "NDRV v3 services table ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverServicesV3) == 144u,
               "NDRV v3 services table ABI drift");
#endif

static inline int rin_driver_v3_ioport_lease_valid(
    const RinDriverV3IoPortLease* port) {
    uint32_t end;
    if (!port || port->struct_size < sizeof(*port) ||
        port->version != RIN_DRIVER_SERVICES_V3_VERSION ||
        port->lease == 0u || port->generation == 0u || port->length == 0u ||
        (port->flags & ~RIN_DRIVER_V3_IOPORT_FLAG_MASK) != 0u ||
        port->flags == 0u || port->reserved0 != 0u || port->reserved1 != 0u ||
        !rin_driver_services_v3_bytes_zero(
            port->reserved, sizeof(port->reserved))) return 0;
    end = (uint32_t)port->base + port->length;
    return end <= 65536u;
}

static inline int rin_driver_v3_firmware_name_valid(
    const char* name, uint32_t length) {
    uint32_t component_start = 0u;
    if (!name || length == 0u ||
        length > RIN_DRIVER_V3_FIRMWARE_NAME_MAX || name[0] == '/' ||
        name[length - 1u] == '/') return 0;
    for (uint32_t index = 0u; index <= length; ++index) {
        uint8_t ch = index == length ? (uint8_t)'/' : (uint8_t)name[index];
        if (index < length &&
            !((ch >= (uint8_t)'a' && ch <= (uint8_t)'z') ||
              (ch >= (uint8_t)'A' && ch <= (uint8_t)'Z') ||
              (ch >= (uint8_t)'0' && ch <= (uint8_t)'9') || ch == '_' ||
              ch == '-' || ch == '.' || ch == '/')) return 0;
        if (ch == '/') {
            uint32_t component_length = index - component_start;
            if (component_length == 0u ||
                (component_length == 1u && name[component_start] == '.') ||
                (component_length == 2u && name[component_start] == '.' &&
                 name[component_start + 1u] == '.')) return 0;
            component_start = index + 1u;
        }
    }
    return 1;
}

static inline int rin_driver_v3_child_descriptor_valid(
    const RinDriverV3ChildDescriptor* descriptor) {
    if (!descriptor || descriptor->struct_size < sizeof(*descriptor) ||
        descriptor->version != RIN_DRIVER_SERVICES_V3_VERSION ||
        descriptor->bus_type == 0u || descriptor->parent_device == 0u ||
        descriptor->hardware_id_length == 0u ||
        descriptor->hardware_id_length >=
            RIN_DRIVER_V3_CHILD_HARDWARE_ID_MAX ||
        descriptor->name_length == 0u ||
        descriptor->name_length >= RIN_DRIVER_V3_CHILD_NAME_MAX ||
        (descriptor->flags & ~RIN_DRIVER_V3_CHILD_FLAG_MASK) != 0u ||
        descriptor->reserved0 != 0u || descriptor->reserved1 != 0u ||
        descriptor->hardware_id[descriptor->hardware_id_length] != '\0' ||
        descriptor->name[descriptor->name_length] != '\0' ||
        !rin_driver_services_v3_bytes_zero(
            descriptor->hardware_id + descriptor->hardware_id_length + 1u,
            RIN_DRIVER_V3_CHILD_HARDWARE_ID_MAX -
                descriptor->hardware_id_length - 1u) ||
        !rin_driver_services_v3_bytes_zero(
            descriptor->name + descriptor->name_length + 1u,
            RIN_DRIVER_V3_CHILD_NAME_MAX - descriptor->name_length - 1u) ||
        !rin_driver_services_v3_bytes_zero(
            descriptor->reserved, sizeof(descriptor->reserved)))
        return 0;
    for (uint32_t index = 0u; index < descriptor->hardware_id_length;
         ++index) {
        uint8_t ch = (uint8_t)descriptor->hardware_id[index];
        if (ch < 0x21u || ch > 0x7eu || ch == '/' || ch == '\\') return 0;
    }
    for (uint32_t index = 0u; index < descriptor->name_length; ++index) {
        uint8_t ch = (uint8_t)descriptor->name[index];
        if (ch < 0x20u || ch > 0x7eu) return 0;
    }
    return 1;
}

static inline RinDriverV3Result rin_driver_v3_ioport_acquire(
    const RinDriverApiV3* api, uint64_t session, uint16_t base,
    uint32_t length, uint32_t flags, RinDriverV3IoPortLease* output) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    uint32_t end = (uint32_t)base + length;
    RinDriverV3Result result;
    if (output) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
    }
    if (!services || !output || session == 0u || length == 0u || end > 65536u ||
        flags == 0u || (flags & ~RIN_DRIVER_V3_IOPORT_FLAG_MASK) != 0u ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_IOPORT) ||
        !services->ioport_acquire || !services->ioport_read ||
        !services->ioport_write || !services->ioport_release)
        return RIN_DRIVER_V3_DENIED;
    result = services->ioport_acquire(services->api_context, session, base,
                                       length, flags, output);
    if (!rin_driver_services_v3_result_valid(result))
        result = RIN_DRIVER_V3_BACKEND_FAILED;
    if (result != RIN_DRIVER_V3_OK) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
        return result;
    }
    if (!rin_driver_v3_ioport_lease_valid(output) ||
        output->base != base || output->length != length ||
        output->flags != flags) {
        if (output->lease != 0u && output->generation != 0u)
            (void)services->ioport_release(
                services->api_context, session, output->lease,
                output->generation);
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    return result;
}

static inline RinDriverV3Result rin_driver_v3_ioport_read(
    const RinDriverApiV3* api, uint64_t session,
    const RinDriverV3IoPortLease* port, uint32_t offset, uint8_t width,
    uint32_t* value) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    uint32_t result_value = 0u;
    RinDriverV3Result result;
    if (!services || !rin_driver_v3_ioport_lease_valid(port) || !value ||
        session == 0u ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_IOPORT) ||
        (port->flags & RIN_DRIVER_V3_IOPORT_READ) == 0u ||
        (width != 1u && width != 2u && width != 4u) ||
        offset > port->length || width > port->length - offset ||
        !services->ioport_read) {
        if (value) *value = 0u;
        return RIN_DRIVER_V3_INVALID;
    }
    *value = 0u;
    result = services->ioport_read(services->api_context, session, port->lease,
                                    offset, width, &result_value);
    if (!rin_driver_services_v3_result_valid(result))
        return RIN_DRIVER_V3_BACKEND_FAILED;
    if (result != RIN_DRIVER_V3_OK) return result;
    if ((width == 1u && result_value > UINT8_MAX) ||
        (width == 2u && result_value > UINT16_MAX))
        return RIN_DRIVER_V3_BACKEND_FAILED;
    *value = result_value;
    return RIN_DRIVER_V3_OK;
}

static inline RinDriverV3Result rin_driver_v3_ioport_write(
    const RinDriverApiV3* api, uint64_t session,
    const RinDriverV3IoPortLease* port, uint32_t offset, uint8_t width,
    uint32_t value) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    if (!services || !rin_driver_v3_ioport_lease_valid(port) ||
        session == 0u ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_IOPORT) ||
        port->generation == 0u ||
        (port->flags & RIN_DRIVER_V3_IOPORT_WRITE) == 0u ||
        (width != 1u && width != 2u && width != 4u) ||
        (width == 1u && value > UINT8_MAX) ||
        (width == 2u && value > UINT16_MAX) ||
        offset > port->length || width > port->length - offset ||
        !services->ioport_write)
        return RIN_DRIVER_V3_INVALID;
    {
        RinDriverV3Result result = services->ioport_write(
            services->api_context, session, port->lease, offset, width,
            value);
        return rin_driver_services_v3_result_valid(result)
            ? result : RIN_DRIVER_V3_BACKEND_FAILED;
    }
}

static inline RinDriverV3Result rin_driver_v3_ioport_release(
    const RinDriverApiV3* api, uint64_t session,
    const RinDriverV3IoPortLease* port) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    if (!services || !rin_driver_v3_ioport_lease_valid(port) ||
        session == 0u ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_IOPORT) ||
        !services->ioport_release)
        return RIN_DRIVER_V3_INVALID;
    {
        RinDriverV3Result result = services->ioport_release(
            services->api_context, session, port->lease, port->generation);
        return rin_driver_services_v3_result_valid(result)
            ? result : RIN_DRIVER_V3_BACKEND_FAILED;
    }
}

static inline RinDriverV3Result rin_driver_v3_irq_poll(
    const RinDriverApiV3* api, uint64_t session, RinDriverV3Lease irq_lease,
    RinDriverV3IrqEvent* events, uint32_t capacity, uint32_t* event_count) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    if (event_count) *event_count = 0u;
    if (events && capacity <= RIN_DRIVER_V3_IRQ_EVENT_MAX_BATCH)
        rin_driver_services_v3_zero(events,
                                    capacity * sizeof(*events));
    if (!services || !events || !event_count || capacity == 0u ||
        capacity > RIN_DRIVER_V3_IRQ_EVENT_MAX_BATCH ||
        session == 0u || irq_lease == 0u ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_IRQ_EVENTS) ||
        !services->irq_poll)
        return RIN_DRIVER_V3_DENIED;
    for (uint32_t index = 0u; index < capacity; ++index) {
        events[index].struct_size = sizeof(events[index]);
        events[index].version = RIN_DRIVER_SERVICES_V3_VERSION;
    }
    {
        RinDriverV3Result result = services->irq_poll(
            services->api_context, session, irq_lease, events, capacity,
            event_count);
        if (!rin_driver_services_v3_result_valid(result))
            result = RIN_DRIVER_V3_BACKEND_FAILED;
        if (result != RIN_DRIVER_V3_OK) {
            rin_driver_services_v3_zero(events,
                                        capacity * sizeof(*events));
            *event_count = 0u;
            return result;
        }
    }
    if (*event_count > capacity) {
        rin_driver_services_v3_zero(events, capacity * sizeof(*events));
        *event_count = 0u;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    for (uint32_t index = 0u; index < *event_count; ++index) {
        if (events[index].struct_size < sizeof(events[index]) ||
            events[index].version != RIN_DRIVER_SERVICES_V3_VERSION ||
            events[index].lease != irq_lease ||
            events[index].generation == 0u ||
            events[index].timestamp_ns == 0u ||
            events[index].occurrences == 0u ||
            events[index].vector < 32u || events[index].vector > 255u ||
            !rin_driver_services_v3_bytes_zero(
                events[index].reserved, sizeof(events[index].reserved))) {
            rin_driver_services_v3_zero(events,
                                        capacity * sizeof(*events));
            *event_count = 0u;
            return RIN_DRIVER_V3_BACKEND_FAILED;
        }
    }
    return RIN_DRIVER_V3_OK;
}

static inline int rin_driver_v3_firmware_info_valid(
    const RinDriverV3FirmwareInfo* firmware) {
    uint8_t digest_nonzero = 0u;
    if (!firmware || firmware->struct_size < sizeof(*firmware) ||
        firmware->version != RIN_DRIVER_SERVICES_V3_VERSION ||
        firmware->lease == 0u || firmware->generation == 0u ||
        firmware->length == 0u ||
        (firmware->flags & ~RIN_DRIVER_V3_FIRMWARE_FLAG_MASK) != 0u ||
        (firmware->flags & RIN_DRIVER_V3_FIRMWARE_AUTHENTICATED) == 0u ||
        firmware->reserved0 != 0u ||
        !rin_driver_services_v3_bytes_zero(
            firmware->reserved, sizeof(firmware->reserved)))
        return 0;
    for (uint32_t index = 0u; index < sizeof(firmware->sha256); ++index)
        digest_nonzero |= firmware->sha256[index];
    return digest_nonzero != 0u;
}

static inline RinDriverV3Result rin_driver_v3_firmware_open(
    const RinDriverApiV3* api, uint64_t session, const char* name,
    uint32_t name_length, RinDriverV3FirmwareInfo* output) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    RinDriverV3Result result;
    if (output) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
    }
    if (!services || !name || !output || session == 0u ||
        !rin_driver_v3_firmware_name_valid(name, name_length) ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_FIRMWARE) ||
        !services->firmware_open || !services->firmware_read ||
        !services->firmware_release)
        return RIN_DRIVER_V3_DENIED;
    result = services->firmware_open(services->api_context, session, name,
                                      name_length, output);
    if (!rin_driver_services_v3_result_valid(result))
        result = RIN_DRIVER_V3_BACKEND_FAILED;
    if (result != RIN_DRIVER_V3_OK) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
        return result;
    }
    if (!rin_driver_v3_firmware_info_valid(output)) {
        if (output->lease != 0u && output->generation != 0u)
            (void)services->firmware_release(
                services->api_context, session, output->lease,
                output->generation);
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    return result;
}

static inline RinDriverV3Result rin_driver_v3_firmware_read(
    const RinDriverApiV3* api, uint64_t session,
    const RinDriverV3FirmwareInfo* firmware, uint64_t offset, void* output,
    uint32_t capacity, uint32_t* bytes_read) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    if (bytes_read) *bytes_read = 0u;
    if (output && capacity <= RIN_DRIVER_V3_FIRMWARE_MAX_READ)
        rin_driver_services_v3_zero(output, capacity);
    if (!services || !rin_driver_v3_firmware_info_valid(firmware) ||
        !output || !bytes_read || capacity == 0u || session == 0u ||
        capacity > RIN_DRIVER_V3_FIRMWARE_MAX_READ ||
        offset > firmware->length ||
        (offset < firmware->length &&
         capacity > firmware->length - offset) ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_FIRMWARE) ||
        !services->firmware_read)
        return RIN_DRIVER_V3_RANGE;
    {
        RinDriverV3Result result = services->firmware_read(
            services->api_context, session, firmware->lease, offset, output,
            capacity, bytes_read);
        if (!rin_driver_services_v3_result_valid(result))
            result = RIN_DRIVER_V3_BACKEND_FAILED;
        if (result != RIN_DRIVER_V3_OK) {
            *bytes_read = 0u;
            for (uint32_t index = 0u; index < capacity; ++index)
                ((uint8_t*)output)[index] = 0u;
            return result;
        }
    }
    if (*bytes_read > capacity ||
        (offset < firmware->length && *bytes_read == 0u)) {
        rin_driver_services_v3_zero(output, capacity);
        *bytes_read = 0u;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    return RIN_DRIVER_V3_OK;
}

static inline RinDriverV3Result rin_driver_v3_firmware_release(
    const RinDriverApiV3* api, uint64_t session,
    const RinDriverV3FirmwareInfo* firmware) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    if (!services || !rin_driver_v3_firmware_info_valid(firmware) ||
        session == 0u ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_FIRMWARE) ||
        !services->firmware_release)
        return RIN_DRIVER_V3_INVALID;
    {
        RinDriverV3Result result = services->firmware_release(
            services->api_context, session, firmware->lease,
            firmware->generation);
        return rin_driver_services_v3_result_valid(result)
            ? result : RIN_DRIVER_V3_BACKEND_FAILED;
    }
}

static inline RinDriverV3Result rin_driver_v3_child_publish(
    const RinDriverApiV3* api, uint64_t session,
    const RinDriverV3ChildDescriptor* descriptor,
    RinDriverV3ChildDevice* output) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    if (output) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
    }
    if (!services || !descriptor || !output || session == 0u ||
        !rin_driver_v3_child_descriptor_valid(descriptor) ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_CHILD_DEVICE) ||
        !services->child_publish)
        return RIN_DRIVER_V3_INVALID;
    {
        RinDriverV3Result result = services->child_publish(
            services->api_context, session, descriptor, output);
        if (!rin_driver_services_v3_result_valid(result))
            result = RIN_DRIVER_V3_BACKEND_FAILED;
        if (result != RIN_DRIVER_V3_OK) {
            rin_driver_services_v3_zero(output, sizeof(*output));
            output->struct_size = sizeof(*output);
            output->version = RIN_DRIVER_SERVICES_V3_VERSION;
            return result;
        }
    }
    if (output->struct_size < sizeof(*output) ||
        output->version != RIN_DRIVER_SERVICES_V3_VERSION ||
        output->lease == 0u || output->device == 0u ||
        output->parent_device != descriptor->parent_device ||
        output->generation == 0u || output->bus_type != descriptor->bus_type ||
        output->flags != descriptor->flags || output->reserved0 != 0u ||
        !rin_driver_services_v3_bytes_zero(
            output->reserved, sizeof(output->reserved))) {
        if (output->lease != 0u && output->generation != 0u)
            (void)services->child_remove(
                services->api_context, session, output->lease,
                output->generation);
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    return RIN_DRIVER_V3_OK;
}

static inline RinDriverV3Result rin_driver_v3_child_remove(
    const RinDriverApiV3* api, uint64_t session,
    const RinDriverV3ChildDevice* child) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    if (!services || !child || session == 0u || child->lease == 0u ||
        child->generation == 0u ||
        child->struct_size < sizeof(*child) ||
        child->version != RIN_DRIVER_SERVICES_V3_VERSION ||
        child->device == 0u || child->parent_device == 0u ||
        child->bus_type == 0u ||
        (child->flags & ~RIN_DRIVER_V3_CHILD_FLAG_MASK) != 0u ||
        child->reserved0 != 0u ||
        !rin_driver_services_v3_bytes_zero(
            child->reserved, sizeof(child->reserved)) ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_CHILD_DEVICE) ||
        !services->child_remove)
        return RIN_DRIVER_V3_INVALID;
    {
        RinDriverV3Result result = services->child_remove(
            services->api_context, session, child->lease, child->generation);
        return rin_driver_services_v3_result_valid(result)
            ? result : RIN_DRIVER_V3_BACKEND_FAILED;
    }
}

static inline RinDriverV3Result rin_driver_v3_property_get(
    const RinDriverApiV3* api, uint64_t session, uint32_t key, uint32_t index,
    RinDriverV3Property* output) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    if (output) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
    }
    if (!services || !output || session == 0u ||
        (key < RIN_DRIVER_V3_PROPERTY_BUS_TYPE ||
         key > RIN_DRIVER_V3_PROPERTY_BAR_LENGTH) ||
        (key == RIN_DRIVER_V3_PROPERTY_BAR_LENGTH ? index >= 6u : index != 0u) ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_DEVICE_PROPERTIES) ||
        !services->property_get)
        return RIN_DRIVER_V3_DENIED;
    {
        RinDriverV3Result result = services->property_get(
            services->api_context, session, key, index, output);
        if (!rin_driver_services_v3_result_valid(result))
            result = RIN_DRIVER_V3_BACKEND_FAILED;
        if (result != RIN_DRIVER_V3_OK) {
            rin_driver_services_v3_zero(output, sizeof(*output));
            output->struct_size = sizeof(*output);
            output->version = RIN_DRIVER_SERVICES_V3_VERSION;
            return result;
        }
    }
    if (output->struct_size < sizeof(*output) ||
        output->version != RIN_DRIVER_SERVICES_V3_VERSION ||
        output->key != key || output->index != index ||
        output->generation == 0u || output->reserved0 != 0u ||
        (output->type != RIN_DRIVER_V3_PROPERTY_U32 &&
         output->type != RIN_DRIVER_V3_PROPERTY_U64) ||
        (output->type == RIN_DRIVER_V3_PROPERTY_U32 &&
         output->value > UINT32_MAX) ||
        !rin_driver_services_v3_bytes_zero(
            output->reserved, sizeof(output->reserved)))
    {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    if ((key == RIN_DRIVER_V3_PROPERTY_BUS_TYPE ||
         key == RIN_DRIVER_V3_PROPERTY_VENDOR_ID ||
         key == RIN_DRIVER_V3_PROPERTY_DEVICE_ID ||
         key == RIN_DRIVER_V3_PROPERTY_SUBSYSTEM_VENDOR_ID ||
         key == RIN_DRIVER_V3_PROPERTY_SUBSYSTEM_DEVICE_ID ||
         key == RIN_DRIVER_V3_PROPERTY_CLASS_CODE ||
         key == RIN_DRIVER_V3_PROPERTY_REVISION) &&
        output->type != RIN_DRIVER_V3_PROPERTY_U32) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    if ((key == RIN_DRIVER_V3_PROPERTY_AVAILABLE_RESOURCES ||
         key == RIN_DRIVER_V3_PROPERTY_GENERATION ||
         key == RIN_DRIVER_V3_PROPERTY_BAR_LENGTH) &&
        output->type != RIN_DRIVER_V3_PROPERTY_U64) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    return RIN_DRIVER_V3_OK;
}

#endif /* RIN_DRIVER_SERVICES_V3_H */
