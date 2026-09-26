/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_CONTRACT_V3_H
#define RIN_DRIVER_CONTRACT_V3_H

#include <stdint.h>

#include <rin/driver/abi_v3.h>

/*
 * NDRV v3 operation contracts. These are wire records and RVA/opaque-handle
 * values, not kernel objects. The OS-Core validates every record before it
 * grants or revokes a resource; the Driver never supplies a physical address
 * or a kernel callback pointer.
 */
#define RIN_DRIVER_CONTRACT_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_MAX_SG_SEGMENTS UINT32_C(64)

typedef enum RinDriverIrqModeV3 {
    RIN_DRIVER_IRQ_LEGACY = 1,
    RIN_DRIVER_IRQ_MSI = 2,
    RIN_DRIVER_IRQ_MSIX = 3
} RinDriverIrqModeV3;

typedef enum RinDriverDmaSyncActionV3 {
    RIN_DRIVER_DMA_PREPARE_DEVICE = 1,
    RIN_DRIVER_DMA_COMPLETE_DEVICE = 2
} RinDriverDmaSyncActionV3;

typedef enum RinDriverRevokeReasonV3 {
    RIN_DRIVER_REVOKE_UNLOAD = 1,
    RIN_DRIVER_REVOKE_DEVICE_REMOVED = 2,
    RIN_DRIVER_REVOKE_GENERATION_CHANGED = 3,
    RIN_DRIVER_REVOKE_QUARANTINED = 4
} RinDriverRevokeReasonV3;

typedef uint32_t RinDriverMmioFlagsV3;
#define RIN_DRIVER_MMIO_READ UINT32_C(0x00000001)
#define RIN_DRIVER_MMIO_WRITE UINT32_C(0x00000002)
#define RIN_DRIVER_MMIO_FLAG_MASK (RIN_DRIVER_MMIO_READ | RIN_DRIVER_MMIO_WRITE)

typedef uint32_t RinDriverDmaFlagsV3;
#define RIN_DRIVER_DMA_READ UINT32_C(0x00000001)
#define RIN_DRIVER_DMA_WRITE UINT32_C(0x00000002)
#define RIN_DRIVER_DMA_COHERENT UINT32_C(0x00000004)
#define RIN_DRIVER_DMA_BELOW_4G UINT32_C(0x00000008)
#define RIN_DRIVER_DMA_CONTIGUOUS UINT32_C(0x00000010)
#define RIN_DRIVER_DMA_FLAG_MASK \
    (RIN_DRIVER_DMA_READ | RIN_DRIVER_DMA_WRITE | \
     RIN_DRIVER_DMA_COHERENT | RIN_DRIVER_DMA_BELOW_4G | \
     RIN_DRIVER_DMA_CONTIGUOUS)

typedef uint32_t RinDriverIrqFlagsV3;
#define RIN_DRIVER_IRQ_SHARED UINT32_C(0x00000001)
#define RIN_DRIVER_IRQ_MASK_ON_QUIESCE UINT32_C(0x00000002)
#define RIN_DRIVER_IRQ_FLAG_MASK \
    (RIN_DRIVER_IRQ_SHARED | RIN_DRIVER_IRQ_MASK_ON_QUIESCE)

typedef uint32_t RinDriverTimerFlagsV3;
#define RIN_DRIVER_TIMER_ONE_SHOT UINT32_C(0x00000001)
#define RIN_DRIVER_TIMER_PERIODIC UINT32_C(0x00000002)
#define RIN_DRIVER_TIMER_COALESCED UINT32_C(0x00000004)
#define RIN_DRIVER_TIMER_FLAG_MASK \
    (RIN_DRIVER_TIMER_ONE_SHOT | RIN_DRIVER_TIMER_PERIODIC | \
     RIN_DRIVER_TIMER_COALESCED)

#pragma pack(push, 1)

