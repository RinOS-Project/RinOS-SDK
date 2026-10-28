// SPDX-License-Identifier: MIT

#include <rin/driver/ps2/set1_decoder.h>

#define RIN_PS2_PHASE_BASE 0u
#define RIN_PS2_PHASE_E0 1u
#define RIN_PS2_PHASE_PRINT_MAKE_E0 2u
#define RIN_PS2_PHASE_PRINT_MAKE_CODE 3u
#define RIN_PS2_PHASE_PRINT_BREAK_E0 4u
#define RIN_PS2_PHASE_PRINT_BREAK_CODE 5u
#define RIN_PS2_PHASE_E1_PAUSE 6u

_Static_assert(sizeof(RinPs2Set1DecoderV1) == 36u,
               "PS/2 Set-1 decoder layout drift");
_Static_assert(sizeof(RinPs2Set1KeyEventV1) == 4u,
               "PS/2 Set-1 key event layout drift");

static const char rin_ps2_set1_ascii_base[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
    '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',
    ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
    '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0,
};

static const char rin_ps2_set1_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+',
    '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{',
    '}', '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',
    '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0,
};

static void rin_ps2_set1_zero(void* destination, uint32_t size) {
    volatile uint8_t* bytes = (volatile uint8_t*)destination;
    uint32_t index;
    for (index = 0u; index < size; ++index) bytes[index] = 0u;
}

static int rin_ps2_set1_base_supported(uint8_t scancode) {
    return (scancode >= 0x01u && scancode <= 0x53u) ||
           scancode == 0x57u || scancode == 0x58u ||
           scancode == 0x70u || scancode == 0x73u ||
           scancode == 0x78u || scancode == 0x79u ||
           scancode == 0x7bu || scancode == 0x7du;
}

