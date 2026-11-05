/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_MODULE_API_V3_H
#define RIN_DRIVER_MODULE_API_V3_H

#include <stddef.h>
#include <stdint.h>

#include <rin/formats_v3.h>
#include <rin/driver/admission_v3.h>

#define RIN_DRIVER_V3_ABI_VERSION UINT32_C(0x00030000)
#define RIN_DRIVER_V3_DMA_READ    UINT32_C(0x00000001)
#define RIN_DRIVER_V3_DMA_WRITE   UINT32_C(0x00000002)
#define RIN_DRIVER_V3_DMA_COHERENT UINT32_C(0x00000004)
#define RIN_DRIVER_V3_DMA_BELOW_4G UINT32_C(0x00000008)
#define RIN_DRIVER_V3_DMA_BOUNCE_ALLOWED UINT32_C(0x00000010)
#define RIN_DRIVER_V3_DMA_CONTIGUOUS UINT32_C(0x00000020)
#define RIN_DRIVER_V3_DMA_FLAG_MASK                                   \
    (RIN_DRIVER_V3_DMA_READ | RIN_DRIVER_V3_DMA_WRITE |               \
     RIN_DRIVER_V3_DMA_COHERENT | RIN_DRIVER_V3_DMA_BELOW_4G |        \
     RIN_DRIVER_V3_DMA_BOUNCE_ALLOWED | RIN_DRIVER_V3_DMA_CONTIGUOUS)
#define RIN_DRIVER_V3_BAR_READ     UINT32_C(0x00000001)
#define RIN_DRIVER_V3_BAR_WRITE    UINT32_C(0x00000002)
#define RIN_DRIVER_V3_BAR_FLAG_MASK                                    \
    (RIN_DRIVER_V3_BAR_READ | RIN_DRIVER_V3_BAR_WRITE)
#define RIN_DRIVER_V3_MAX_SG_SEGMENTS 64u
#define RIN_DRIVER_V3_CAP_DMA_ISOLATED UINT64_C(0x0000000000000001)
#define RIN_DRIVER_V3_CAP_BAR           UINT64_C(0x0000000000000002)
#define RIN_DRIVER_V3_CAP_MSI           UINT64_C(0x0000000000000004)
#define RIN_DRIVER_V3_CAP_MSIX          UINT64_C(0x0000000000000008)
#define RIN_DRIVER_V3_CAP_TIMER         UINT64_C(0x0000000000000010)
#define RIN_DRIVER_V3_CAP_REGISTER      UINT64_C(0x0000000000000020)
#define RIN_DRIVER_V3_CAP_PCI_CONFIG    UINT64_C(0x0000000000000040)
#define RIN_DRIVER_V3_CAP_DMA_START      UINT64_C(0x0000000000000080)
#define RIN_DRIVER_V3_CAP_DMA_CONTIGUOUS UINT64_C(0x0000000000000100)
#define RIN_DRIVER_V3_CAP_IOPORT          UINT64_C(0x0000000000000200)
#define RIN_DRIVER_V3_CAP_IRQ_EVENTS       UINT64_C(0x0000000000000400)
#define RIN_DRIVER_V3_CAP_FIRMWARE         UINT64_C(0x0000000000000800)
#define RIN_DRIVER_V3_CAP_CHILD_DEVICE     UINT64_C(0x0000000000001000)
#define RIN_DRIVER_V3_CAP_DEVICE_PROPERTIES UINT64_C(0x0000000000002000)
#define RIN_DRIVER_V3_CAP_ENTROPY_MIX      UINT64_C(0x0000000000004000)
#define RIN_DRIVER_V3_CAP_IRQ_ROUTE        UINT64_C(0x0000000000008000)
#define RIN_DRIVER_V3_CAP_DMA_STOP          UINT64_C(0x0000000000010000)
#define RIN_DRIVER_V3_CAP_NPU_PROCESS_PASID_PROVIDER \
    UINT64_C(0x0000000000020000)
#define RIN_DRIVER_V3_CAPABILITY_MASK                                   \
    (RIN_DRIVER_V3_CAP_DMA_ISOLATED | RIN_DRIVER_V3_CAP_BAR |            \
     RIN_DRIVER_V3_CAP_MSI | RIN_DRIVER_V3_CAP_MSIX |                    \
     RIN_DRIVER_V3_CAP_TIMER | RIN_DRIVER_V3_CAP_REGISTER |              \
     RIN_DRIVER_V3_CAP_PCI_CONFIG | RIN_DRIVER_V3_CAP_DMA_START |          \
     RIN_DRIVER_V3_CAP_DMA_CONTIGUOUS | RIN_DRIVER_V3_CAP_IOPORT |       \
     RIN_DRIVER_V3_CAP_IRQ_EVENTS | RIN_DRIVER_V3_CAP_FIRMWARE |          \
     RIN_DRIVER_V3_CAP_CHILD_DEVICE | RIN_DRIVER_V3_CAP_DEVICE_PROPERTIES | \
     RIN_DRIVER_V3_CAP_ENTROPY_MIX | RIN_DRIVER_V3_CAP_IRQ_ROUTE |       \
     RIN_DRIVER_V3_CAP_DMA_STOP | RIN_DRIVER_V3_CAP_NPU_PROCESS_PASID_PROVIDER)

#define RIN_DRIVER_V3_DEVICE_BLOCK   UINT32_C(1)
#define RIN_DRIVER_V3_DEVICE_NET     UINT32_C(2)
#define RIN_DRIVER_V3_DEVICE_INPUT   UINT32_C(3)
#define RIN_DRIVER_V3_DEVICE_DISPLAY UINT32_C(4)
#define RIN_DRIVER_V3_DEVICE_AUDIO   UINT32_C(5)
#define RIN_DRIVER_V3_DEVICE_USB_CONTROLLER UINT32_C(6)
#define RIN_DRIVER_V3_DEVICE_SERIAL   UINT32_C(7)
#define RIN_DRIVER_V3_DEVICE_BLUETOOTH_HCI UINT32_C(8)
#define RIN_DRIVER_V3_DEVICE_WIFI     UINT32_C(9)
#define RIN_DRIVER_V3_DEVICE_NPU      UINT32_C(10)

#define RIN_DRIVER_V3_OPS_VERSION UINT32_C(0x00010000)

typedef uint64_t RinDriverV3Lease;

typedef enum RinDriverV3Result {
    RIN_DRIVER_V3_OK = 0,
    RIN_DRIVER_V3_INVALID = -1,
    RIN_DRIVER_V3_DENIED = -2,
    RIN_DRIVER_V3_NO_SPACE = -3,
    RIN_DRIVER_V3_STALE_LEASE = -4,
    RIN_DRIVER_V3_BACKEND_FAILED = -5,
    RIN_DRIVER_V3_REMOVED = -6,
    RIN_DRIVER_V3_QUARANTINED = -7,
    RIN_DRIVER_V3_BUSY = -8,
    RIN_DRIVER_V3_TIMEOUT = -9,
    RIN_DRIVER_V3_CANCELLED = -10,
    RIN_DRIVER_V3_READ_ONLY = -11,
    RIN_DRIVER_V3_RANGE = -12,
    RIN_DRIVER_V3_UNSUPPORTED = -13
} RinDriverV3Result;

/* NDRV v3 power-entry contract. Drivers start in D0. D3 is the system
 * suspend state; D1 and D2 are reserved until the platform defines their
 * device-wide transition semantics. */
#define RIN_DRIVER_POWER_STATE_D0 UINT32_C(0)
#define RIN_DRIVER_POWER_STATE_D1 UINT32_C(1)
#define RIN_DRIVER_POWER_STATE_D2 UINT32_C(2)
#define RIN_DRIVER_POWER_STATE_D3 UINT32_C(3)
#define RIN_DRIVER_POWER_STATE_MAX RIN_DRIVER_POWER_STATE_D3

/* A versioned Stop callback result. Device generation is the exact catalog
 * generation bound to this driver instance. */
#define RIN_DRIVER_V3_STOP_REASON_INFO_VERSION UINT32_C(0x00010000)
typedef enum RinDriverV3StopReason {
    RIN_DRIVER_V3_STOP_REASON_NORMAL = 0,
    RIN_DRIVER_V3_STOP_REASON_DEVICE_REMOVED = 1,
    RIN_DRIVER_V3_STOP_REASON_SURPRISE_REMOVAL = 2,
    RIN_DRIVER_V3_STOP_REASON_START_FAILURE = 3
} RinDriverV3StopReason;

typedef struct RinDriverV3StopReasonInfo {
    uint32_t struct_size;
    uint32_t version;
    uint32_t reason;
    uint32_t reserved0;
    uint64_t device_generation;
    uint64_t reserved[2];
} RinDriverV3StopReasonInfo;

/* A quarantined session must never be presented as healthy by a management
 * or recovery owner.  The health value is monotonic until the session is
 * closed and reopened; ACTIVE is the only state in which new leases may be
 * granted. */
#define RIN_DRIVER_V3_STATUS_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_STATUS_FLAG_QUARANTINED UINT32_C(0x00000001)
#define RIN_DRIVER_V3_STATUS_FLAG_REMOVING   UINT32_C(0x00000002)
#define RIN_DRIVER_V3_STATUS_FLAG_DMA_STARTED UINT32_C(0x00000004)

typedef enum RinDriverV3HealthState {
    RIN_DRIVER_V3_HEALTH_ACTIVE = 1,
    RIN_DRIVER_V3_HEALTH_DEGRADED = 2,
    RIN_DRIVER_V3_HEALTH_RECOVERY_REQUIRED = 3,
    RIN_DRIVER_V3_HEALTH_LOST = 4
} RinDriverV3HealthState;

typedef enum RinDriverV3FailureReason {
    RIN_DRIVER_V3_FAILURE_NONE = 0,
    RIN_DRIVER_V3_FAILURE_BACKEND = 1,
    RIN_DRIVER_V3_FAILURE_DMA_SYNC = 2,
    RIN_DRIVER_V3_FAILURE_RESET = 3,
    RIN_DRIVER_V3_FAILURE_RESOURCE_RELEASE = 4,
    RIN_DRIVER_V3_FAILURE_DOMAIN_DESTROY = 5,
    RIN_DRIVER_V3_FAILURE_EXTERNAL_FATAL = 6
} RinDriverV3FailureReason;

typedef struct RinDriverV3Status {
    uint32_t struct_size;
    uint32_t version;
    uint32_t health_state;
    uint32_t failure_reason;
    uint64_t failure_generation;
    uint32_t dma_leases;
    uint32_t bar_leases;
    uint32_t irq_leases;
    uint32_t timer_leases;
    uint32_t registration_leases;
    uint32_t flags;
    uint64_t device_generation;
    uint64_t reserved[3];
} RinDriverV3Status;

typedef enum RinDriverV3IrqMode {
    RIN_DRIVER_V3_IRQ_LEGACY = 1,
    RIN_DRIVER_V3_IRQ_MSI = 2,
    RIN_DRIVER_V3_IRQ_MSIX = 3
} RinDriverV3IrqMode;

typedef enum RinDriverV3DmaSyncAction {
    RIN_DRIVER_V3_DMA_PREPARE_DEVICE = 1,
    RIN_DRIVER_V3_DMA_COMPLETE_DEVICE = 2
} RinDriverV3DmaSyncAction;

#define RIN_DRIVER_V3_TIMER_EVENT_FLAG_ONE_SHOT UINT32_C(0x00000001)
#define RIN_DRIVER_V3_TIMER_EVENT_FLAG_PERIODIC UINT32_C(0x00000002)
#define RIN_DRIVER_V3_TIMER_EVENT_FLAG_COALESCED UINT32_C(0x00000004)
#define RIN_DRIVER_V3_TIMER_EVENT_FLAG_MASK \
    (RIN_DRIVER_V3_TIMER_EVENT_FLAG_ONE_SHOT | \
     RIN_DRIVER_V3_TIMER_EVENT_FLAG_PERIODIC | \
     RIN_DRIVER_V3_TIMER_EVENT_FLAG_COALESCED)

/* Timer delivery is an explicit bounded poll.  The kernel owns the deadline
 * and returns the current device generation with every event; a driver must
 * discard an event after reset or teardown rather than reusing its lease. */
typedef struct RinDriverV3TimerEvent {
    RinDriverV3Lease lease;
    uint64_t device_generation;
    uint64_t deadline_ns;
    uint32_t expirations;
    uint32_t flags;
} RinDriverV3TimerEvent;

/* memory_lease names kernel-owned/pinned memory. A driver never supplies a
 * pointer or a physical page number to the DMA mapper. */
typedef struct RinDriverV3MemorySlice {
    uint64_t memory_lease;
    uint64_t offset;
    uint64_t length;
} RinDriverV3MemorySlice;

typedef struct RinDriverV3DmaSegment {
    uint64_t iova;
    uint64_t length;
} RinDriverV3DmaSegment;

typedef struct RinDriverV3DmaLeaseInfo {
    RinDriverV3Lease lease;
    uint64_t driver_virtual_address;
    uint64_t iova;
    uint64_t length;
    uint32_t alignment;
    uint32_t flags;
    uint32_t segment_count;
    uint32_t reserved;
} RinDriverV3DmaLeaseInfo;

typedef struct RinDriverV3BarLeaseInfo {
    RinDriverV3Lease lease;
    uint64_t driver_virtual_address;
    uint64_t length;
    uint32_t bar_index;
    uint32_t flags;
} RinDriverV3BarLeaseInfo;

typedef struct RinDriverV3IrqLeaseInfo {
    RinDriverV3Lease lease;
    uint32_t vector;
    uint32_t mode;
} RinDriverV3IrqLeaseInfo;

