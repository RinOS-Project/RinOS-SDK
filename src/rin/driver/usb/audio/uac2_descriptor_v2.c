/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "uac2_descriptor_v2.h"

#define USB_DESCRIPTOR_CONFIGURATION 2u
#define USB_DESCRIPTOR_INTERFACE 4u
#define USB_DESCRIPTOR_ENDPOINT 5u
#define USB_DESCRIPTOR_CS_INTERFACE 0x24u
#define USB_DESCRIPTOR_CS_ENDPOINT 0x25u
#define USB_CLASS_AUDIO 0x01u
#define USB_AUDIO_SUBCLASS_CONTROL 0x01u
#define USB_AUDIO_SUBCLASS_STREAMING 0x02u
#define USB_AUDIO_PROTOCOL_UAC2 0x20u
#define UAC2_AC_HEADER 0x01u
#define UAC2_AC_INPUT_TERMINAL 0x02u
#define UAC2_AC_OUTPUT_TERMINAL 0x03u
#define UAC2_AC_CLOCK_SOURCE 0x0au
#define UAC2_AS_GENERAL 0x01u
#define UAC2_AS_FORMAT_TYPE 0x02u
#define UAC2_FORMAT_TYPE_I 0x01u
#define UAC2_FORMAT_PCM UINT32_C(0x00000001)
#define UAC2_TERMINAL_USB_STREAMING 0x0101u
#define UAC2_EP_GENERAL 0x01u
#define USB_ENDPOINT_DIRECTION_IN 0x80u
#define USB_ENDPOINT_NUMBER_MASK 0x0fu
#define USB_ENDPOINT_TYPE_MASK 0x03u
#define USB_ENDPOINT_ISOCHRONOUS 0x01u
#define USB_ENDPOINT_USAGE_MASK 0x30u
#define USB_ENDPOINT_USAGE_DATA 0x00u
#define USB_ENDPOINT_USAGE_FEEDBACK 0x10u

typedef struct Uac2Graph {
    uint8_t clock_ids[32];
    uint8_t clock_attributes[32];
    uint8_t clock_controls[32];
    uint8_t terminal_ids[32];
    uint16_t terminal_types[32];
    uint8_t terminal_clocks[32];
    uint8_t clock_count;
    uint8_t terminal_count;
    uint8_t control_found;
    uint8_t control_interface;
} Uac2Graph;

static void uac2_zero(void* pointer, uint32_t size) {
    volatile uint8_t* output = (volatile uint8_t*)pointer;
    while (size-- != 0u) *output++ = 0u;
}

static void uac2_copy(void* destination, const void* source, uint32_t size) {
    volatile uint8_t* output = (volatile uint8_t*)destination;
    const volatile uint8_t* input = (const volatile uint8_t*)source;
    while (size-- != 0u) *output++ = *input++;
}

static uint16_t uac2_read_u16(const uint8_t* input) {
    return (uint16_t)((uint16_t)input[0] | ((uint16_t)input[1] << 8u));
}

static uint32_t uac2_read_u24(const uint8_t* input) {
    return (uint32_t)input[0] | ((uint32_t)input[1] << 8u) |
           ((uint32_t)input[2] << 16u);
}

static uint32_t uac2_read_u32(const uint8_t* input) {
    return (uint32_t)input[0] | ((uint32_t)input[1] << 8u) |
           ((uint32_t)input[2] << 16u) | ((uint32_t)input[3] << 24u);
}

static int uac2_ranges_overlap(const void* first, uint32_t first_size,
                               const void* second, uint32_t second_size) {
    uintptr_t first_begin = (uintptr_t)first;
    uintptr_t second_begin = (uintptr_t)second;
    uintptr_t first_end;
    uintptr_t second_end;
    if (first_size == 0u || second_size == 0u) return 0;
    first_end = first_begin + (uintptr_t)first_size;
    second_end = second_begin + (uintptr_t)second_size;
    if (first_end < first_begin || second_end < second_begin) return 1;
    return first_begin < second_end && second_begin < first_end;
}

