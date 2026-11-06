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
#define RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_V1 \
    RIN_DRIVER_SERVICES_V3_VERSION
/* V1.1 is the same fixed-size descriptor with an OS-resolved PCI location in
 * reserved[0]. The driver supplies a location hint only; the kernel resolves
 * it against its live PCI catalog before granting any device resources. */
#define RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_PCI_V1 UINT32_C(0x00010001)
/* V1.2 logical children may delegate resources to a signed, parent-owned
 * transport provider. reserved[0] carries a pointer to a static provider
 * table in the signed parent image during publish; the kernel snapshots the
 * table and clears the pointer before publishing the child descriptor. Keep
 * its callback and context alive until the child is removed. The published
 * descriptor stores the provider's capability grant in reserved[1]. */
#define RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_RESOURCE_V1 UINT32_C(0x00010002)
#define RIN_DRIVER_V3_CHILD_RESOURCE_BROKER_V1_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_CHILD_RESOURCE_BROKER_V1_FLAG_POWER_MANAGEMENT \
    UINT64_C(0x0000000000000001)
#define RIN_DRIVER_V3_CHILD_RESOURCE_BROKER_V1_FLAG_MASK \
    RIN_DRIVER_V3_CHILD_RESOURCE_BROKER_V1_FLAG_POWER_MANAGEMENT
#define RIN_DRIVER_V3_CHILD_RESOURCE_BROKER_V1_CAPABILITIES \
    (RIN_DRIVER_V3_CAP_DMA_ISOLATED | RIN_DRIVER_V3_CAP_BAR | \
     RIN_DRIVER_V3_CAP_MSI | RIN_DRIVER_V3_CAP_MSIX | \
     RIN_DRIVER_V3_CAP_TIMER | RIN_DRIVER_V3_CAP_REGISTER | \
     RIN_DRIVER_V3_CAP_DMA_START | RIN_DRIVER_V3_CAP_DMA_CONTIGUOUS | \
     RIN_DRIVER_V3_CAP_DMA_STOP)
#define RIN_DRIVER_V3_CHILD_RESOURCE_REQUEST_V1_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_CHILD_PCI_LOCATION(segment, bus, device, function) \
    ((uint64_t)(segment) | ((uint64_t)(bus) << 16u) | \
     ((uint64_t)(device) << 24u) | ((uint64_t)(function) << 29u))
#define RIN_DRIVER_V3_FIRMWARE_NAME_MAX 128u
#define RIN_DRIVER_V3_FIRMWARE_MAX_READ UINT32_C(65536)
#define RIN_DRIVER_V3_IRQ_EVENT_MAX_BATCH 64u
#define RIN_DRIVER_V3_IRQ_ROUTE_V1_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_FIRMWARE_CATALOG_MAGIC UINT32_C(0x57464452) /* RDFW */
#define RIN_DRIVER_V3_FIRMWARE_CATALOG_VERSION UINT16_C(1)
#define RIN_DRIVER_V3_FIRMWARE_CATALOG_HEADER_SIZE 32u
#define RIN_DRIVER_V3_FIRMWARE_CATALOG_ENTRY_SIZE 184u
#define RIN_DRIVER_V3_FIRMWARE_CATALOG_MAX_ENTRIES 128u
#define RIN_DRIVER_V3_FIRMWARE_CATALOG_MAX_SIZE UINT32_C(0x01000000)
#define RIN_DRIVER_V3_FIRMWARE_ENTRY_AUTHENTICATED UINT32_C(0x00000001)

/* Optional bytes in an NDRV v3 RESOURCES section. The NDRV signature covers
 * this catalog and all payload bytes, so entries can be served directly from
 * the read-only loaded image after the kernel checks their digest. */
typedef struct RinDriverFirmwareCatalogV1 {
    uint32_t magic;
    uint16_t version;
    uint16_t header_size;
    uint32_t entry_count;
    uint32_t entry_size;
    uint32_t table_offset;
    uint32_t data_offset;
    uint32_t section_size;
    uint32_t reserved;
} RinDriverFirmwareCatalogV1;

typedef struct RinDriverFirmwareEntryV1 {
    uint32_t flags;
    uint32_t name_length;
    uint64_t data_offset;
    uint64_t data_size;
    uint8_t sha256[32];
    char name[RIN_DRIVER_V3_FIRMWARE_NAME_MAX];
} RinDriverFirmwareEntryV1;

#if defined(__cplusplus)
static_assert(sizeof(RinDriverFirmwareCatalogV1) == 32u,
              "NDRV firmware catalog ABI drift");
static_assert(sizeof(RinDriverFirmwareEntryV1) == 184u,
              "NDRV firmware entry ABI drift");
static_assert(offsetof(RinDriverFirmwareEntryV1, name) == 56u,
              "NDRV firmware name offset drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverFirmwareCatalogV1) == 32u,
               "NDRV firmware catalog ABI drift");