typedef struct RinDriverEntryContractV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint64_t probe_rva;
    uint64_t start_rva;
    uint64_t quiesce_rva;
    uint64_t stop_rva;
    uint64_t remove_rva;
    uint64_t required_capabilities;
    uint64_t reserved[2];
} RinDriverEntryContractV3;

typedef struct RinDriverBarLeaseV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint16_t bar_index;
    uint16_t flags;
    RinDriverResourceHandleV3 lease;
    RinDriverGenerationV3 generation;
    uint64_t offset;
    uint64_t length;
    RinDriverCapabilitySetV3 capability;
    uint64_t reserved[2];
} RinDriverBarLeaseV3;

typedef struct RinDriverDmaSegmentV3 {
    uint64_t iova;
    uint64_t length;
} RinDriverDmaSegmentV3;

typedef struct RinDriverDmaLeaseV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    RinDriverResourceHandleV3 lease;
    RinDriverGenerationV3 generation;
    RinDriverResourceHandleV3 memory_lease;
    uint64_t length;
    uint32_t alignment;
    RinDriverDmaFlagsV3 flags;
    uint32_t segment_count;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverDmaLeaseV3;

typedef struct RinDriverIrqLeaseV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    RinDriverResourceHandleV3 lease;
    RinDriverGenerationV3 generation;
    uint32_t mode;
    uint32_t vector;
    RinDriverIrqFlagsV3 flags;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverIrqLeaseV3;

typedef struct RinDriverTimerLeaseV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    RinDriverResourceHandleV3 lease;
    RinDriverGenerationV3 generation;
    uint64_t deadline_ns;
    uint64_t period_ns;
    RinDriverTimerFlagsV3 flags;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverTimerLeaseV3;

typedef struct RinDriverTimerEventV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    RinDriverResourceHandleV3 lease;
    RinDriverGenerationV3 generation;
    uint64_t deadline_ns;
    uint32_t expirations;
    RinDriverTimerFlagsV3 flags;
    uint64_t reserved[2];
} RinDriverTimerEventV3;

typedef struct RinDriverRegistrationV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t device_class;
    uint32_t flags;
    uint64_t operations_rva;
    uint64_t operations_size;
    RinDriverHandleV3 device;
    RinDriverGenerationV3 generation;
    RinDriverCapabilitySetV3 capability;
    uint64_t reserved[2];
} RinDriverRegistrationV3;

typedef struct RinDriverCharacterOpsV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint64_t read_rva;
    uint64_t write_rva;
    uint64_t control_rva;
    uint64_t remove_rva;
    uint64_t reserved[2];
} RinDriverCharacterOpsV3;

typedef struct RinDriverBlockOpsV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint64_t geometry_rva;
    uint64_t submit_rva;
    uint64_t poll_rva;
    uint64_t cancel_rva;
    uint64_t reset_rva;
    uint64_t remove_rva;
    uint64_t reserved[2];
} RinDriverBlockOpsV3;

typedef struct RinDriverNetworkOpsV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint64_t transmit_rva;
    uint64_t receive_rva;
    uint64_t poll_rva;
    uint64_t info_rva;
    uint64_t remove_rva;
    uint64_t reserved[2];
} RinDriverNetworkOpsV3;

typedef struct RinDriverInputOpsV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint64_t read_rva;
    uint64_t remove_rva;
    uint64_t reserved[2];
} RinDriverInputOpsV3;

typedef struct RinDriverLogRecordV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t level;
    uint32_t flags;
    uint64_t message_rva;
    uint32_t message_length;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinDriverLogRecordV3;

typedef struct RinDriverLogOpsV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    uint64_t emit_rva;
    uint64_t reserved[2];
} RinDriverLogOpsV3;

typedef struct RinDriverRevokeContractV3 {
    uint32_t struct_size;
    uint16_t version_major;
    uint16_t version_minor;
    RinDriverResourceHandleV3 lease;
    RinDriverGenerationV3 generation;
    uint32_t reason;
    uint32_t flags;
    uint64_t reserved[2];
} RinDriverRevokeContractV3;