static int uac2_find_clock(const Uac2Graph* graph, uint8_t id) {
    uint32_t index;
    if (!graph || id == 0u) return -1;
    for (index = 0u; index < graph->clock_count; ++index)
        if (graph->clock_ids[index] == id) return (int)index;
    return -1;
}

static int uac2_find_terminal(const Uac2Graph* graph, uint8_t id) {
    uint32_t index;
    if (!graph || id == 0u) return -1;
    for (index = 0u; index < graph->terminal_count; ++index)
        if (graph->terminal_ids[index] == id) return (int)index;
    return -1;
}

static RinUsbAudioUac2Result uac2_validate_shape(
    const uint8_t* descriptor, uint32_t size) {
    uint16_t interface_keys[RIN_USB_AUDIO_UAC2_MAX_INTERFACE_DESCRIPTORS];
    uint32_t interface_count = 0u;
    uint32_t declared_interfaces;
    uint32_t distinct_interfaces = 0u;
    uint32_t offset;
    uint32_t current_endpoint_count = 0u;
    uint32_t declared_endpoint_count = 0u;
    uint32_t endpoint_addresses = 0u;
    uint8_t interface_numbers[32];
    int have_interface = 0;

    if (!descriptor || size < 9u ||
        size > RIN_USB_AUDIO_UAC2_MAX_CONFIGURATION_BYTES ||
        descriptor[0] != 9u || descriptor[1] != USB_DESCRIPTOR_CONFIGURATION ||
        uac2_read_u16(descriptor + 2u) != size || descriptor[4] == 0u ||
        descriptor[4] > 32u || descriptor[5] == 0u) {
        return size > RIN_USB_AUDIO_UAC2_MAX_CONFIGURATION_BYTES
            ? RIN_USB_AUDIO_UAC2_LIMIT_EXCEEDED
            : RIN_USB_AUDIO_UAC2_MALFORMED;
    }
    declared_interfaces = descriptor[4];
    uac2_zero(interface_numbers, sizeof(interface_numbers));
    offset = 9u;
    while (offset < size) {
        uint32_t length;
        uint8_t type;
        if (size - offset < 2u) return RIN_USB_AUDIO_UAC2_MALFORMED;
        length = descriptor[offset];
        type = descriptor[offset + 1u];
        if (length < 2u || length > size - offset)
            return RIN_USB_AUDIO_UAC2_MALFORMED;
        if (type == USB_DESCRIPTOR_CONFIGURATION)
            return RIN_USB_AUDIO_UAC2_MALFORMED;
        if (type == USB_DESCRIPTOR_INTERFACE) {
            uint8_t number;
            uint16_t key;
            uint32_t index;
            if (length != 9u) return RIN_USB_AUDIO_UAC2_MALFORMED;
            if (have_interface && current_endpoint_count != declared_endpoint_count)
                return RIN_USB_AUDIO_UAC2_MALFORMED;
            if (interface_count >= RIN_USB_AUDIO_UAC2_MAX_INTERFACE_DESCRIPTORS)
                return RIN_USB_AUDIO_UAC2_LIMIT_EXCEEDED;
            number = descriptor[offset + 2u];
            key = (uint16_t)(((uint16_t)number << 8u) | descriptor[offset + 3u]);
            for (index = 0u; index < interface_count; ++index)
                if (interface_keys[index] == key)
                    return RIN_USB_AUDIO_UAC2_MALFORMED;
            interface_keys[interface_count++] = key;
            if ((interface_numbers[number / 8u] &
                 (uint8_t)(1u << (number % 8u))) == 0u) {
                interface_numbers[number / 8u] |=
                    (uint8_t)(1u << (number % 8u));
                ++distinct_interfaces;
            }
            have_interface = 1;
            declared_endpoint_count = descriptor[offset + 4u];
            current_endpoint_count = 0u;
            endpoint_addresses = 0u;
        } else if (type == USB_DESCRIPTOR_ENDPOINT) {
            uint8_t address;
            uint8_t endpoint_number;
            uint32_t endpoint_bit;
            if (!have_interface || length < 7u)
                return RIN_USB_AUDIO_UAC2_MALFORMED;
            address = descriptor[offset + 2u];
            endpoint_number = address & USB_ENDPOINT_NUMBER_MASK;
            if (endpoint_number == 0u || (address & 0x70u) != 0u ||
                (descriptor[offset + 3u] & 0xc0u) != 0u ||
                (uac2_read_u16(descriptor + offset + 4u) & 0x07ffu) == 0u)
                return RIN_USB_AUDIO_UAC2_MALFORMED;
            endpoint_bit = UINT32_C(1) <<
                (endpoint_number + ((address & USB_ENDPOINT_DIRECTION_IN) != 0u
                                        ? 16u : 0u));
            if ((endpoint_addresses & endpoint_bit) != 0u)
                return RIN_USB_AUDIO_UAC2_MALFORMED;
            endpoint_addresses |= endpoint_bit;
            if (++current_endpoint_count > declared_endpoint_count)
                return RIN_USB_AUDIO_UAC2_MALFORMED;
        }
        offset += length;
    }
    if (!have_interface || current_endpoint_count != declared_endpoint_count ||
        distinct_interfaces != declared_interfaces)
        return RIN_USB_AUDIO_UAC2_MALFORMED;
    return RIN_USB_AUDIO_UAC2_OK;
}