_Static_assert(sizeof(RinDriverFirmwareEntryV1) == 184u,
               "NDRV firmware entry ABI drift");
_Static_assert(offsetof(RinDriverFirmwareEntryV1, name) == 56u,
               "NDRV firmware name offset drift");
#endif

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
    /* Ready monotonic sample captured with the interrupt; epoch zero is
     * valid. */
    uint64_t timestamp_ns;
    uint32_t vector;
    uint32_t occurrences;
    uint64_t reserved[2];
} RinDriverV3IrqEvent;

/* A live public IRQ lease has two distinct vector identities on MSI-X:
 * cpu_vector is owned by the OS interrupt controller, while
 * msix_table_index selects the device's MSI-X table entry. */
typedef struct RinDriverV3IrqRouteV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_cookie;
    uint64_t device_generation;
    RinDriverV3Lease lease;
    uint32_t cpu_vector;
    uint32_t msix_table_index;
    uint32_t mode;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverV3IrqRouteV1;

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

/* A pinned entry from the signed firmware catalog embedded in this NDRV
 * image. The NDRV signature authenticates the entry digest and signer. */
typedef struct RinDriverV3FirmwareCatalogLeaseV1 {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3Lease lease;
    uint64_t length;
    uint32_t flags;
    uint32_t reserved0;
    uint8_t sha256[32];
    uint8_t trusted_key_id[32];
    uint64_t reserved[2];
} RinDriverV3FirmwareCatalogLeaseV1;

#define RIN_DRIVER_V3_FIRMWARE_CATALOG_LEASE_V1_VERSION \
    UINT32_C(0x00010000)

#if defined(__cplusplus)
static_assert(sizeof(RinDriverV3FirmwareCatalogLeaseV1) == 112u,
              "NDRV signed firmware catalog lease ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverV3FirmwareCatalogLeaseV1) == 112u,
               "NDRV signed firmware catalog lease ABI drift");
#endif

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

typedef enum RinDriverV3ChildResourceOperationV1 {
    RIN_DRIVER_V3_CHILD_RESOURCE_DOMAIN_CREATE = 1,
    RIN_DRIVER_V3_CHILD_RESOURCE_DOMAIN_DESTROY = 2,
    RIN_DRIVER_V3_CHILD_RESOURCE_DMA_ALLOCATE = 3,
    RIN_DRIVER_V3_CHILD_RESOURCE_DMA_RELEASE = 4,
    RIN_DRIVER_V3_CHILD_RESOURCE_DMA_MAP_SG = 5,
    RIN_DRIVER_V3_CHILD_RESOURCE_DMA_SYNC = 6,
    RIN_DRIVER_V3_CHILD_RESOURCE_BAR_MAP = 7,
    RIN_DRIVER_V3_CHILD_RESOURCE_BAR_UNMAP = 8,
    RIN_DRIVER_V3_CHILD_RESOURCE_IRQ_ACQUIRE = 9,
    RIN_DRIVER_V3_CHILD_RESOURCE_IRQ_RELEASE = 10,
    RIN_DRIVER_V3_CHILD_RESOURCE_TIMER_ARM = 11,
    RIN_DRIVER_V3_CHILD_RESOURCE_TIMER_CANCEL = 12,
    RIN_DRIVER_V3_CHILD_RESOURCE_DEVICE_REGISTER = 13,
    RIN_DRIVER_V3_CHILD_RESOURCE_DEVICE_UNREGISTER = 14,
    RIN_DRIVER_V3_CHILD_RESOURCE_DEVICE_RESET = 15,
    RIN_DRIVER_V3_CHILD_RESOURCE_DMA_START = 16,
    RIN_DRIVER_V3_CHILD_RESOURCE_DMA_STOP = 17,
    RIN_DRIVER_V3_CHILD_RESOURCE_IRQ_ACQUIRE_INDEX = 18,
    RIN_DRIVER_V3_CHILD_RESOURCE_DEVICE_SUSPEND = 19,
    RIN_DRIVER_V3_CHILD_RESOURCE_DEVICE_RESUME = 20,
    RIN_DRIVER_V3_CHILD_RESOURCE_DMA_PHYSICAL_ADDRESS = 21
} RinDriverV3ChildResourceOperationV1;

