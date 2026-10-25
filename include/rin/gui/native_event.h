// SPDX-License-Identifier: MIT

#ifndef RIN_SDK_GUI_NATIVE_EVENT_H
#define RIN_SDK_GUI_NATIVE_EVENT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#define RIN_GUI_NATIVE_EVENT_VERSION 1u

#define RIN_GUI_NATIVE_EVENT_MOUSE_WHEEL 7u
#define RIN_GUI_NATIVE_EVENT_MOUSE_HORIZONTAL_WHEEL 12u
#define RIN_GUI_NATIVE_EVENT_POINTER_MOVE 13u
#define RIN_GUI_NATIVE_EVENT_POINTER_BUTTON 14u
#define RIN_GUI_NATIVE_EVENT_KEY_DOWN 15u
#define RIN_GUI_NATIVE_EVENT_KEY_UP 16u
#define RIN_GUI_NATIVE_EVENT_INPUT_DEVICE_RESET 19u

#define RIN_GUI_NATIVE_EVENT_FLAG_KEY_REPEAT 0x0001u
#define RIN_GUI_NATIVE_EVENT_FLAG_KNOWN_MASK 0x0001u
#define RIN_GUI_NATIVE_LEGACY_KEY_REPEAT 0x20u

#define RIN_GUI_NATIVE_KEY_SCANCODE_MASK 0x7fu
#define RIN_GUI_NATIVE_KEY_EXTENDED      0x80u
#define RIN_GUI_NATIVE_KEY_PAUSE         0xffu
#define RIN_GUI_NATIVE_KEY_DATA_MASK     0xffu

#define RIN_GUI_NATIVE_POINTER_ACTION_DOWN 0x0100u
#define RIN_GUI_NATIVE_POINTER_ACTION_UP   0x0200u
#define RIN_GUI_NATIVE_POINTER_ACTION_MASK 0x0300u

#define RIN_GUI_NATIVE_KEYMOD_SHIFT 0x01u
#define RIN_GUI_NATIVE_KEYMOD_CTRL 0x02u
#define RIN_GUI_NATIVE_KEYMOD_ALT 0x04u
#define RIN_GUI_NATIVE_KEYMOD_CAPS 0x08u
#define RIN_GUI_NATIVE_KEYMOD_SUPER 0x10u
#define RIN_GUI_NATIVE_KEYMOD_KNOWN_MASK 0x1fu

/* Per-seat modifier bookkeeping shared by kernel input projection and the
 * Compositor.  Set-1's high bit identifies E0 keys; Caps is a lock state. */
#define RIN_GUI_KEY_HELD_LSHIFT 0x0001u
#define RIN_GUI_KEY_HELD_RSHIFT 0x0002u
#define RIN_GUI_KEY_HELD_LCTRL  0x0004u
#define RIN_GUI_KEY_HELD_RCTRL  0x0008u
#define RIN_GUI_KEY_HELD_LALT   0x0010u
#define RIN_GUI_KEY_HELD_RALT   0x0020u
#define RIN_GUI_KEY_HELD_LSUPER 0x0040u
#define RIN_GUI_KEY_HELD_RSUPER 0x0080u
#define RIN_GUI_KEY_HELD_CAPS   0x0100u

typedef struct RinGuiKeyboardModifierStateV1 {
    uint8_t modifiers;
    uint8_t reserved;
    uint16_t held_keys;
} RinGuiKeyboardModifierStateV1;

static inline void rin_gui_keyboard_modifier_state_reset(
    RinGuiKeyboardModifierStateV1* state, int preserve_caps_lock) {
    if (!state) return;
    state->modifiers = preserve_caps_lock
        ? (uint8_t)(state->modifiers & RIN_GUI_NATIVE_KEYMOD_CAPS)
        : 0u;
    state->reserved = 0u;
    state->held_keys = 0u;
}

