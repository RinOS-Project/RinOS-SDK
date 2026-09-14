/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_NET_IPV6_TEXT_H
#define RIN_SDK_NET_IPV6_TEXT_H

/* This header is shared by hosted consumers and RinOS libc itself.  When a
 * libc header includes it by relative path, do not resolve the angle-bracket
 * includes back to the host SDK after RinOS has already supplied the types. */
#ifndef _STDDEF_H
#include <stddef.h>
#endif
#ifndef _STDINT_H
#include <stdint.h>
#endif

#define RIN_IPV6_ADDRESS_BYTES 16u
#define RIN_IPV6_TEXT_MAX 46u

static inline void rin_ipv6_text_zero(uint8_t address[16])
{
    size_t index;
    if (!address) return;
    for (index = 0u; index < RIN_IPV6_ADDRESS_BYTES; ++index) {
        address[index] = 0u;
    }
}

static inline int rin_ipv6_text_hex_value(char character)
{
    if (character >= '0' && character <= '9') {
        return (int)(character - '0');
    }
    if (character >= 'a' && character <= 'f') {
        return (int)(character - 'a') + 10;
    }
    if (character >= 'A' && character <= 'F') {
        return (int)(character - 'A') + 10;
    }
    return -1;
}

static inline int rin_ipv6_text_parse_ipv4_tail(const char* text,
                                                 const char* end,
                                                 uint8_t output[4])
{
    unsigned int octet = 0u;
    unsigned int value = 0u;
    unsigned int digits = 0u;
    int leading_zero = 0;
    const char* cursor;
    if (!text || !end || !output || text >= end) return -1;
    for (cursor = text; cursor < end; ++cursor) {
        if (*cursor >= '0' && *cursor <= '9') {
            unsigned int digit = (unsigned int)(*cursor - '0');
            if ((digits != 0u && leading_zero) || digits >= 3u || value > 25u ||
                (value == 25u && digit > 5u)) {
                return -1;
            }
            if (digits == 0u) leading_zero = digit == 0u;
            value = value * 10u + digit;
            ++digits;
        } else if (*cursor == '.') {
            if (octet >= 3u || digits == 0u ||
                (digits > 1u && value < 10u)) {
                return -1;
            }
            output[octet++] = (uint8_t)value;
            value = 0u;
            digits = 0u;
            leading_zero = 0;
        } else {
            return -1;
        }
    }
    if (octet != 3u || digits == 0u || (digits > 1u && value < 10u)) {
        return -1;
    }
    output[3] = (uint8_t)value;
    return 0;
}

/* Parse an RFC 4291 IPv6 literal. One :: compression marker and a final
 * dotted-quad tail are accepted. Zone identifiers are deliberately rejected:
 * callers must carry scope IDs separately from the address bytes. */