/* The dispatch request is synchronous. Pointer-valued input/output fields
 * name kernel mappings supplied by the child runtime and are valid only for
 * the duration of the call. A provider must bind every lease to both child
 * identity fields and must finish or roll back before returning.
 * If dispatch fails after returning a nonzero domain/backend cookie, the
 * runtime invokes the corresponding destroy/release operation. Failed
 * operations without a cookie must leave no resource behind.
 *
 * Operation payloads use values[] as follows:
 * DOMAIN_CREATE returns domain_cookie; DOMAIN_DESTROY consumes it.
 * DMA_ALLOCATE: length, alignment in values[0..1]; returns driver address and
 * IOVA in values[2..3] plus backend_cookie. DMA_MAP_SG uses input/output for
 * slice/segment arrays, count/capacity, and returns count plus backend_cookie.
 * DMA_SYNC uses flags for action and values[0..1] for offset/length.
 * BAR_MAP uses values[0..2] for index/offset/length and returns mapped address
 * in values[3] plus backend_cookie. IRQ_ACQUIRE returns vector in values[0];
 * IRQ_ACQUIRE_INDEX consumes values[0] as table index and returns vector in
 * values[1]; both return backend_cookie.
 * TIMER_ARM uses values[0..1] for deadline/period and returns backend_cookie.
 * DEVICE_REGISTER uses input for RinDriverV3DeviceRegistration and returns
 * backend_cookie. DEVICE_RESET uses values[0] for timeout milliseconds.
 * DMA_PHYSICAL_ADDRESS consumes backend_cookie and returns values[0]. The
 * remaining release, DMA_START/STOP, and power operations use their named
 * cookie, child identity, and flags fields. */
typedef struct RinDriverV3ChildResourceRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t flags;
    uint64_t child_device;
    uint64_t child_generation;
    uint64_t domain_cookie;
    uint64_t backend_cookie;
    uint64_t values[4];
    uint64_t input;
    uint64_t output;
    uint32_t count;
    uint32_t capacity;
    uint32_t width;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverV3ChildResourceRequestV1;

typedef int (*RinDriverV3ChildResourceDispatchV1)(
    uint64_t context, RinDriverV3ChildResourceRequestV1* request);

typedef struct RinDriverV3ChildResourceBrokerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t flags;
    uint64_t capabilities;
    uint64_t context;
    RinDriverV3ChildResourceDispatchV1 dispatch;
    uint64_t reserved[2];
} RinDriverV3ChildResourceBrokerV1;

/* V1.2 kernel validation requires dispatch and any nonzero context to point
 * into the signed parent image graph. Static module state is the supported
 * context storage for this broker. */

#if defined(__cplusplus)
static_assert(sizeof(RinDriverV3ChildResourceRequestV1) == 128u,
              "NDRV child-resource request ABI drift");
static_assert(offsetof(RinDriverV3ChildResourceRequestV1, reserved) == 112u,
              "NDRV child-resource request offset drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverV3ChildResourceRequestV1) == 128u,
               "NDRV child-resource request ABI drift");
_Static_assert(offsetof(RinDriverV3ChildResourceRequestV1, reserved) == 112u,
               "NDRV child-resource request offset drift");
#endif

static inline uint64_t rin_driver_v3_child_resources_from_capabilities(
    uint64_t capabilities)
{
    uint64_t resources = 0u;
    if ((capabilities & RIN_DRIVER_V3_CAP_BAR) != 0u)
        resources |= RIN_DRIVER_RESOURCE_MMIO;
    if ((capabilities & RIN_DRIVER_V3_CAP_DMA_ISOLATED) != 0u)
        resources |= RIN_DRIVER_RESOURCE_DMA;
    if ((capabilities &
         (RIN_DRIVER_V3_CAP_MSI | RIN_DRIVER_V3_CAP_MSIX)) != 0u)
        resources |= RIN_DRIVER_RESOURCE_IRQ;
    return resources;
}

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
    RIN_DRIVER_V3_PROPERTY_BAR_LENGTH = 10,
    /* ACPI _WOV result for the device's exact PCI companion. */
    RIN_DRIVER_V3_PROPERTY_ACPI_WOV = 11
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
typedef RinDriverV3Result (*RinDriverV3IrqRouteFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease irq_lease,
    RinDriverV3IrqRouteV1* route_out);
/* Wait until an absolute monotonic deadline or an implementation-defined
 * event. Callers must recheck the device condition after every wake. */
typedef RinDriverV3Result (*RinDriverV3WaitUntilFn)(
    uint64_t api_context, uint64_t session, uint64_t deadline_ns);
typedef RinDriverV3Result (*RinDriverV3FirmwareOpenFn)(
    uint64_t api_context, uint64_t session, const char* name,
    uint32_t name_length, RinDriverV3FirmwareInfo* output);
typedef RinDriverV3Result (*RinDriverV3FirmwareReadFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease firmware,
    uint64_t offset, void* output, uint32_t capacity, uint32_t* bytes_read);
typedef RinDriverV3Result (*RinDriverV3FirmwareReleaseFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease firmware,
    uint64_t generation);
typedef RinDriverV3Result (*RinDriverV3FirmwareCatalogOpenFn)(
    uint64_t api_context, uint64_t session, const char* name,
    uint32_t name_length, RinDriverV3FirmwareCatalogLeaseV1* output);