typedef struct RinDriverContextV3 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t session;
    uint64_t device;
    uint64_t generation;
    uint64_t driver_cookie;
    uint64_t granted_resources;
    uint64_t required_cpu_features;
    uint32_t architecture;
    uint32_t flags;
    uint64_t reserved[8];
    /* Append-only DMI extension. The NDRV loader leaves these fields zero
     * for the legacy outer entry and fills them only for the embedded RIN
     * kernel-module phase callback. This exposes the exact device identity
     * selected by the signed NDRV match without changing the NDRV v3 entry
     * signature or any existing field offsets. */
    uint32_t kernel_module_context_version;
    uint32_t kernel_module_matched_index;
    RinDriverDeviceIdentityV3 kernel_module_device;
} RinDriverContextV3;

#if defined(__cplusplus)
static_assert(sizeof(RinDriverContextV3) == 200u,
              "RinDriverContextV3 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverContextV3) == 200u,
               "RinDriverContextV3 ABI drift");
#endif

#define RIN_DRIVER_KERNEL_MODULE_CONTEXT_V1_VERSION UINT32_C(0x00010000)

/* NDRV ABI minor 2 embeds a signed RIN executable in its authenticated RDFW
 * resource catalog. Its RinDriverEntryV3 receives the same device API and
 * context as the outer NDRV entry, with this phase record in reserved[]. For
 * this kernel-module root only, RIN required_capabilities use the
 * RIN_DRIVER_V3_CAP_* namespace and are bounded by the granted NDRV API. */
#define RIN_DRIVER_V3_KERNEL_MODULE_PHASE_MAGIC UINT64_C(0x314D494E524B4452)
#define RIN_DRIVER_V3_KERNEL_MODULE_PHASE_ABI UINT64_C(1)

typedef enum RinDriverKernelModulePhaseV1 {
    RIN_DRIVER_KERNEL_MODULE_PHASE_PROBE = 1,
    RIN_DRIVER_KERNEL_MODULE_PHASE_BIND = 2,
    RIN_DRIVER_KERNEL_MODULE_PHASE_START = 3,
    RIN_DRIVER_KERNEL_MODULE_PHASE_STOP = 4,
    RIN_DRIVER_KERNEL_MODULE_PHASE_UNBIND = 5,
    RIN_DRIVER_KERNEL_MODULE_PHASE_POWER = 6
} RinDriverKernelModulePhaseV1;

/* STOP's phase argument distinguishes a deliberate unload from loss of the
 * device. START_FAILURE is used only while unwinding a partial start. */
typedef enum RinDriverKernelModuleStopReasonV1 {
    RIN_DRIVER_KERNEL_MODULE_STOP_NORMAL = 0,
    RIN_DRIVER_KERNEL_MODULE_STOP_DEVICE_REMOVED = 1,
    RIN_DRIVER_KERNEL_MODULE_STOP_SURPRISE_REMOVAL = 2,
    RIN_DRIVER_KERNEL_MODULE_STOP_START_FAILURE = 3
} RinDriverKernelModuleStopReasonV1;

/* context->reserved[0] = magic, [1] = phase, [2] = phase argument,
 * [3] = phase ABI. reserved[4..7] stay zero. The appended kernel-module
 * context extension carries the matched device identity and match index.
 * POWER's argument is a RIN_DRIVER_POWER_STATE_* value; STOP's argument is
 * a stop-reason enum. */
#define RIN_DRIVER_V3_KERNEL_MODULE_RESOURCE_ROOT "kernel-module-v1/root.rin"
#define RIN_DRIVER_V3_KERNEL_MODULE_RESOURCE_PREFIX "kernel-module-v1/"
#define RIN_DRIVER_V3_KERNEL_MODULE_LIBRARY_PREFIX "kernel-module-v1/lib/"

#define RIN_DRIVER_V3_DIAGNOSTIC_SNAPSHOT_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_DIAGNOSTIC_SNAPSHOT_MAX_BYTES 256u

/* The callback runs synchronously while the registration is retained. It
 * writes only a bounded diagnostic payload; it must not re-enter the session
 * API. The kernel supplies the current device generation so the payload can
 * be tied to the same reset boundary as status and DMA leases. */
typedef RinDriverV3Result (*RinDriverV3DiagnosticSnapshotFn)(
    uint64_t driver_context, uint64_t device_generation, uint8_t* data,
    uint32_t capacity, uint32_t* data_size);

typedef struct RinDriverV3DiagnosticSnapshot {
    uint32_t struct_size;
    uint32_t version;
    uint32_t health_state;
    uint32_t failure_reason;
    uint64_t failure_generation;
    uint64_t device_generation;
    uint32_t data_size;
    uint32_t reserved0;
    uint8_t data[RIN_DRIVER_V3_DIAGNOSTIC_SNAPSHOT_MAX_BYTES];
    uint64_t reserved[2];
} RinDriverV3DiagnosticSnapshot;

typedef struct RinDriverV3DeviceRegistration {
    uint32_t struct_size;
    uint32_t version;
    uint32_t device_class;
    uint32_t flags;
    uint64_t operations;
    uint64_t operations_size;
    uint64_t driver_context;
    uint64_t diagnostic_snapshot;
    uint64_t reserved[3];
} RinDriverV3DeviceRegistration;

typedef struct RinDriverV3Rect {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
} RinDriverV3Rect;

typedef struct RinDriverV3DisplayMode {
    uint32_t struct_size;
    uint32_t version;
    uint32_t width;
    uint32_t height;
    uint32_t stride_bytes;
    uint32_t pixel_format;
    uint32_t refresh_millihz;
    uint32_t flags;
} RinDriverV3DisplayMode;

#define RIN_DRIVER_V3_DISPLAY_EVENT_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_DISPLAY_EVENT_CONNECTED UINT32_C(1)
#define RIN_DRIVER_V3_DISPLAY_EVENT_DISCONNECTED UINT32_C(2)
#define RIN_DRIVER_V3_DISPLAY_EVENT_MODE_CHANGED UINT32_C(3)
#define RIN_DRIVER_V3_DISPLAY_EVENT_FLAG_MASK UINT32_C(0)
#define RIN_DRIVER_V3_DISPLAY_EDID_BLOCK_SIZE UINT32_C(128)
#define RIN_DRIVER_V3_DISPLAY_EDID_MAX_BLOCKS UINT32_C(4)
typedef struct RinDriverV3DisplayEvent {
    uint32_t struct_size;
    uint32_t version;
    uint32_t type;
    uint32_t flags;
    uint64_t device_generation;
    RinDriverV3DisplayMode mode;
    uint64_t reserved[2];
} RinDriverV3DisplayEvent;

#define RIN_DRIVER_V3_DISPLAY_PIXEL_RGBX8888 UINT32_C(1)
#define RIN_DRIVER_V3_DISPLAY_PIXEL_BGRX8888 UINT32_C(2)
#define RIN_DRIVER_V3_DISPLAY_PIXEL_RGB565 UINT32_C(3)
#define RIN_DRIVER_V3_DISPLAY_PIXEL_XRGB2101010 UINT32_C(4)
#define RIN_DRIVER_V3_DISPLAY_PIXEL_BGRA8888 UINT32_C(5)

typedef struct RinDriverV3InputEvent {
    /* Ready monotonic sample; epoch zero is valid. */
    uint64_t timestamp_ns;
    uint16_t type;
    uint16_t code;
    int32_t value;
} RinDriverV3InputEvent;

typedef struct RinDriverV3NetPacket {
    uint64_t memory_lease;
    uint64_t offset;
    uint32_t length;
    uint32_t flags;
} RinDriverV3NetPacket;

/* A batch with this flag describes one Ethernet frame split across the
 * supplied packet records. Drivers retain every lease until the frame's final
 * DMA descriptor completes. Zero flags retain the independent-frame batch. */
#define RIN_DRIVER_V3_NET_PACKET_FLAG_FRAME_FRAGMENT UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NET_MAX_FRAME_FRAGMENTS UINT32_C(4)

/* The extended packet record is carried only by the optional NetOps V2
 * callback.  The original packet and NetOps layouts remain unchanged for
 * drivers that do not consume transmit metadata. */
#define RIN_DRIVER_V3_NET_PACKET_V2_VERSION UINT32_C(1)
#define RIN_DRIVER_V3_NET_TX_CHECKSUM_IPV4_HEADER UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NET_TX_CHECKSUM_TRANSPORT UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NET_TX_CHECKSUM_UDP UINT32_C(0x00000004)
#define RIN_DRIVER_V3_NET_TX_CHECKSUM_KNOWN_FLAGS UINT32_C(0x00000007)
typedef struct RinDriverV3NetPacketV2 {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3NetPacket packet;
    uint32_t checksum_flags;
    uint32_t checksum_start;
    uint32_t checksum_length;
    uint16_t mac_header_length;
    uint16_t network_header_length;
    uint16_t transport_header_length;
    uint16_t checksum_offset;
    uint16_t pseudo_header_seed;
    uint16_t reserved;
} RinDriverV3NetPacketV2;

/* Hardware RX checksum results describe this one copied frame only.  A
 * driver sets a flag only when the device reports a successful checksum and
 * the descriptor has no corresponding checksum-error/ignore status. */
typedef struct RinDriverV3NetRxMetadataV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t checksum_flags;
    uint32_t reserved;
} RinDriverV3NetRxMetadataV1;

/* NDRV v3 network data is intentionally bounded to one ordinary Ethernet
 * frame.  Receive copies are control-plane calls: the NIC-owned DMA ring
 * never leaks into the network stack as an unvalidated pointer. */
#define RIN_DRIVER_V3_NET_MIN_FRAME_SIZE UINT32_C(14)
#define RIN_DRIVER_V3_NET_MAX_FRAME_SIZE UINT32_C(1518)
#define RIN_DRIVER_V3_NET_INFO_VERSION_V1 UINT32_C(0x00010000)
#define RIN_DRIVER_V3_NET_INFO_VERSION_V2 UINT32_C(0x00020000)
#define RIN_DRIVER_V3_NET_INFO_VERSION_V3 UINT32_C(0x00030000)
#define RIN_DRIVER_V3_NET_INFO_VERSION RIN_DRIVER_V3_NET_INFO_VERSION_V2
#define RIN_DRIVER_V3_NET_LINK_UP UINT32_C(0x00000001)
/* LINK_UP is meaningful only when LINK_KNOWN is set.  This prevents a
 * transport without a negotiated carrier-status feature from fabricating an
 * online state for DHCP, routing, or UI consumers. */
#define RIN_DRIVER_V3_NET_LINK_KNOWN UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NET_INFO_FLAG_MASK \
    (RIN_DRIVER_V3_NET_LINK_UP | RIN_DRIVER_V3_NET_LINK_KNOWN)
#define RIN_DRIVER_V3_NET_INTERFACE_ADMIN_KNOWN UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NET_INTERFACE_ADMIN_UP UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NET_INTERFACE_TYPE_KNOWN UINT32_C(0x00000004)
#define RIN_DRIVER_V3_NET_INTERFACE_CAPABILITIES_KNOWN UINT32_C(0x00000008)
#define RIN_DRIVER_V3_NET_INTERFACE_KNOWN_FLAGS UINT32_C(0x0000000f)
#define RIN_DRIVER_V3_NET_INTERFACE_CAP_BROADCAST UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NET_INTERFACE_CAP_MULTICAST UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NET_INTERFACE_CAP_POINT_TO_POINT UINT32_C(0x00000004)
#define RIN_DRIVER_V3_NET_INTERFACE_CAP_TX_SCATTER_GATHER UINT32_C(0x00000008)
#define RIN_DRIVER_V3_NET_INTERFACE_CAP_TX_CHECKSUM_IPV4 UINT32_C(0x00000010)
#define RIN_DRIVER_V3_NET_INTERFACE_CAP_RX_CHECKSUM_IPV4 UINT32_C(0x00000020)
#define RIN_DRIVER_V3_NET_INTERFACE_KNOWN_CAPABILITIES UINT32_C(0x0000003f)
#define RIN_DRIVER_V3_NET_RX_METADATA_VERSION UINT32_C(1)
#define RIN_DRIVER_V3_NET_RX_CHECKSUM_IPV4_HEADER UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NET_RX_CHECKSUM_IPV4_TRANSPORT UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NET_RX_CHECKSUM_FLAG_MASK UINT32_C(0x00000003)
/* set_rx_mode receives this independent bitmask. ALL_MULTICAST admits
 * multicast Ethernet frames while preserving the interface's unicast filter. */
#define RIN_DRIVER_V3_NET_RX_MODE_PROMISCUOUS UINT32_C(0x00000001)
#define RIN_DRIVER_V3_NET_RX_MODE_ALL_MULTICAST UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NET_RX_MODE_FLAG_MASK UINT32_C(0x00000003)
#define RIN_DRIVER_V3_NET_INTERFACE_UNKNOWN UINT32_C(0)
#define RIN_DRIVER_V3_NET_INTERFACE_OTHER UINT32_C(1)
#define RIN_DRIVER_V3_NET_INTERFACE_LOOPBACK UINT32_C(2)
#define RIN_DRIVER_V3_NET_INTERFACE_BROADCAST UINT32_C(3)
#define RIN_DRIVER_V3_NET_INTERFACE_POINT_TO_POINT UINT32_C(4)
#define RIN_DRIVER_V3_NET_INTERFACE_TYPE_MAX \
    RIN_DRIVER_V3_NET_INTERFACE_POINT_TO_POINT

typedef struct RinDriverV3NetInfo {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint16_t mtu;
    uint8_t mac[6];
    /* Version 3 reuses the V1/V2 reserved field without changing the ABI
     * layout. For V1/V2 it must remain zero. For V3, zero means unknown. */
    union {
        uint16_t reserved0;
        uint16_t link_speed_mbps;
    };
    /* Introduced by INFO_VERSION_V2. Version 1 drivers leave this extension
     * zero; they remain valid but publish no L3/admin properties. */
    uint32_t interface_flags;
    uint32_t interface_type;
    uint32_t interface_capabilities;
    uint8_t ipv4_peer[4];
} RinDriverV3NetInfo;

