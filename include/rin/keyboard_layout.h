/* SPDX-License-Identifier: MIT */
/* Typed helpers for the per-session keyboard-layout preference syscalls. */
#ifndef RIN_SDK_KEYBOARD_LAYOUT_H
#define RIN_SDK_KEYBOARD_LAYOUT_H

#include <rin/input/keyboard_layout_preference_v1.h>
#include <rin/syscall_abi.h>
#include <sys/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int rin_keyboard_layout_preference_get(
    RinKeyboardLayoutPreferenceV1* preference)
{
    if (preference == 0) return (int)RIN_RESULT_INVALID_ARGUMENT;
    return (int)syscall(RIN_SYS_KEYBOARD_LAYOUT_GET, preference);
}

static inline int rin_keyboard_layout_preference_set(
    const RinKeyboardLayoutPreferenceV1* preference)
{
    if (preference == 0) return (int)RIN_RESULT_INVALID_ARGUMENT;
    return (int)syscall(RIN_SYS_KEYBOARD_LAYOUT_SET, preference);
}

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_KEYBOARD_LAYOUT_H */