static inline void rin_gui_keyboard_modifier_state_update(
    RinGuiKeyboardModifierStateV1* state, uint8_t encoded_scancode,
    int pressed) {
    uint8_t scancode;
    int extended;
    uint16_t key = 0u;

    if (!state || (pressed != 0 && pressed != 1)) return;
    scancode = encoded_scancode & RIN_GUI_NATIVE_KEY_SCANCODE_MASK;
    extended = (encoded_scancode & RIN_GUI_NATIVE_KEY_EXTENDED) != 0u;
    if (!extended && scancode == 0x3au) {
        if (pressed) {
            if ((state->held_keys & RIN_GUI_KEY_HELD_CAPS) == 0u)
                state->modifiers ^= RIN_GUI_NATIVE_KEYMOD_CAPS;
            state->held_keys |= RIN_GUI_KEY_HELD_CAPS;
        } else {
            state->held_keys &= (uint16_t)~RIN_GUI_KEY_HELD_CAPS;
        }
        return;
    }
    switch (scancode) {
        case 0x2au: if (!extended) key = RIN_GUI_KEY_HELD_LSHIFT; break;
        case 0x36u: if (!extended) key = RIN_GUI_KEY_HELD_RSHIFT; break;
        case 0x1du:
            key = extended ? RIN_GUI_KEY_HELD_RCTRL
                           : RIN_GUI_KEY_HELD_LCTRL;
            break;
        case 0x38u:
            key = extended ? RIN_GUI_KEY_HELD_RALT
                           : RIN_GUI_KEY_HELD_LALT;
            break;
        case 0x5bu: if (extended) key = RIN_GUI_KEY_HELD_LSUPER; break;
        case 0x5cu: if (extended) key = RIN_GUI_KEY_HELD_RSUPER; break;
        default: break;
    }
    if (key == 0u) return;
    if (pressed) state->held_keys |= key;
    else state->held_keys &= (uint16_t)~key;
    state->modifiers &= RIN_GUI_NATIVE_KEYMOD_CAPS;
    if ((state->held_keys & (RIN_GUI_KEY_HELD_LSHIFT |
                             RIN_GUI_KEY_HELD_RSHIFT)) != 0u)
        state->modifiers |= RIN_GUI_NATIVE_KEYMOD_SHIFT;
    if ((state->held_keys & (RIN_GUI_KEY_HELD_LCTRL |
                             RIN_GUI_KEY_HELD_RCTRL)) != 0u)
        state->modifiers |= RIN_GUI_NATIVE_KEYMOD_CTRL;
    if ((state->held_keys & (RIN_GUI_KEY_HELD_LALT |
                             RIN_GUI_KEY_HELD_RALT)) != 0u)
        state->modifiers |= RIN_GUI_NATIVE_KEYMOD_ALT;
    if ((state->held_keys & (RIN_GUI_KEY_HELD_LSUPER |
                             RIN_GUI_KEY_HELD_RSUPER)) != 0u)
        state->modifiers |= RIN_GUI_NATIVE_KEYMOD_SUPER;
}

#define RIN_GUI_NATIVE_POINTER_LEFT 0x01u
#define RIN_GUI_NATIVE_POINTER_RIGHT 0x02u
#define RIN_GUI_NATIVE_POINTER_MIDDLE 0x04u
#define RIN_GUI_NATIVE_POINTER_SIDE 0x08u
#define RIN_GUI_NATIVE_POINTER_EXTRA 0x10u
#define RIN_GUI_NATIVE_POINTER_KNOWN_MASK 0x1fu

typedef struct RinGuiNativeEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t type;
    uint32_t data;
    int32_t screen_x;
    int32_t screen_y;
    int16_t wheel_delta;
    uint8_t pointer_buttons;
    uint8_t key_modifiers;
    uint32_t reserved[2];
} RinGuiNativeEventV1;

/* The legacy polling ABI stores an event kind in the high byte and up to 24
 * bits of payload below it.  Native64 exposes the same queue through the
 * versioned GUI call, so keep this decoder beside the wire definition rather
 * than duplicating fragile bit extraction in every userspace runtime. */