#define RIN_DRIVER_V3_AUDIO_PLAYBACK UINT32_C(0x00000001)
#define RIN_DRIVER_V3_AUDIO_CAPTURE  UINT32_C(0x00000002)
#define RIN_DRIVER_V3_AUDIO_DIRECTION_MASK                         \
    (RIN_DRIVER_V3_AUDIO_PLAYBACK | RIN_DRIVER_V3_AUDIO_CAPTURE)

#define RIN_DRIVER_V3_AUDIO_SIGNED_PCM UINT32_C(1)

#define RIN_DRIVER_V3_AUDIO_FORMAT_INTERLEAVED UINT32_C(0x00000001)
#define RIN_DRIVER_V3_AUDIO_FORMAT_FLAG_MASK \
    RIN_DRIVER_V3_AUDIO_FORMAT_INTERLEAVED

#define RIN_DRIVER_V3_AUDIO_BUFFER_END_OF_STREAM UINT32_C(0x00000001)
/* A zero direction flag retains the direction selected by configure().  The
 * explicit flags let one audio owner submit playback and capture buffers at
 * the same time without changing the selected direction between calls. */
#define RIN_DRIVER_V3_AUDIO_BUFFER_PLAYBACK UINT32_C(0x00000002)
#define RIN_DRIVER_V3_AUDIO_BUFFER_CAPTURE UINT32_C(0x00000004)
#define RIN_DRIVER_V3_AUDIO_BUFFER_FLAG_MASK \
    (RIN_DRIVER_V3_AUDIO_BUFFER_END_OF_STREAM | \
     RIN_DRIVER_V3_AUDIO_BUFFER_PLAYBACK | \
     RIN_DRIVER_V3_AUDIO_BUFFER_CAPTURE)

#define RIN_DRIVER_V3_AUDIO_CONTROL_VOLUME_PERCENT UINT32_C(1)
#define RIN_DRIVER_V3_AUDIO_CONTROL_MUTE           UINT32_C(2)
#define RIN_DRIVER_V3_AUDIO_CONTROL_CAPTURE_VOLUME_PERCENT UINT32_C(3)
#define RIN_DRIVER_V3_AUDIO_CONTROL_CAPTURE_MUTE           UINT32_C(4)
#define RIN_DRIVER_V3_AUDIO_CONTROL_MASK \
    (RIN_DRIVER_V3_AUDIO_CONTROL_VOLUME_PERCENT | \
     RIN_DRIVER_V3_AUDIO_CONTROL_MUTE | \
     RIN_DRIVER_V3_AUDIO_CONTROL_CAPTURE_VOLUME_PERCENT | \
     RIN_DRIVER_V3_AUDIO_CONTROL_CAPTURE_MUTE)

#define RIN_DRIVER_V3_AUDIO_COMPLETION_XRUN UINT32_C(0x00000001)
#define RIN_DRIVER_V3_AUDIO_COMPLETION_FLAG_MASK \
    RIN_DRIVER_V3_AUDIO_COMPLETION_XRUN

typedef struct RinDriverV3AudioFormat {
    uint32_t struct_size;
    uint32_t version;
    uint32_t sample_rate_hz;
    uint32_t direction;
    uint16_t channel_count;
    uint16_t bits_per_sample;
    uint32_t sample_format;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t channel_mask;
    uint64_t reserved[2];
} RinDriverV3AudioFormat;

/* Audio data is never passed as a pointer. memory_lease names pinned memory
 * owned by the audio service; offset and frame_count select the submitted
 * interleaved frame range. */
typedef struct RinDriverV3AudioBuffer {
    uint32_t struct_size;
    uint32_t version;
    uint64_t buffer_id;
    uint64_t memory_lease;
    uint64_t offset;
    uint32_t frame_count;
    uint32_t flags;
    uint64_t deadline_ns;
    uint64_t reserved[2];
} RinDriverV3AudioBuffer;

typedef struct RinDriverV3AudioCompletion {
    uint32_t struct_size;
    uint32_t version;
    uint64_t buffer_id;
    int32_t result;
    uint32_t completed_frames;
    uint32_t flags;
    uint32_t device_status;
    uint64_t reserved[2];
} RinDriverV3AudioCompletion;

#define RIN_DRIVER_V3_USB_PORT_CONNECTED   UINT32_C(0x00000001)
#define RIN_DRIVER_V3_USB_PORT_ENABLED     UINT32_C(0x00000002)
#define RIN_DRIVER_V3_USB_PORT_POWERED     UINT32_C(0x00000004)
#define RIN_DRIVER_V3_USB_PORT_OVERCURRENT UINT32_C(0x00000008)
#define RIN_DRIVER_V3_USB_PORT_FLAG_MASK                         \
    (RIN_DRIVER_V3_USB_PORT_CONNECTED | RIN_DRIVER_V3_USB_PORT_ENABLED | \
     RIN_DRIVER_V3_USB_PORT_POWERED | RIN_DRIVER_V3_USB_PORT_OVERCURRENT)

#define RIN_DRIVER_V3_USB_PORT_CHANGE_CONNECTION UINT32_C(0x00000001)
#define RIN_DRIVER_V3_USB_PORT_CHANGE_ENABLE     UINT32_C(0x00000002)
#define RIN_DRIVER_V3_USB_PORT_CHANGE_OVERCURRENT UINT32_C(0x00000004)
#define RIN_DRIVER_V3_USB_PORT_CHANGE_LINK       UINT32_C(0x00000008)
#define RIN_DRIVER_V3_USB_PORT_CHANGE_FLAG_MASK                       \
    (RIN_DRIVER_V3_USB_PORT_CHANGE_CONNECTION |                       \
     RIN_DRIVER_V3_USB_PORT_CHANGE_ENABLE |                            \
     RIN_DRIVER_V3_USB_PORT_CHANGE_OVERCURRENT |                       \
     RIN_DRIVER_V3_USB_PORT_CHANGE_LINK)

#define RIN_DRIVER_V3_USB_SPEED_LOW       UINT32_C(1)
#define RIN_DRIVER_V3_USB_SPEED_FULL      UINT32_C(2)
#define RIN_DRIVER_V3_USB_SPEED_HIGH      UINT32_C(3)
#define RIN_DRIVER_V3_USB_SPEED_SUPER     UINT32_C(4)
#define RIN_DRIVER_V3_USB_SPEED_SUPER_PLUS UINT32_C(5)
#define RIN_DRIVER_V3_USB_MAX_ISOCH_ESIT_PAYLOAD UINT32_C(0x00ef0000)
#define RIN_DRIVER_V3_USB_MAX_SS_ISOCH_ESIT_PAYLOAD UINT32_C(49152)
#define RIN_DRIVER_V3_USB_ISOCH_TRB_MAX_BURSTS UINT32_C(4)
#define RIN_DRIVER_V3_USB_ENDPOINT_FLAG_SSP_ISOCHRONOUS UINT64_C(1)

typedef struct RinDriverV3UsbPortStatus {
    uint32_t struct_size;
    uint32_t version;
    uint32_t port_index;
    uint32_t flags;
    uint32_t speed;
    uint32_t generation;
    uint32_t change_flags;
    uint32_t reserved0;
    uint64_t reserved[1];
} RinDriverV3UsbPortStatus;

#define RIN_DRIVER_V3_USB_DEVICE_ADDRESS   UINT32_C(1)
#define RIN_DRIVER_V3_USB_DEVICE_UPDATE_EP0 UINT32_C(2)
#define RIN_DRIVER_V3_USB_DEVICE_CONFIGURE UINT32_C(3)
#define RIN_DRIVER_V3_USB_DEVICE_RELEASE   UINT32_C(4)
#define RIN_DRIVER_V3_USB_DEVICE_DECLARE_HUB UINT32_C(5)
#define RIN_DRIVER_V3_USB_DEVICE_ENDPOINTS_MAX UINT32_C(30)

#define RIN_DRIVER_V3_USB_DEVICE_HUB_MULTI_TT UINT32_C(0x00000001)
#define RIN_DRIVER_V3_USB_DEVICE_HUB_FLAG_MASK \
    RIN_DRIVER_V3_USB_DEVICE_HUB_MULTI_TT

/* Device lifecycle calls are generation-bound and synchronous. Endpoint
 * descriptors are copied by the driver before device_control returns.
 *
 * ADDRESS uses topology.route_string == 0 for a root-port device.  A
 * descendant supplies a canonical xHCI route string, its USB speed, and its
 * immediate parent hub address/port.  DECLARE_HUB uses hub and must precede
 * the first CONFIGURE so the xHC can initialize the hub Slot Context during
 * the Addressed -> Configured transition. */
typedef struct RinDriverV3UsbDeviceRequest {
    uint32_t struct_size;
    uint32_t version;
    uint64_t deadline_ns;
    uint32_t operation;
    uint32_t port_index;
    uint32_t port_generation;
    uint32_t device_address;
    uint32_t configuration_value;
    uint32_t ep0_max_packet_size;
    uint32_t endpoint_count;
    uint32_t flags;
    union {
        struct {
            uint32_t route_string;
            uint32_t device_speed;
            uint32_t parent_device_address;
            uint32_t parent_port;
        } topology;
        struct {
            uint32_t port_count;
            uint32_t tt_think_time;
            uint32_t reserved0;
            uint32_t reserved1;
        } hub;
        uint64_t reserved[2];
    };
} RinDriverV3UsbDeviceRequest;

typedef struct RinDriverV3UsbEndpointConfiguration {
    uint32_t struct_size;
    uint32_t version;
    uint32_t endpoint_address;
    uint32_t transfer_type;
    uint32_t max_packet_size;
    uint32_t interval;
    uint32_t max_burst;
    /* Isochronous endpoints carry 1..3 USB 2.0 transactions or USB 3.x
     * Mult bursts per service interval. Zero is the legacy encoding for one. */
    uint32_t transactions_per_interval;
    /* For USB 3.x isochronous endpoints, reserved[0] carries the exact
     * Max ESIT Payload. For bulk endpoints it carries xHCI MaxPStreams
     * (currently 1), or zero when Streams are disabled. reserved[1] is the SSP
     * isochronous flag and is zero for all other endpoint configurations. */
    uint64_t reserved[2];
} RinDriverV3UsbEndpointConfiguration;

typedef struct RinDriverV3UsbDeviceResponse {
    uint32_t struct_size;
    uint32_t version;
    uint32_t port_index;
    uint32_t port_generation;
    uint32_t device_address;
    uint32_t configuration_value;
    uint32_t endpoint_count;
    uint32_t flags;
    uint64_t reserved[2];
} RinDriverV3UsbDeviceResponse;

#define RIN_DRIVER_V3_USB_TRANSFER_CONTROL   UINT32_C(1)
#define RIN_DRIVER_V3_USB_TRANSFER_INTERRUPT UINT32_C(2)
#define RIN_DRIVER_V3_USB_TRANSFER_BULK      UINT32_C(3)
#define RIN_DRIVER_V3_USB_TRANSFER_ISOCHRONOUS UINT32_C(4)

#define RIN_DRIVER_V3_USB_TRANSFER_SHORT_OK UINT32_C(0x00000001)
#define RIN_DRIVER_V3_USB_TRANSFER_FLAG_MASK \
    RIN_DRIVER_V3_USB_TRANSFER_SHORT_OK
#define RIN_DRIVER_V3_USB_STREAM_ID_NONE UINT32_C(0)
#define RIN_DRIVER_V3_USB_STREAM_ID_MAX UINT32_C(0xfffd)
#define RIN_DRIVER_V3_USB_MAX_PSTREAMS UINT32_C(3)

/* A USB transfer refers only to a kernel-owned memory lease. The driver must
 * copy this descriptor before returning from submit_transfer and must map the
 * selected range through dma_map_sg. setup_packet contains the eight USB
 * setup bytes in little-endian order and is zero for bulk and interrupt
 * transfers. stream_id is zero for ordinary endpoints and nonzero only for
 * a bulk endpoint configured with Streams. */
typedef struct RinDriverV3UsbTransfer {
    uint32_t struct_size;
    uint32_t version;
    uint64_t transfer_id;
    uint64_t memory_lease;
    uint64_t offset;
    uint64_t deadline_ns;
    uint64_t setup_packet;
    uint32_t port_index;
    uint32_t port_generation;
    uint32_t device_address;
    uint32_t endpoint_address;
    uint32_t transfer_type;
    uint32_t flags;
    uint32_t length;
    uint32_t stream_id;
    uint64_t reserved[2];
} RinDriverV3UsbTransfer;

typedef struct RinDriverV3UsbTransferCompletion {
    uint32_t struct_size;
    uint32_t version;
    uint64_t transfer_id;
    int32_t result;
    uint32_t transferred_length;
    uint32_t port_index;
    uint32_t port_generation;
    uint32_t device_status;
    uint32_t flags;
    uint64_t reserved[2];
} RinDriverV3UsbTransferCompletion;

#define RIN_DRIVER_V3_BLOCK_READ    UINT32_C(1)
#define RIN_DRIVER_V3_BLOCK_WRITE   UINT32_C(2)
#define RIN_DRIVER_V3_BLOCK_FLUSH   UINT32_C(3)
#define RIN_DRIVER_V3_BLOCK_DISCARD UINT32_C(4)

#define RIN_DRIVER_V3_BLOCK_REQUEST_FUA     UINT32_C(0x00000001)
#define RIN_DRIVER_V3_BLOCK_REQUEST_BARRIER UINT32_C(0x00000002)
#define RIN_DRIVER_V3_BLOCK_REQUEST_FLAG_MASK                         \
    (RIN_DRIVER_V3_BLOCK_REQUEST_FUA | RIN_DRIVER_V3_BLOCK_REQUEST_BARRIER)