static inline int rin_ipv6_text_parse(const char* text, uint8_t output[16])
{
    uint16_t words[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
    unsigned int word_count = 0u;
    int compression = -1;
    const char* cursor = text;
    unsigned int index;

    rin_ipv6_text_zero(output);
    if (!text || !output || *text == '\0') return -1;

    if (cursor[0] == ':' && cursor[1] == ':') {
        compression = 0;
        cursor += 2;
        if (*cursor == '\0') return 0;
    } else if (*cursor == ':') {
        return -1;
    }

    while (*cursor != '\0') {
        const char* token = cursor;
        const char* end = cursor;
        uint32_t value = 0u;
        unsigned int digits = 0u;
        int dotted = 0;

        if (word_count >= 8u) goto fail;
        while (*end != '\0' && *end != ':') {
            if (*end == '.') dotted = 1;
            ++end;
        }
        if (end == token) goto fail;

        if (dotted) {
            uint8_t ipv4[4];
            if (*end != '\0' || word_count > 6u ||
                rin_ipv6_text_parse_ipv4_tail(token, end, ipv4) != 0) {
                goto fail;
            }
            words[word_count++] =
                (uint16_t)(((uint16_t)ipv4[0] << 8) | ipv4[1]);
            words[word_count++] =
                (uint16_t)(((uint16_t)ipv4[2] << 8) | ipv4[3]);
            break;
        }

        while (token < end) {
            int hex = rin_ipv6_text_hex_value(*token++);
            if (hex < 0 || ++digits > 4u) goto fail;
            value = (value << 4) | (uint32_t)hex;
        }
        words[word_count++] = (uint16_t)value;
        cursor = end;
        if (*cursor == '\0') break;

        ++cursor;
        if (*cursor == ':') {
            if (compression >= 0) goto fail;
            compression = (int)word_count;
            ++cursor;
            if (*cursor == '\0') break;
        } else if (*cursor == '\0') {
            goto fail;
        }
    }

    if (compression >= 0) {
        unsigned int compressed = (unsigned int)compression;
        unsigned int suffix = word_count - compressed;
        unsigned int zeros;
        if (word_count >= 8u) goto fail;
        zeros = 8u - word_count;
        for (index = 0u; index < suffix; ++index) {
            words[7u - index] = words[word_count - 1u - index];
        }
        for (index = 0u; index < zeros; ++index) {
            words[compressed + index] = 0u;
        }
    } else if (word_count != 8u) {
        goto fail;
    }

    for (index = 0u; index < 8u; ++index) {
        output[index * 2u] = (uint8_t)(words[index] >> 8);
        output[index * 2u + 1u] = (uint8_t)words[index];
    }
    return 0;

fail:
    rin_ipv6_text_zero(output);
    return -1;
}

static inline int rin_ipv6_text_append(char* output, size_t capacity,
                                       size_t* size, char character)
{
    if (!output || !size || *size + 1u >= capacity) return -1;
    output[(*size)++] = character;
    return 0;
}

/* Format in RFC 5952 form: lowercase hexadecimal, no leading zeroes, and the
 * first longest run of at least two zero words compressed with ::. */
static inline int rin_ipv6_text_format(const uint8_t address[16],
                                       char* output, size_t capacity)
{
    static const char hexadecimal[] = "0123456789abcdef";
    uint16_t words[8];
    unsigned int best_start = 8u;
    unsigned int best_length = 0u;
    unsigned int index;
    size_t size = 0u;

    if (output && capacity != 0u) output[0] = '\0';
    if (!address || !output || capacity == 0u) return -1;
    for (index = 0u; index < 8u; ++index) {
        words[index] = (uint16_t)(((uint16_t)address[index * 2u] << 8) |
                                  address[index * 2u + 1u]);
    }
    for (index = 0u; index < 8u;) {
        unsigned int end;
        if (words[index] != 0u) {
            ++index;
            continue;
        }
        end = index;
        while (end < 8u && words[end] == 0u) ++end;
        if (end - index >= 2u && end - index > best_length) {
            best_start = index;
            best_length = end - index;
        }
        index = end;
    }

    for (index = 0u; index < 8u;) {
        uint16_t value;
        int shift;
        int emitted = 0;
        if (index == best_start) {
            if (rin_ipv6_text_append(output, capacity, &size, ':') != 0 ||
                rin_ipv6_text_append(output, capacity, &size, ':') != 0) {
                goto fail;
            }
            index += best_length;
            continue;
        }
        if (size != 0u && output[size - 1u] != ':' &&
            rin_ipv6_text_append(output, capacity, &size, ':') != 0) {
            goto fail;
        }
        value = words[index++];
        for (shift = 12; shift >= 0; shift -= 4) {
            unsigned int digit = (unsigned int)((value >> shift) & 0x0fu);
            if (digit != 0u || emitted || shift == 0) {
                if (rin_ipv6_text_append(output, capacity, &size,
                                         hexadecimal[digit]) != 0) {
                    goto fail;
                }
                emitted = 1;
            }
        }
    }
    output[size] = '\0';
    return 0;

fail:
    output[0] = '\0';
    return -1;
}

#endif /* RIN_SDK_NET_IPV6_TEXT_H */
