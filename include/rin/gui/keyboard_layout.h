/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_GUI_KEYBOARD_LAYOUT_H
#define RIN_SDK_GUI_KEYBOARD_LAYOUT_H

#include <stdint.h>
#include "native_event.h"

#define RIN_GUI_KEYBOARD_LAYOUT_US 0u
#define RIN_GUI_KEYBOARD_LAYOUT_JIS 1u

/* ASCII compatibility projection for physical Set-1 keys. Japanese text
 * composition remains the responsibility of the active input method. */
static inline int rin_gui_keyboard_jis_override(uint8_t scancode,
                                                int shifted) {
    if (shifted) {
        switch (scancode) {
            case 0x03: return '"';
            case 0x07: return '&';
            case 0x08: return '\'';
            case 0x09: return '(';
            case 0x0a: return ')';
            case 0x0b: return 0;
            case 0x0c: return '=';
            case 0x0d: return '~';
            case 0x1a: return '`';
            case 0x1b: return '{';
            case 0x27: return '+';
            case 0x28: return '*';
            case 0x29: return 0;
            case 0x2b: return '}';
            case 0x73: return '_';
            case 0x7d: return '|';
            default: return -1;
        }
    }
    switch (scancode) {
        case 0x0d: return '^';
        case 0x1a: return '@';
        case 0x1b: return '[';
        case 0x27: return ';';
        case 0x28: return ':';
        case 0x29: return 0;
        case 0x2b: return ']';
        case 0x73: return '\\';
        case 0x7d: return '\\';
        default: return -1;
    }
}

static inline uint8_t rin_gui_keyboard_key_to_ascii(
    uint8_t encoded_scancode, uint8_t modifiers, uint8_t layout) {
    static const uint8_t unshifted[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
        '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',
        ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
        '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.',
        '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    static const uint8_t shifted_map[128] = {
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
        '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{',
        '}', '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
        '"', '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>',
        '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    uint8_t scancode;
    uint8_t base;
    int shifted;
    int override;

    /* E0-extended keys must never alias a printable base-key position. */
    if (encoded_scancode >= 128u ||
        (layout != RIN_GUI_KEYBOARD_LAYOUT_US &&
         layout != RIN_GUI_KEYBOARD_LAYOUT_JIS))
        return 0u;
    scancode = encoded_scancode;
    shifted = (modifiers & RIN_GUI_NATIVE_KEYMOD_SHIFT) != 0u;
    base = unshifted[scancode];
    if (base >= 'a' && base <= 'z') {
        const int caps = (modifiers & RIN_GUI_NATIVE_KEYMOD_CAPS) != 0u;
        return (uint8_t)((shifted != caps) ? base - 'a' + 'A' : base);
    }
    if (layout == RIN_GUI_KEYBOARD_LAYOUT_JIS) {
        override = rin_gui_keyboard_jis_override(scancode, shifted);
        if (override >= 0) return (uint8_t)override;
    }
    return shifted ? shifted_map[scancode] : base;
}

#endif
