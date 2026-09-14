/* SPDX-License-Identifier: MIT */
/* Public normalized window-event ABI. */
#ifndef RIN_SDK_GUI_EVENT_ABI_H
#define RIN_SDK_GUI_EVENT_ABI_H

#include <stdint.h>
#include "native_event.h"

#define RIN_WINDOW_EVENT_ABI_VERSION 1u

enum RinWindowEventType {
    RIN_WINDOW_EVENT_NONE = 0,
    RIN_WINDOW_EVENT_CLOSE = 1,
    RIN_WINDOW_EVENT_RESIZE = 2,
    RIN_WINDOW_EVENT_MOVE = 3,
    RIN_WINDOW_EVENT_FOCUS = 4,
    RIN_WINDOW_EVENT_BLUR = 5,
    RIN_WINDOW_EVENT_KEY_DOWN = 6,
    RIN_WINDOW_EVENT_KEY_UP = 7,
    RIN_WINDOW_EVENT_POINTER_MOVE = 8,
    RIN_WINDOW_EVENT_POINTER_BUTTON = 9,
    RIN_WINDOW_EVENT_SCROLL = 10,
    RIN_WINDOW_EVENT_HORIZONTAL_SCROLL = 11,
    RIN_WINDOW_EVENT_TEXT_INPUT = 12,
    RIN_WINDOW_EVENT_TEXT_COMPOSITION = 13,
    RIN_WINDOW_EVENT_SCALE_CHANGED = 14,
    RIN_WINDOW_EVENT_DISPLAY_CHANGED = 15,
};

#define RIN_WINDOW_EVENT_FLAG_NONE UINT32_C(0)
#define RIN_WINDOW_EVENT_FLAG_NATIVE UINT32_C(0x00000001)
#define RIN_WINDOW_EVENT_FLAG_KEY_REPEAT UINT32_C(0x00000002)
#define RIN_WINDOW_EVENT_FLAG_KNOWN_MASK \
    (RIN_WINDOW_EVENT_FLAG_NATIVE | RIN_WINDOW_EVENT_FLAG_KEY_REPEAT)

typedef struct RinWindowEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t type;
    uint32_t reserved0;
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t scale_numerator;
    uint32_t scale_denominator;
    RinGuiNativeEventV1 native;
    uint32_t reserved[3];
} RinWindowEventV1;

static inline int rin_window_event_valid(const RinWindowEventV1* value) {
    return value != 0 && value->struct_size == sizeof(*value) &&
           value->version == RIN_WINDOW_EVENT_ABI_VERSION &&
           value->reserved0 == 0u && value->reserved[0] == 0u &&
           value->reserved[1] == 0u && value->reserved[2] == 0u;
}

/* Translate the existing native queue ABI without losing the original event.
 * Resize/move/focus/display events are produced by the compositor as
 * normalized events; input events retain their native payload for clients
 * that need scancodes or pointer-button masks. */
static inline int rin_window_event_from_native(
    const RinGuiNativeEventV1* native, RinWindowEventV1* event_out) {
    uint32_t type;
    if (!native || !event_out || native->struct_size != sizeof(*native) ||
        native->version != RIN_GUI_NATIVE_EVENT_VERSION ||
        native->reserved[0] != 0u || native->reserved[1] != 0u)
        return 0;
#ifdef __cplusplus
    *event_out = RinWindowEventV1{};
#else
    *event_out = (RinWindowEventV1){0};
#endif
    event_out->struct_size = (uint32_t)sizeof(*event_out);
    event_out->version = RIN_WINDOW_EVENT_ABI_VERSION;
    event_out->flags = RIN_WINDOW_EVENT_FLAG_NATIVE;
    event_out->native = *native;
    type = native->type;
    switch (type) {
        case 2u:
            event_out->type = RIN_WINDOW_EVENT_CLOSE;
            return 1;
        case RIN_GUI_NATIVE_EVENT_KEY_DOWN:
            event_out->type = RIN_WINDOW_EVENT_KEY_DOWN;
            if ((native->flags & RIN_GUI_NATIVE_EVENT_FLAG_KEY_REPEAT) != 0u)
                event_out->flags |= RIN_WINDOW_EVENT_FLAG_KEY_REPEAT;
            return 1;
        case RIN_GUI_NATIVE_EVENT_KEY_UP:
            event_out->type = RIN_WINDOW_EVENT_KEY_UP;
            return 1;
        case RIN_GUI_NATIVE_EVENT_POINTER_MOVE:
            event_out->type = RIN_WINDOW_EVENT_POINTER_MOVE;
            event_out->x = native->screen_x;
            event_out->y = native->screen_y;
            return 1;
        case RIN_GUI_NATIVE_EVENT_POINTER_BUTTON:
            event_out->type = RIN_WINDOW_EVENT_POINTER_BUTTON;
            event_out->x = native->screen_x;
            event_out->y = native->screen_y;
            return 1;
        case RIN_GUI_NATIVE_EVENT_MOUSE_WHEEL:
            event_out->type = RIN_WINDOW_EVENT_SCROLL;
            event_out->x = native->screen_x;
            event_out->y = native->screen_y;
            return 1;
        case RIN_GUI_NATIVE_EVENT_MOUSE_HORIZONTAL_WHEEL:
            event_out->type = RIN_WINDOW_EVENT_HORIZONTAL_SCROLL;
            event_out->x = native->screen_x;
            event_out->y = native->screen_y;
            return 1;
        case 10u:
            event_out->type = RIN_WINDOW_EVENT_TEXT_INPUT;
            return 1;
        case 17u:
            event_out->type = RIN_WINDOW_EVENT_TEXT_COMPOSITION;
            return 1;
        default:
            /* Preserve an extension event for a newer client.  The native
             * payload remains available even when this ABI has no normalized
             * enum for it yet. */
            event_out->type = RIN_WINDOW_EVENT_NONE;
            return 1;
    }
}

#ifdef __cplusplus
static_assert(sizeof(RinWindowEventV1) == 88u,
              "RinWindowEventV1 ABI drift");
#else
_Static_assert(sizeof(RinWindowEventV1) == 88u,
               "RinWindowEventV1 ABI drift");
#endif

#endif /* RIN_SDK_GUI_EVENT_ABI_H */
