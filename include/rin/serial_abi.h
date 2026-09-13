/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_SERIAL_ABI_H
#define RIN_SDK_SERIAL_ABI_H

#include <stdint.h>

#define RIN_SERIAL_ABI_VERSION UINT32_C(0x00010000)
#define RIN_SERIAL_NAME_MAX UINT32_C(64)
#define RIN_SERIAL_MANUFACTURER_MAX UINT32_C(64)
#define RIN_SERIAL_PRODUCT_MAX UINT32_C(64)
#define RIN_SERIAL_NUMBER_MAX UINT32_C(64)
#define RIN_SERIAL_PATH_MAX UINT32_C(32)
#define RIN_SERIAL_BUFFER_MAX UINT32_C(4096)

typedef enum RinSerialTransportV1 {
    RIN_SERIAL_TRANSPORT_PLATFORM_UART = 1,
    RIN_SERIAL_TRANSPORT_USB_CDC_ACM = 2,
    RIN_SERIAL_TRANSPORT_USB_VENDOR = 3
} RinSerialTransportV1;

typedef enum RinSerialParityV1 {
    RIN_SERIAL_PARITY_NONE = 0,
    RIN_SERIAL_PARITY_ODD = 1,
    RIN_SERIAL_PARITY_EVEN = 2
} RinSerialParityV1;

typedef enum RinSerialFlowControlV1 {
    RIN_SERIAL_FLOW_NONE = 0,
    RIN_SERIAL_FLOW_HARDWARE = 1
} RinSerialFlowControlV1;

typedef enum RinSerialDeviceStateV1 {
    RIN_SERIAL_DEVICE_DISCONNECTED = 0,
    RIN_SERIAL_DEVICE_AVAILABLE = 1,
    RIN_SERIAL_DEVICE_OPEN = 2
} RinSerialDeviceStateV1;

enum {
    RIN_SERIAL_DEVICE_FLAG_SYSTEM = UINT32_C(1) << 0,
    RIN_SERIAL_DEVICE_FLAG_DEBUG = UINT32_C(1) << 1,
    RIN_SERIAL_DEVICE_FLAG_CRITICAL = UINT32_C(1) << 2,
    RIN_SERIAL_DEVICE_FLAG_BROWSER_DENY = UINT32_C(1) << 3
};

enum {
    RIN_SERIAL_CAP_READ = UINT32_C(1) << 0,
    RIN_SERIAL_CAP_WRITE = UINT32_C(1) << 1,
    RIN_SERIAL_CAP_CONFIGURE = UINT32_C(1) << 2,
    RIN_SERIAL_CAP_MODEM_SIGNALS = UINT32_C(1) << 3,
    RIN_SERIAL_CAP_BREAK = UINT32_C(1) << 4,
    RIN_SERIAL_CAP_HARDWARE_FLOW = UINT32_C(1) << 5
};

enum {
    RIN_SERIAL_OPEN_PRIVILEGED = UINT32_C(1) << 0
};

enum {
    RIN_SERIAL_WAIT_READABLE = UINT32_C(1) << 0,
    RIN_SERIAL_WAIT_WRITABLE = UINT32_C(1) << 1,
    RIN_SERIAL_WAIT_HANGUP = UINT32_C(1) << 2,
    RIN_SERIAL_WAIT_ERROR = UINT32_C(1) << 3
};

enum {
    RIN_SERIAL_OK = 0,
    RIN_SERIAL_EINVAL = -22,
    RIN_SERIAL_ENODEV = -19,
    RIN_SERIAL_EBADF = -9,
    RIN_SERIAL_EBUSY = -16,
    RIN_SERIAL_EPERM = -1,
    RIN_SERIAL_EAGAIN = -11,
    RIN_SERIAL_EIO = -5,
    RIN_SERIAL_ENOSPC = -28,
    RIN_SERIAL_ENOTSUP = -95,
    RIN_SERIAL_EOVERFLOW = -75
};

typedef struct RinSerialConfigV1 {
    uint32_t baud_rate;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t flow_control;
    uint8_t rts;
    uint8_t dtr;
    uint8_t break_signal;
    uint8_t reserved0;
    uint32_t buffer_size;
} RinSerialConfigV1;

typedef struct RinSerialStatusV1 {
    uint32_t state;
    uint32_t rx_queued;
    uint32_t tx_queued;
    uint32_t rx_overflow;
    uint32_t tx_overflow;
    uint32_t framing_errors;
    uint32_t parity_errors;
    uint32_t overrun_errors;
    uint32_t break_detected;
    uint32_t event_mask;
    uint64_t event_sequence;
    uint8_t cts;
    uint8_t dsr;
    uint8_t dcd;
    uint8_t ri;
    /* Keep the status record's wire size stable on 32-bit callers. */
    uint8_t reserved_tail[4];
} RinSerialStatusV1;