#pragma pack(pop)

#if defined(__cplusplus)
static_assert(sizeof(RinDriverEntryContractV3) == 72u,
              "NDRV v3 entry contract ABI drift");
static_assert(sizeof(RinDriverBarLeaseV3) == 68u,
              "NDRV v3 BAR lease ABI drift");
static_assert(sizeof(RinDriverDmaSegmentV3) == 16u,
              "NDRV v3 DMA segment ABI drift");
static_assert(sizeof(RinDriverDmaLeaseV3) == 72u,
              "NDRV v3 DMA lease ABI drift");
static_assert(sizeof(RinDriverIrqLeaseV3) == 56u,
              "NDRV v3 IRQ lease ABI drift");
static_assert(sizeof(RinDriverTimerLeaseV3) == 64u,
              "NDRV v3 timer lease ABI drift");
static_assert(sizeof(RinDriverTimerEventV3) == 56u,
              "NDRV v3 timer event ABI drift");
static_assert(sizeof(RinDriverRegistrationV3) == 72u,
              "NDRV v3 registration ABI drift");
static_assert(sizeof(RinDriverCharacterOpsV3) == 56u,
              "NDRV v3 character operations ABI drift");
static_assert(sizeof(RinDriverBlockOpsV3) == 72u,
              "NDRV v3 block operations ABI drift");
static_assert(sizeof(RinDriverNetworkOpsV3) == 64u,
              "NDRV v3 network operations ABI drift");
static_assert(sizeof(RinDriverInputOpsV3) == 40u,
              "NDRV v3 input operations ABI drift");
static_assert(sizeof(RinDriverLogRecordV3) == 48u,
              "NDRV v3 log record ABI drift");
static_assert(sizeof(RinDriverLogOpsV3) == 32u,
              "NDRV v3 log operations ABI drift");
static_assert(sizeof(RinDriverRevokeContractV3) == 48u,
              "NDRV v3 revoke ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverEntryContractV3) == 72u,
               "NDRV v3 entry contract ABI drift");
_Static_assert(sizeof(RinDriverBarLeaseV3) == 68u,
               "NDRV v3 BAR lease ABI drift");
_Static_assert(sizeof(RinDriverDmaSegmentV3) == 16u,
               "NDRV v3 DMA segment ABI drift");
_Static_assert(sizeof(RinDriverDmaLeaseV3) == 72u,
               "NDRV v3 DMA lease ABI drift");
_Static_assert(sizeof(RinDriverIrqLeaseV3) == 56u,
               "NDRV v3 IRQ lease ABI drift");
_Static_assert(sizeof(RinDriverTimerLeaseV3) == 64u,
               "NDRV v3 timer lease ABI drift");
_Static_assert(sizeof(RinDriverTimerEventV3) == 56u,
               "NDRV v3 timer event ABI drift");
_Static_assert(sizeof(RinDriverRegistrationV3) == 72u,
               "NDRV v3 registration ABI drift");
_Static_assert(sizeof(RinDriverCharacterOpsV3) == 56u,
               "NDRV v3 character operations ABI drift");
_Static_assert(sizeof(RinDriverBlockOpsV3) == 72u,
               "NDRV v3 block operations ABI drift");
_Static_assert(sizeof(RinDriverNetworkOpsV3) == 64u,
               "NDRV v3 network operations ABI drift");
_Static_assert(sizeof(RinDriverInputOpsV3) == 40u,
               "NDRV v3 input operations ABI drift");
_Static_assert(sizeof(RinDriverLogRecordV3) == 48u,
               "NDRV v3 log record ABI drift");
_Static_assert(sizeof(RinDriverLogOpsV3) == 32u,
               "NDRV v3 log operations ABI drift");
_Static_assert(sizeof(RinDriverRevokeContractV3) == 48u,
               "NDRV v3 revoke ABI drift");
#endif

#endif /* RIN_DRIVER_CONTRACT_V3_H */
