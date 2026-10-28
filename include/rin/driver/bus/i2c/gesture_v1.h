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

#define RIN_I2C_HID_TOUCHPAD_INPUT_SERVICE_OPS_V1_VERSION UINT32_C(0x00010000)

typedef int (*RinI2cHidTouchpadInputServiceValidV1)(void* context);
typedef RinResultCode (*RinI2cHidTouchpadInputServiceGetDeviceIdV1)(
    void* context, uint64_t* device_id_out);
typedef RinResultCode (*RinI2cHidTouchpadInputServiceConfigureV1)(
    void* context, const RinI2cHidTouchpadGesturePolicyV1* policy,
    RinI2cHidTouchpadGesturePublishV1 publish, void* publish_context);
typedef RinResultCode (*RinI2cHidTouchpadInputServiceLifecycleV1)(
    void* context);

/* Kernel input owners consume this bounded callback table instead of
 * including the concrete driver's private state header. context_size lets
 * the owner reject storage overlap before retaining the opaque context. */
typedef struct RinI2cHidTouchpadInputServiceOpsV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t context_size;
    uint32_t reserved0;
    RinI2cHidTouchpadInputServiceValidV1 is_valid;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t is_valid_padding;
#endif
    RinI2cHidTouchpadInputServiceGetDeviceIdV1 get_device_id;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t get_device_id_padding;
#endif
    RinI2cHidTouchpadInputServiceConfigureV1 configure;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t configure_padding;
#endif
    RinI2cHidTouchpadInputServiceLifecycleV1 suspend;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t suspend_padding;
#endif
    RinI2cHidTouchpadInputServiceLifecycleV1 resume;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t resume_padding;
#endif
    RinI2cHidTouchpadInputServiceLifecycleV1 detach;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t detach_padding;
#endif
    uint64_t reserved[2];
} RinI2cHidTouchpadInputServiceOpsV1;

static inline int rin_i2c_hid_touchpad_input_service_ops_v1_valid(
    const RinI2cHidTouchpadInputServiceOpsV1* ops)
{
    if (!ops || ops->struct_size != sizeof(*ops) ||
        ops->version != RIN_I2C_HID_TOUCHPAD_INPUT_SERVICE_OPS_V1_VERSION ||
        ops->context_size == 0u || ops->reserved0 != 0u || !ops->is_valid ||
        !ops->get_device_id || !ops->configure || !ops->suspend ||
        !ops->resume || !ops->detach || ops->reserved[0] != 0u ||
        ops->reserved[1] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (ops->is_valid_padding != 0u || ops->get_device_id_padding != 0u ||
        ops->configure_padding != 0u || ops->suspend_padding != 0u ||
        ops->resume_padding != 0u || ops->detach_padding != 0u)
        return 0;
#endif
    return 1;
}

#if defined(__cplusplus)
static_assert(sizeof(RinI2cHidTouchpadInputServiceOpsV1) == 80u,
              "I2C-HID input-service operations ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinI2cHidTouchpadInputServiceOpsV1) == 80u,
               "I2C-HID input-service operations ABI drift");
#endif

/* Return the input-service callbacks for one live driver context. The
 * caller copies the table and retains the opaque context only until detach. */
RinResultCode rin_i2c_hid_touchpad_driver_v1_get_input_service_ops(
    void* driver_context, RinI2cHidTouchpadInputServiceOpsV1* ops_out);

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