#define RIN_DRIVER_V3_BLOCK_FLAG_READ_ONLY     UINT32_C(0x00000001)
#define RIN_DRIVER_V3_BLOCK_FLAG_REMOVABLE     UINT32_C(0x00000002)
#define RIN_DRIVER_V3_BLOCK_FLAG_FLUSH         UINT32_C(0x00000004)
#define RIN_DRIVER_V3_BLOCK_FLAG_FUA           UINT32_C(0x00000008)
#define RIN_DRIVER_V3_BLOCK_FLAG_DISCARD       UINT32_C(0x00000010)
#define RIN_DRIVER_V3_BLOCK_FLAG_RESET         UINT32_C(0x00000020)
#define RIN_DRIVER_V3_BLOCK_FLAG_CONTROLLED_IO UINT32_C(0x00000040)
#define RIN_DRIVER_V3_BLOCK_FLAG_MASK                              \
    (RIN_DRIVER_V3_BLOCK_FLAG_READ_ONLY |                            \
     RIN_DRIVER_V3_BLOCK_FLAG_REMOVABLE |                            \
     RIN_DRIVER_V3_BLOCK_FLAG_FLUSH | RIN_DRIVER_V3_BLOCK_FLAG_FUA | \
     RIN_DRIVER_V3_BLOCK_FLAG_DISCARD |                              \
     RIN_DRIVER_V3_BLOCK_FLAG_RESET |                                \
     RIN_DRIVER_V3_BLOCK_FLAG_CONTROLLED_IO)

typedef struct RinDriverV3BlockGeometry {
    uint32_t struct_size;
    uint32_t version;
    uint64_t block_count;
    uint32_t logical_block_size;
    uint32_t physical_block_size;
    uint32_t optimal_transfer_blocks;
    uint32_t maximum_transfer_blocks;
    uint32_t flags;
    uint32_t reserved;
} RinDriverV3BlockGeometry;

/* The request contains no data pointer. Each slice names kernel-owned memory
 * which the driver may map only through dma_map_sg for this device session. */
typedef struct RinDriverV3BlockRequest {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t flags;
    uint64_t request_id;
    uint64_t lba;
    uint64_t deadline_ns;
    uint32_t block_count;
    uint32_t slice_count;
    uint64_t reserved[2];
} RinDriverV3BlockRequest;

typedef struct RinDriverV3BlockCompletion {
    uint32_t struct_size;
    uint32_t version;
    uint64_t request_id;
    int32_t result;
    uint32_t transferred_blocks;
    uint32_t flags;
    uint32_t device_status;
    uint64_t reserved[2];
} RinDriverV3BlockCompletion;

typedef RinDriverV3Result (*RinDriverV3DisplayGetModeFn)(
    uint64_t driver_context, RinDriverV3DisplayMode* output);
typedef RinDriverV3Result (*RinDriverV3DisplayDamageFn)(
    uint64_t driver_context, const RinDriverV3Rect* rectangles,
    uint32_t rectangle_count);
typedef RinDriverV3Result (*RinDriverV3DisplayCursorFn)(
    uint64_t driver_context, int32_t x, int32_t y, uint32_t visible);
typedef RinDriverV3Result (*RinDriverV3DisplaySetModeFn)(
    uint64_t driver_context, const RinDriverV3DisplayMode* mode);
typedef RinDriverV3Result (*RinDriverV3DisplayPollEventsFn)(
    uint64_t driver_context, RinDriverV3DisplayEvent* events,
    uint32_t capacity, uint32_t* event_count);
/* Reads one complete, validated 128-byte EDID block. UNSUPPORTED means the
 * display link has no EDID source. The driver retains no caller buffer. */
typedef RinDriverV3Result (*RinDriverV3DisplayGetEdidBlockFn)(
    uint64_t driver_context, uint32_t block_index,
    uint8_t output[RIN_DRIVER_V3_DISPLAY_EDID_BLOCK_SIZE]);
/* Synchronously copies a kernel-owned compositor frame into device backing
 * and presents the supplied damage. The driver must not retain pixels or
 * rectangles after returning. */
typedef RinDriverV3Result (*RinDriverV3DisplayPresentPixelsFn)(
    uint64_t driver_context, const uint8_t* pixels,
    uint32_t width, uint32_t height, uint32_t stride_bytes,
    uint32_t pixel_format, const RinDriverV3Rect* rectangles,
    uint32_t rectangle_count);
typedef RinDriverV3Result (*RinDriverV3InputReadFn)(
    uint64_t driver_context, RinDriverV3InputEvent* events,
    uint32_t capacity, uint32_t* event_count);
typedef RinDriverV3Result (*RinDriverV3NetTransmitFn)(
    uint64_t driver_context, const RinDriverV3NetPacket* packets,
    uint32_t packet_count);
typedef RinDriverV3Result (*RinDriverV3NetTransmitV2Fn)(
    uint64_t driver_context, const RinDriverV3NetPacketV2* packets,
    uint32_t packet_count);
typedef RinDriverV3Result (*RinDriverV3NetPollFn)(
    uint64_t driver_context, uint32_t budget, uint32_t* completed);
/* `buffer` is caller-owned, transient control memory.  The driver copies a
 * fully validated received frame before it returns and immediately re-arms
 * the DMA descriptor. `received` is zero when no frame is ready. */
typedef RinDriverV3Result (*RinDriverV3NetReceiveFn)(
    uint64_t driver_context, void* buffer, uint32_t capacity,
    uint32_t* received);
typedef RinDriverV3Result (*RinDriverV3NetReceiveV2Fn)(
    uint64_t driver_context, void* buffer, uint32_t capacity,
    uint32_t* received, RinDriverV3NetRxMetadataV1* metadata);
typedef RinDriverV3Result (*RinDriverV3NetGetInfoFn)(
    uint64_t driver_context, RinDriverV3NetInfo* output);
/* Set only the explicitly supported receive flags. A driver must return an
 * error without changing its filter when the requested mode is unavailable. */
typedef RinDriverV3Result (*RinDriverV3NetSetRxModeFn)(
    uint64_t driver_context, uint32_t flags);
/* Switch receive notification between device interrupts and network-owned
 * polling. A null callback retains the poll-only contract. */
typedef RinDriverV3Result (*RinDriverV3NetSetRxInterruptsFn)(
    uint64_t driver_context, uint32_t enabled);
/* Recover a driver after a hardware failure. Success means DMA is quiesced,
 * outstanding mappings are retired, and normal transmit/receive operation has
 * restarted. The kernel invokes this after a device operation fails; malformed
 * callbacks and kernel-owned DMA lease failures remain quarantined. */
typedef RinDriverV3Result (*RinDriverV3NetRecoverFn)(
    uint64_t driver_context);
typedef RinDriverV3Result (*RinDriverV3AudioGetFormatFn)(
    uint64_t driver_context, RinDriverV3AudioFormat* output);
typedef RinDriverV3Result (*RinDriverV3AudioConfigureFn)(
    uint64_t driver_context, const RinDriverV3AudioFormat* format);
typedef RinDriverV3Result (*RinDriverV3AudioSetControlFn)(
    uint64_t driver_context, uint32_t control, uint32_t value);
/* submit must copy the buffer descriptor before returning. The driver may
 * retain only the named memory lease until it reports a terminal completion. */
typedef RinDriverV3Result (*RinDriverV3AudioSubmitFn)(
    uint64_t driver_context, const RinDriverV3AudioBuffer* buffer);
typedef RinDriverV3Result (*RinDriverV3AudioPollFn)(
    uint64_t driver_context, RinDriverV3AudioCompletion* completions,
    uint32_t capacity, uint32_t* completion_count);
/* stop may return OK only after the device can no longer DMA through leases
 * belonging to submitted buffers. */
typedef RinDriverV3Result (*RinDriverV3AudioStopFn)(
    uint64_t driver_context, uint64_t deadline_ns);
typedef RinDriverV3Result (*RinDriverV3UsbGetPortCountFn)(
    uint64_t driver_context, uint32_t* port_count);
typedef RinDriverV3Result (*RinDriverV3UsbGetPortStatusFn)(
    uint64_t driver_context, uint32_t port_index,
    RinDriverV3UsbPortStatus* output);
typedef RinDriverV3Result (*RinDriverV3UsbResetPortFn)(
    uint64_t driver_context, uint32_t port_index, uint64_t deadline_ns);
typedef RinDriverV3Result (*RinDriverV3UsbPollPortChangesFn)(
    uint64_t driver_context, RinDriverV3UsbPortStatus* changes,
    uint32_t capacity, uint32_t* change_count);
typedef RinDriverV3Result (*RinDriverV3UsbResetControllerFn)(
    uint64_t driver_context, uint64_t deadline_ns);
typedef RinDriverV3Result (*RinDriverV3UsbDeviceControlFn)(
    uint64_t driver_context, const RinDriverV3UsbDeviceRequest* request,
    const RinDriverV3UsbEndpointConfiguration* endpoints,
    RinDriverV3UsbDeviceResponse* response);
typedef RinDriverV3Result (*RinDriverV3UsbSubmitTransferFn)(
    uint64_t driver_context, const RinDriverV3UsbTransfer* transfer);
typedef RinDriverV3Result (*RinDriverV3UsbPollTransfersFn)(
    uint64_t driver_context, RinDriverV3UsbTransferCompletion* completions,
    uint32_t capacity, uint32_t* completion_count);
/* OK is terminal: after cancel_transfer returns OK, the controller can no
 * longer DMA through the transfer's memory lease. */
typedef RinDriverV3Result (*RinDriverV3UsbCancelTransferFn)(
    uint64_t driver_context, uint64_t transfer_id, uint64_t deadline_ns);
typedef RinDriverV3Result (*RinDriverV3BlockGetGeometryFn)(
    uint64_t driver_context, RinDriverV3BlockGeometry* output);
/* submit must copy the request and slice descriptors before returning. The
 * control pointers are never retained; only the named memory leases may stay
 * mapped until completion. */
typedef RinDriverV3Result (*RinDriverV3BlockSubmitFn)(
    uint64_t driver_context, const RinDriverV3BlockRequest* request,
    const RinDriverV3MemorySlice* slices);
typedef RinDriverV3Result (*RinDriverV3BlockPollFn)(
    uint64_t driver_context, RinDriverV3BlockCompletion* completions,
    uint32_t capacity, uint32_t* completion_count);
/* A reported completion is terminal: before publishing it, the driver must
 * ensure that the device can no longer access any request memory lease. */
typedef RinDriverV3Result (*RinDriverV3BlockCancelFn)(
    uint64_t driver_context, uint64_t request_id);
/* cancel/reset may return OK only after the device can no longer DMA through
 * leases belonging to the cancelled request(s). */
typedef RinDriverV3Result (*RinDriverV3BlockResetFn)(
    uint64_t driver_context, uint64_t deadline_ns);
/* Returns a bounded RinStorageHealthSnapshotV1 byte payload for this block
 * device generation. Implementations must leave unsupported values absent;
 * this read-only diagnostic callback does not grant block I/O permissions. */
typedef RinDriverV3Result (*RinDriverV3BlockGetStorageHealthSnapshotFn)(
    uint64_t driver_context, uint64_t device_generation, uint8_t* data,
    uint32_t capacity, uint32_t* data_size);
/* NVMe namespace metadata is a registration discriminator and reported
 * correlation data. It is never physical-media attestation. */
#define RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_VERSION UINT32_C(1)
#define RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_MAX_ALIASES UINT32_C(2)
#define RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_BYTES UINT32_C(16)
typedef enum RinDriverV3BlockNamespaceAliasKind {
    RIN_DRIVER_V3_BLOCK_NAMESPACE_ALIAS_NGUID = 1,
    RIN_DRIVER_V3_BLOCK_NAMESPACE_ALIAS_EUI64 = 2
} RinDriverV3BlockNamespaceAliasKind;
typedef struct RinDriverV3BlockNamespaceAliasV1 {
    uint32_t kind;
    uint32_t size;
    uint8_t identifier[RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_BYTES];
} RinDriverV3BlockNamespaceAliasV1;
typedef struct RinDriverV3BlockNamespaceIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t namespace_id;
    uint32_t alias_count;
    RinDriverV3BlockNamespaceAliasV1
        aliases[RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_MAX_ALIASES];
} RinDriverV3BlockNamespaceIdentityV1;
typedef RinDriverV3Result (*RinDriverV3BlockGetNamespaceIdentityFn)(
    uint64_t driver_context,
    RinDriverV3BlockNamespaceIdentityV1* output);
/* Returns an OS-visible identity for removable media. This value is only
 * alias-correlation data; the kernel-owned live controller path supplies the
 * physical ancestry evidence used by destructive-media separation policy. */
#define RIN_DRIVER_V3_BLOCK_MEDIA_IDENTITY_VERSION UINT32_C(1)
#define RIN_DRIVER_V3_BLOCK_MEDIA_IDENTITY_BYTES UINT32_C(16)
enum {
    RIN_DRIVER_V3_BLOCK_MEDIA_IDENTITY_SD_CID = 1
};
typedef struct RinDriverV3BlockMediaIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t kind;
    uint32_t size;
    uint8_t identifier[RIN_DRIVER_V3_BLOCK_MEDIA_IDENTITY_BYTES];
} RinDriverV3BlockMediaIdentityV1;
typedef RinDriverV3Result (*RinDriverV3BlockGetMediaIdentityFn)(
    uint64_t driver_context, RinDriverV3BlockMediaIdentityV1* output);
typedef RinDriverV3Result (*RinDriverV3DeviceRemoveFn)(uint64_t driver_context);