static int rin_ps2_set1_extended_supported(uint8_t scancode) {
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

int rin_ps2_set1_repeat_eligible(uint8_t encoded_scancode) {
    return rin_ps2_set1_repeat_eligible_v1(encoded_scancode);
}

char rin_ps2_set1_ascii_us(uint8_t encoded_scancode,
                           int shift, int caps_lock) {
    uint8_t scancode;
    char ascii;
    if (encoded_scancode == RIN_PS2_SET1_PAUSE ||
        (encoded_scancode & RIN_PS2_SET1_EXTENDED) != 0u ||
        (shift != 0 && shift != 1) ||
        (caps_lock != 0 && caps_lock != 1)) {
        return 0;
    }
    scancode = encoded_scancode & 0x7fu;
    if (scancode >= sizeof(rin_ps2_set1_ascii_base)) return 0;
    ascii = shift ? rin_ps2_set1_ascii_shift[scancode]
                  : rin_ps2_set1_ascii_base[scancode];
    if (caps_lock && ascii >= 'a' && ascii <= 'z')
        ascii = (char)(ascii - 'a' + 'A');
    else if (caps_lock && ascii >= 'A' && ascii <= 'Z')
        ascii = (char)(ascii - 'A' + 'a');
    return ascii;
}

static void rin_ps2_set1_reset(RinPs2Set1DecoderV1* decoder) {
    rin_ps2_set1_zero(decoder, sizeof(*decoder));
}

static void rin_ps2_set1_sequence_reset(RinPs2Set1DecoderV1* decoder) {
    decoder->phase = RIN_PS2_PHASE_BASE;
    decoder->pause_index = 0u;
    decoder->reserved[0] = 0u;
    decoder->reserved[1] = 0u;
}

static int rin_ps2_set1_is_held(const RinPs2Set1DecoderV1* decoder,
                                uint8_t encoded_scancode) {
    uint8_t mask = (uint8_t)(1u << (encoded_scancode & 7u));
    return (decoder->held[encoded_scancode >> 3u] & mask) != 0u;
}

static void rin_ps2_set1_set_held(RinPs2Set1DecoderV1* decoder,
                                  uint8_t encoded_scancode, int held) {
    uint8_t* slot = &decoder->held[encoded_scancode >> 3u];
    uint8_t mask = (uint8_t)(1u << (encoded_scancode & 7u));
    if (held)
        *slot = (uint8_t)(*slot | mask);
    else
        *slot = (uint8_t)(*slot & (uint8_t)~mask);
}

static int rin_ps2_set1_emit(RinPs2Set1DecoderV1* decoder,
                             RinPs2Set1KeyEventV1* event_out,
                             uint8_t action, uint8_t scancode,
                             int report_repeats) {
    rin_ps2_set1_sequence_reset(decoder);
    if (scancode != RIN_PS2_SET1_PAUSE) {
        if (action == RIN_PS2_SET1_KEY_DOWN &&
            rin_ps2_set1_is_held(decoder, scancode)) {
            if (!report_repeats ||
                !rin_ps2_set1_repeat_eligible(scancode)) {
                return RIN_PS2_SET1_NO_EVENT;
            }
            action = RIN_PS2_SET1_KEY_REPEAT;
        } else {
            rin_ps2_set1_set_held(
                decoder, scancode, action == RIN_PS2_SET1_KEY_DOWN);
        }
    }
    event_out->action = action;
    event_out->encoded_scancode = scancode;
    return RIN_PS2_SET1_EVENT;
}

static int rin_ps2_set1_reject(RinPs2Set1DecoderV1* decoder) {
    rin_ps2_set1_sequence_reset(decoder);
    return RIN_PS2_SET1_INVALID;
}

void rin_ps2_set1_decoder_init(RinPs2Set1DecoderV1* decoder) {
    if (decoder) rin_ps2_set1_reset(decoder);
}

static int rin_ps2_set1_decoder_feed_mode(
    RinPs2Set1DecoderV1* decoder, uint8_t byte,
    RinPs2Set1KeyEventV1* event_out, int report_repeats) {
    static const uint8_t pause_tail[5] = {
        0x1du, 0x45u, 0xe1u, 0x9du, 0xc5u,
    };
    uint8_t scancode;
    uint8_t action;

    if (event_out) rin_ps2_set1_zero(event_out, sizeof(*event_out));
    if (!decoder || !event_out || decoder->reserved[0] != 0u ||
        decoder->reserved[1] != 0u ||
        decoder->phase > RIN_PS2_PHASE_E1_PAUSE ||
        (decoder->phase == RIN_PS2_PHASE_E1_PAUSE &&
         decoder->pause_index >= sizeof(pause_tail))) {
        if (decoder) rin_ps2_set1_reset(decoder);
        return RIN_PS2_SET1_INVALID;
    }

    if (decoder->phase == RIN_PS2_PHASE_E1_PAUSE) {
        if (byte != pause_tail[decoder->pause_index])
            return rin_ps2_set1_reject(decoder);
        ++decoder->pause_index;
        if (decoder->pause_index != sizeof(pause_tail))
            return RIN_PS2_SET1_NO_EVENT;
        return rin_ps2_set1_emit(decoder, event_out,
                                 RIN_PS2_SET1_KEY_DOWN,
                                 RIN_PS2_SET1_PAUSE,
                                 report_repeats);
    }

    if (decoder->phase == RIN_PS2_PHASE_PRINT_MAKE_E0 ||
        decoder->phase == RIN_PS2_PHASE_PRINT_BREAK_E0) {
        if (byte != 0xe0u) return rin_ps2_set1_reject(decoder);
        decoder->phase =
            decoder->phase == RIN_PS2_PHASE_PRINT_MAKE_E0
                ? RIN_PS2_PHASE_PRINT_MAKE_CODE
                : RIN_PS2_PHASE_PRINT_BREAK_CODE;
        return RIN_PS2_SET1_NO_EVENT;
    }
    if (decoder->phase == RIN_PS2_PHASE_PRINT_MAKE_CODE) {
        if (byte != 0x37u) return rin_ps2_set1_reject(decoder);
        return rin_ps2_set1_emit(
            decoder, event_out, RIN_PS2_SET1_KEY_DOWN,
            RIN_PS2_SET1_EXTENDED | 0x37u, report_repeats);
    }
    if (decoder->phase == RIN_PS2_PHASE_PRINT_BREAK_CODE) {
        if (byte != 0xaau) return rin_ps2_set1_reject(decoder);
        return rin_ps2_set1_emit(
            decoder, event_out, RIN_PS2_SET1_KEY_UP,
            RIN_PS2_SET1_EXTENDED | 0x37u, report_repeats);
    }

    if (decoder->phase == RIN_PS2_PHASE_E0) {
        if (byte == 0x2au) {
            decoder->phase = RIN_PS2_PHASE_PRINT_MAKE_E0;
            return RIN_PS2_SET1_NO_EVENT;
        }
        if (byte == 0xb7u) {
            decoder->phase = RIN_PS2_PHASE_PRINT_BREAK_E0;
            return RIN_PS2_SET1_NO_EVENT;
        }
        scancode = byte & 0x7fu;
        if (!rin_ps2_set1_extended_supported(scancode))
            return rin_ps2_set1_reject(decoder);
        action = (byte & 0x80u) != 0u ? RIN_PS2_SET1_KEY_UP
                                      : RIN_PS2_SET1_KEY_DOWN;
        return rin_ps2_set1_emit(
            decoder, event_out, action,
            RIN_PS2_SET1_EXTENDED | scancode, report_repeats);
    }

    if (byte == 0xe0u) {
        decoder->phase = RIN_PS2_PHASE_E0;
        return RIN_PS2_SET1_NO_EVENT;
    }
    if (byte == 0xe1u) {
        decoder->phase = RIN_PS2_PHASE_E1_PAUSE;
        decoder->pause_index = 0u;
        return RIN_PS2_SET1_NO_EVENT;
    }

    scancode = byte & 0x7fu;
    if (!rin_ps2_set1_base_supported(scancode) || byte == 0xffu)
        return rin_ps2_set1_reject(decoder);
    action = (byte & 0x80u) != 0u ? RIN_PS2_SET1_KEY_UP
                                  : RIN_PS2_SET1_KEY_DOWN;
    return rin_ps2_set1_emit(
        decoder, event_out, action, scancode, report_repeats);
}

int rin_ps2_set1_decoder_feed(RinPs2Set1DecoderV1* decoder,
                              uint8_t byte,
                              RinPs2Set1KeyEventV1* event_out) {
    return rin_ps2_set1_decoder_feed_mode(
        decoder, byte, event_out, 0);
}

int rin_ps2_set1_decoder_feed_raw(RinPs2Set1DecoderV1* decoder,
                                  uint8_t byte,
                                  RinPs2Set1KeyEventV1* event_out) {
    return rin_ps2_set1_decoder_feed_mode(
        decoder, byte, event_out, 1);
}
