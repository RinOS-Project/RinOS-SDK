/* SPDX-License-Identifier: MIT */
#ifndef RIN_API_RIN_LEGACY_INPUT_ABI_H
#define RIN_API_RIN_LEGACY_INPUT_ABI_H

#include <stdint.h>

/* Canonical legacy SDK contract; retained only for the legacy namespace. */

#define RIN_LEGACY_INPUT_QUEUE_MAGIC UINT32_C(0x32504e49) /* "INP2" */
#define RIN_LEGACY_INPUT_QUEUE_VERSION UINT16_C(2)
#define RIN_LEGACY_INPUT_MAX_AXES 16u
#define RIN_LEGACY_INPUT_MAX_EVENTS 64u
#define RIN_LEGACY_INPUT_MAX_MT_SLOTS 16u

#define RIN_LEGACY_INPUT_PROFILE_KEY UINT32_C(0x00000001)
#define RIN_LEGACY_INPUT_PROFILE_REL UINT32_C(0x00000002)
#define RIN_LEGACY_INPUT_PROFILE_ABS UINT32_C(0x00000004)
#define RIN_LEGACY_INPUT_PROFILE_MT UINT32_C(0x00000008)
#define RIN_LEGACY_INPUT_PROFILE_POINTER UINT32_C(0x00000010)
#define RIN_LEGACY_INPUT_PROFILE_RAW_HID UINT32_C(0x00000020)
#define RIN_LEGACY_INPUT_PROFILE_FLAG_MASK                              \
    (RIN_LEGACY_INPUT_PROFILE_KEY | RIN_LEGACY_INPUT_PROFILE_REL |     \
     RIN_LEGACY_INPUT_PROFILE_ABS | RIN_LEGACY_INPUT_PROFILE_MT |      \
     RIN_LEGACY_INPUT_PROFILE_POINTER | RIN_LEGACY_INPUT_PROFILE_RAW_HID)

#define RIN_LEGACY_INPUT_EV_SYN UINT16_C(0x00)
#define RIN_LEGACY_INPUT_EV_KEY UINT16_C(0x01)
#define RIN_LEGACY_INPUT_EV_REL UINT16_C(0x02)
#define RIN_LEGACY_INPUT_EV_ABS UINT16_C(0x03)
#define RIN_LEGACY_INPUT_EV_RAW_HID UINT16_C(0x04)
#define RIN_LEGACY_INPUT_SYN_REPORT UINT16_C(0x00)

/* A RAW_HID profile is report-only.  Each event's code is the byte index and
 * value is the corresponding unsigned report byte.  The bounded event count
 * and strict 0..N-1 ordering make one SYN_REPORT an atomic report without a
 * user-controlled pointer or variable-length allocation. */
#define RIN_LEGACY_INPUT_RAW_HID_MAX_REPORT_BYTES UINT16_C(20)
#define RIN_LEGACY_INPUT_RAW_HID_BYTE_INDEX_MAX \
    (RIN_LEGACY_INPUT_RAW_HID_MAX_REPORT_BYTES - UINT16_C(1))

/* EV_KEY code encodes an admitted PS/2 Set-1 key. E0 keys set bit 7;
 * Pause is the complete E1 sequence collapsed to the reserved 0xff code.
 * Values are 0=up, 1=down, and 2=repeat. */
#define RIN_LEGACY_INPUT_KEY_SCANCODE_MASK UINT16_C(0x7f)
#define RIN_LEGACY_INPUT_KEY_EXTENDED UINT16_C(0x80)
#define RIN_LEGACY_INPUT_KEY_PAUSE UINT16_C(0xff)

#define RIN_LEGACY_INPUT_REL_X UINT16_C(0x00)
#define RIN_LEGACY_INPUT_REL_Y UINT16_C(0x01)
#define RIN_LEGACY_INPUT_REL_HWHEEL UINT16_C(0x06)
#define RIN_LEGACY_INPUT_REL_WHEEL UINT16_C(0x08)

#define RIN_LEGACY_INPUT_ABS_X UINT16_C(0x00)
#define RIN_LEGACY_INPUT_ABS_Y UINT16_C(0x01)
#define RIN_LEGACY_INPUT_ABS_Z UINT16_C(0x02)
#define RIN_LEGACY_INPUT_ABS_RX UINT16_C(0x03)
#define RIN_LEGACY_INPUT_ABS_RY UINT16_C(0x04)
#define RIN_LEGACY_INPUT_ABS_RZ UINT16_C(0x05)
#define RIN_LEGACY_INPUT_ABS_HAT0X UINT16_C(0x10)
#define RIN_LEGACY_INPUT_ABS_HAT0Y UINT16_C(0x11)
#define RIN_LEGACY_INPUT_ABS_PRESSURE UINT16_C(0x18)
#define RIN_LEGACY_INPUT_ABS_MT_SLOT UINT16_C(0x2f)
#define RIN_LEGACY_INPUT_ABS_MT_POSITION_X UINT16_C(0x35)
#define RIN_LEGACY_INPUT_ABS_MT_POSITION_Y UINT16_C(0x36)
#define RIN_LEGACY_INPUT_ABS_MT_TRACKING_ID UINT16_C(0x39)

