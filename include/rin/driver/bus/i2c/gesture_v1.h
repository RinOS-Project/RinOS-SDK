/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_BUS_I2C_GESTURE_V1_H
#define RIN_DRIVER_BUS_I2C_GESTURE_V1_H

#include <stdint.h>

#include <rin/contract_abi.h>

#define RIN_I2C_HID_TOUCHPAD_GESTURE_V1_VERSION UINT32_C(0x00010000)
#define RIN_I2C_HID_TOUCHPAD_GESTURE_POINTER UINT32_C(1)
#define RIN_I2C_HID_TOUCHPAD_GESTURE_SCROLL UINT32_C(2)
#define RIN_I2C_HID_TOUCHPAD_GESTURE_TAP UINT32_C(4)

struct RinI2cHidTouchpadDriverV1;

/* Bounded gesture records shared by an I2C-HID driver and its input-service
 * adapter. They carry input data and policy; they do not grant device access. */
typedef struct RinI2cHidTouchpadGestureEventV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_id;
    uint64_t report_sequence;
    uint32_t kinds;
    int32_t pointer_dx;
    int32_t pointer_dy;
    int32_t scroll_x;
    int32_t scroll_y;
    uint8_t contact_count;
    uint8_t tap_count;
    uint8_t reserved0[2];
    uint64_t reserved[2];
} RinI2cHidTouchpadGestureEventV1;

typedef struct RinI2cHidTouchpadGesturePolicyV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t pointer_gain_numerator;
    uint32_t pointer_gain_denominator;
    uint32_t scroll_gain_numerator;
    uint32_t scroll_gain_denominator;
    uint32_t max_abs_delta;
    uint32_t arbitration;
    uint64_t reserved[2];
} RinI2cHidTouchpadGesturePolicyV1;

typedef RinResultCode (*RinI2cHidTouchpadGesturePublishV1)(
    void* context, const RinI2cHidTouchpadGestureEventV1* event);

int rin_i2c_hid_touchpad_driver_v1_configure_gesture(
    struct RinI2cHidTouchpadDriverV1* driver,
    const RinI2cHidTouchpadGesturePolicyV1* policy,
    RinI2cHidTouchpadGesturePublishV1 publish, void* publish_context);

#if defined(__cplusplus)
static_assert(sizeof(RinI2cHidTouchpadGestureEventV1) == 64u,
              "I2C-HID gesture event ABI drift");
static_assert(sizeof(RinI2cHidTouchpadGesturePolicyV1) == 48u,
              "I2C-HID gesture policy ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinI2cHidTouchpadGestureEventV1) == 64u,
               "I2C-HID gesture event ABI drift");
_Static_assert(sizeof(RinI2cHidTouchpadGesturePolicyV1) == 48u,
               "I2C-HID gesture policy ABI drift");
#endif

#endif /* RIN_DRIVER_BUS_I2C_GESTURE_V1_H */