typedef struct RinDriverV3BlockOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3BlockGetGeometryFn get_geometry;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_geometry_padding;
#endif
    RinDriverV3BlockSubmitFn submit;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t submit_padding;
#endif
    RinDriverV3BlockPollFn poll;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_padding;
#endif
    RinDriverV3BlockCancelFn cancel;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t cancel_padding;
#endif
    RinDriverV3BlockResetFn reset;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t reset_padding;
#endif
    RinDriverV3DeviceRemoveFn remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t remove_padding;
#endif
    /* Reuse two reserved pointer-sized slots without changing the ABI size.
     * Older drivers leave both callbacks null and remain valid. */
    union {
        uint64_t reserved[4];
        struct {
            RinDriverV3BlockGetStorageHealthSnapshotFn
                get_storage_health_snapshot;
#if UINTPTR_MAX == UINT32_MAX
            uint32_t get_storage_health_snapshot_padding;
#endif
            RinDriverV3BlockGetNamespaceIdentityFn get_namespace_identity;
#if UINTPTR_MAX == UINT32_MAX
            uint32_t get_namespace_identity_padding;
#endif
            RinDriverV3BlockGetMediaIdentityFn get_media_identity;
#if UINTPTR_MAX == UINT32_MAX
            uint32_t get_media_identity_padding;
#endif
            uint64_t reserved_tail[1];
        } extensions;
    };
} RinDriverV3BlockOps;

typedef struct RinDriverV3DisplayOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3DisplayGetModeFn get_mode;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_mode_padding;
#endif
    RinDriverV3DisplayDamageFn damage;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t damage_padding;
#endif
    RinDriverV3DisplayCursorFn cursor;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t cursor_padding;
#endif
    RinDriverV3DeviceRemoveFn remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t remove_padding;
#endif
    RinDriverV3DisplaySetModeFn set_mode;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t set_mode_padding;
#endif
    RinDriverV3DisplayPollEventsFn poll_events;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_events_padding;
#endif
    RinDriverV3DisplayGetEdidBlockFn get_edid_block;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_edid_block_padding;
#endif
    union {
        RinDriverV3DisplayPresentPixelsFn present_pixels;
        uint64_t reserved[1];
    } extensions;
} RinDriverV3DisplayOps;

static inline int rin_driver_v3_display_mode_valid(
    const RinDriverV3DisplayMode* mode) {
    uint32_t bytes_per_pixel;
    uint64_t minimum_stride;
    if (!mode || mode->struct_size < sizeof(*mode) ||
        mode->version != RIN_DRIVER_V3_OPS_VERSION || mode->width == 0u ||
        mode->height == 0u || mode->width > 16384u ||
        mode->height > 16384u || mode->refresh_millihz == 0u ||
        mode->refresh_millihz > 1000000u || mode->flags != 0u)
        return 0;
    switch (mode->pixel_format) {
    case RIN_DRIVER_V3_DISPLAY_PIXEL_RGBX8888:
    case RIN_DRIVER_V3_DISPLAY_PIXEL_BGRX8888:
    case RIN_DRIVER_V3_DISPLAY_PIXEL_XRGB2101010:
        bytes_per_pixel = 4u;
        break;
    case RIN_DRIVER_V3_DISPLAY_PIXEL_RGB565:
        bytes_per_pixel = 2u;
        break;
    default:
        return 0;
    }
    minimum_stride = (uint64_t)mode->width * bytes_per_pixel;
    return mode->stride_bytes >= minimum_stride &&
           (uint64_t)mode->stride_bytes * mode->height <=
               UINT64_C(1073741824);
}

static inline int rin_driver_v3_display_event_valid(
    const RinDriverV3DisplayEvent* event) {
    if (!event || event->struct_size < sizeof(*event) ||
        event->version != RIN_DRIVER_V3_DISPLAY_EVENT_VERSION ||
        event->type < RIN_DRIVER_V3_DISPLAY_EVENT_CONNECTED ||
        event->type > RIN_DRIVER_V3_DISPLAY_EVENT_MODE_CHANGED ||
        (event->flags & ~RIN_DRIVER_V3_DISPLAY_EVENT_FLAG_MASK) != 0u ||
        event->device_generation == 0u || event->reserved[0] != 0u ||
        event->reserved[1] != 0u)
        return 0;
    if (event->type == RIN_DRIVER_V3_DISPLAY_EVENT_DISCONNECTED)
        return event->mode.struct_size == 0u && event->mode.version == 0u &&
               event->mode.width == 0u && event->mode.height == 0u &&
               event->mode.stride_bytes == 0u &&
               event->mode.pixel_format == 0u &&
               event->mode.refresh_millihz == 0u &&
               event->mode.flags == 0u;
    return rin_driver_v3_display_mode_valid(&event->mode);
}

static inline int rin_driver_v3_display_ops_valid(
    const RinDriverV3DisplayOps* ops) {
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_OPS_VERSION || !ops->get_mode ||
        !ops->damage || !ops->remove)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->get_mode_padding != 0u || ops->damage_padding != 0u ||
        ops->cursor_padding != 0u || ops->remove_padding != 0u ||
        ops->set_mode_padding != 0u || ops->poll_events_padding != 0u ||
        ops->get_edid_block_padding != 0u)
        return 0;
#endif
    return 1;
}

typedef struct RinDriverV3InputOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3InputReadFn read;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t read_padding;
#endif
    RinDriverV3DeviceRemoveFn remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t remove_padding;
#endif
    uint64_t reserved[4];
} RinDriverV3InputOps;

typedef struct RinDriverV3NetOps {
    uint32_t struct_size;
    uint32_t version;
    /* v3 reserves an eight-byte ABI slot for every callback on both x86
     * targets.  Keep the native function-pointer API, but explicitly retain
     * the high half of each slot when a consumer is 32-bit. */
    RinDriverV3NetTransmitFn transmit;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t transmit_padding;
#endif
    RinDriverV3NetPollFn poll_rx;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_rx_padding;
#endif
    RinDriverV3DeviceRemoveFn remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t remove_padding;
#endif
    /* These consume two formerly reserved slots, preserving the v3 wire
     * size. Images predating receive/info remain valid but cannot be
     * published as a product network interface. */
    RinDriverV3NetReceiveFn receive;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t receive_padding;
#endif
    RinDriverV3NetGetInfoFn get_info;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_info_padding;
#endif
    /* Optional device receive-mode and interrupt-mode controls consume two
     * reserved callback slots. Older images leave them null. */
    RinDriverV3NetSetRxModeFn set_rx_mode;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t set_rx_mode_padding;
#endif
    RinDriverV3NetSetRxInterruptsFn set_rx_interrupts;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t set_rx_interrupts_padding;
#endif
} RinDriverV3NetOps;

/* Optional V2 network transmit extension.  The embedded V1 table preserves
 * its 64-byte wire layout; registration.operations_size distinguishes this
 * extension from an older image before the kernel reads it. */
#define RIN_DRIVER_V3_NET_OPS_EXTENSION_VERSION UINT32_C(1)
typedef struct RinDriverV3NetOpsV2 {
    RinDriverV3NetOps base;
    uint32_t extension_size;
    uint32_t extension_version;
    RinDriverV3NetTransmitV2Fn transmit_v2;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t transmit_v2_padding;
#endif
} RinDriverV3NetOpsV2;

/* Optional V3 receive metadata extension.  It preserves both earlier table
 * layouts and is accepted only when registration.operations_size matches. */
#define RIN_DRIVER_V3_NET_OPS_V3_EXTENSION_VERSION UINT32_C(1)
typedef struct RinDriverV3NetOpsV3 {
    RinDriverV3NetOpsV2 base;
    uint32_t extension_size;
    uint32_t extension_version;
    RinDriverV3NetReceiveV2Fn receive_v2;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t receive_v2_padding;
#endif
} RinDriverV3NetOpsV3;

/* Optional V4 hardware recovery extension. Existing V1-V3 operation tables
 * remain valid and keep their original wire sizes. */
#define RIN_DRIVER_V3_NET_OPS_V4_EXTENSION_VERSION UINT32_C(1)
typedef struct RinDriverV3NetOpsV4 {
    RinDriverV3NetOpsV3 base;
    uint32_t extension_size;
    uint32_t extension_version;
    RinDriverV3NetRecoverFn recover;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t recover_padding;
#endif
} RinDriverV3NetOpsV4;

typedef struct RinDriverV3AudioOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3AudioGetFormatFn get_format;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_format_padding;
#endif
    RinDriverV3AudioConfigureFn configure;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t configure_padding;
#endif
    RinDriverV3AudioSubmitFn submit;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t submit_padding;
#endif
    RinDriverV3AudioPollFn poll;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_padding;
#endif
    RinDriverV3AudioStopFn stop;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t stop_padding;
#endif
    RinDriverV3DeviceRemoveFn remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t remove_padding;
#endif
    RinDriverV3AudioSetControlFn set_control;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t set_control_padding;
#endif
    uint64_t reserved[3];
} RinDriverV3AudioOps;

typedef struct RinDriverV3UsbControllerOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3UsbGetPortCountFn get_port_count;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_port_count_padding;
#endif
    RinDriverV3UsbGetPortStatusFn get_port_status;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_port_status_padding;
#endif
    RinDriverV3UsbResetPortFn reset_port;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t reset_port_padding;
#endif
    RinDriverV3UsbPollPortChangesFn poll_port_changes;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_port_changes_padding;
#endif
    RinDriverV3UsbResetControllerFn reset_controller;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t reset_controller_padding;
#endif
    RinDriverV3DeviceRemoveFn remove;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t remove_padding;
#endif
    RinDriverV3UsbSubmitTransferFn submit_transfer;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t submit_transfer_padding;
#endif
    RinDriverV3UsbPollTransfersFn poll_transfers;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t poll_transfers_padding;
#endif
    RinDriverV3UsbCancelTransferFn cancel_transfer;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t cancel_transfer_padding;
#endif
    RinDriverV3UsbDeviceControlFn device_control;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t device_control_padding;
#endif
} RinDriverV3UsbControllerOps;

static inline int rin_driver_v3_block_ops_valid(
    const RinDriverV3BlockOps* ops) {
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_OPS_VERSION || !ops->get_geometry ||
        !ops->submit || !ops->poll || !ops->cancel || !ops->reset ||
        !ops->remove || ops->extensions.reserved_tail[0] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->get_geometry_padding != 0u || ops->submit_padding != 0u ||
        ops->poll_padding != 0u || ops->cancel_padding != 0u ||
        ops->reset_padding != 0u || ops->remove_padding != 0u ||
        ops->extensions.get_storage_health_snapshot_padding != 0u ||
        ops->extensions.get_namespace_identity_padding != 0u ||
        ops->extensions.get_media_identity_padding != 0u)
        return 0;
#endif
    return 1;
}

static inline int rin_driver_v3_block_media_identity_valid(
    const RinDriverV3BlockMediaIdentityV1* identity) {
    uint8_t any_nonzero = 0u;
    uint8_t all_ones = 1u;
    if (!identity || identity->struct_size != sizeof(*identity) ||
        identity->version != RIN_DRIVER_V3_BLOCK_MEDIA_IDENTITY_VERSION ||
        identity->kind != RIN_DRIVER_V3_BLOCK_MEDIA_IDENTITY_SD_CID ||
        identity->size != RIN_DRIVER_V3_BLOCK_MEDIA_IDENTITY_BYTES)
        return 0;
    for (uint32_t index = 0u;
         index < RIN_DRIVER_V3_BLOCK_MEDIA_IDENTITY_BYTES; ++index) {
        any_nonzero |= identity->identifier[index];
        if (identity->identifier[index] != UINT8_MAX) all_ones = 0u;
    }
    return any_nonzero != 0u && all_ones == 0u;
}

static inline int rin_driver_v3_block_namespace_identity_valid(
    const RinDriverV3BlockNamespaceIdentityV1* identity) {
    if (!identity || identity->struct_size != sizeof(*identity) ||
        identity->version !=
            RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_VERSION ||
        identity->namespace_id == 0u || identity->namespace_id == UINT32_MAX ||
        identity->alias_count >
            RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_MAX_ALIASES)
        return 0;
    for (uint32_t index = 0u;
         index < RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_MAX_ALIASES;
         ++index) {
        const RinDriverV3BlockNamespaceAliasV1* alias =
            &identity->aliases[index];
        if (index >= identity->alias_count) {
            if (alias->kind != 0u || alias->size != 0u) return 0;
            for (uint32_t byte = 0u;
                 byte < RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_BYTES; ++byte)
                if (alias->identifier[byte] != 0u) return 0;
            continue;
        }
        if ((alias->kind == RIN_DRIVER_V3_BLOCK_NAMESPACE_ALIAS_NGUID &&
             alias->size != 16u) ||
            (alias->kind == RIN_DRIVER_V3_BLOCK_NAMESPACE_ALIAS_EUI64 &&
             alias->size != 8u) ||
            (alias->kind != RIN_DRIVER_V3_BLOCK_NAMESPACE_ALIAS_NGUID &&
             alias->kind != RIN_DRIVER_V3_BLOCK_NAMESPACE_ALIAS_EUI64))
            return 0;
        uint8_t any_nonzero = 0u;
        uint8_t all_ones = 1u;
        for (uint32_t byte = 0u;
             byte < RIN_DRIVER_V3_BLOCK_NAMESPACE_IDENTITY_BYTES; ++byte) {
            if (byte < alias->size) {
                any_nonzero |= alias->identifier[byte];
                if (alias->identifier[byte] != UINT8_MAX) all_ones = 0u;
            } else if (alias->identifier[byte] != 0u) {
                return 0;
            }
        }
        if (any_nonzero == 0u || all_ones != 0u) return 0;
        for (uint32_t prior = 0u; prior < index; ++prior)
            if (identity->aliases[prior].kind == alias->kind) return 0;
    }
    return 1;
}

