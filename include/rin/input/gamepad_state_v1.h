/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_INPUT_GAMEPAD_STATE_V1_H
#define RIN_INPUT_GAMEPAD_STATE_V1_H

#include <stdint.h>

#define RIN_GAMEPAD_STATE_VERSION 1u
#define RIN_GAMEPAD_STATE_FLAG_CONNECTED UINT32_C(1)

/* Stable, transport-neutral snapshot shared by native input providers and
 * compatibility runtimes. Device identifiers are boot-local; generation and
 * sample sequence prevent a recycled device from appearing continuous. */
typedef struct RinGamepadStateV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t user_index;
    uint32_t flags;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t sample_sequence;
    uint16_t buttons;
    uint8_t left_trigger;
    uint8_t right_trigger;
    int16_t thumb_lx;
    int16_t thumb_ly;
    int16_t thumb_rx;
    int16_t thumb_ry;
    uint32_t reserved;
    uint64_t reserved2;
} RinGamepadStateV1;

#if defined(__cplusplus)
static_assert(sizeof(RinGamepadStateV1) == 64u,
              "gamepad state ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGamepadStateV1) == 64u,
               "gamepad state ABI drift");
#endif

#endif /* RIN_INPUT_GAMEPAD_STATE_V1_H */
