// SPDX-License-Identifier: MIT

#ifndef RIN_DRIVER_PS2_SET1_DECODER_H
#define RIN_DRIVER_PS2_SET1_DECODER_H

#define RIN_PS2_SET1_DECODER_API_VERSION 1u

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_PS2_SET1_NO_EVENT 0
#define RIN_PS2_SET1_EVENT 1
#define RIN_PS2_SET1_INVALID (-1)

#define RIN_PS2_SET1_KEY_DOWN 1u
#define RIN_PS2_SET1_KEY_UP 2u
#define RIN_PS2_SET1_KEY_REPEAT 3u

#define RIN_PS2_SET1_EXTENDED 0x80u
#define RIN_PS2_SET1_PAUSE 0xffu

static inline int rin_ps2_set1_repeat_base_supported_v1(uint8_t scancode)
{
    return (scancode >= 0x01u && scancode <= 0x53u) ||
           scancode == 0x56u || scancode == 0x57u || scancode == 0x58u ||
           scancode == 0x70u || scancode == 0x73u ||
           scancode == 0x78u || scancode == 0x79u ||
           scancode == 0x7bu || scancode == 0x7du;
}

static inline int rin_ps2_set1_repeat_extended_supported_v1(
    uint8_t scancode)
{
    switch (scancode) {
        case 0x1cu:
        case 0x1du:
        case 0x35u:
        case 0x38u:
        case 0x47u:
        case 0x48u:
        case 0x49u:
        case 0x4bu:
        case 0x4du:
        case 0x4fu:
        case 0x50u:
        case 0x51u:
        case 0x52u:
        case 0x53u:
        case 0x5bu:
        case 0x5cu:
        case 0x5du:
            return 1;
        default:
            return 0;
    }
}

static inline int rin_ps2_set1_repeat_eligible_v1(uint8_t encoded_scancode)
{
    uint8_t scancode;
    int extended;

    if (encoded_scancode == RIN_PS2_SET1_PAUSE) return 0;
    extended = (encoded_scancode & RIN_PS2_SET1_EXTENDED) != 0u;
    scancode = encoded_scancode & 0x7fu;
    if (extended) {
        if (scancode == 0x37u ||
            !rin_ps2_set1_repeat_extended_supported_v1(scancode)) {
            return 0;
        }
        return scancode != 0x1du && scancode != 0x38u &&
               scancode != 0x5bu && scancode != 0x5cu &&
               scancode != 0x5du;
    }
    if (!rin_ps2_set1_repeat_base_supported_v1(scancode)) return 0;
    switch (scancode) {
        case 0x1du:
        case 0x2au:
        case 0x36u:
        case 0x38u:
        case 0x3au:
        case 0x45u:
        case 0x46u:
        case 0x70u:
        case 0x73u:
        case 0x78u:
        case 0x79u:
        case 0x7bu:
        case 0x7du:
            return 0;
        default:
            return 1;
    }
}

typedef struct RinPs2Set1DecoderV1 {
    uint8_t phase;
    uint8_t pause_index;
    uint8_t reserved[2];
    uint8_t held[32];
} RinPs2Set1DecoderV1;

typedef struct RinPs2Set1KeyEventV1 {
    uint8_t action;
    uint8_t encoded_scancode;
    uint8_t reserved[2];
} RinPs2Set1KeyEventV1;

void rin_ps2_set1_decoder_init(RinPs2Set1DecoderV1* decoder);
int rin_ps2_set1_decoder_feed(RinPs2Set1DecoderV1* decoder,
                              uint8_t byte,
                              RinPs2Set1KeyEventV1* event_out);
int rin_ps2_set1_decoder_feed_raw(RinPs2Set1DecoderV1* decoder,
                                  uint8_t byte,
                                  RinPs2Set1KeyEventV1* event_out);
int rin_ps2_set1_repeat_eligible(uint8_t encoded_scancode);
char rin_ps2_set1_ascii_us(uint8_t encoded_scancode,
                           int shift, int caps_lock);

#ifdef __cplusplus
}
#endif

#endif