static inline int rin_driver_v3_input_ops_valid(
    const RinDriverV3InputOps* ops) {
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_OPS_VERSION || !ops->read ||
        !ops->remove || ops->reserved[0] != 0u || ops->reserved[1] != 0u ||
        ops->reserved[2] != 0u || ops->reserved[3] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->read_padding != 0u || ops->remove_padding != 0u) return 0;
#endif
    return 1;
}

static inline int rin_driver_v3_net_ops_valid(const RinDriverV3NetOps* ops) {
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_OPS_VERSION || !ops->transmit ||
        !ops->poll_rx || !ops->remove || !ops->receive || !ops->get_info)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->transmit_padding != 0u || ops->poll_rx_padding != 0u ||
        ops->remove_padding != 0u || ops->receive_padding != 0u ||
        ops->get_info_padding != 0u || ops->set_rx_mode_padding != 0u ||
        ops->set_rx_interrupts_padding != 0u)
        return 0;
#endif
    return 1;
}

static inline int rin_driver_v3_net_ops_v2_valid(
    const RinDriverV3NetOpsV2* ops) {
    if (!ops || !rin_driver_v3_net_ops_valid(&ops->base) ||
        ops->extension_size != sizeof(*ops) - sizeof(ops->base) ||
        ops->extension_version !=
            RIN_DRIVER_V3_NET_OPS_EXTENSION_VERSION ||
        !ops->transmit_v2)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->transmit_v2_padding != 0u) return 0;
#endif
    return 1;
}

static inline int rin_driver_v3_net_rx_metadata_valid(
    const RinDriverV3NetRxMetadataV1* metadata) {
    return metadata && metadata->struct_size == sizeof(*metadata) &&
        metadata->version == RIN_DRIVER_V3_NET_RX_METADATA_VERSION &&
        (metadata->checksum_flags &
         ~RIN_DRIVER_V3_NET_RX_CHECKSUM_FLAG_MASK) == 0u &&
        metadata->reserved == 0u;
}

static inline int rin_driver_v3_net_ops_v3_valid(
    const RinDriverV3NetOpsV3* ops) {
    if (!ops || !rin_driver_v3_net_ops_v2_valid(&ops->base) ||
        ops->extension_size != sizeof(*ops) - sizeof(ops->base) ||
        ops->extension_version !=
            RIN_DRIVER_V3_NET_OPS_V3_EXTENSION_VERSION ||
        !ops->receive_v2)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->receive_v2_padding != 0u) return 0;
#endif
    return 1;
}

static inline int rin_driver_v3_net_ops_v4_valid(
    const RinDriverV3NetOpsV4* ops) {
    if (!ops || !rin_driver_v3_net_ops_v3_valid(&ops->base) ||
        ops->extension_size != sizeof(*ops) - sizeof(ops->base) ||
        ops->extension_version !=
            RIN_DRIVER_V3_NET_OPS_V4_EXTENSION_VERSION)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->recover_padding != 0u) return 0;
#endif
    return 1;
}

static inline int rin_driver_v3_audio_ops_valid(
    const RinDriverV3AudioOps* ops) {
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_OPS_VERSION || !ops->get_format ||
        !ops->configure || !ops->submit || !ops->poll || !ops->stop ||
        !ops->remove || ops->reserved[0] != 0u || ops->reserved[1] != 0u ||
        ops->reserved[2] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->get_format_padding != 0u || ops->configure_padding != 0u ||
        ops->submit_padding != 0u || ops->poll_padding != 0u ||
        ops->stop_padding != 0u || ops->remove_padding != 0u ||
        ops->set_control_padding != 0u)
        return 0;
#endif
    return 1;
}

static inline int rin_driver_v3_usb_controller_ops_valid(
    const RinDriverV3UsbControllerOps* ops) {
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_DRIVER_V3_OPS_VERSION || !ops->get_port_count ||
        !ops->get_port_status || !ops->reset_port ||
        !ops->poll_port_changes || !ops->reset_controller || !ops->remove ||
        !ops->submit_transfer || !ops->poll_transfers ||
        !ops->cancel_transfer || !ops->device_control)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->get_port_count_padding != 0u ||
        ops->get_port_status_padding != 0u ||
        ops->reset_port_padding != 0u ||
        ops->poll_port_changes_padding != 0u ||
        ops->reset_controller_padding != 0u || ops->remove_padding != 0u ||
        ops->submit_transfer_padding != 0u ||
        ops->poll_transfers_padding != 0u ||
        ops->cancel_transfer_padding != 0u ||
        ops->device_control_padding != 0u)
        return 0;
#endif
    return 1;
}

static inline int rin_driver_v3_input_event_valid(
    const RinDriverV3InputEvent* event) {
    return event && event->type != 0u;
}

static inline int rin_driver_v3_net_packet_valid(
    const RinDriverV3NetPacket* packet) {
    return packet && packet->memory_lease != 0u &&
           packet->length >= RIN_DRIVER_V3_NET_MIN_FRAME_SIZE &&
           packet->length <= RIN_DRIVER_V3_NET_MAX_FRAME_SIZE &&
           packet->flags == 0u &&
           packet->offset <= UINT64_MAX - packet->length;
}

static inline int rin_driver_v3_net_packet_v2_valid(
    const RinDriverV3NetPacketV2* packet) {
    uint32_t flags;
    if (!packet || packet->struct_size != sizeof(*packet) ||
        packet->version != RIN_DRIVER_V3_NET_PACKET_V2_VERSION ||
        packet->packet.memory_lease == 0u ||
        packet->packet.length == 0u ||
        (((packet->packet.flags &
           RIN_DRIVER_V3_NET_PACKET_FLAG_FRAME_FRAGMENT) == 0u) &&
         packet->packet.length < RIN_DRIVER_V3_NET_MIN_FRAME_SIZE) ||
        packet->packet.length > RIN_DRIVER_V3_NET_MAX_FRAME_SIZE ||
        (packet->packet.flags &
         ~RIN_DRIVER_V3_NET_PACKET_FLAG_FRAME_FRAGMENT) != 0u ||
        packet->packet.offset >
            UINT64_MAX - packet->packet.length ||
        packet->reserved != 0u)
        return 0;
    flags = packet->checksum_flags;
    if (flags == 0u)
        return packet->checksum_start == 0u &&
               packet->checksum_length == 0u &&
               packet->mac_header_length == 0u &&
               packet->network_header_length == 0u &&
               packet->transport_header_length == 0u &&
               packet->checksum_offset == 0u &&
               packet->pseudo_header_seed == 0u;
    if ((flags & ~RIN_DRIVER_V3_NET_TX_CHECKSUM_KNOWN_FLAGS) != 0u ||
        (packet->packet.flags &
         RIN_DRIVER_V3_NET_PACKET_FLAG_FRAME_FRAGMENT) != 0u ||
        (flags & (RIN_DRIVER_V3_NET_TX_CHECKSUM_IPV4_HEADER |
                  RIN_DRIVER_V3_NET_TX_CHECKSUM_TRANSPORT)) !=
            (RIN_DRIVER_V3_NET_TX_CHECKSUM_IPV4_HEADER |
             RIN_DRIVER_V3_NET_TX_CHECKSUM_TRANSPORT) ||
        packet->mac_header_length < 14u ||
        packet->mac_header_length > 30u ||
        packet->network_header_length < 20u ||
        packet->network_header_length > 60u ||
        (packet->network_header_length & 3u) != 0u ||
        packet->checksum_start !=
            (uint32_t)packet->mac_header_length +
                packet->network_header_length ||
        packet->checksum_start > packet->packet.length ||
        packet->checksum_length == 0u ||
        packet->checksum_length >
            packet->packet.length - packet->checksum_start ||
        packet->transport_header_length == 0u ||
        packet->transport_header_length > packet->checksum_length)
        return 0;
    if ((flags & RIN_DRIVER_V3_NET_TX_CHECKSUM_UDP) != 0u)
        return packet->transport_header_length == 8u &&
               packet->checksum_offset == 6u;
    return packet->transport_header_length >= 20u &&
           packet->transport_header_length <= 60u &&
           (packet->transport_header_length & 3u) == 0u &&
           packet->checksum_offset == 16u;
}

static inline int rin_driver_v3_net_info_valid(
    const RinDriverV3NetInfo* info) {
    uint8_t mac_nonzero = 0u;
    uint8_t peer_nonzero = 0u;
    if (!info || info->struct_size < sizeof(*info) ||
        (info->version != RIN_DRIVER_V3_NET_INFO_VERSION_V1 &&
         info->version != RIN_DRIVER_V3_NET_INFO_VERSION_V2 &&
         info->version != RIN_DRIVER_V3_NET_INFO_VERSION_V3))
        return 0;
    if (info->version == RIN_DRIVER_V3_NET_INFO_VERSION_V3) {
        if ((info->link_speed_mbps != 0u &&
             info->link_speed_mbps != 10u &&
             info->link_speed_mbps != 100u &&
             info->link_speed_mbps != 1000u &&
             info->link_speed_mbps != 2500u &&
             info->link_speed_mbps != 5000u) ||
            (info->link_speed_mbps != 0u &&
             (info->flags & (RIN_DRIVER_V3_NET_LINK_KNOWN |
                             RIN_DRIVER_V3_NET_LINK_UP)) !=
             (RIN_DRIVER_V3_NET_LINK_KNOWN |
              RIN_DRIVER_V3_NET_LINK_UP)))
            return 0;
    } else if (info->reserved0 != 0u) {
        return 0;
    }
    if ((info->flags & ~RIN_DRIVER_V3_NET_INFO_FLAG_MASK) != 0u ||
        ((info->flags & RIN_DRIVER_V3_NET_LINK_UP) != 0u &&
         (info->flags & RIN_DRIVER_V3_NET_LINK_KNOWN) == 0u) ||
        info->mtu < 68u || info->mtu > 1500u)
        return 0;
    if (info->version == RIN_DRIVER_V3_NET_INFO_VERSION_V1) {
        if (info->interface_flags != 0u || info->interface_type != 0u ||
            info->interface_capabilities != 0u ||
            info->ipv4_peer[0] != 0u || info->ipv4_peer[1] != 0u ||
            info->ipv4_peer[2] != 0u || info->ipv4_peer[3] != 0u)
            return 0;
    } else if ((info->interface_flags &
                ~RIN_DRIVER_V3_NET_INTERFACE_KNOWN_FLAGS) != 0u ||
               info->interface_type >
                   RIN_DRIVER_V3_NET_INTERFACE_TYPE_MAX ||
               (info->interface_capabilities &
                ~RIN_DRIVER_V3_NET_INTERFACE_KNOWN_CAPABILITIES) != 0u ||
               ((info->interface_flags &
                 RIN_DRIVER_V3_NET_INTERFACE_ADMIN_UP) != 0u &&
                (info->interface_flags &
                 RIN_DRIVER_V3_NET_INTERFACE_ADMIN_KNOWN) == 0u) ||
               (((info->interface_flags &
                  RIN_DRIVER_V3_NET_INTERFACE_TYPE_KNOWN) != 0u) !=
                (info->interface_type !=
                 RIN_DRIVER_V3_NET_INTERFACE_UNKNOWN)) ||
               ((info->interface_flags &
                 RIN_DRIVER_V3_NET_INTERFACE_CAPABILITIES_KNOWN) == 0u &&
                info->interface_capabilities != 0u) ||
               ((info->interface_flags &
                 RIN_DRIVER_V3_NET_INTERFACE_CAPABILITIES_KNOWN) != 0u &&
                info->interface_type ==
                    RIN_DRIVER_V3_NET_INTERFACE_BROADCAST &&
                (info->interface_capabilities &
                 RIN_DRIVER_V3_NET_INTERFACE_CAP_BROADCAST) == 0u) ||
               ((info->interface_flags &
                 RIN_DRIVER_V3_NET_INTERFACE_TYPE_KNOWN) != 0u &&
                info->interface_type ==
                    RIN_DRIVER_V3_NET_INTERFACE_POINT_TO_POINT &&
                (info->interface_flags &
                 RIN_DRIVER_V3_NET_INTERFACE_CAPABILITIES_KNOWN) != 0u &&
                ((info->interface_capabilities &
                  RIN_DRIVER_V3_NET_INTERFACE_CAP_POINT_TO_POINT) == 0u ||
                 (info->interface_capabilities &
                  RIN_DRIVER_V3_NET_INTERFACE_CAP_BROADCAST) != 0u)) ||
               ((info->interface_capabilities &
                 RIN_DRIVER_V3_NET_INTERFACE_CAP_POINT_TO_POINT) != 0u &&
                (((info->interface_flags &
                   RIN_DRIVER_V3_NET_INTERFACE_TYPE_KNOWN) != 0u &&
                  info->interface_type !=
                      RIN_DRIVER_V3_NET_INTERFACE_POINT_TO_POINT) ||
                 (info->interface_capabilities &
                  RIN_DRIVER_V3_NET_INTERFACE_CAP_BROADCAST) != 0u)))
        return 0;
    if (info->version >= RIN_DRIVER_V3_NET_INFO_VERSION_V2)
        for (uint32_t index = 0u; index < sizeof(info->ipv4_peer); ++index)
            peer_nonzero |= info->ipv4_peer[index];
    if (peer_nonzero != 0u &&
        (((info->interface_flags &
           RIN_DRIVER_V3_NET_INTERFACE_TYPE_KNOWN) != 0u &&
          info->interface_type !=
              RIN_DRIVER_V3_NET_INTERFACE_POINT_TO_POINT) ||
         ((info->interface_flags &
           RIN_DRIVER_V3_NET_INTERFACE_CAPABILITIES_KNOWN) != 0u &&
          (info->interface_capabilities &
           RIN_DRIVER_V3_NET_INTERFACE_CAP_POINT_TO_POINT) == 0u)))
        return 0;
    for (uint32_t index = 0u; index < sizeof(info->mac); ++index)
        mac_nonzero |= info->mac[index];
    return mac_nonzero != 0u && (info->mac[0] & 1u) == 0u;
}