typedef RinDriverV3Result (*RinDriverV3FirmwareCatalogReadFn)(
    uint64_t api_context, uint64_t session,
    const RinDriverV3FirmwareCatalogLeaseV1* lease, uint64_t offset,
    void* output, uint32_t capacity, uint32_t* bytes_read);
typedef RinDriverV3Result (*RinDriverV3FirmwareCatalogReleaseFn)(
    uint64_t api_context, uint64_t session,
    RinDriverV3FirmwareCatalogLeaseV1* lease);
typedef RinDriverV3Result (*RinDriverV3ChildPublishFn)(
    uint64_t api_context, uint64_t session,
    const RinDriverV3ChildDescriptor* descriptor,
    RinDriverV3ChildDevice* output);
typedef RinDriverV3Result (*RinDriverV3ChildRemoveFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease child,
    uint64_t generation);
typedef RinDriverV3Result (*RinDriverV3ChildRemoveExFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease child,
    uint64_t generation, RinDriverKernelModuleStopReasonV1 reason);
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
    /* Optional V1 extension. This occupies the first former reserved slot,
     * keeping every pre-existing callback offset unchanged. */
    RinDriverV3IrqRouteFn irq_route;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t irq_route_padding;
#endif
    /* Optional V1 extension occupying a former reserved pointer slot. */
    RinDriverV3WaitUntilFn wait_until;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t wait_until_padding;
#endif
    uint64_t reserved[2];
    /* Optional, size-gated tail extension. */
    RinDriverV3ChildRemoveExFn child_remove_ex;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t child_remove_ex_padding;
#endif
    /* Optional V1 extension, guarded by struct_size for older modules. */
    RinDriverV3FirmwareCatalogOpenFn firmware_catalog_open;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t firmware_catalog_open_padding;
#endif
    RinDriverV3FirmwareCatalogReadFn firmware_catalog_read;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t firmware_catalog_read_padding;
#endif
    RinDriverV3FirmwareCatalogReleaseFn firmware_catalog_release;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t firmware_catalog_release_padding;
#endif
} RinDriverServicesV3;

static inline const RinDriverServicesV3* rin_driver_services_v3_get(
    const RinDriverApiV3* api) {
    const RinDriverServicesV3* services;
    uint64_t known = RIN_DRIVER_V3_CAP_IOPORT |
                     RIN_DRIVER_V3_CAP_IRQ_EVENTS |
                     RIN_DRIVER_V3_CAP_IRQ_ROUTE |
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
    if (services->struct_size <
            offsetof(RinDriverServicesV3, child_remove_ex) ||
        (services->struct_size >
             offsetof(RinDriverServicesV3, firmware_catalog_open) &&
         services->struct_size <
             offsetof(RinDriverServicesV3, firmware_catalog_release) +
                 sizeof(services->firmware_catalog_release)) ||
        services->version != RIN_DRIVER_SERVICES_V3_VERSION ||
        (services->capabilities != 0u && services->api_context == 0u) ||
        (services->capabilities & ~known) != 0u ||
        (services->capabilities & ~api->capabilities) != 0u ||
        (((services->capabilities & RIN_DRIVER_V3_CAP_IRQ_ROUTE) != 0u) &&
         ((services->capabilities & RIN_DRIVER_V3_CAP_IRQ_EVENTS) == 0u ||
          (api->capabilities & RIN_DRIVER_V3_CAP_MSIX) == 0u ||
          !services->irq_route)) ||
        services->reserved[0] != 0u || services->reserved[1] != 0u ||
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
         !services->property_get) ||
        (services->struct_size >=
             offsetof(RinDriverServicesV3, firmware_catalog_release) +
                 sizeof(services->firmware_catalog_release) &&
         (services->firmware_catalog_open ||
          services->firmware_catalog_read ||
          services->firmware_catalog_release) &&
         ((services->capabilities & RIN_DRIVER_V3_CAP_FIRMWARE) == 0u ||
          !services->firmware_catalog_open ||
          !services->firmware_catalog_read ||
          !services->firmware_catalog_release)))
        return NULL;
#if UINTPTR_MAX == UINT32_MAX
    if (services->ioport_acquire_padding != 0u ||
        services->ioport_read_padding != 0u ||
        services->ioport_write_padding != 0u ||
        services->ioport_release_padding != 0u ||
        services->irq_poll_padding != 0u ||
        services->irq_route_padding != 0u ||
        services->firmware_open_padding != 0u ||
        services->firmware_read_padding != 0u ||
        services->firmware_release_padding != 0u ||
        services->child_publish_padding != 0u ||
        services->child_remove_padding != 0u ||
        services->property_get_padding != 0u ||
        services->wait_until_padding != 0u)
        return NULL;
    if (services->struct_size >=
            offsetof(RinDriverServicesV3, child_remove_ex_padding) +
                sizeof(services->child_remove_ex_padding) &&
        services->child_remove_ex_padding != 0u)
        return NULL;
    if (services->struct_size >=
            offsetof(RinDriverServicesV3, firmware_catalog_open_padding) +
                sizeof(services->firmware_catalog_open_padding) &&
        services->firmware_catalog_open_padding != 0u)
        return NULL;
    if (services->struct_size >=
            offsetof(RinDriverServicesV3, firmware_catalog_read_padding) +
                sizeof(services->firmware_catalog_read_padding) &&
        services->firmware_catalog_read_padding != 0u)
        return NULL;
    if (services->struct_size >=
            offsetof(RinDriverServicesV3, firmware_catalog_release_padding) +
                sizeof(services->firmware_catalog_release_padding) &&
        services->firmware_catalog_release_padding != 0u)
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