static RinUsbAudioUac2Result uac2_parse_control(
    const uint8_t* descriptor, uint32_t size, Uac2Graph* graph) {
    uint32_t offset = 9u;
    if (!graph) return RIN_USB_AUDIO_UAC2_INVALID_ARGUMENT;
    uac2_zero(graph, sizeof(*graph));
    while (offset < size) {
        uint32_t length = descriptor[offset];
        if (descriptor[offset + 1u] == USB_DESCRIPTOR_INTERFACE &&
            length == 9u && descriptor[offset + 5u] == USB_CLASS_AUDIO &&
            descriptor[offset + 6u] == USB_AUDIO_SUBCLASS_CONTROL &&
            descriptor[offset + 3u] == 0u) {
            uint32_t end = offset + length;
            uint32_t cursor;
            uint32_t header_total = 0u;
            uint8_t header_seen = 0u;
            if (graph->control_found) return RIN_USB_AUDIO_UAC2_UNSUPPORTED;
            graph->control_found = 1u;
            graph->control_interface = descriptor[offset + 2u];
            cursor = end;
            while (cursor < size && descriptor[cursor + 1u] != USB_DESCRIPTOR_INTERFACE) {
                uint32_t dlength = descriptor[cursor];
                uint8_t subtype;
                if (dlength < 3u || dlength > size - cursor)
                    return RIN_USB_AUDIO_UAC2_MALFORMED;
                if (descriptor[cursor + 1u] != USB_DESCRIPTOR_CS_INTERFACE) {
                    cursor += dlength;
                    continue;
                }
                subtype = descriptor[cursor + 2u];
                if (subtype == UAC2_AC_HEADER) {
                    if (header_seen || dlength < 8u + descriptor[cursor + 7u] ||
                        uac2_read_u16(descriptor + cursor + 3u) < 0x0200u)
                        return RIN_USB_AUDIO_UAC2_MALFORMED;
                    header_seen = 1u;
                    header_total = uac2_read_u16(descriptor + cursor + 5u);
                    if (header_total < dlength || header_total > size - end)
                        return RIN_USB_AUDIO_UAC2_MALFORMED;
                } else if (subtype == UAC2_AC_CLOCK_SOURCE) {
                    uint8_t id;
                    uint32_t index;
                    if (dlength < 8u || graph->clock_count >= 32u)
                        return dlength < 8u ? RIN_USB_AUDIO_UAC2_MALFORMED
                                            : RIN_USB_AUDIO_UAC2_LIMIT_EXCEEDED;
                    id = descriptor[cursor + 3u];
                    if (id == 0u || uac2_find_clock(graph, id) >= 0 ||
                        (descriptor[cursor + 4u] & 0xfcu) != 0u)
                        return RIN_USB_AUDIO_UAC2_MALFORMED;
                    index = graph->clock_count++;
                    graph->clock_ids[index] = id;
                    graph->clock_attributes[index] = descriptor[cursor + 4u];
                    graph->clock_controls[index] = descriptor[cursor + 5u];
                } else if (subtype == UAC2_AC_INPUT_TERMINAL ||
                           subtype == UAC2_AC_OUTPUT_TERMINAL) {
                    uint8_t id;
                    uint8_t clock;
                    uint16_t type;
                    uint32_t index;
                    uint32_t minimum = subtype == UAC2_AC_INPUT_TERMINAL ? 17u : 12u;
                    uint32_t clock_offset = subtype == UAC2_AC_INPUT_TERMINAL ? 7u : 8u;
                    if (dlength < minimum || graph->terminal_count >= 32u)
                        return dlength < minimum ? RIN_USB_AUDIO_UAC2_MALFORMED
                                                 : RIN_USB_AUDIO_UAC2_LIMIT_EXCEEDED;
                    id = descriptor[cursor + 3u];
                    type = uac2_read_u16(descriptor + cursor + 4u);
                    clock = descriptor[cursor + clock_offset];
                    if (id == 0u || uac2_find_terminal(graph, id) >= 0 ||
                        uac2_find_clock(graph, clock) < 0)
                        return RIN_USB_AUDIO_UAC2_MALFORMED;
                    index = graph->terminal_count++;
                    graph->terminal_ids[index] = id;
                    graph->terminal_types[index] = type;
                    graph->terminal_clocks[index] = clock;
                }
                cursor += dlength;
            }
            if (!header_seen || header_total == 0u || header_total != cursor - end)
                return RIN_USB_AUDIO_UAC2_MALFORMED;
            offset = cursor;
            continue;
        }
        offset += length;
    }
    return graph->control_found && graph->clock_count != 0u
        ? RIN_USB_AUDIO_UAC2_OK : RIN_USB_AUDIO_UAC2_UNSUPPORTED;
}