typedef struct RinSerialDeviceInfoV1 {
    uint64_t object_id;
    uint64_t generation;
    uint32_t stable_instance;
    uint32_t flags;
    uint32_t capabilities;
    uint16_t vendor_id;
    uint16_t product_id;
    uint8_t transport;
    uint8_t device_class;
    uint8_t reserved0[2];
    char name[RIN_SERIAL_NAME_MAX];
    char manufacturer[RIN_SERIAL_MANUFACTURER_MAX];
    char product[RIN_SERIAL_PRODUCT_MAX];
    char serial_number[RIN_SERIAL_NUMBER_MAX];
    char path[RIN_SERIAL_PATH_MAX];
    /* The native layout already carries this tail padding; make it explicit
     * so compat32 callers use the same serial portal ABI size. */
    uint8_t reserved_tail[4];
} RinSerialDeviceInfoV1;

typedef struct RinSerialCapabilityV1 {
    uint64_t object_id;
    uint64_t generation;
} RinSerialCapabilityV1;

typedef struct RinSerialWaitResultV1 {
    uint32_t events;
    uint32_t reserved0;
    uint64_t sequence;
} RinSerialWaitResultV1;

/* Pointer-free read/write ioctl payload for a nonblocking readiness probe.
 * seen_sequence lets a caller suppress an already-consumed event while the
 * result remains owned by the kernel/driver boundary. */
typedef struct RinSerialWaitRequestV1 {
    uint32_t events;
    uint32_t reserved0;
    uint64_t seen_sequence;
    RinSerialWaitResultV1 result;
} RinSerialWaitRequestV1;

typedef struct RinSerialSignalsV1 {
    uint8_t dtr;
    uint8_t rts;
    uint8_t break_signal;
    uint8_t reserved0;
} RinSerialSignalsV1;

/* Encoded CHAR ioctl requests used by the V2 serial driver.  The size and
 * direction bits are intentional: the kernel's bounded ioctl marshaller can
 * therefore copy exactly the pointer-free ABI object for either native or
 * compat callers. */
#define RIN_SERIAL_IOCTL_WRITE 1u
#define RIN_SERIAL_IOCTL_READ 2u
#define RIN_SERIAL_IOCTL_ENCODE(direction, size, number) \
    ((((uint32_t)(direction) & 3u) << 30u) | \
     (((uint32_t)(size) & 0x3FFFu) << 16u) | \
     ((uint32_t)0x53u << 8u) | ((uint32_t)(number) & 0xFFu))
#define RIN_SERIAL_IOCTL_SET_CONFIG \
    RIN_SERIAL_IOCTL_ENCODE(RIN_SERIAL_IOCTL_WRITE, sizeof(RinSerialConfigV1), 6u)
#define RIN_SERIAL_IOCTL_SET_SIGNALS \
    RIN_SERIAL_IOCTL_ENCODE(RIN_SERIAL_IOCTL_WRITE, sizeof(RinSerialSignalsV1), 7u)
#define RIN_SERIAL_IOCTL_GET_SIGNALS \
    RIN_SERIAL_IOCTL_ENCODE(RIN_SERIAL_IOCTL_READ, sizeof(RinSerialStatusV1), 8u)
#define RIN_SERIAL_IOCTL_WAIT \
    RIN_SERIAL_IOCTL_ENCODE(RIN_SERIAL_IOCTL_READ | RIN_SERIAL_IOCTL_WRITE, \
                            sizeof(RinSerialWaitRequestV1), 9u)

#if defined(__cplusplus)
static_assert(sizeof(RinSerialStatusV1) == 56u, "serial status ABI drift");
static_assert(sizeof(RinSerialDeviceInfoV1) == 328u,
              "serial device info ABI drift");
static_assert(sizeof(RinSerialCapabilityV1) == 16u, "serial capability ABI drift");
static_assert(sizeof(RinSerialWaitRequestV1) == 32u, "serial wait ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinSerialStatusV1) == 56u, "serial status ABI drift");
_Static_assert(sizeof(RinSerialDeviceInfoV1) == 328u,
               "serial device info ABI drift");
_Static_assert(sizeof(RinSerialCapabilityV1) == 16u, "serial capability ABI drift");
_Static_assert(sizeof(RinSerialSignalsV1) == 4u, "serial signals ABI drift");
_Static_assert(sizeof(RinSerialWaitRequestV1) == 32u, "serial wait ABI drift");
#endif

#endif /* RIN_SDK_SERIAL_ABI_H */