static inline int rin_driver_services_v3_has_firmware_catalog(
    const RinDriverApiV3* api, const RinDriverServicesV3* services) {
    const uint32_t required_size = (uint32_t)(
        offsetof(RinDriverServicesV3, firmware_catalog_release) +
        sizeof(services->firmware_catalog_release));
    return api && services && services->struct_size >= required_size &&
           (api->capabilities & RIN_DRIVER_V3_CAP_FIRMWARE) != 0u &&
           (services->capabilities & RIN_DRIVER_V3_CAP_FIRMWARE) != 0u &&
           services->firmware_catalog_open &&
           services->firmware_catalog_read &&
           services->firmware_catalog_release;
}

static inline int rin_driver_services_v3_bytes_zero(
    const void* memory, uint32_t size) {
    const uint8_t* bytes = (const uint8_t*)memory;
    for (uint32_t index = 0u; index < size; ++index) {
        if (bytes[index] != 0u) return 0;
    }
    return 1;
}

static inline int rin_driver_services_v3_ranges_overlap(
    const void* first, size_t first_size,
    const void* second, size_t second_size) {
    uintptr_t first_begin;
    uintptr_t second_begin;
    if (!first || !second || first_size == 0u || second_size == 0u)
        return 0;
    first_begin = (uintptr_t)first;
    second_begin = (uintptr_t)second;
    if (first_begin > UINTPTR_MAX - first_size ||
        second_begin > UINTPTR_MAX - second_size)
        return 1;
    return first_begin < second_begin + second_size &&
           second_begin < first_begin + first_size;
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
static_assert(offsetof(RinDriverServicesV3, irq_poll) == 56u,
              "NDRV v3 IRQ callback offset drift");
static_assert(offsetof(RinDriverServicesV3, firmware_open) == 64u,
              "NDRV v3 firmware callback offset drift");
static_assert(offsetof(RinDriverServicesV3, child_publish) == 88u,
              "NDRV v3 child callback offset drift");
static_assert(offsetof(RinDriverServicesV3, property_get) == 104u,
              "NDRV v3 property callback offset drift");
static_assert(offsetof(RinDriverServicesV3, irq_route) == 112u,
              "NDRV v3 IRQ route extension offset drift");
static_assert(offsetof(RinDriverServicesV3, wait_until) == 120u,
              "NDRV v3 wait extension offset drift");
static_assert(offsetof(RinDriverServicesV3, child_remove_ex) == 144u,
              "NDRV v3 child removal extension offset drift");
static_assert(offsetof(RinDriverServicesV3, firmware_catalog_open) == 152u,
              "NDRV v3 firmware catalog open extension offset drift");
static_assert(offsetof(RinDriverServicesV3, firmware_catalog_read) == 160u,
              "NDRV v3 firmware catalog read extension offset drift");
static_assert(offsetof(RinDriverServicesV3, firmware_catalog_release) == 168u,
              "NDRV v3 firmware catalog release extension offset drift");
static_assert(sizeof(RinDriverServicesV3) == 176u,
              "NDRV v3 services tail extension drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverServicesV3) == 176u,
               "NDRV v3 services table ABI drift");
_Static_assert(offsetof(RinDriverServicesV3, irq_poll) == 56u,
               "NDRV v3 IRQ callback offset drift");
_Static_assert(offsetof(RinDriverServicesV3, firmware_open) == 64u,
               "NDRV v3 firmware callback offset drift");
_Static_assert(offsetof(RinDriverServicesV3, child_publish) == 88u,
               "NDRV v3 child callback offset drift");
_Static_assert(offsetof(RinDriverServicesV3, property_get) == 104u,
               "NDRV v3 property callback offset drift");
_Static_assert(offsetof(RinDriverServicesV3, irq_route) == 112u,
               "NDRV v3 IRQ route extension offset drift");
_Static_assert(offsetof(RinDriverServicesV3, wait_until) == 120u,
               "NDRV v3 wait extension offset drift");
_Static_assert(offsetof(RinDriverServicesV3, child_remove_ex) == 144u,
               "NDRV v3 child removal extension offset drift");
_Static_assert(offsetof(RinDriverServicesV3, firmware_catalog_open) == 152u,
               "NDRV v3 firmware catalog open extension offset drift");
_Static_assert(offsetof(RinDriverServicesV3, firmware_catalog_read) == 160u,
               "NDRV v3 firmware catalog read extension offset drift");
_Static_assert(offsetof(RinDriverServicesV3, firmware_catalog_release) == 168u,
               "NDRV v3 firmware catalog release extension offset drift");
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
    int pci_location;
    if (!descriptor || descriptor->struct_size < sizeof(*descriptor) ||
        (descriptor->version != RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_V1 &&
         descriptor->version !=
             RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_PCI_V1 &&
         descriptor->version !=
             RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_RESOURCE_V1) ||
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
        (descriptor->version == RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_V1 &&
         !rin_driver_services_v3_bytes_zero(
             descriptor->reserved, sizeof(descriptor->reserved))) ||
        (descriptor->version ==
             RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_RESOURCE_V1 &&
         descriptor->bus_type == RIN_DRIVER_BUS_PCI))
        return 0;
    pci_location = descriptor->version ==
        RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_PCI_V1;
    if (pci_location &&
        (descriptor->bus_type != RIN_DRIVER_BUS_PCI ||
         (descriptor->reserved[0] >> 32u) != 0u ||
         descriptor->reserved[1] != 0u))
        return 0;
    if (descriptor->version ==
            RIN_DRIVER_V3_CHILD_DESCRIPTOR_VERSION_RESOURCE_V1 &&
        (descriptor->reserved[1] &
             ~RIN_DRIVER_V3_CHILD_RESOURCE_BROKER_V1_CAPABILITIES) != 0u)
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

static inline int rin_driver_v3_wait_until_available(
    const RinDriverApiV3* api, uint64_t session)
{
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    return services && session != 0u && services->wait_until != NULL;
}

static inline RinDriverV3Result rin_driver_v3_wait_until(
    const RinDriverApiV3* api, uint64_t session, uint64_t deadline_ns)
{
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    RinDriverV3Result result;
    if (!services || session == 0u || deadline_ns == 0u)
        return RIN_DRIVER_V3_INVALID;
    if (!services->wait_until) return RIN_DRIVER_V3_UNSUPPORTED;
    result = services->wait_until(
        services->api_context, session, deadline_ns);
    return rin_driver_services_v3_result_valid(result)
        ? result : RIN_DRIVER_V3_BACKEND_FAILED;
}

static inline int rin_driver_v3_irq_route_v1_valid(
    const RinDriverV3IrqRouteV1* route)
{
    return route && route->struct_size >= sizeof(*route) &&
           route->version == RIN_DRIVER_V3_IRQ_ROUTE_V1_VERSION &&
           route->device_cookie != 0u && route->device_generation != 0u &&
           route->lease != 0u && route->cpu_vector >= 32u &&
           route->cpu_vector <= 255u &&
           route->mode == RIN_DRIVER_V3_IRQ_MSIX && route->reserved0 == 0u &&
           rin_driver_services_v3_bytes_zero(
               route->reserved, sizeof(route->reserved));
}

static inline RinDriverV3Result rin_driver_v3_irq_route_v1(
    const RinDriverApiV3* api, uint64_t session,
    RinDriverV3Lease irq_lease, RinDriverV3IrqRouteV1* route_out)
{
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    RinDriverV3IrqRouteV1 route = {0};
    RinDriverV3Result result;
    if (route_out) rin_driver_services_v3_zero(route_out, sizeof(*route_out));
    if (!route_out || !services || session == 0u || irq_lease == 0u)
        return RIN_DRIVER_V3_DENIED;
    if (!rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_IRQ_ROUTE) ||
        !services->irq_route)
        return RIN_DRIVER_V3_UNSUPPORTED;
    route.struct_size = sizeof(route);
    route.version = RIN_DRIVER_V3_IRQ_ROUTE_V1_VERSION;
    result = services->irq_route(
        services->api_context, session, irq_lease, &route);
    if (!rin_driver_services_v3_result_valid(result))
        result = RIN_DRIVER_V3_BACKEND_FAILED;
    if (result != RIN_DRIVER_V3_OK) return result;
    if (!rin_driver_v3_irq_route_v1_valid(&route) ||
        route.lease != irq_lease)
        return RIN_DRIVER_V3_BACKEND_FAILED;
    *route_out = route;
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
        !output || !bytes_read || session == 0u)
        return RIN_DRIVER_V3_INVALID;
    if (!rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_FIRMWARE) ||
        !services->firmware_read)
        return RIN_DRIVER_V3_DENIED;
    if (capacity == 0u || capacity > RIN_DRIVER_V3_FIRMWARE_MAX_READ ||
        offset > firmware->length ||
        (offset < firmware->length &&
         capacity > firmware->length - offset))
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