static inline int rin_driver_v3_audio_format_valid(
    const RinDriverV3AudioFormat* format) {
    uint32_t channel_bits = 0u;
    uint64_t mask;
    if (!format || format->struct_size < sizeof(*format) ||
        format->version != RIN_DRIVER_V3_OPS_VERSION ||
        format->sample_rate_hz < 8000u ||
        format->sample_rate_hz > 384000u ||
        format->direction == 0u ||
        (format->direction & ~RIN_DRIVER_V3_AUDIO_DIRECTION_MASK) != 0u ||
        format->channel_count == 0u || format->channel_count > 32u ||
        (format->bits_per_sample != 8u &&
         format->bits_per_sample != 16u &&
         format->bits_per_sample != 24u &&
         format->bits_per_sample != 32u) ||
        format->sample_format != RIN_DRIVER_V3_AUDIO_SIGNED_PCM ||
        (format->flags & ~RIN_DRIVER_V3_AUDIO_FORMAT_FLAG_MASK) != 0u ||
        (format->flags & RIN_DRIVER_V3_AUDIO_FORMAT_INTERLEAVED) == 0u ||
        format->reserved0 != 0u || format->reserved[0] != 0u ||
        format->reserved[1] != 0u)
        return 0;
    mask = format->channel_mask;
    while (mask != 0u) {
        channel_bits += (uint32_t)(mask & 1u);
        mask >>= 1u;
    }
    return format->channel_mask == 0u ||
           channel_bits == format->channel_count;
}

static inline int rin_driver_v3_audio_buffer_valid(
    const RinDriverV3AudioBuffer* buffer,
    const RinDriverV3AudioFormat* format) {
    uint64_t bytes_per_frame;
    uint64_t byte_length;
    if (!buffer || !rin_driver_v3_audio_format_valid(format) ||
        buffer->struct_size < sizeof(*buffer) ||
        buffer->version != RIN_DRIVER_V3_OPS_VERSION ||
        buffer->buffer_id == 0u || buffer->deadline_ns == 0u ||
        (buffer->flags & ~RIN_DRIVER_V3_AUDIO_BUFFER_FLAG_MASK) != 0u ||
        ((buffer->flags & RIN_DRIVER_V3_AUDIO_BUFFER_PLAYBACK) != 0u &&
         (buffer->flags & RIN_DRIVER_V3_AUDIO_BUFFER_CAPTURE) != 0u) ||
        buffer->reserved[0] != 0u || buffer->reserved[1] != 0u)
        return 0;
    if (buffer->frame_count == 0u)
        return (buffer->flags & RIN_DRIVER_V3_AUDIO_BUFFER_END_OF_STREAM) != 0u &&
               buffer->memory_lease == 0u && buffer->offset == 0u;
    if (buffer->memory_lease == 0u) return 0;
    bytes_per_frame = (uint64_t)format->channel_count *
                      (format->bits_per_sample / 8u);
    byte_length = bytes_per_frame * buffer->frame_count;
    return buffer->offset <= UINT64_MAX - byte_length;
}

static inline int rin_driver_v3_audio_completion_valid(
    const RinDriverV3AudioCompletion* completion,
    const RinDriverV3AudioBuffer* buffer) {
    return completion && buffer && completion->struct_size >= sizeof(*completion) &&
           completion->version == RIN_DRIVER_V3_OPS_VERSION &&
           completion->buffer_id == buffer->buffer_id &&
           completion->result <= RIN_DRIVER_V3_OK &&
           completion->result >= RIN_DRIVER_V3_UNSUPPORTED &&
           completion->completed_frames <= buffer->frame_count &&
           (completion->flags &
            ~RIN_DRIVER_V3_AUDIO_COMPLETION_FLAG_MASK) == 0u &&
           completion->reserved[0] == 0u && completion->reserved[1] == 0u;
}

/* On success, output owns the DMA allocation. On any failure, the
 * implementation must zero the entire output and transfer no lease. */
typedef RinDriverV3Result (*RinDriverV3DmaAllocateFn)(
    uint64_t api_context, uint64_t session, uint64_t length,
    uint32_t alignment, uint32_t flags, RinDriverV3DmaLeaseInfo* output);
/* `*segment_count` is the input capacity. On success, the implementation
 * transfers the mapping through `*lease` and replaces the capacity with the
 * number of returned segments. On failure, it must set `*lease` to zero,
 * leave `*segment_count` unchanged, and transfer no mapping lease. Segment
 * contents are unspecified on failure. */
typedef RinDriverV3Result (*RinDriverV3DmaMapSgFn)(
    uint64_t api_context, uint64_t session,
    const RinDriverV3MemorySlice* slices, uint32_t slice_count,
    uint32_t flags, RinDriverV3Lease* lease,
    RinDriverV3DmaSegment* segments, uint32_t* segment_count);
typedef RinDriverV3Result (*RinDriverV3DmaSyncFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease lease,
    uint32_t action, uint64_t offset, uint64_t length);
/* Returns the host physical base of a live contiguous DMA allocation. This is
 * restricted to signed kernel drivers that need a platform secure-loader
 * interface; user processes never receive this address. */
typedef RinDriverV3Result (*RinDriverV3DmaGetPhysicalAddressFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease lease,
    uint64_t* physical_address_out);
/* On success, output owns the BAR mapping. On any failure, the
 * implementation must zero the entire output and transfer no lease. */
typedef RinDriverV3Result (*RinDriverV3BarMapFn)(
    uint64_t api_context, uint64_t session, uint32_t bar_index,
    uint32_t flags, RinDriverV3BarLeaseInfo* output);
/* On success, output owns the IRQ allocation. On any failure, the
 * implementation must zero the entire output and transfer no lease. */
typedef RinDriverV3Result (*RinDriverV3IrqAcquireFn)(
    uint64_t api_context, uint64_t session, uint32_t mode,
    uint32_t vector_index, RinDriverV3IrqLeaseInfo* output);
typedef RinDriverV3Result (*RinDriverV3LeaseReleaseFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease lease);
/* On success, output owns the timer. On any failure, it must be zero. */
typedef RinDriverV3Result (*RinDriverV3TimerArmFn)(
    uint64_t api_context, uint64_t session, uint64_t deadline_ns,
    uint64_t period_ns, RinDriverV3Lease* output);
/* event_count is zero on failure. Event contents are unspecified on failure. */
typedef RinDriverV3Result (*RinDriverV3TimerPollFn)(
    uint64_t api_context, uint64_t session, uint64_t now_ns,
    RinDriverV3TimerEvent* events, uint32_t capacity, uint32_t* event_count);
/* Registered handlers run from a normal kernel safe point, never from an
 * interrupt or while the session lock is held.  The event array is borrowed
 * for the duration of the callback and every event carries the device
 * generation that was live when it was delivered.  A handler must not
 * re-enter timer_set_handler or stop its owning runtime. */
typedef RinDriverV3Result (*RinDriverV3TimerHandlerFn)(
    uint64_t driver_context, const RinDriverV3TimerEvent* events,
    uint32_t event_count);
typedef RinDriverV3Result (*RinDriverV3TimerSetHandlerFn)(
    uint64_t api_context, uint64_t session, RinDriverV3TimerHandlerFn handler,
    uint64_t driver_context);
/* On success, output owns the registration. On any failure, it must be zero. */
typedef RinDriverV3Result (*RinDriverV3RegisterDeviceFn)(
    uint64_t api_context, uint64_t session,
    const RinDriverV3DeviceRegistration* registration,
    RinDriverV3Lease* output);
/* value is zero on failure. */
typedef RinDriverV3Result (*RinDriverV3PciConfigReadFn)(
    uint64_t api_context, uint64_t session, uint16_t offset,
    uint8_t width, uint32_t* value);
/* A non-OK result does not guarantee that the device register was unchanged.
 * Callers that may need rollback must retain the old value and attempt its
 * restoration after every write attempt, including failed writes. */
typedef RinDriverV3Result (*RinDriverV3PciConfigWriteFn)(
    uint64_t api_context, uint64_t session, uint16_t offset,
    uint8_t width, uint32_t value);
typedef void (*RinDriverV3LogFn)(
    uint64_t api_context, uint64_t session, uint32_t level,
    const char* message, uint32_t message_length);
/* Returns monotonic nanoseconds. Zero is a valid timestamp; this callback
 * has no separate error result. */
typedef uint64_t (*RinDriverV3ClockFn)(uint64_t api_context);
typedef RinDriverV3Result (*RinDriverV3DeviceDmaStartFn)(
    uint64_t api_context, uint64_t session);
typedef RinDriverV3Result (*RinDriverV3DeviceDmaStopFn)(
    uint64_t api_context, uint64_t session);
/* ABI minor 4: available only to NDRV images that opt into the stop-reason
 * extension. The callback succeeds while Stop or Unbind is executing and
 * reports the platform removal reason for this device instance. */
typedef RinDriverV3Result (*RinDriverV3GetStopReasonFn)(
    uint64_t api_context, uint64_t session,
    RinDriverV3StopReasonInfo* info_out);
/* Mixes a bounded sample from an active DMA lease into the kernel DRBG.
 * The sample is never credited as trusted entropy and is never returned as
 * random output. The runtime copies the bytes before mixing them. */
typedef RinDriverV3Result (*RinDriverV3EntropyMixFn)(
    uint64_t api_context, uint64_t session,
    const void* dma_sample, uint32_t sample_length);

typedef struct RinDriverApiV3 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t api_context;
    uint64_t capabilities;
    RinDriverV3DmaAllocateFn dma_allocate;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t dma_allocate_padding;
#endif
    RinDriverV3DmaMapSgFn dma_map_sg;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t dma_map_sg_padding;
#endif
    RinDriverV3DmaSyncFn dma_sync;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t dma_sync_padding;
#endif
    RinDriverV3BarMapFn bar_map;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t bar_map_padding;
#endif
    RinDriverV3IrqAcquireFn irq_acquire;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t irq_acquire_padding;
#endif
    RinDriverV3LeaseReleaseFn lease_release;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t lease_release_padding;
#endif
    RinDriverV3TimerArmFn timer_arm;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t timer_arm_padding;
#endif
    RinDriverV3TimerPollFn timer_poll;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t timer_poll_padding;
#endif
    RinDriverV3TimerSetHandlerFn timer_set_handler;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t timer_set_handler_padding;
#endif
    RinDriverV3RegisterDeviceFn register_device;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t register_device_padding;
#endif
    RinDriverV3PciConfigReadFn pci_config_read;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t pci_config_read_padding;
#endif
    RinDriverV3PciConfigWriteFn pci_config_write;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t pci_config_write_padding;
#endif
    RinDriverV3LogFn log;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t log_padding;
#endif
    RinDriverV3ClockFn monotonic_time_ns;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t monotonic_time_ns_padding;
#endif
    /* Consumes one formerly reserved pointer slot without changing the
     * RinDriverApiV3 size. Older v3 images may leave it unused. */
    RinDriverV3DeviceDmaStartFn device_dma_start;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t device_dma_start_padding;
#endif
    /* Address of the optional, versioned capability broker table from
     * <rin/driver/services_v3.h>. This consumes one reserved 64-bit slot. */
    uint64_t services_api_address;
    /* Entropy mix input is capability-gated and consumes one former reserved
     * pointer slot without changing the 184-byte API table. */
    RinDriverV3EntropyMixFn entropy_mix;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t entropy_mix_padding;
#endif
    /* ABI minor 1: stop device-originated DMA while retaining IOMMU mappings.
     * Older images (minor 0) continue to receive a null callback. */
    RinDriverV3DeviceDmaStopFn device_dma_stop;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t device_dma_stop_padding;
#endif
    /* ABI v3 extension consumes one former reserved pointer slot. Only
     * contiguous allocations with a backend physical-address provider work. */
    RinDriverV3DmaGetPhysicalAddressFn dma_get_physical_address;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t dma_get_physical_address_padding;
#endif
    union {
        RinDriverV3GetStopReasonFn get_stop_reason;
        uint64_t reserved[1];
    };
} RinDriverApiV3;

static inline RinDriverV3Result rin_driver_v3_get_stop_reason(
    const RinDriverApiV3* api, const RinDriverContextV3* context,
    RinDriverV3StopReasonInfo* info_out)
{
    RinDriverV3StopReasonInfo info;
    RinDriverV3Result result;
    info.struct_size = 0u;
    info.version = 0u;
    info.reason = 0u;
    info.reserved0 = 0u;
    info.device_generation = 0u;
    info.reserved[0] = 0u;
    info.reserved[1] = 0u;
    if (!api || !context || !info_out ||
        context->struct_size < sizeof(*context) ||
        api->struct_size < offsetof(RinDriverApiV3, get_stop_reason) +
                               sizeof(api->get_stop_reason) ||
        !api->get_stop_reason)
        return RIN_DRIVER_V3_UNSUPPORTED;
    info.struct_size = sizeof(info);
    info.version = RIN_DRIVER_V3_STOP_REASON_INFO_VERSION;
    result = api->get_stop_reason(api->api_context, context->session,
                                  &info);
    if (result != RIN_DRIVER_V3_OK) return result;
    if (info.struct_size != sizeof(info) ||
        info.version != RIN_DRIVER_V3_STOP_REASON_INFO_VERSION ||
        info.reason > RIN_DRIVER_V3_STOP_REASON_START_FAILURE ||
        info.reserved0 != 0u || info.reserved[0] != 0u ||
        info.reserved[1] != 0u || info.device_generation == 0u)
        return RIN_DRIVER_V3_INVALID;
    *info_out = info;
    return RIN_DRIVER_V3_OK;
}

