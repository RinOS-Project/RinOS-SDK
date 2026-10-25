/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_KEYBOARD_LAYOUT_PREFERENCE_V1_H
#define RIN_SDK_KEYBOARD_LAYOUT_PREFERENCE_V1_H

#include <stddef.h>
#include <stdint.h>

#define RIN_KEYBOARD_LAYOUT_PREFERENCE_VERSION_V1 UINT16_C(1)

/* AUTO follows the active session's locale default. The explicit values are
 * independent of both locale and input-method selection. */
enum RinKeyboardLayoutPreferenceValueV1 {
    RIN_KEYBOARD_LAYOUT_PREFERENCE_AUTO = 0u,
    RIN_KEYBOARD_LAYOUT_PREFERENCE_US = 1u,
    RIN_KEYBOARD_LAYOUT_PREFERENCE_JIS = 2u
};

typedef struct RinKeyboardLayoutPreferenceV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t generation;
    uint32_t layout;
    uint64_t reserved[2];
} RinKeyboardLayoutPreferenceV1;

#if defined(__cplusplus)
static_assert(sizeof(RinKeyboardLayoutPreferenceV1) == 32u,
              "keyboard layout preference ABI drift");
static_assert(offsetof(RinKeyboardLayoutPreferenceV1, generation) == 8u,
              "keyboard layout generation offset drift");
static_assert(offsetof(RinKeyboardLayoutPreferenceV1, reserved) == 16u,
              "keyboard layout reserved offset drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinKeyboardLayoutPreferenceV1) == 32u,
               "keyboard layout preference ABI drift");
_Static_assert(offsetof(RinKeyboardLayoutPreferenceV1, generation) == 8u,
               "keyboard layout generation offset drift");
_Static_assert(offsetof(RinKeyboardLayoutPreferenceV1, reserved) == 16u,
               "keyboard layout reserved offset drift");
#endif

#endif /* RIN_SDK_KEYBOARD_LAYOUT_PREFERENCE_V1_H */