#define RIN_LEGACY_INPUT_BTN_LEFT UINT16_C(0x110)
#define RIN_LEGACY_INPUT_BTN_RIGHT UINT16_C(0x111)
#define RIN_LEGACY_INPUT_BTN_MIDDLE UINT16_C(0x112)
#define RIN_LEGACY_INPUT_BTN_SIDE UINT16_C(0x113)
#define RIN_LEGACY_INPUT_BTN_EXTRA UINT16_C(0x114)
#define RIN_LEGACY_INPUT_BTN_TOUCH UINT16_C(0x14a)
#define RIN_LEGACY_INPUT_BTN_STYLUS UINT16_C(0x14b)
#define RIN_LEGACY_INPUT_BTN_STYLUS2 UINT16_C(0x14c)

#define RIN_LEGACY_INPUT_BTN_SOUTH UINT16_C(0x130)
#define RIN_LEGACY_INPUT_BTN_EAST UINT16_C(0x131)
#define RIN_LEGACY_INPUT_BTN_NORTH UINT16_C(0x133)
#define RIN_LEGACY_INPUT_BTN_WEST UINT16_C(0x134)
#define RIN_LEGACY_INPUT_BTN_TL UINT16_C(0x136)
#define RIN_LEGACY_INPUT_BTN_TR UINT16_C(0x137)
#define RIN_LEGACY_INPUT_BTN_TL2 UINT16_C(0x138)
#define RIN_LEGACY_INPUT_BTN_TR2 UINT16_C(0x139)
#define RIN_LEGACY_INPUT_BTN_SELECT UINT16_C(0x13a)
#define RIN_LEGACY_INPUT_BTN_START UINT16_C(0x13b)
#define RIN_LEGACY_INPUT_BTN_THUMBL UINT16_C(0x13d)
#define RIN_LEGACY_INPUT_BTN_THUMBR UINT16_C(0x13e)
#define RIN_LEGACY_INPUT_BTN_DPAD_UP UINT16_C(0x220)
#define RIN_LEGACY_INPUT_BTN_DPAD_DOWN UINT16_C(0x221)
#define RIN_LEGACY_INPUT_BTN_DPAD_LEFT UINT16_C(0x222)
#define RIN_LEGACY_INPUT_BTN_DPAD_RIGHT UINT16_C(0x223)

typedef struct RinLegacyInputAxisV2 {
    uint16_t code;
    uint16_t flags;
    int32_t minimum;
    int32_t maximum;
} RinLegacyInputAxisV2;

typedef struct RinLegacyInputProfileV2 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t flags;
    uint32_t axis_count;
    uint32_t mt_slot_count;
    uint32_t reserved0;
    RinLegacyInputAxisV2 axes[RIN_LEGACY_INPUT_MAX_AXES];
    uint64_t reserved[4];
} RinLegacyInputProfileV2;

typedef struct RinLegacyInputEventV2 {
    uint16_t type;
    uint16_t code;
    int32_t value;
} RinLegacyInputEventV2;

typedef struct RinLegacyInputQueueV2 {
    uint32_t magic;
    uint16_t version;
    uint16_t struct_size;
    volatile uint32_t head;
    volatile uint32_t tail;
    volatile uint32_t overflow;
    uint32_t reserved0;
    RinLegacyInputProfileV2 profile;
    RinLegacyInputEventV2 events[RIN_LEGACY_INPUT_MAX_EVENTS];
} RinLegacyInputQueueV2;

#if defined(__cplusplus)
static_assert(sizeof(RinLegacyInputAxisV2) == 12u,
              "legacy input axis ABI drift");
static_assert(sizeof(RinLegacyInputProfileV2) == 248u,
              "legacy input profile ABI drift");
static_assert(sizeof(RinLegacyInputEventV2) == 8u,
              "legacy input event ABI drift");
static_assert(sizeof(RinLegacyInputQueueV2) == 784u,
              "legacy input queue ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinLegacyInputAxisV2) == 12u,
               "legacy input axis ABI drift");
_Static_assert(sizeof(RinLegacyInputProfileV2) == 248u,
               "legacy input profile ABI drift");
_Static_assert(sizeof(RinLegacyInputEventV2) == 8u,
               "legacy input event ABI drift");
_Static_assert(sizeof(RinLegacyInputQueueV2) == 784u,
               "legacy input queue ABI drift");
#endif

#endif /* RIN_API_RIN_LEGACY_INPUT_ABI_H */
