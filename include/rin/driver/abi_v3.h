/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_ABI_V3_H
#define RIN_DRIVER_ABI_V3_H

#include <stdint.h>

#include <rin/formats_v3.h>

/*
 * Public NDRV v3 contract.
 *
 * This header intentionally contains fixed-width values only.  It does not
 * include OS-Core headers, expose kernel objects, or make a physical pointer
 * part of the Driver ownership contract.
 */
#define RIN_DRIVER_PUBLIC_ABI_MAJOR UINT16_C(3)
#define RIN_DRIVER_PUBLIC_ABI_MINOR UINT16_C(0)
#define RIN_DRIVER_PUBLIC_ABI_VERSION UINT32_C(0x00030000)
#define RIN_DRIVER_PUBLIC_MAX_RESERVED_WORDS UINT32_C(4)

typedef uint64_t RinDriverHandleV3;
typedef uint64_t RinDriverResourceHandleV3;
typedef uint64_t RinDriverGenerationV3;
typedef uint64_t RinDriverCapabilitySetV3;

typedef enum RinDriverResultV3 {
    RIN_DRIVER_RESULT_OK = 0,
    RIN_DRIVER_RESULT_INVALID = -1,
    RIN_DRIVER_RESULT_DENIED = -2,
    RIN_DRIVER_RESULT_NO_SPACE = -3,
    RIN_DRIVER_RESULT_STALE = -4,
    RIN_DRIVER_RESULT_BACKEND = -5,
    RIN_DRIVER_RESULT_REMOVED = -6,
    RIN_DRIVER_RESULT_BUSY = -7,
    RIN_DRIVER_RESULT_TIMEOUT = -8,
    RIN_DRIVER_RESULT_UNSUPPORTED = -9
} RinDriverResultV3;

typedef enum RinDriverResourceTypeV3 {
    RIN_DRIVER_RESOURCE_TYPE_NONE = 0,
    RIN_DRIVER_RESOURCE_TYPE_PCI_CONFIG = 1,
    RIN_DRIVER_RESOURCE_TYPE_BAR = 2,
    RIN_DRIVER_RESOURCE_TYPE_DMA = 3,
    RIN_DRIVER_RESOURCE_TYPE_IRQ = 4,
    RIN_DRIVER_RESOURCE_TYPE_TIMER = 5,
    RIN_DRIVER_RESOURCE_TYPE_REGISTRATION = 6
} RinDriverResourceTypeV3;

typedef uint32_t RinDriverResourceFlagsV3;
#define RIN_DRIVER_RESOURCE_READ UINT32_C(0x00000001)
#define RIN_DRIVER_RESOURCE_WRITE UINT32_C(0x00000002)
#define RIN_DRIVER_RESOURCE_COHERENT UINT32_C(0x00000004)
#define RIN_DRIVER_RESOURCE_EXCLUSIVE UINT32_C(0x00000008)
#define RIN_DRIVER_RESOURCE_FLAG_MASK \
    (RIN_DRIVER_RESOURCE_READ | RIN_DRIVER_RESOURCE_WRITE | \
     RIN_DRIVER_RESOURCE_COHERENT | RIN_DRIVER_RESOURCE_EXCLUSIVE)

typedef uint64_t RinDriverCapabilityV3;
#define RIN_DRIVER_CAP_PCI_CONFIG UINT64_C(0x0000000000000001)
#define RIN_DRIVER_CAP_BAR UINT64_C(0x0000000000000002)
#define RIN_DRIVER_CAP_DMA UINT64_C(0x0000000000000004)
#define RIN_DRIVER_CAP_IRQ UINT64_C(0x0000000000000008)
#define RIN_DRIVER_CAP_TIMER UINT64_C(0x0000000000000010)
#define RIN_DRIVER_CAP_REGISTER UINT64_C(0x0000000000000020)
#define RIN_DRIVER_CAPABILITY_MASK \
    (RIN_DRIVER_CAP_PCI_CONFIG | RIN_DRIVER_CAP_BAR | RIN_DRIVER_CAP_DMA | \
     RIN_DRIVER_CAP_IRQ | RIN_DRIVER_CAP_TIMER | RIN_DRIVER_CAP_REGISTER)

typedef enum RinDriverLifecycleStateV3 {
    RIN_DRIVER_LIFECYCLE_PROBE = 1,
    RIN_DRIVER_LIFECYCLE_START = 2,
    RIN_DRIVER_LIFECYCLE_QUIESCE = 3,
    RIN_DRIVER_LIFECYCLE_STOP = 4,
    RIN_DRIVER_LIFECYCLE_REMOVE = 5
} RinDriverLifecycleStateV3;

#pragma pack(push, 1)

typedef struct RinDriverAbiHeaderV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t reserved;
} RinDriverAbiHeaderV3;

typedef struct RinDriverResourceDescriptorV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint16_t type;
    uint16_t flags;
    RinDriverResourceHandleV3 handle;
    RinDriverGenerationV3 generation;
    uint64_t start;
    uint64_t length;
    RinDriverCapabilitySetV3 capability;
    uint64_t reserved[2];
} RinDriverResourceDescriptorV3;

typedef struct RinDriverPciMatchV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_device_id;
    uint32_t class_code;
    uint32_t class_mask;
    uint32_t revision;
    uint32_t revision_mask;
    uint64_t required_capabilities;
    uint64_t reserved[2];
} RinDriverPciMatchV3;

typedef struct RinDriverContextV3Public {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    RinDriverHandleV3 driver;
    RinDriverHandleV3 device;
    RinDriverGenerationV3 generation;
    RinDriverCapabilitySetV3 granted_capabilities;
    uint64_t reserved[4];
} RinDriverContextV3Public;

typedef struct RinDriverLifecycleV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint64_t probe_rva;
    uint64_t start_rva;
    uint64_t quiesce_rva;
    uint64_t stop_rva;
    uint64_t remove_rva;
    uint64_t reserved[2];
} RinDriverLifecycleV3;

#pragma pack(pop)

#if defined(__cplusplus)
static_assert(sizeof(RinDriverAbiHeaderV3) == 12u,
              "RinDriverAbiHeaderV3 ABI drift");
static_assert(sizeof(RinDriverResourceDescriptorV3) == 68u,
              "RinDriverResourceDescriptorV3 ABI drift");
static_assert(sizeof(RinDriverPciMatchV3) == 56u,
              "RinDriverPciMatchV3 ABI drift");
static_assert(sizeof(RinDriverContextV3Public) == 72u,
              "RinDriverContextV3Public ABI drift");
static_assert(sizeof(RinDriverLifecycleV3) == 64u,
              "RinDriverLifecycleV3 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverAbiHeaderV3) == 12u,
               "RinDriverAbiHeaderV3 ABI drift");
_Static_assert(sizeof(RinDriverResourceDescriptorV3) == 68u,
               "RinDriverResourceDescriptorV3 ABI drift");
_Static_assert(sizeof(RinDriverPciMatchV3) == 56u,
               "RinDriverPciMatchV3 ABI drift");
_Static_assert(sizeof(RinDriverContextV3Public) == 72u,
               "RinDriverContextV3Public ABI drift");
_Static_assert(sizeof(RinDriverLifecycleV3) == 64u,
               "RinDriverLifecycleV3 ABI drift");
#endif

#endif /* RIN_DRIVER_ABI_V3_H */