static inline int rin_driver_v3_firmware_catalog_lease_v1_valid(
    const RinDriverV3FirmwareCatalogLeaseV1* lease) {
    uint8_t digest_nonzero = 0u;
    uint8_t key_nonzero = 0u;
    if (!lease || lease->struct_size != sizeof(*lease) ||
        lease->version !=
            RIN_DRIVER_V3_FIRMWARE_CATALOG_LEASE_V1_VERSION ||
        lease->lease == 0u || lease->length == 0u ||
        lease->length > RIN_DRIVER_V3_FIRMWARE_CATALOG_MAX_SIZE ||
        lease->flags != (RIN_DRIVER_V3_FIRMWARE_AUTHENTICATED |
                         RIN_DRIVER_V3_FIRMWARE_EMBEDDED) ||
        lease->reserved0 != 0u ||
        !rin_driver_services_v3_bytes_zero(
            lease->reserved, sizeof(lease->reserved)))
        return 0;
    for (uint32_t index = 0u; index < sizeof(lease->sha256); ++index)
        digest_nonzero |= lease->sha256[index];
    for (uint32_t index = 0u; index < sizeof(lease->trusted_key_id); ++index)
        key_nonzero |= lease->trusted_key_id[index];
    return digest_nonzero != 0u && key_nonzero != 0u;
}

