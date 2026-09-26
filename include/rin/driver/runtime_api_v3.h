/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_RUNTIME_API_V3_H
#define RIN_DRIVER_RUNTIME_API_V3_H

#include <stddef.h>
#include <stdint.h>

#include <rin/formats_v3.h>

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
#define RIN_DRIVER_V3_CAPABILITY_MASK                                   \
    (RIN_DRIVER_V3_CAP_DMA_ISOLATED | RIN_DRIVER_V3_CAP_BAR |            \
     RIN_DRIVER_V3_CAP_MSI | RIN_DRIVER_V3_CAP_MSIX |                    \
     RIN_DRIVER_V3_CAP_TIMER | RIN_DRIVER_V3_CAP_REGISTER |              \
     RIN_DRIVER_V3_CAP_PCI_CONFIG | RIN_DRIVER_V3_CAP_DMA_START |          \
     RIN_DRIVER_V3_CAP_DMA_CONTIGUOUS)

#define RIN_DRIVER_V3_DEVICE_BLOCK   UINT32_C(1)
#define RIN_DRIVER_V3_DEVICE_NET     UINT32_C(2)
#define RIN_DRIVER_V3_DEVICE_INPUT   UINT32_C(3)
#define RIN_DRIVER_V3_DEVICE_DISPLAY UINT32_C(4)
#define RIN_DRIVER_V3_DEVICE_AUDIO   UINT32_C(5)
#define RIN_DRIVER_V3_DEVICE_USB_CONTROLLER UINT32_C(6)

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
} RinDriverContextV3;

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

typedef struct RinDriverV3InputEvent {
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

/* NDRV v3 network data is intentionally bounded to one ordinary Ethernet
 * frame.  Receive copies are control-plane calls: the NIC-owned DMA ring
 * never leaks into the network stack as an unvalidated pointer. */
#define RIN_DRIVER_V3_NET_MIN_FRAME_SIZE UINT32_C(14)
#define RIN_DRIVER_V3_NET_MAX_FRAME_SIZE UINT32_C(1518)
#define RIN_DRIVER_V3_NET_INFO_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_V3_NET_LINK_UP UINT32_C(0x00000001)
/* LINK_UP is meaningful only when LINK_KNOWN is set.  This prevents a
 * transport without a negotiated carrier-status feature from fabricating an
 * online state for DHCP, routing, or UI consumers. */
#define RIN_DRIVER_V3_NET_LINK_KNOWN UINT32_C(0x00000002)
#define RIN_DRIVER_V3_NET_INFO_FLAG_MASK \
    (RIN_DRIVER_V3_NET_LINK_UP | RIN_DRIVER_V3_NET_LINK_KNOWN)

typedef struct RinDriverV3NetInfo {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint16_t mtu;
    uint8_t mac[6];
    uint16_t reserved0;
    uint64_t reserved[2];
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
    /* Isochronous high-speed endpoints may carry 1..3 transactions per
     * service interval.  Zero is the legacy encoding for one transaction. */
    uint32_t transactions_per_interval;
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

/* A USB transfer refers only to a kernel-owned memory lease. The driver must
 * copy this descriptor before returning from submit_transfer and must map the
 * selected range through dma_map_sg. setup_packet contains the eight USB
 * setup bytes in little-endian order and is zero for bulk and interrupt
 * transfers. */
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
    uint32_t reserved0;
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
typedef RinDriverV3Result (*RinDriverV3InputReadFn)(
    uint64_t driver_context, RinDriverV3InputEvent* events,
    uint32_t capacity, uint32_t* event_count);
typedef RinDriverV3Result (*RinDriverV3NetTransmitFn)(
    uint64_t driver_context, const RinDriverV3NetPacket* packets,
    uint32_t packet_count);
typedef RinDriverV3Result (*RinDriverV3NetPollFn)(
    uint64_t driver_context, uint32_t budget, uint32_t* completed);
/* `buffer` is caller-owned, transient control memory.  The driver copies a
 * fully validated received frame before it returns and immediately re-arms
 * the DMA descriptor. `received` is zero when no frame is ready. */
typedef RinDriverV3Result (*RinDriverV3NetReceiveFn)(
    uint64_t driver_context, void* buffer, uint32_t capacity,
    uint32_t* received);
typedef RinDriverV3Result (*RinDriverV3NetGetInfoFn)(
    uint64_t driver_context, RinDriverV3NetInfo* output);
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
typedef RinDriverV3Result (*RinDriverV3DeviceRemoveFn)(uint64_t driver_context);

typedef struct RinDriverV3BlockOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3BlockGetGeometryFn get_geometry;
    RinDriverV3BlockSubmitFn submit;
    RinDriverV3BlockPollFn poll;
    RinDriverV3BlockCancelFn cancel;
    RinDriverV3BlockResetFn reset;
    RinDriverV3DeviceRemoveFn remove;
    uint64_t reserved[4];
} RinDriverV3BlockOps;

typedef struct RinDriverV3DisplayOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3DisplayGetModeFn get_mode;
    RinDriverV3DisplayDamageFn damage;
    RinDriverV3DisplayCursorFn cursor;
    RinDriverV3DeviceRemoveFn remove;
    uint64_t reserved[4];
} RinDriverV3DisplayOps;