static uint32_t uac2_next_interface(const uint8_t* descriptor, uint32_t size,
                                    uint32_t offset) {
    uint32_t cursor = offset + descriptor[offset];
    while (cursor < size) {
        if (descriptor[cursor + 1u] == USB_DESCRIPTOR_INTERFACE) return cursor;
        cursor += descriptor[cursor];
    }
    return size;
}

static RinUsbAudioUac2Result uac2_parse_profile(
    const uint8_t* descriptor, uint32_t start, uint32_t end,
    const Uac2Graph* graph, RinUsbAudioUac2ProfileV1* profile) {
    uint32_t cursor = start + 9u;
    uint32_t data_endpoint = 0u;
    uint32_t feedback_endpoint = 0u;
    uint16_t data_packet = 0u;
    uint8_t data_address = 0u;
    uint8_t data_attributes = 0u;
    uint8_t data_interval = 0u;
    uint32_t endpoint_count = 0u;
    uint8_t general_seen = 0u;
    uint8_t format_seen = 0u;
    uint8_t class_endpoint_seen = 0u;
    uint8_t terminal_link = 0u;
    uint8_t format_type = 0u;
    uint8_t channels = 0u;
    uint8_t subslot = 0u;
    uint8_t bits = 0u;
    uint8_t rate_count = 0u;
    uint32_t rates[RIN_USB_AUDIO_UAC2_MAX_SAMPLE_RATES];
    uint32_t rate_min = 0u;
    uint32_t rate_max = 0u;
    uint8_t clock_id;
    uint32_t index;

    uac2_zero(profile, sizeof(*profile));
    uac2_zero(rates, sizeof(rates));
    while (cursor < end) {
        uint32_t length = descriptor[cursor];
        uint8_t type = descriptor[cursor + 1u];
        if (length < 2u || length > end - cursor)
            return RIN_USB_AUDIO_UAC2_MALFORMED;
        if (type == USB_DESCRIPTOR_CS_INTERFACE) {
            uint8_t subtype = length >= 3u ? descriptor[cursor + 2u] : 0u;
            if (subtype == UAC2_AS_GENERAL) {
                if (general_seen || length < 16u)
                    return length < 16u ? RIN_USB_AUDIO_UAC2_MALFORMED
                                       : RIN_USB_AUDIO_UAC2_MALFORMED;
                general_seen = 1u;
                terminal_link = descriptor[cursor + 3u];
                format_type = descriptor[cursor + 5u];
                channels = descriptor[cursor + 10u];
                if (format_type != UAC2_FORMAT_TYPE_I || channels == 0u ||
                    channels > 8u || (uac2_read_u32(descriptor + cursor + 6u) &
                                      UAC2_FORMAT_PCM) == 0u)
                    return RIN_USB_AUDIO_UAC2_UNSUPPORTED;
            } else if (subtype == UAC2_AS_FORMAT_TYPE) {
                uint8_t count;
                if (format_seen || length < 7u || descriptor[cursor + 3u] != 1u)
                    return length < 7u ? RIN_USB_AUDIO_UAC2_MALFORMED
                                       : RIN_USB_AUDIO_UAC2_UNSUPPORTED;
                format_seen = 1u;
                subslot = descriptor[cursor + 4u];
                bits = descriptor[cursor + 5u];
                count = descriptor[cursor + 6u];
                if (subslot == 0u || subslot > 4u || bits == 0u ||
                    bits > (uint8_t)(subslot * 8u))
                    return RIN_USB_AUDIO_UAC2_UNSUPPORTED;
                if (count == 0u) {
                    if (length != 13u) return RIN_USB_AUDIO_UAC2_MALFORMED;
                    rate_min = uac2_read_u24(descriptor + cursor + 7u);
                    rate_max = uac2_read_u24(descriptor + cursor + 10u);
                    if (rate_min == 0u || rate_min > rate_max)
                        return RIN_USB_AUDIO_UAC2_MALFORMED;
                } else {
                    if (count > RIN_USB_AUDIO_UAC2_MAX_SAMPLE_RATES ||
                        length != 7u + 3u * count)
                        return count > RIN_USB_AUDIO_UAC2_MAX_SAMPLE_RATES
                            ? RIN_USB_AUDIO_UAC2_LIMIT_EXCEEDED
                            : RIN_USB_AUDIO_UAC2_MALFORMED;
                    for (index = 0u; index < count; ++index) {
                        uint32_t rate = uac2_read_u24(descriptor + cursor + 7u + 3u * index);
                        if (rate == 0u || (index != 0u && rate <= rates[index - 1u]))
                            return RIN_USB_AUDIO_UAC2_MALFORMED;
                        rates[index] = rate;
                    }
                    rate_count = count;
                    rate_min = rates[0];
                    rate_max = rates[count - 1u];
                }
            }
        } else if (type == USB_DESCRIPTOR_ENDPOINT) {
            uint16_t packet = uac2_read_u16(descriptor + cursor + 4u);
            uint8_t attributes = descriptor[cursor + 3u];
            ++endpoint_count;
            if (length < 7u || (attributes & USB_ENDPOINT_TYPE_MASK) != USB_ENDPOINT_ISOCHRONOUS)
                return RIN_USB_AUDIO_UAC2_UNSUPPORTED;
            if ((attributes & USB_ENDPOINT_USAGE_MASK) == USB_ENDPOINT_USAGE_FEEDBACK) {
                if (feedback_endpoint != 0u || (descriptor[cursor + 2u] & USB_ENDPOINT_DIRECTION_IN) == 0u)
                    return RIN_USB_AUDIO_UAC2_MALFORMED;
                feedback_endpoint = cursor;
            } else if ((attributes & USB_ENDPOINT_USAGE_MASK) == USB_ENDPOINT_USAGE_DATA) {
                if (data_endpoint != 0u) return RIN_USB_AUDIO_UAC2_MALFORMED;
                data_endpoint = cursor;
                data_packet = packet;
                data_address = descriptor[cursor + 2u];
                data_attributes = attributes;
                data_interval = descriptor[cursor + 6u];
            } else {
                return RIN_USB_AUDIO_UAC2_UNSUPPORTED;
            }
        } else if (type == USB_DESCRIPTOR_CS_ENDPOINT && length >= 3u &&
                   descriptor[cursor + 2u] == UAC2_EP_GENERAL) {
            if (length < 8u) return RIN_USB_AUDIO_UAC2_MALFORMED;
            if (class_endpoint_seen) return RIN_USB_AUDIO_UAC2_MALFORMED;
            class_endpoint_seen = 1u;
        }
        cursor += length;
    }
    if (endpoint_count == 0u || endpoint_count > 2u || data_endpoint == 0u ||
        !general_seen || !format_seen || !class_endpoint_seen || terminal_link == 0u ||
        uac2_find_terminal(graph, terminal_link) < 0)
        return RIN_USB_AUDIO_UAC2_MALFORMED;
    if (feedback_endpoint != 0u && endpoint_count != 2u)
        return RIN_USB_AUDIO_UAC2_MALFORMED;
    index = (uint32_t)uac2_find_terminal(graph, terminal_link);
    if (graph->terminal_types[index] != UAC2_TERMINAL_USB_STREAMING)
        return RIN_USB_AUDIO_UAC2_UNSUPPORTED;
    clock_id = graph->terminal_clocks[index];
    if (uac2_find_clock(graph, clock_id) < 0) return RIN_USB_AUDIO_UAC2_MALFORMED;
    if ((data_packet & 0x07ffu) == 0u || data_packet > 0x0fffu)
        return RIN_USB_AUDIO_UAC2_MALFORMED;
    profile->abi_version = RIN_USB_AUDIO_UAC2_ABI_VERSION;
    profile->struct_size = sizeof(*profile);
    profile->descriptor_offset = start;
    profile->descriptor_length = end - start;
    profile->flags = (data_address & USB_ENDPOINT_DIRECTION_IN) != 0u
        ? RIN_USB_AUDIO_UAC2_PROFILE_CAPTURE
        : RIN_USB_AUDIO_UAC2_PROFILE_PLAYBACK;
    if (feedback_endpoint != 0u) profile->flags |= RIN_USB_AUDIO_UAC2_PROFILE_EXPLICIT_FEEDBACK;
    if ((data_packet & 0x1800u) != 0u) profile->flags |= RIN_USB_AUDIO_UAC2_PROFILE_HIGH_BANDWIDTH;
    if (rate_count == 0u) profile->flags |= RIN_USB_AUDIO_UAC2_PROFILE_CONTINUOUS_RATE;
    if ((profile->clock_controls & 0x03u) != 0u)
        profile->flags |= RIN_USB_AUDIO_UAC2_PROFILE_FREQUENCY_CONTROL;
    profile->clock_source_id = clock_id;
    profile->clock_attributes = graph->clock_attributes[uac2_find_clock(graph, clock_id)];
    profile->clock_controls = graph->clock_controls[uac2_find_clock(graph, clock_id)];
    profile->terminal_link = terminal_link;
    profile->sample_rate_minimum = rate_min;
    profile->sample_rate_maximum = rate_max;
    profile->sample_rate_count = rate_count;
    for (index = 0u; index < rate_count; ++index) profile->sample_rates[index] = rates[index];
    profile->interface_number = descriptor[start + 2u];
    profile->alternate_setting = descriptor[start + 3u];
    profile->endpoint_address = data_address;
    profile->feedback_endpoint_address = feedback_endpoint == 0u ? 0u : descriptor[feedback_endpoint + 2u];
    profile->endpoint_attributes = data_attributes;
    profile->endpoint_interval = data_interval;
    profile->channels = channels;
    profile->subslot_size = subslot;
    profile->bit_resolution = bits;
    profile->maximum_packet_size = data_packet;
    profile->maximum_payload_per_interval = (uint16_t)((data_packet & 0x07ffu) * (1u + ((data_packet >> 11u) & 3u)));
    profile->format_type = format_type;
    profile->control_interface_number = graph->control_interface;
    return rin_usb_audio_uac2_validate_profile(profile, end);
}