static inline RinDriverV3Result rin_driver_v3_firmware_catalog_open_v1(
    const RinDriverApiV3* api, uint64_t session, const char* name,
    uint32_t name_length, RinDriverV3FirmwareCatalogLeaseV1* output) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    RinDriverV3Result result;
    int output_disjoint = output &&
        !rin_driver_services_v3_ranges_overlap(
            output, sizeof(*output), name, name_length) &&
        !rin_driver_services_v3_ranges_overlap(
            output, sizeof(*output), api, api ? sizeof(*api) : 0u) &&
        !rin_driver_services_v3_ranges_overlap(
            output, sizeof(*output), services,
            services ? services->struct_size : 0u);
    if (output_disjoint) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_V3_FIRMWARE_CATALOG_LEASE_V1_VERSION;
    }
    if (!services || !name || !output || session == 0u ||
        !output_disjoint ||
        !rin_driver_v3_firmware_name_valid(name, name_length))
        return RIN_DRIVER_V3_INVALID;
    if (!rin_driver_services_v3_has_firmware_catalog(api, services))
        return RIN_DRIVER_V3_UNSUPPORTED;
    result = services->firmware_catalog_open(
        services->api_context, session, name, name_length, output);
    if (!rin_driver_services_v3_result_valid(result))
        result = RIN_DRIVER_V3_BACKEND_FAILED;
    if (result != RIN_DRIVER_V3_OK) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_V3_FIRMWARE_CATALOG_LEASE_V1_VERSION;
        return result;
    }
    if (!rin_driver_v3_firmware_catalog_lease_v1_valid(output)) {
        if (output->lease != 0u)
            (void)services->firmware_catalog_release(
                services->api_context, session, output);
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_V3_FIRMWARE_CATALOG_LEASE_V1_VERSION;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    return RIN_DRIVER_V3_OK;
}

static inline RinDriverV3Result rin_driver_v3_firmware_catalog_read_v1(
    const RinDriverApiV3* api, uint64_t session,
    const RinDriverV3FirmwareCatalogLeaseV1* lease, uint64_t offset,
    void* output, uint32_t capacity, uint32_t* bytes_read) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    RinDriverV3Result result;
    int output_disjoint = output && lease && bytes_read &&
        !rin_driver_services_v3_ranges_overlap(
            output, capacity, lease, sizeof(*lease)) &&
        !rin_driver_services_v3_ranges_overlap(
            output, capacity, bytes_read, sizeof(*bytes_read)) &&
        !rin_driver_services_v3_ranges_overlap(
            output, capacity, api, api ? sizeof(*api) : 0u) &&
        !rin_driver_services_v3_ranges_overlap(
            output, capacity, services,
            services ? services->struct_size : 0u);
    int count_disjoint = bytes_read &&
        !rin_driver_services_v3_ranges_overlap(
            bytes_read, sizeof(*bytes_read), lease,
            lease ? sizeof(*lease) : 0u) &&
        !rin_driver_services_v3_ranges_overlap(
            bytes_read, sizeof(*bytes_read), output, output ? capacity : 0u) &&
        !rin_driver_services_v3_ranges_overlap(
            bytes_read, sizeof(*bytes_read), api, api ? sizeof(*api) : 0u) &&
        !rin_driver_services_v3_ranges_overlap(
            bytes_read, sizeof(*bytes_read), services,
            services ? services->struct_size : 0u);
    if (count_disjoint) *bytes_read = 0u;
    if (output_disjoint && capacity <= RIN_DRIVER_V3_FIRMWARE_MAX_READ)
        rin_driver_services_v3_zero(output, capacity);
    if (!services || !lease || !output || !bytes_read || !output_disjoint ||
        !count_disjoint || session == 0u ||
        !rin_driver_v3_firmware_catalog_lease_v1_valid(lease) ||
        capacity == 0u || capacity > RIN_DRIVER_V3_FIRMWARE_MAX_READ ||
        offset >= lease->length || capacity > lease->length - offset)
        return RIN_DRIVER_V3_INVALID;
    if (!rin_driver_services_v3_has_firmware_catalog(api, services))
        return RIN_DRIVER_V3_UNSUPPORTED;
    result = services->firmware_catalog_read(
        services->api_context, session, lease, offset, output, capacity,
        bytes_read);
    if (!rin_driver_services_v3_result_valid(result))
        result = RIN_DRIVER_V3_BACKEND_FAILED;
    if (result != RIN_DRIVER_V3_OK || *bytes_read != capacity) {
        if (capacity <= RIN_DRIVER_V3_FIRMWARE_MAX_READ)
            rin_driver_services_v3_zero(output, capacity);
        *bytes_read = 0u;
        return result == RIN_DRIVER_V3_OK ? RIN_DRIVER_V3_BACKEND_FAILED
                                          : result;
    }
    return RIN_DRIVER_V3_OK;
}