typedef struct RinDriverV3InputOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3InputReadFn read;
    RinDriverV3DeviceRemoveFn remove;
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
    uint64_t reserved[2];
} RinDriverV3NetOps;

typedef struct RinDriverV3AudioOps {
    uint32_t struct_size;
    uint32_t version;
    RinDriverV3AudioGetFormatFn get_format;
    RinDriverV3AudioConfigureFn configure;
    RinDriverV3AudioSubmitFn submit;
    RinDriverV3AudioPollFn poll;
    RinDriverV3AudioStopFn stop;
    RinDriverV3DeviceRemoveFn remove;
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
    RinDriverV3UsbGetPortStatusFn get_port_status;
    RinDriverV3UsbResetPortFn reset_port;
    RinDriverV3UsbPollPortChangesFn poll_port_changes;
    RinDriverV3UsbResetControllerFn reset_controller;
    RinDriverV3DeviceRemoveFn remove;
    RinDriverV3UsbSubmitTransferFn submit_transfer;
    RinDriverV3UsbPollTransfersFn poll_transfers;
    RinDriverV3UsbCancelTransferFn cancel_transfer;
    RinDriverV3UsbDeviceControlFn device_control;
} RinDriverV3UsbControllerOps;

typedef RinDriverV3Result (*RinDriverV3DmaAllocateFn)(
    uint64_t api_context, uint64_t session, uint64_t length,
    uint32_t alignment, uint32_t flags, RinDriverV3DmaLeaseInfo* output);
typedef RinDriverV3Result (*RinDriverV3DmaMapSgFn)(
    uint64_t api_context, uint64_t session,
    const RinDriverV3MemorySlice* slices, uint32_t slice_count,
    uint32_t flags, RinDriverV3Lease* lease,
    RinDriverV3DmaSegment* segments, uint32_t* segment_count);
typedef RinDriverV3Result (*RinDriverV3DmaSyncFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease lease,
    uint32_t action, uint64_t offset, uint64_t length);
typedef RinDriverV3Result (*RinDriverV3BarMapFn)(
    uint64_t api_context, uint64_t session, uint32_t bar_index,
    uint32_t flags, RinDriverV3BarLeaseInfo* output);
typedef RinDriverV3Result (*RinDriverV3IrqAcquireFn)(
    uint64_t api_context, uint64_t session, uint32_t mode,
    uint32_t vector_index, RinDriverV3IrqLeaseInfo* output);
typedef RinDriverV3Result (*RinDriverV3LeaseReleaseFn)(
    uint64_t api_context, uint64_t session, RinDriverV3Lease lease);
typedef RinDriverV3Result (*RinDriverV3TimerArmFn)(
    uint64_t api_context, uint64_t session, uint64_t deadline_ns,
    uint64_t period_ns, RinDriverV3Lease* output);
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
typedef RinDriverV3Result (*RinDriverV3RegisterDeviceFn)(
    uint64_t api_context, uint64_t session,
    const RinDriverV3DeviceRegistration* registration,
    RinDriverV3Lease* output);
typedef RinDriverV3Result (*RinDriverV3PciConfigReadFn)(
    uint64_t api_context, uint64_t session, uint16_t offset,
    uint8_t width, uint32_t* value);
typedef RinDriverV3Result (*RinDriverV3PciConfigWriteFn)(
    uint64_t api_context, uint64_t session, uint16_t offset,
    uint8_t width, uint32_t value);
typedef void (*RinDriverV3LogFn)(
    uint64_t api_context, uint64_t session, uint32_t level,
    const char* message, uint32_t message_length);
typedef uint64_t (*RinDriverV3ClockFn)(uint64_t api_context);
typedef RinDriverV3Result (*RinDriverV3DeviceDmaStartFn)(
    uint64_t api_context, uint64_t session);

typedef struct RinDriverApiV3 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t api_context;
    uint64_t capabilities;
    RinDriverV3DmaAllocateFn dma_allocate;
    RinDriverV3DmaMapSgFn dma_map_sg;
    RinDriverV3DmaSyncFn dma_sync;
    RinDriverV3BarMapFn bar_map;
    RinDriverV3IrqAcquireFn irq_acquire;
    RinDriverV3LeaseReleaseFn lease_release;
    RinDriverV3TimerArmFn timer_arm;
    RinDriverV3TimerPollFn timer_poll;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t timer_poll_padding;
#endif
    RinDriverV3TimerSetHandlerFn timer_set_handler;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t timer_set_handler_padding;
#endif
    RinDriverV3RegisterDeviceFn register_device;
    RinDriverV3PciConfigReadFn pci_config_read;
    RinDriverV3PciConfigWriteFn pci_config_write;
    RinDriverV3LogFn log;
    RinDriverV3ClockFn monotonic_time_ns;
    /* Consumes one formerly reserved pointer slot without changing the
     * RinDriverApiV3 size. Older v3 images may leave it unused. */
    RinDriverV3DeviceDmaStartFn device_dma_start;
    uint64_t reserved[5];
} RinDriverApiV3;