RinUsbAudioUac2Result rin_usb_audio_uac2_validate_profile(
    const RinUsbAudioUac2ProfileV1* profile, uint32_t configuration_length) {
    uint32_t index;
    uint32_t known = RIN_USB_AUDIO_UAC2_PROFILE_FLAGS;
    uint32_t packet_base;
    uint32_t transactions;
    if (!profile) return RIN_USB_AUDIO_UAC2_INVALID_ARGUMENT;
    if (profile->abi_version != RIN_USB_AUDIO_UAC2_ABI_VERSION ||
        profile->struct_size != sizeof(*profile) || profile->descriptor_length < 9u ||
        profile->descriptor_offset > configuration_length ||
        profile->descriptor_length > configuration_length - profile->descriptor_offset ||
        (profile->flags & ~known) != 0u ||
        (profile->flags & (RIN_USB_AUDIO_UAC2_PROFILE_PLAYBACK |
                           RIN_USB_AUDIO_UAC2_PROFILE_CAPTURE)) == 0u ||
        profile->clock_source_id == 0u || profile->terminal_link == 0u ||
        profile->channels == 0u || profile->channels > 8u ||
        profile->subslot_size == 0u || profile->subslot_size > 4u ||
        profile->bit_resolution == 0u ||
        profile->bit_resolution > (uint8_t)(profile->subslot_size * 8u) ||
        profile->format_type != UAC2_FORMAT_TYPE_I || profile->endpoint_address == 0u ||
        (profile->endpoint_attributes & USB_ENDPOINT_TYPE_MASK) != USB_ENDPOINT_ISOCHRONOUS ||
        (profile->endpoint_attributes & 0xc0u) != 0u ||
        (profile->endpoint_attributes & USB_ENDPOINT_USAGE_MASK) != USB_ENDPOINT_USAGE_DATA ||
        profile->endpoint_interval == 0u || profile->maximum_packet_size == 0u ||
        profile->maximum_payload_per_interval == 0u ||
        profile->maximum_payload_per_interval > 3072u ||
        (((profile->flags & RIN_USB_AUDIO_UAC2_PROFILE_EXPLICIT_FEEDBACK) != 0u) &&
         profile->feedback_endpoint_address == 0u) ||
        (((profile->flags & RIN_USB_AUDIO_UAC2_PROFILE_EXPLICIT_FEEDBACK) == 0u) &&
         profile->feedback_endpoint_address != 0u))
        return RIN_USB_AUDIO_UAC2_MALFORMED;
    packet_base = profile->maximum_packet_size & 0x07ffu;
    transactions = ((profile->maximum_packet_size >> 11u) & 3u) + 1u;
    if (packet_base == 0u || transactions == 4u ||
        (profile->maximum_packet_size & 0xe000u) != 0u ||
        profile->maximum_payload_per_interval != packet_base * transactions ||
        (((profile->flags & RIN_USB_AUDIO_UAC2_PROFILE_HIGH_BANDWIDTH) != 0u) !=
         (transactions > 1u)))
        return RIN_USB_AUDIO_UAC2_MALFORMED;
    if (((profile->endpoint_address & USB_ENDPOINT_DIRECTION_IN) != 0u) !=
        ((profile->flags & RIN_USB_AUDIO_UAC2_PROFILE_CAPTURE) != 0u) ||
        (((profile->flags & RIN_USB_AUDIO_UAC2_PROFILE_EXPLICIT_FEEDBACK) != 0u) &&
         (profile->feedback_endpoint_address & USB_ENDPOINT_DIRECTION_IN) == 0u))
        return RIN_USB_AUDIO_UAC2_MALFORMED;
    if ((profile->flags & RIN_USB_AUDIO_UAC2_PROFILE_CONTINUOUS_RATE) != 0u) {
        if (profile->sample_rate_count != 0u || profile->sample_rate_minimum == 0u ||
            profile->sample_rate_minimum > profile->sample_rate_maximum) return RIN_USB_AUDIO_UAC2_MALFORMED;
        for (index = 0u; index < RIN_USB_AUDIO_UAC2_MAX_SAMPLE_RATES; ++index)
            if (profile->sample_rates[index] != 0u) return RIN_USB_AUDIO_UAC2_MALFORMED;
    } else {
        if (profile->sample_rate_count == 0u || profile->sample_rate_count > RIN_USB_AUDIO_UAC2_MAX_SAMPLE_RATES ||
            profile->sample_rate_minimum != profile->sample_rates[0] ||
            profile->sample_rate_maximum != profile->sample_rates[profile->sample_rate_count - 1u]) return RIN_USB_AUDIO_UAC2_MALFORMED;
        for (index = 0u; index < profile->sample_rate_count; ++index)
            if (profile->sample_rates[index] == 0u || (index != 0u && profile->sample_rates[index] <= profile->sample_rates[index - 1u])) return RIN_USB_AUDIO_UAC2_MALFORMED;
        for (; index < RIN_USB_AUDIO_UAC2_MAX_SAMPLE_RATES; ++index)
            if (profile->sample_rates[index] != 0u) return RIN_USB_AUDIO_UAC2_MALFORMED;
    }
    for (index = 0u; index < sizeof(profile->reserved) / sizeof(profile->reserved[0]); ++index)
        if (profile->reserved[index] != 0u) return RIN_USB_AUDIO_UAC2_MALFORMED;
    return RIN_USB_AUDIO_UAC2_OK;
}