static inline RinDriverV3Result rin_driver_v3_firmware_catalog_release_v1(
    const RinDriverApiV3* api, uint64_t session,
    RinDriverV3FirmwareCatalogLeaseV1* lease) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    RinDriverV3Result result;
    if (!services || !lease || session == 0u || lease->lease == 0u ||
        rin_driver_services_v3_ranges_overlap(
            lease, sizeof(*lease), api, api ? sizeof(*api) : 0u) ||
        rin_driver_services_v3_ranges_overlap(
            lease, sizeof(*lease), services,
            services ? services->struct_size : 0u))
        return RIN_DRIVER_V3_INVALID;
    if (!rin_driver_services_v3_has_firmware_catalog(api, services))
        return RIN_DRIVER_V3_UNSUPPORTED;
    result = services->firmware_catalog_release(
        services->api_context, session, lease);
    if (!rin_driver_services_v3_result_valid(result))
        result = RIN_DRIVER_V3_BACKEND_FAILED;
    if (result == RIN_DRIVER_V3_OK)
        rin_driver_services_v3_zero(lease, sizeof(*lease));
    return result;
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

static inline RinDriverV3Result rin_driver_v3_child_remove_ex(
    const RinDriverApiV3* api, uint64_t session,
    const RinDriverV3ChildDevice* child,
    RinDriverKernelModuleStopReasonV1 reason) {
    const RinDriverServicesV3* services = rin_driver_services_v3_get(api);
    if (!services || !child || session == 0u ||
        (uint32_t)reason >
            (uint32_t)RIN_DRIVER_KERNEL_MODULE_STOP_START_FAILURE ||
        child->lease == 0u || child->generation == 0u ||
        child->struct_size < sizeof(*child) ||
        child->version != RIN_DRIVER_SERVICES_V3_VERSION ||
        child->device == 0u || child->parent_device == 0u ||
        child->bus_type == 0u ||
        (child->flags & ~RIN_DRIVER_V3_CHILD_FLAG_MASK) != 0u ||
        child->reserved0 != 0u ||
        !rin_driver_services_v3_bytes_zero(
            child->reserved, sizeof(child->reserved)) ||
        !rin_driver_services_v3_has(
            api, services, RIN_DRIVER_V3_CAP_CHILD_DEVICE))
        return RIN_DRIVER_V3_INVALID;
    if (services->struct_size <
            offsetof(RinDriverServicesV3, child_remove_ex) +
                sizeof(services->child_remove_ex) ||
        !services->child_remove_ex) {
        if (reason == RIN_DRIVER_KERNEL_MODULE_STOP_NORMAL)
            return rin_driver_v3_child_remove(api, session, child);
        return RIN_DRIVER_V3_UNSUPPORTED;
    }
    RinDriverV3Result result = services->child_remove_ex(
        services->api_context, session, child->lease, child->generation,
        reason);
    return rin_driver_services_v3_result_valid(result)
        ? result : RIN_DRIVER_V3_BACKEND_FAILED;
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
         key > RIN_DRIVER_V3_PROPERTY_ACPI_WOV) ||
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
    if (key == RIN_DRIVER_V3_PROPERTY_ACPI_WOV &&
        (output->type != RIN_DRIVER_V3_PROPERTY_U32 || output->value > 1u)) {
        rin_driver_services_v3_zero(output, sizeof(*output));
        output->struct_size = sizeof(*output);
        output->version = RIN_DRIVER_SERVICES_V3_VERSION;
        return RIN_DRIVER_V3_BACKEND_FAILED;
    }
    return RIN_DRIVER_V3_OK;
}

#endif /* RIN_DRIVER_SERVICES_V3_H */