typedef RinDriverV3Result (*RinDriverEntryV3)(
    const RinDriverApiV3* api, RinDriverContextV3* context);
typedef RinDriverV3Result (*RinDriverPowerV3)(
    const RinDriverApiV3* api, RinDriverContextV3* context, uint32_t state);

#if defined(__cplusplus)
static_assert(sizeof(RinDriverApiV3) == 184u,
              "NDRV v3 API table ABI drift");
static_assert(offsetof(RinDriverApiV3, dma_allocate) == 24u &&
                  offsetof(RinDriverApiV3, timer_poll) == 80u &&
                  offsetof(RinDriverApiV3, services_api_address) == 144u &&
                  offsetof(RinDriverApiV3, entropy_mix) == 152u &&
                  offsetof(RinDriverApiV3, device_dma_stop) == 160u &&
                  offsetof(RinDriverApiV3, dma_get_physical_address) == 168u &&
                  offsetof(RinDriverApiV3, get_stop_reason) == 176u,
              "NDRV v3 API callback slot ABI drift");
static_assert(sizeof(RinDriverV3StopReasonInfo) == 40u,
              "NDRV v3 Stop reason ABI drift");
static_assert(sizeof(RinDriverV3DmaLeaseInfo) == 48u,
              "NDRV v3 DMA lease ABI drift");
static_assert(sizeof(RinDriverV3TimerEvent) == 32u,
              "NDRV v3 timer event ABI drift");
static_assert(sizeof(RinDriverV3Status) == 80u,
              "NDRV v3 status ABI drift");
static_assert(sizeof(RinDriverV3DiagnosticSnapshot) == 312u,
              "NDRV v3 diagnostic snapshot ABI drift");
static_assert(sizeof(RinDriverV3BarLeaseInfo) == 32u,
              "NDRV v3 BAR lease ABI drift");
static_assert(sizeof(RinDriverV3IrqLeaseInfo) == 16u,
              "NDRV v3 IRQ lease ABI drift");
static_assert(sizeof(RinDriverV3MemorySlice) == 24u,
              "NDRV v3 memory slice ABI drift");
static_assert(sizeof(RinDriverV3DmaSegment) == 16u,
              "NDRV v3 DMA segment ABI drift");
static_assert(sizeof(RinDriverContextV3) == 200u,
              "NDRV v3 context ABI drift");
static_assert(sizeof(RinDriverV3DeviceRegistration) == 72u,
              "NDRV v3 registration ABI drift");
static_assert(sizeof(RinDriverV3NetInfo) == 40u &&
                  offsetof(RinDriverV3NetInfo, link_speed_mbps) == 20u,
              "NDRV v3 network information ABI drift");
static_assert(sizeof(RinDriverV3NetPacket) == 24u &&
                  sizeof(RinDriverV3NetPacketV2) == 56u,
              "NDRV v3 network packet ABI drift");
static_assert(sizeof(RinDriverV3NetOps) == 64u,
              "NDRV v3 network operations ABI drift");
static_assert(sizeof(RinDriverV3NetOpsV2) == 80u,
              "NDRV v3 network operations V2 ABI drift");
static_assert(sizeof(RinDriverV3NetRxMetadataV1) == 16u &&
                  sizeof(RinDriverV3NetOpsV3) == 96u,
              "NDRV v3 network receive metadata ABI drift");
static_assert(sizeof(RinDriverV3NetOpsV4) == 112u,
              "NDRV v3 network recovery operations ABI drift");
static_assert(offsetof(RinDriverV3NetOps, transmit) == 8u &&
                  offsetof(RinDriverV3NetOps, poll_rx) == 16u &&
                  offsetof(RinDriverV3NetOps, remove) == 24u &&
                  offsetof(RinDriverV3NetOps, receive) == 32u &&
                  offsetof(RinDriverV3NetOps, get_info) == 40u &&
                  offsetof(RinDriverV3NetOps, set_rx_mode) == 48u &&
                  offsetof(RinDriverV3NetOps, set_rx_interrupts) == 56u,
              "NDRV v3 network operation slot ABI drift");
static_assert(sizeof(RinDriverV3BlockNamespaceIdentityV1) == 64u,
              "NDRV v3 block namespace identity ABI drift");
static_assert(sizeof(RinDriverV3BlockMediaIdentityV1) == 32u,
              "NDRV v3 block media identity ABI drift");
static_assert(sizeof(RinDriverV3BlockGeometry) == 40u,
              "NDRV v3 block geometry ABI drift");
static_assert(sizeof(RinDriverV3BlockRequest) == 64u,
              "NDRV v3 block request ABI drift");
static_assert(sizeof(RinDriverV3BlockCompletion) == 48u,
              "NDRV v3 block completion ABI drift");
static_assert(sizeof(RinDriverV3BlockOps) == 88u,
              "NDRV v3 block operation ABI drift");
static_assert(offsetof(RinDriverV3BlockOps, extensions) == 56u,
              "NDRV v3 block extension slot ABI drift");
static_assert(sizeof(RinDriverV3AudioFormat) == 56u,
              "NDRV v3 audio format ABI drift");
static_assert(sizeof(RinDriverV3AudioBuffer) == 64u,
              "NDRV v3 audio buffer ABI drift");
static_assert(sizeof(RinDriverV3AudioCompletion) == 48u,
              "NDRV v3 audio completion ABI drift");
static_assert(sizeof(RinDriverV3AudioOps) == 88u,
              "NDRV v3 audio operation ABI drift");
static_assert(sizeof(RinDriverV3UsbPortStatus) == 40u,
              "NDRV v3 USB port status ABI drift");
static_assert(sizeof(RinDriverV3UsbDeviceRequest) == 64u,
              "NDRV v3 USB device request ABI drift");
static_assert(sizeof(RinDriverV3UsbEndpointConfiguration) == 48u,
              "NDRV v3 USB endpoint configuration ABI drift");
static_assert(sizeof(RinDriverV3UsbDeviceResponse) == 48u,
              "NDRV v3 USB device response ABI drift");
static_assert(sizeof(RinDriverV3UsbTransfer) == 96u,
              "NDRV v3 USB transfer ABI drift");
static_assert(sizeof(RinDriverV3UsbTransferCompletion) == 56u,
              "NDRV v3 USB transfer completion ABI drift");
static_assert(sizeof(RinDriverV3UsbControllerOps) == 88u,
              "NDRV v3 USB controller operation ABI drift");
static_assert(sizeof(RinDriverV3InputOps) == 56u,
              "NDRV v3 input operation ABI drift");
static_assert(sizeof(RinDriverV3DisplayEvent) == 72u,
              "NDRV v3 display event ABI drift");
static_assert(sizeof(RinDriverV3DisplayOps) == 72u,
              "NDRV v3 display operation ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverApiV3) == 184u,
               "NDRV v3 API table ABI drift");
_Static_assert(offsetof(RinDriverApiV3, dma_allocate) == 24u &&
                   offsetof(RinDriverApiV3, timer_poll) == 80u &&
                   offsetof(RinDriverApiV3, services_api_address) == 144u &&
                   offsetof(RinDriverApiV3, entropy_mix) == 152u &&
                   offsetof(RinDriverApiV3, device_dma_stop) == 160u &&
                   offsetof(RinDriverApiV3, dma_get_physical_address) == 168u &&
                  offsetof(RinDriverApiV3, get_stop_reason) == 176u,
               "NDRV v3 API callback slot ABI drift");
_Static_assert(sizeof(RinDriverV3StopReasonInfo) == 40u,
               "NDRV v3 Stop reason ABI drift");
_Static_assert(sizeof(RinDriverV3DmaLeaseInfo) == 48u,
               "NDRV v3 DMA lease ABI drift");
_Static_assert(sizeof(RinDriverV3TimerEvent) == 32u,
               "NDRV v3 timer event ABI drift");
_Static_assert(sizeof(RinDriverV3Status) == 80u,
               "NDRV v3 status ABI drift");
_Static_assert(sizeof(RinDriverV3DiagnosticSnapshot) == 312u,
               "NDRV v3 diagnostic snapshot ABI drift");
_Static_assert(sizeof(RinDriverV3BarLeaseInfo) == 32u,
               "NDRV v3 BAR lease ABI drift");
_Static_assert(sizeof(RinDriverV3IrqLeaseInfo) == 16u,
               "NDRV v3 IRQ lease ABI drift");
_Static_assert(sizeof(RinDriverV3MemorySlice) == 24u,
               "NDRV v3 memory slice ABI drift");
_Static_assert(sizeof(RinDriverV3DmaSegment) == 16u,
               "NDRV v3 DMA segment ABI drift");
_Static_assert(sizeof(RinDriverContextV3) == 200u,
               "NDRV v3 context ABI drift");
_Static_assert(sizeof(RinDriverV3DeviceRegistration) == 72u,
               "NDRV v3 registration ABI drift");
_Static_assert(sizeof(RinDriverV3NetInfo) == 40u &&
                   offsetof(RinDriverV3NetInfo, link_speed_mbps) == 20u,
               "NDRV v3 network information ABI drift");
_Static_assert(sizeof(RinDriverV3NetPacket) == 24u &&
                   sizeof(RinDriverV3NetPacketV2) == 56u,
               "NDRV v3 network packet ABI drift");
_Static_assert(sizeof(RinDriverV3NetOps) == 64u,
               "NDRV v3 network operations ABI drift");
_Static_assert(sizeof(RinDriverV3NetOpsV2) == 80u,
               "NDRV v3 network operations V2 ABI drift");
_Static_assert(sizeof(RinDriverV3NetRxMetadataV1) == 16u &&
                   sizeof(RinDriverV3NetOpsV3) == 96u,
               "NDRV v3 network receive metadata ABI drift");
_Static_assert(sizeof(RinDriverV3NetOpsV4) == 112u,
               "NDRV v3 network recovery operations ABI drift");
_Static_assert(offsetof(RinDriverV3NetOps, transmit) == 8u &&
                   offsetof(RinDriverV3NetOps, poll_rx) == 16u &&
                   offsetof(RinDriverV3NetOps, remove) == 24u &&
                   offsetof(RinDriverV3NetOps, receive) == 32u &&
                   offsetof(RinDriverV3NetOps, get_info) == 40u &&
                   offsetof(RinDriverV3NetOps, set_rx_mode) == 48u &&
                   offsetof(RinDriverV3NetOps, set_rx_interrupts) == 56u,
               "NDRV v3 network operation slot ABI drift");
_Static_assert(sizeof(RinDriverV3BlockNamespaceIdentityV1) == 64u,
               "NDRV v3 block namespace identity ABI drift");
_Static_assert(sizeof(RinDriverV3BlockMediaIdentityV1) == 32u,
               "NDRV v3 block media identity ABI drift");
_Static_assert(sizeof(RinDriverV3BlockGeometry) == 40u,
               "NDRV v3 block geometry ABI drift");
_Static_assert(sizeof(RinDriverV3BlockRequest) == 64u,
               "NDRV v3 block request ABI drift");
_Static_assert(sizeof(RinDriverV3BlockCompletion) == 48u,
               "NDRV v3 block completion ABI drift");
_Static_assert(sizeof(RinDriverV3BlockOps) == 88u,
               "NDRV v3 block operation ABI drift");
_Static_assert(offsetof(RinDriverV3BlockOps, extensions) == 56u,
               "NDRV v3 block extension slot ABI drift");
_Static_assert(sizeof(RinDriverV3AudioFormat) == 56u,
               "NDRV v3 audio format ABI drift");
_Static_assert(sizeof(RinDriverV3AudioBuffer) == 64u,
               "NDRV v3 audio buffer ABI drift");
_Static_assert(sizeof(RinDriverV3AudioCompletion) == 48u,
               "NDRV v3 audio completion ABI drift");
_Static_assert(sizeof(RinDriverV3AudioOps) == 88u,
               "NDRV v3 audio operation ABI drift");
_Static_assert(sizeof(RinDriverV3UsbPortStatus) == 40u,
               "NDRV v3 USB port status ABI drift");
_Static_assert(sizeof(RinDriverV3UsbDeviceRequest) == 64u,
               "NDRV v3 USB device request ABI drift");
_Static_assert(sizeof(RinDriverV3UsbEndpointConfiguration) == 48u,
               "NDRV v3 USB endpoint configuration ABI drift");
_Static_assert(sizeof(RinDriverV3UsbDeviceResponse) == 48u,
               "NDRV v3 USB device response ABI drift");
_Static_assert(sizeof(RinDriverV3UsbTransfer) == 96u,
               "NDRV v3 USB transfer ABI drift");
_Static_assert(sizeof(RinDriverV3UsbTransferCompletion) == 56u,
               "NDRV v3 USB transfer completion ABI drift");
_Static_assert(sizeof(RinDriverV3UsbControllerOps) == 88u,
               "NDRV v3 USB controller operation ABI drift");
_Static_assert(sizeof(RinDriverV3InputOps) == 56u,
               "NDRV v3 input operation ABI drift");
_Static_assert(sizeof(RinDriverV3DisplayEvent) == 72u,
               "NDRV v3 display event ABI drift");
_Static_assert(sizeof(RinDriverV3DisplayOps) == 72u,
               "NDRV v3 display operation ABI drift");
_Static_assert(sizeof(RinDriverV3InputEvent) == 16u,
               "NDRV v3 input event ABI drift");
_Static_assert(sizeof(RinDriverV3NetPacket) == 24u,
               "NDRV v3 net packet ABI drift");
#endif

#endif /* RIN_DRIVER_MODULE_API_V3_H */