RinUsbAudioUac2Result rin_usb_audio_uac2_admit_configuration(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbAudioUac2CatalogV1* catalog) {
    Uac2Graph graph;
    RinUsbAudioUac2CatalogV1 staged;
    uint32_t offset = 9u;
    RinUsbAudioUac2Result result;
    if (!descriptor || !catalog) {
        if (catalog) uac2_zero(catalog, sizeof(*catalog));
        return RIN_USB_AUDIO_UAC2_INVALID_ARGUMENT;
    }
    if (uac2_ranges_overlap(descriptor, descriptor_size, catalog, sizeof(*catalog))) {
        /* An aliased output cannot be cleared without corrupting the input. */
        return RIN_USB_AUDIO_UAC2_INVALID_ARGUMENT;
    }
    uac2_zero(&staged, sizeof(staged));
    result = uac2_validate_shape(descriptor, descriptor_size);
    if (result != RIN_USB_AUDIO_UAC2_OK) {
        uac2_zero(catalog, sizeof(*catalog));
        return result;
    }
    result = uac2_parse_control(descriptor, descriptor_size, &graph);
    if (result != RIN_USB_AUDIO_UAC2_OK) {
        uac2_zero(catalog, sizeof(*catalog));
        return result;
    }
    staged.abi_version = RIN_USB_AUDIO_UAC2_ABI_VERSION;
    staged.struct_size = sizeof(staged);
    staged.configuration_length = descriptor_size;
    while (offset < descriptor_size) {
        uint32_t length = descriptor[offset];
        if (descriptor[offset + 1u] == USB_DESCRIPTOR_INTERFACE && length == 9u &&
            descriptor[offset + 5u] == USB_CLASS_AUDIO &&
            descriptor[offset + 6u] == USB_AUDIO_SUBCLASS_STREAMING &&
            descriptor[offset + 7u] == USB_AUDIO_PROTOCOL_UAC2 &&
            descriptor[offset + 3u] != 0u) {
            uint32_t end = uac2_next_interface(descriptor, descriptor_size, offset);
            RinUsbAudioUac2ProfileV1 profile;
            result = uac2_parse_profile(descriptor, offset, end, &graph, &profile);
            if (result == RIN_USB_AUDIO_UAC2_OK) {
                if (staged.profile_count >= RIN_USB_AUDIO_UAC2_MAX_PROFILES) {
                    uac2_zero(catalog, sizeof(*catalog));
                    return RIN_USB_AUDIO_UAC2_LIMIT_EXCEEDED;
                }
                staged.profiles[staged.profile_count++] = profile;
                staged.flags |= profile.flags & RIN_USB_AUDIO_UAC2_CATALOG_FLAGS;
            } else if (result != RIN_USB_AUDIO_UAC2_UNSUPPORTED) {
                uac2_zero(catalog, sizeof(*catalog));
                return result;
            }
            offset = end;
            continue;
        }
        offset += length;
    }
    if (staged.profile_count == 0u) {
        uac2_zero(catalog, sizeof(*catalog));
        return RIN_USB_AUDIO_UAC2_UNSUPPORTED;
    }
    uac2_copy(catalog, &staged, sizeof(staged));
    return RIN_USB_AUDIO_UAC2_OK;
}