typedef RinDriverV3Result (*RinDriverEntryV3)(
    const RinDriverApiV3* api, RinDriverContextV3* context);
typedef RinDriverV3Result (*RinDriverPowerV3)(
    const RinDriverApiV3* api, RinDriverContextV3* context, uint32_t state);

#if defined(__cplusplus)
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
static_assert(sizeof(RinDriverContextV3) == 128u,
              "NDRV v3 context ABI drift");
static_assert(sizeof(RinDriverV3DeviceRegistration) == 72u,
              "NDRV v3 registration ABI drift");
static_assert(sizeof(RinDriverV3NetInfo) == 40u,
              "NDRV v3 network information ABI drift");
static_assert(sizeof(RinDriverV3NetOps) == 64u,
              "NDRV v3 network operations ABI drift");
static_assert(offsetof(RinDriverV3NetOps, transmit) == 8u &&
                  offsetof(RinDriverV3NetOps, poll_rx) == 16u &&
                  offsetof(RinDriverV3NetOps, remove) == 24u &&
                  offsetof(RinDriverV3NetOps, receive) == 32u &&
                  offsetof(RinDriverV3NetOps, get_info) == 40u &&
                  offsetof(RinDriverV3NetOps, reserved) == 48u,
              "NDRV v3 network operation slot ABI drift");
static_assert(sizeof(RinDriverV3BlockGeometry) == 40u,
              "NDRV v3 block geometry ABI drift");
static_assert(sizeof(RinDriverV3BlockRequest) == 64u,
              "NDRV v3 block request ABI drift");
static_assert(sizeof(RinDriverV3BlockCompletion) == 48u,
              "NDRV v3 block completion ABI drift");
static_assert(sizeof(RinDriverV3AudioFormat) == 56u,
              "NDRV v3 audio format ABI drift");
static_assert(sizeof(RinDriverV3AudioBuffer) == 64u,
              "NDRV v3 audio buffer ABI drift");
static_assert(sizeof(RinDriverV3AudioCompletion) == 48u,
              "NDRV v3 audio completion ABI drift");
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(RinDriverV3AudioOps) == 64u,
              "NDRV v3 32-bit audio operation ABI drift");
#else
static_assert(sizeof(RinDriverV3AudioOps) == 88u,
              "NDRV v3 64-bit audio operation ABI drift");
#endif
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
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
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
_Static_assert(sizeof(RinDriverContextV3) == 128u,
               "NDRV v3 context ABI drift");
_Static_assert(sizeof(RinDriverV3DeviceRegistration) == 72u,
               "NDRV v3 registration ABI drift");
_Static_assert(sizeof(RinDriverV3NetInfo) == 40u,
               "NDRV v3 network information ABI drift");
_Static_assert(sizeof(RinDriverV3NetOps) == 64u,
               "NDRV v3 network operations ABI drift");
_Static_assert(offsetof(RinDriverV3NetOps, transmit) == 8u &&
                   offsetof(RinDriverV3NetOps, poll_rx) == 16u &&
                   offsetof(RinDriverV3NetOps, remove) == 24u &&
                   offsetof(RinDriverV3NetOps, receive) == 32u &&
                   offsetof(RinDriverV3NetOps, get_info) == 40u &&
                   offsetof(RinDriverV3NetOps, reserved) == 48u,
               "NDRV v3 network operation slot ABI drift");
_Static_assert(sizeof(RinDriverV3BlockGeometry) == 40u,
               "NDRV v3 block geometry ABI drift");
_Static_assert(sizeof(RinDriverV3BlockRequest) == 64u,
               "NDRV v3 block request ABI drift");
_Static_assert(sizeof(RinDriverV3BlockCompletion) == 48u,
               "NDRV v3 block completion ABI drift");
_Static_assert(sizeof(RinDriverV3AudioFormat) == 56u,
               "NDRV v3 audio format ABI drift");
_Static_assert(sizeof(RinDriverV3AudioBuffer) == 64u,
               "NDRV v3 audio buffer ABI drift");
_Static_assert(sizeof(RinDriverV3AudioCompletion) == 48u,
               "NDRV v3 audio completion ABI drift");
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(RinDriverV3AudioOps) == 64u,
               "NDRV v3 32-bit audio operation ABI drift");
#else
_Static_assert(sizeof(RinDriverV3AudioOps) == 88u,
               "NDRV v3 64-bit audio operation ABI drift");
#endif
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
_Static_assert(sizeof(RinDriverV3InputEvent) == 16u,
               "NDRV v3 input event ABI drift");
_Static_assert(sizeof(RinDriverV3NetPacket) == 24u,
               "NDRV v3 net packet ABI drift");
#endif

#endif /* RIN_DRIVER_RUNTIME_API_V3_H */