static inline int rin_gui_native_event_from_packed(
    uint32_t packed, RinGuiNativeEventV1* event_out) {
    uint32_t type;
    uint32_t data;

    if (event_out == 0 || packed == 0u) return 0;
    type = packed >> 24u;
    data = packed & 0x00ffffffu;
    event_out->struct_size = (uint32_t)sizeof(*event_out);
    event_out->version = RIN_GUI_NATIVE_EVENT_VERSION;
    event_out->flags = 0u;
    event_out->type = type;
    event_out->data = data;
    event_out->screen_x = 0;
    event_out->screen_y = 0;
    event_out->wheel_delta = 0;
    event_out->pointer_buttons = 0u;
    event_out->key_modifiers = 0u;
    event_out->reserved[0] = 0u;
    event_out->reserved[1] = 0u;

    switch (type) {
        case 1u:  /* legacy button click */
        case 2u:  /* close */
        case 9u:  /* locale changed */
        case 10u: /* text input */
        case 11u: /* input method changed */
        case 17u: /* text composition snapshot changed */
        case 18u: /* timezone changed */
            return 1;
        case RIN_GUI_NATIVE_EVENT_KEY_DOWN:
        case RIN_GUI_NATIVE_EVENT_KEY_UP:
            event_out->data = (data >> 8u) & RIN_GUI_NATIVE_KEY_DATA_MASK;
            event_out->key_modifiers =
                (uint8_t)((data >> 16u) & RIN_GUI_NATIVE_KEYMOD_KNOWN_MASK);
            if ((data & (RIN_GUI_NATIVE_LEGACY_KEY_REPEAT << 16u)) != 0u)
                event_out->flags |= RIN_GUI_NATIVE_EVENT_FLAG_KEY_REPEAT;
            return 1;
        case RIN_GUI_NATIVE_EVENT_MOUSE_WHEEL:
        case RIN_GUI_NATIVE_EVENT_MOUSE_HORIZONTAL_WHEEL:
            event_out->wheel_delta = (int16_t)(int8_t)(data & 0xffu);
            event_out->data = (uint32_t)(uint8_t)event_out->wheel_delta;
            return event_out->wheel_delta != 0 ? 1 : 0;
        case RIN_GUI_NATIVE_EVENT_POINTER_MOVE:
            event_out->screen_x = (int32_t)(data & 0x0fffu);
            event_out->screen_y = (int32_t)((data >> 12u) & 0x0fffu);
            event_out->data = 0u;
            return 1;
        case 3u: /* legacy left button down */
            event_out->type = RIN_GUI_NATIVE_EVENT_POINTER_BUTTON;
            event_out->data = RIN_GUI_NATIVE_POINTER_ACTION_DOWN |
                              RIN_GUI_NATIVE_POINTER_LEFT;
            event_out->screen_x = (int32_t)(data & 0x0fffu);
            event_out->screen_y = (int32_t)((data >> 12u) & 0x0fffu);
            event_out->pointer_buttons = RIN_GUI_NATIVE_POINTER_LEFT;
            return 1;
        case 4u: /* legacy left button up */
            event_out->type = RIN_GUI_NATIVE_EVENT_POINTER_BUTTON;
            event_out->data = RIN_GUI_NATIVE_POINTER_ACTION_UP |
                              RIN_GUI_NATIVE_POINTER_LEFT;
            event_out->screen_x = (int32_t)(data & 0x0fffu);
            event_out->screen_y = (int32_t)((data >> 12u) & 0x0fffu);
            return 1;
        case 8u: /* legacy right button down */
            event_out->type = RIN_GUI_NATIVE_EVENT_POINTER_BUTTON;
            event_out->data = RIN_GUI_NATIVE_POINTER_ACTION_DOWN |
                              RIN_GUI_NATIVE_POINTER_RIGHT;
            event_out->screen_x = (int32_t)(data & 0x0fffu);
            event_out->screen_y = (int32_t)((data >> 12u) & 0x0fffu);
            event_out->pointer_buttons = RIN_GUI_NATIVE_POINTER_RIGHT;
            return 1;
        default:
            return 0;
    }
}

#ifdef __cplusplus
static_assert(sizeof(RinGuiNativeEventV1) == 36u,
              "native GUI event v1 layout drift");
#else
_Static_assert(sizeof(RinGuiNativeEventV1) == 36u,
               "native GUI event v1 layout drift");
#endif

#ifdef __cplusplus
}
#endif

#endif
