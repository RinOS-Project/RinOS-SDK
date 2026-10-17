/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <rin/driver/usb/audio/uac1_descriptor_v1.h>

#define USB_DESCRIPTOR_CONFIGURATION 2u
#define USB_DESCRIPTOR_INTERFACE 4u
#define USB_DESCRIPTOR_ENDPOINT 5u
#define USB_DESCRIPTOR_CS_INTERFACE 0x24u
#define USB_DESCRIPTOR_CS_ENDPOINT 0x25u

#define USB_CLASS_AUDIO 0x01u
#define USB_AUDIO_SUBCLASS_CONTROL 0x01u
#define USB_AUDIO_SUBCLASS_STREAMING 0x02u
#define USB_AUDIO_PROTOCOL_UAC1 0x00u

#define UAC1_AC_HEADER 0x01u
#define UAC1_AC_INPUT_TERMINAL 0x02u
#define UAC1_AC_OUTPUT_TERMINAL 0x03u
#define UAC1_AS_GENERAL 0x01u
#define UAC1_AS_FORMAT_TYPE 0x02u
#define UAC1_FORMAT_TYPE_I 0x01u
#define UAC1_FORMAT_PCM 0x0001u
#define UAC1_TERMINAL_USB_STREAMING 0x0101u
#define UAC1_EP_GENERAL 0x01u

#define USB_ENDPOINT_DIRECTION_IN 0x80u
#define USB_ENDPOINT_NUMBER_MASK 0x0fu
#define USB_ENDPOINT_TYPE_MASK 0x03u
#define USB_ENDPOINT_ISOCHRONOUS 0x01u
#define USB_ENDPOINT_USAGE_MASK 0x30u
#define USB_ENDPOINT_USAGE_DATA 0x00u
#define USB_ENDPOINT_USAGE_FEEDBACK 0x10u

#define UAC1_CS_EP_FREQUENCY_CONTROL 0x01u
#define UAC1_CS_EP_PITCH_CONTROL 0x02u
#define UAC1_CS_EP_MAX_PACKETS_ONLY 0x80u
#define UAC1_CS_EP_ATTRIBUTE_MASK                                      \
    (UAC1_CS_EP_FREQUENCY_CONTROL | UAC1_CS_EP_PITCH_CONTROL |        \
     UAC1_CS_EP_MAX_PACKETS_ONLY)

typedef struct Uac1Graph {
    uint8_t entity_ids[32];
    uint8_t input_usb_terminals[32];
    uint8_t output_usb_terminals[32];
    uint8_t streaming_interfaces[32];
    uint8_t control_found;
    uint8_t reserved[3];
} Uac1Graph;

static void uac1_zero(void* pointer, uint32_t size) {
    volatile uint8_t* output = (volatile uint8_t*)pointer;
    while (size-- != 0u) *output++ = 0u;
}

static void uac1_copy(void* destination, const void* source, uint32_t size) {
    volatile uint8_t* output = (volatile uint8_t*)destination;
    const volatile uint8_t* input = (const volatile uint8_t*)source;
    while (size-- != 0u) *output++ = *input++;
}

static int uac1_all_zero(const void* pointer, uint32_t size) {
    const volatile uint8_t* input = (const volatile uint8_t*)pointer;
    uint8_t combined = 0u;
    while (size-- != 0u) combined |= *input++;
    return combined == 0u;
}

static uint16_t uac1_read_u16(const uint8_t* input) {
    return (uint16_t)((uint16_t)input[0] | ((uint16_t)input[1] << 8u));
}

static uint32_t uac1_read_u24(const uint8_t* input) {
    return (uint32_t)input[0] | ((uint32_t)input[1] << 8u) |
           ((uint32_t)input[2] << 16u);
}

static int uac1_ranges_overlap(const void* first, uint32_t first_size,
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

static int uac1_bit_test(const uint8_t bits[32], uint8_t value) {
    return (bits[value / 8u] & (uint8_t)(1u << (value % 8u))) != 0u;
}

static void uac1_bit_set(uint8_t bits[32], uint8_t value) {
    bits[value / 8u] |= (uint8_t)(1u << (value % 8u));
}

static uint32_t uac1_next_interface(const uint8_t* descriptor,
                                    uint32_t size, uint32_t offset) {
    uint32_t cursor = offset + descriptor[offset];
    while (cursor < size) {
        if (descriptor[cursor + 1u] == USB_DESCRIPTOR_INTERFACE)
            return cursor;
        cursor += descriptor[cursor];
    }
    return size;
}

static RinUsbAudioUac1Result uac1_validate_configuration_shape(
    const uint8_t* descriptor, uint32_t size) {
    uint16_t interface_keys[RIN_USB_AUDIO_UAC1_MAX_INTERFACE_DESCRIPTORS];
    uint8_t interface_numbers[32];
    uint32_t endpoint_addresses = 0u;
    uint32_t interface_descriptor_count = 0u;
    uint32_t distinct_interfaces = 0u;
    uint32_t offset;
    uint32_t current_endpoint_count = 0u;
    uint32_t declared_endpoint_count = 0u;
    int have_interface = 0;

    if (size < 9u || size > RIN_USB_AUDIO_UAC1_MAX_CONFIGURATION_BYTES ||
        descriptor[0] != 9u ||
        descriptor[1] != USB_DESCRIPTOR_CONFIGURATION ||
        uac1_read_u16(descriptor + 2u) != size || descriptor[4] == 0u ||
        descriptor[4] > 32u || descriptor[5] == 0u) {
        return size > RIN_USB_AUDIO_UAC1_MAX_CONFIGURATION_BYTES
            ? RIN_USB_AUDIO_UAC1_LIMIT_EXCEEDED
            : RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    uac1_zero(interface_numbers, sizeof(interface_numbers));
    offset = 9u;
    while (offset < size) {
        uint32_t length;
        uint8_t type;
        if (size - offset < 2u) return RIN_USB_AUDIO_UAC1_MALFORMED;
        length = descriptor[offset];
        type = descriptor[offset + 1u];
        if (length < 2u || length > size - offset)
            return RIN_USB_AUDIO_UAC1_MALFORMED;
        if (type == USB_DESCRIPTOR_CONFIGURATION)
            return RIN_USB_AUDIO_UAC1_MALFORMED;
        if (type == USB_DESCRIPTOR_INTERFACE) {
            uint8_t number;
            uint8_t alternate;
            uint16_t key;
            uint32_t index;
            if (length != 9u) return RIN_USB_AUDIO_UAC1_MALFORMED;
            if (have_interface &&
                current_endpoint_count != declared_endpoint_count) {
                return RIN_USB_AUDIO_UAC1_MALFORMED;
            }
            if (interface_descriptor_count >=
                RIN_USB_AUDIO_UAC1_MAX_INTERFACE_DESCRIPTORS) {
                return RIN_USB_AUDIO_UAC1_LIMIT_EXCEEDED;
            }
            number = descriptor[offset + 2u];
            alternate = descriptor[offset + 3u];
            key = (uint16_t)(((uint16_t)number << 8u) | alternate);
            for (index = 0u; index < interface_descriptor_count; ++index) {
                if (interface_keys[index] == key)
                    return RIN_USB_AUDIO_UAC1_MALFORMED;
            }
            interface_keys[interface_descriptor_count++] = key;
            if (!uac1_bit_test(interface_numbers, number)) {
                uac1_bit_set(interface_numbers, number);
                ++distinct_interfaces;
            }
            have_interface = 1;
            current_endpoint_count = 0u;
            declared_endpoint_count = descriptor[offset + 4u];
            endpoint_addresses = 0u;
        } else if (type == USB_DESCRIPTOR_ENDPOINT) {
            uint8_t address;
            uint8_t endpoint_number;
            uint32_t endpoint_bit;
            uint16_t packet;
            if (!have_interface || length < 7u)
                return RIN_USB_AUDIO_UAC1_MALFORMED;
            address = descriptor[offset + 2u];
            endpoint_number = address & USB_ENDPOINT_NUMBER_MASK;
            packet = uac1_read_u16(descriptor + offset + 4u);
            if (endpoint_number == 0u || (address & 0x70u) != 0u ||
                (packet & 0x07ffu) == 0u ||
                (descriptor[offset + 3u] & 0xc0u) != 0u) {
                return RIN_USB_AUDIO_UAC1_MALFORMED;
            }
            endpoint_bit = UINT32_C(1) <<
                (endpoint_number +
                 ((address & USB_ENDPOINT_DIRECTION_IN) != 0u ? 16u : 0u));
            if ((endpoint_addresses & endpoint_bit) != 0u)
                return RIN_USB_AUDIO_UAC1_MALFORMED;
            endpoint_addresses |= endpoint_bit;
            ++current_endpoint_count;
            if (current_endpoint_count > declared_endpoint_count)
                return RIN_USB_AUDIO_UAC1_MALFORMED;
        }
        offset += length;
    }
    if (!have_interface || offset != size ||
        current_endpoint_count != declared_endpoint_count ||
        distinct_interfaces != descriptor[4]) {
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    return RIN_USB_AUDIO_UAC1_OK;
}

static RinUsbAudioUac1Result uac1_parse_control_graph(
    const uint8_t* descriptor, uint32_t size, Uac1Graph* graph) {
    uint32_t offset = 9u;
    while (offset < size) {
        if (descriptor[offset + 1u] == USB_DESCRIPTOR_INTERFACE &&
            descriptor[offset + 5u] == USB_CLASS_AUDIO &&
            descriptor[offset + 6u] == USB_AUDIO_SUBCLASS_CONTROL &&
            descriptor[offset + 7u] == USB_AUDIO_PROTOCOL_UAC1) {
            uint32_t end = uac1_next_interface(descriptor, size, offset);
            uint32_t cursor = offset + descriptor[offset];
            uint32_t class_length = 0u;
            uint32_t expected_class_length = 0u;
            int class_descriptors_closed = 0;
            int header_found = 0;
            if (graph->control_found || descriptor[offset + 3u] != 0u)
                return RIN_USB_AUDIO_UAC1_UNSUPPORTED;
            graph->control_found = 1u;
            while (cursor < end) {
                uint8_t type = descriptor[cursor + 1u];
                if (type != USB_DESCRIPTOR_CS_INTERFACE) {
                    if (header_found) class_descriptors_closed = 1;
                    cursor += descriptor[cursor];
                    continue;
                }
                if (class_descriptors_closed || descriptor[cursor] < 3u)
                    return RIN_USB_AUDIO_UAC1_MALFORMED;
                class_length += descriptor[cursor];
                if (descriptor[cursor + 2u] == UAC1_AC_HEADER) {
                    uint32_t count;
                    uint32_t index;
                    if (header_found || descriptor[cursor] < 8u)
                        return RIN_USB_AUDIO_UAC1_MALFORMED;
                    count = descriptor[cursor + 7u];
                    if (count == 0u || count > 32u ||
                        descriptor[cursor] != 8u + count ||
                        uac1_read_u16(descriptor + cursor + 3u) != 0x0100u) {
                        return RIN_USB_AUDIO_UAC1_MALFORMED;
                    }
                    expected_class_length =
                        uac1_read_u16(descriptor + cursor + 5u);
                    if (expected_class_length < descriptor[cursor])
                        return RIN_USB_AUDIO_UAC1_MALFORMED;
                    for (index = 0u; index < count; ++index) {
                        uint8_t interface_number = descriptor[cursor + 8u + index];
                        if (uac1_bit_test(graph->streaming_interfaces,
                                         interface_number)) {
                            return RIN_USB_AUDIO_UAC1_MALFORMED;
                        }
                        uac1_bit_set(graph->streaming_interfaces,
                                     interface_number);
                    }
                    header_found = 1;
                } else {
                    uint8_t entity_id;
                    uint8_t subtype = descriptor[cursor + 2u];
                    if (!header_found || descriptor[cursor] < 4u ||
                        subtype < UAC1_AC_INPUT_TERMINAL || subtype > 0x08u) {
                        return RIN_USB_AUDIO_UAC1_MALFORMED;
                    }
                    entity_id = descriptor[cursor + 3u];
                    if (entity_id == 0u ||
                        uac1_bit_test(graph->entity_ids, entity_id)) {
                        return RIN_USB_AUDIO_UAC1_MALFORMED;
                    }
                    uac1_bit_set(graph->entity_ids, entity_id);
                    if (subtype == UAC1_AC_INPUT_TERMINAL) {
                        if (descriptor[cursor] != 12u ||
                            descriptor[cursor + 7u] == 0u) {
                            return RIN_USB_AUDIO_UAC1_MALFORMED;
                        }
                        if (uac1_read_u16(descriptor + cursor + 4u) ==
                            UAC1_TERMINAL_USB_STREAMING) {
                            uac1_bit_set(graph->input_usb_terminals,
                                         entity_id);
                        }
                    } else if (subtype == UAC1_AC_OUTPUT_TERMINAL) {
                        if (descriptor[cursor] != 9u ||
                            descriptor[cursor + 7u] == 0u) {
                            return RIN_USB_AUDIO_UAC1_MALFORMED;
                        }
                        if (uac1_read_u16(descriptor + cursor + 4u) ==
                            UAC1_TERMINAL_USB_STREAMING) {
                            uac1_bit_set(graph->output_usb_terminals,
                                         entity_id);
                        }
                    }
                }
                cursor += descriptor[cursor];
            }
            if (!header_found || class_length != expected_class_length)
                return RIN_USB_AUDIO_UAC1_MALFORMED;
        }
        offset += descriptor[offset];
    }
    return graph->control_found ? RIN_USB_AUDIO_UAC1_OK
                                : RIN_USB_AUDIO_UAC1_UNSUPPORTED;
}

static RinUsbAudioUac1Result uac1_parse_format_type(
    const uint8_t* descriptor, RinUsbAudioUac1ProfileV1* profile) {
    uint32_t length = descriptor[0];
    uint32_t count;
    uint32_t index;
    if (length < 8u || descriptor[3] != UAC1_FORMAT_TYPE_I)
        return RIN_USB_AUDIO_UAC1_UNSUPPORTED;
    profile->channels = descriptor[4];
    profile->subframe_size = descriptor[5];
    profile->bit_resolution = descriptor[6];
    count = descriptor[7];
    if (profile->channels == 0u || profile->channels > 8u ||
        profile->subframe_size == 0u || profile->subframe_size > 4u ||
        profile->bit_resolution == 0u ||
        profile->bit_resolution > profile->subframe_size * 8u) {
        return RIN_USB_AUDIO_UAC1_UNSUPPORTED;
    }
    if (count == 0u) {
        if (length != 14u) return RIN_USB_AUDIO_UAC1_MALFORMED;
        profile->sample_rate_minimum = uac1_read_u24(descriptor + 8u);
        profile->sample_rate_maximum = uac1_read_u24(descriptor + 11u);
        if (profile->sample_rate_minimum == 0u ||
            profile->sample_rate_minimum > profile->sample_rate_maximum) {
            return RIN_USB_AUDIO_UAC1_MALFORMED;
        }
        profile->flags |= RIN_USB_AUDIO_UAC1_PROFILE_CONTINUOUS_RATE;
        return RIN_USB_AUDIO_UAC1_OK;
    }
    if (count > RIN_USB_AUDIO_UAC1_MAX_SAMPLE_RATES)
        return RIN_USB_AUDIO_UAC1_LIMIT_EXCEEDED;
    if (length != 8u + count * 3u)
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    profile->sample_rate_count = (uint8_t)count;
    for (index = 0u; index < count; ++index) {
        uint32_t rate = uac1_read_u24(descriptor + 8u + index * 3u);
        uint32_t insertion = index;
        if (rate == 0u) return RIN_USB_AUDIO_UAC1_MALFORMED;
        while (insertion != 0u &&
               profile->sample_rates[insertion - 1u] > rate) {
            profile->sample_rates[insertion] =
                profile->sample_rates[insertion - 1u];
            --insertion;
        }
        if ((insertion != 0u &&
             profile->sample_rates[insertion - 1u] == rate) ||
            (insertion != index && profile->sample_rates[insertion] == rate)) {
            return RIN_USB_AUDIO_UAC1_MALFORMED;
        }
        profile->sample_rates[insertion] = rate;
    }
    profile->sample_rate_minimum = profile->sample_rates[0];
    profile->sample_rate_maximum = profile->sample_rates[count - 1u];
    return RIN_USB_AUDIO_UAC1_OK;
}

static RinUsbAudioUac1Result uac1_parse_streaming_interface(
    const uint8_t* descriptor, uint32_t size, uint32_t offset,
    RinUsbAudioUac1ProfileV1* profile) {
    uint32_t end = uac1_next_interface(descriptor, size, offset);
    uint32_t cursor = offset + descriptor[offset];
    uint32_t data_packet = 0u;
    uint8_t feedback_attributes = 0u;
    uint8_t feedback_interval = 0u;
    uint8_t feedback_refresh = 0u;
    uint8_t feedback_sync_address = 0u;
    uint16_t feedback_packet = 0u;
    uint8_t current_endpoint = 0u;
    int general_found = 0;
    int format_found = 0;
    int data_endpoint_found = 0;
    int feedback_endpoint_found = 0;
    int class_endpoint_found = 0;
    RinUsbAudioUac1Result result;

    uac1_zero(profile, sizeof(*profile));
    profile->abi_version = RIN_USB_AUDIO_UAC1_ABI_VERSION;
    profile->struct_size = sizeof(*profile);
    profile->descriptor_offset = offset;
    profile->descriptor_length = end - offset;
    profile->interface_number = descriptor[offset + 2u];
    profile->alternate_setting = descriptor[offset + 3u];
    while (cursor < end) {
        uint8_t type = descriptor[cursor + 1u];
        uint8_t length = descriptor[cursor];
        if (type == USB_DESCRIPTOR_CS_INTERFACE) {
            uint8_t subtype;
            if (length < 3u) return RIN_USB_AUDIO_UAC1_MALFORMED;
            subtype = descriptor[cursor + 2u];
            if (subtype == UAC1_AS_GENERAL) {
                if (general_found || length != 7u)
                    return RIN_USB_AUDIO_UAC1_MALFORMED;
                profile->terminal_link = descriptor[cursor + 3u];
                profile->format_tag =
                    uac1_read_u16(descriptor + cursor + 5u);
                if (profile->terminal_link == 0u)
                    return RIN_USB_AUDIO_UAC1_MALFORMED;
                if (profile->format_tag != UAC1_FORMAT_PCM)
                    return RIN_USB_AUDIO_UAC1_UNSUPPORTED;
                general_found = 1;
            } else if (subtype == UAC1_AS_FORMAT_TYPE) {
                if (format_found)
                    return RIN_USB_AUDIO_UAC1_MALFORMED;
                result = uac1_parse_format_type(descriptor + cursor, profile);
                if (result != RIN_USB_AUDIO_UAC1_OK) return result;
                format_found = 1;
            } else {
                return RIN_USB_AUDIO_UAC1_UNSUPPORTED;
            }
        } else if (type == USB_DESCRIPTOR_ENDPOINT) {
            uint8_t address;
            uint8_t attributes;
            uint8_t usage;
            uint16_t packet;
            if (length != 9u) return RIN_USB_AUDIO_UAC1_MALFORMED;
            address = descriptor[cursor + 2u];
            attributes = descriptor[cursor + 3u];
            packet = uac1_read_u16(descriptor + cursor + 4u);
            usage = attributes & USB_ENDPOINT_USAGE_MASK;
            if ((attributes & USB_ENDPOINT_TYPE_MASK) !=
                USB_ENDPOINT_ISOCHRONOUS) {
                return RIN_USB_AUDIO_UAC1_UNSUPPORTED;
            }
            if (usage == USB_ENDPOINT_USAGE_DATA) {
                uint32_t transactions = ((packet >> 11u) & 3u) + 1u;
                if (data_endpoint_found || ((packet >> 11u) & 3u) == 3u ||
                    (packet & 0xe000u) != 0u) {
                    return RIN_USB_AUDIO_UAC1_MALFORMED;
                }
                data_packet = (uint32_t)(packet & 0x07ffu) * transactions;
                profile->endpoint_address = address;
                profile->endpoint_attributes = attributes;
                profile->maximum_packet_size = packet;
                profile->maximum_payload_per_interval =
                    (uint16_t)data_packet;
                profile->endpoint_interval = descriptor[cursor + 6u];
                profile->endpoint_refresh = descriptor[cursor + 7u];
                profile->endpoint_sync_address = descriptor[cursor + 8u];
                if (profile->endpoint_interval == 0u ||
                    profile->endpoint_interval > 16u ||
                    profile->endpoint_refresh != 0u) {
                    return RIN_USB_AUDIO_UAC1_MALFORMED;
                }
                if (transactions > 1u)
                    profile->flags |=
                        RIN_USB_AUDIO_UAC1_PROFILE_HIGH_BANDWIDTH;
                data_endpoint_found = 1;
                current_endpoint = 1u;
            } else if (usage == USB_ENDPOINT_USAGE_FEEDBACK) {
                if (feedback_endpoint_found ||
                    (address & USB_ENDPOINT_DIRECTION_IN) == 0u ||
                    (packet & 0xf800u) != 0u ||
                    ((packet & 0x07ffu) != 3u &&
                     (packet & 0x07ffu) != 4u)) {
                    return RIN_USB_AUDIO_UAC1_MALFORMED;
                }
                profile->feedback_endpoint_address = address;
                feedback_attributes = attributes;
                feedback_packet = packet;
                feedback_interval = descriptor[cursor + 6u];
                feedback_refresh = descriptor[cursor + 7u];
                feedback_sync_address = descriptor[cursor + 8u];
                feedback_endpoint_found = 1;
                current_endpoint = 2u;
            } else {
                return RIN_USB_AUDIO_UAC1_UNSUPPORTED;
            }
        } else if (type == USB_DESCRIPTOR_CS_ENDPOINT) {
            if (length != 7u || descriptor[cursor + 2u] != UAC1_EP_GENERAL ||
                current_endpoint != 1u || class_endpoint_found) {
                return RIN_USB_AUDIO_UAC1_MALFORMED;
            }
            profile->class_endpoint_attributes = descriptor[cursor + 3u];
            profile->lock_delay_units = descriptor[cursor + 4u];
            profile->lock_delay = uac1_read_u16(descriptor + cursor + 5u);
            if ((profile->class_endpoint_attributes &
                 ~UAC1_CS_EP_ATTRIBUTE_MASK) != 0u ||
                profile->lock_delay_units > 2u ||
                (profile->lock_delay_units == 0u &&
                 profile->lock_delay != 0u)) {
                return RIN_USB_AUDIO_UAC1_MALFORMED;
            }
            if ((profile->class_endpoint_attributes &
                 UAC1_CS_EP_FREQUENCY_CONTROL) != 0u) {
                profile->flags |=
                    RIN_USB_AUDIO_UAC1_PROFILE_FREQUENCY_CONTROL;
            }
            if ((profile->class_endpoint_attributes &
                 UAC1_CS_EP_PITCH_CONTROL) != 0u) {
                profile->flags |= RIN_USB_AUDIO_UAC1_PROFILE_PITCH_CONTROL;
            }
            class_endpoint_found = 1;
        }
        cursor += length;
    }
    if (!general_found || !format_found || !data_endpoint_found ||
        !class_endpoint_found || data_packet == 0u) {
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    if ((profile->endpoint_address & USB_ENDPOINT_DIRECTION_IN) != 0u) {
        profile->flags |= RIN_USB_AUDIO_UAC1_PROFILE_CAPTURE;
    } else {
        profile->flags |= RIN_USB_AUDIO_UAC1_PROFILE_PLAYBACK;
    }
    if (feedback_endpoint_found) {
        if ((profile->flags & RIN_USB_AUDIO_UAC1_PROFILE_PLAYBACK) == 0u ||
            profile->endpoint_sync_address !=
                profile->feedback_endpoint_address ||
            feedback_interval == 0u || feedback_interval > 16u ||
            feedback_sync_address != 0u || feedback_packet == 0u ||
            feedback_refresh == 0u || feedback_refresh > 9u ||
            (feedback_attributes & USB_ENDPOINT_USAGE_MASK) !=
                USB_ENDPOINT_USAGE_FEEDBACK ||
            (feedback_attributes & 0x0cu) != 0u) {
            return RIN_USB_AUDIO_UAC1_MALFORMED;
        }
        profile->flags |= RIN_USB_AUDIO_UAC1_PROFILE_EXPLICIT_FEEDBACK;
    } else if (profile->endpoint_sync_address != 0u) {
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    return RIN_USB_AUDIO_UAC1_OK;
}

RinUsbAudioUac1Result rin_usb_audio_uac1_validate_profile(
    const RinUsbAudioUac1ProfileV1* profile,
    uint32_t configuration_length) {
    uint32_t count;
    uint32_t index;
    uint32_t packet_base;
    uint32_t transactions;
    uint32_t expected_payload;
    uint32_t direction;
    if (!profile ||
        profile->abi_version != RIN_USB_AUDIO_UAC1_ABI_VERSION ||
        profile->struct_size != sizeof(*profile) ||
        profile->descriptor_offset < 9u || profile->descriptor_length < 9u ||
        profile->descriptor_offset > configuration_length ||
        profile->descriptor_length >
            configuration_length - profile->descriptor_offset ||
        (profile->flags & ~RIN_USB_AUDIO_UAC1_PROFILE_FLAGS) != 0u ||
        profile->reserved0 != 0u || profile->reserved1 != 0u ||
        !uac1_all_zero(profile->reserved, sizeof(profile->reserved)) ||
        profile->alternate_setting == 0u || profile->terminal_link == 0u ||
        profile->format_tag != UAC1_FORMAT_PCM || profile->channels == 0u ||
        profile->channels > 8u || profile->subframe_size == 0u ||
        profile->subframe_size > 4u || profile->bit_resolution == 0u ||
        profile->bit_resolution > profile->subframe_size * 8u ||
        (profile->endpoint_address & USB_ENDPOINT_NUMBER_MASK) == 0u ||
        (profile->endpoint_address & 0x70u) != 0u ||
        (profile->endpoint_attributes & 0xc0u) != 0u ||
        (profile->endpoint_attributes & USB_ENDPOINT_TYPE_MASK) !=
            USB_ENDPOINT_ISOCHRONOUS ||
        (profile->endpoint_attributes & USB_ENDPOINT_USAGE_MASK) !=
            USB_ENDPOINT_USAGE_DATA || profile->endpoint_interval == 0u ||
        profile->endpoint_interval > 16u ||
        profile->endpoint_refresh != 0u ||
        (profile->class_endpoint_attributes &
         ~UAC1_CS_EP_ATTRIBUTE_MASK) != 0u ||
        profile->lock_delay_units > 2u ||
        (profile->lock_delay_units == 0u && profile->lock_delay != 0u)) {
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    direction = profile->flags &
        (RIN_USB_AUDIO_UAC1_PROFILE_PLAYBACK |
         RIN_USB_AUDIO_UAC1_PROFILE_CAPTURE);
    if (direction != RIN_USB_AUDIO_UAC1_PROFILE_PLAYBACK &&
        direction != RIN_USB_AUDIO_UAC1_PROFILE_CAPTURE) {
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    if (((profile->endpoint_address & USB_ENDPOINT_DIRECTION_IN) != 0u) !=
        (direction == RIN_USB_AUDIO_UAC1_PROFILE_CAPTURE)) {
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    packet_base = profile->maximum_packet_size & 0x07ffu;
    transactions = ((profile->maximum_packet_size >> 11u) & 3u) + 1u;
    if (packet_base == 0u ||
        ((profile->maximum_packet_size >> 11u) & 3u) == 3u ||
        (profile->maximum_packet_size & 0xe000u) != 0u) {
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    expected_payload = packet_base * transactions;
    if (profile->maximum_payload_per_interval != expected_payload ||
        (((profile->flags & RIN_USB_AUDIO_UAC1_PROFILE_HIGH_BANDWIDTH) != 0u) !=
         (transactions > 1u)) ||
        (((profile->flags & RIN_USB_AUDIO_UAC1_PROFILE_FREQUENCY_CONTROL) !=
          0u) != ((profile->class_endpoint_attributes &
                    UAC1_CS_EP_FREQUENCY_CONTROL) != 0u)) ||
        (((profile->flags & RIN_USB_AUDIO_UAC1_PROFILE_PITCH_CONTROL) != 0u) !=
         ((profile->class_endpoint_attributes &
           UAC1_CS_EP_PITCH_CONTROL) != 0u))) {
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    if ((profile->flags & RIN_USB_AUDIO_UAC1_PROFILE_EXPLICIT_FEEDBACK) != 0u) {
        if (direction != RIN_USB_AUDIO_UAC1_PROFILE_PLAYBACK ||
            (profile->feedback_endpoint_address &
             USB_ENDPOINT_DIRECTION_IN) == 0u ||
            (profile->feedback_endpoint_address &
             USB_ENDPOINT_NUMBER_MASK) == 0u ||
            (profile->feedback_endpoint_address & 0x70u) != 0u ||
            profile->endpoint_sync_address !=
                profile->feedback_endpoint_address) {
            return RIN_USB_AUDIO_UAC1_MALFORMED;
        }
    } else if (profile->feedback_endpoint_address != 0u ||
               profile->endpoint_sync_address != 0u) {
        return RIN_USB_AUDIO_UAC1_MALFORMED;
    }
    count = profile->sample_rate_count;
    if ((profile->flags & RIN_USB_AUDIO_UAC1_PROFILE_CONTINUOUS_RATE) != 0u) {
        if (count != 0u || profile->sample_rate_minimum == 0u ||
            profile->sample_rate_minimum > profile->sample_rate_maximum ||
            !uac1_all_zero(profile->sample_rates,
                           sizeof(profile->sample_rates))) {
            return RIN_USB_AUDIO_UAC1_MALFORMED;
        }
    } else {
        if (count == 0u || count > RIN_USB_AUDIO_UAC1_MAX_SAMPLE_RATES ||
            profile->sample_rate_minimum != profile->sample_rates[0] ||
            profile->sample_rate_maximum != profile->sample_rates[count - 1u]) {
            return RIN_USB_AUDIO_UAC1_MALFORMED;
        }
        for (index = 0u; index < count; ++index) {
            if (profile->sample_rates[index] == 0u ||
                (index != 0u && profile->sample_rates[index - 1u] >=
                                      profile->sample_rates[index])) {
                return RIN_USB_AUDIO_UAC1_MALFORMED;
            }
        }
        for (; index < RIN_USB_AUDIO_UAC1_MAX_SAMPLE_RATES; ++index) {
            if (profile->sample_rates[index] != 0u)
                return RIN_USB_AUDIO_UAC1_MALFORMED;
        }
    }
    return RIN_USB_AUDIO_UAC1_OK;
}

RinUsbAudioUac1Result rin_usb_audio_uac1_admit_configuration(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbAudioUac1CatalogV1* catalog) {
    Uac1Graph graph;
    uint32_t offset;
    RinUsbAudioUac1Result result;
    if (!catalog) return RIN_USB_AUDIO_UAC1_INVALID_ARGUMENT;
    if (descriptor && uac1_ranges_overlap(
            descriptor, descriptor_size, catalog, sizeof(*catalog))) {
        return RIN_USB_AUDIO_UAC1_INVALID_ARGUMENT;
    }
    uac1_zero(catalog, sizeof(*catalog));
    if (!descriptor || descriptor_size == 0u)
        return RIN_USB_AUDIO_UAC1_INVALID_ARGUMENT;
    result = uac1_validate_configuration_shape(descriptor, descriptor_size);
    if (result != RIN_USB_AUDIO_UAC1_OK) return result;
    uac1_zero(&graph, sizeof(graph));
    result = uac1_parse_control_graph(descriptor, descriptor_size, &graph);
    if (result != RIN_USB_AUDIO_UAC1_OK) return result;
    offset = 9u;
    while (offset < descriptor_size) {
        if (descriptor[offset + 1u] == USB_DESCRIPTOR_INTERFACE &&
            descriptor[offset + 5u] == USB_CLASS_AUDIO &&
            descriptor[offset + 6u] == USB_AUDIO_SUBCLASS_STREAMING &&
            descriptor[offset + 7u] == USB_AUDIO_PROTOCOL_UAC1 &&
            descriptor[offset + 3u] != 0u &&
            descriptor[offset + 4u] != 0u) {
            RinUsbAudioUac1ProfileV1 candidate;
            if (!uac1_bit_test(graph.streaming_interfaces,
                               descriptor[offset + 2u])) {
                result = RIN_USB_AUDIO_UAC1_MALFORMED;
                goto fail;
            }
            result = uac1_parse_streaming_interface(
                descriptor, descriptor_size, offset, &candidate);
            if (result == RIN_USB_AUDIO_UAC1_UNSUPPORTED) {
                offset = uac1_next_interface(descriptor, descriptor_size,
                                             offset);
                continue;
            }
            if (result != RIN_USB_AUDIO_UAC1_OK) goto fail;
            if ((candidate.flags & RIN_USB_AUDIO_UAC1_PROFILE_PLAYBACK) != 0u) {
                if (!uac1_bit_test(graph.input_usb_terminals,
                                   candidate.terminal_link)) {
                    result = RIN_USB_AUDIO_UAC1_MALFORMED;
                    goto fail;
                }
            } else if (!uac1_bit_test(graph.output_usb_terminals,
                                      candidate.terminal_link)) {
                result = RIN_USB_AUDIO_UAC1_MALFORMED;
                goto fail;
            }
            result = rin_usb_audio_uac1_validate_profile(
                &candidate, descriptor_size);
            if (result != RIN_USB_AUDIO_UAC1_OK) goto fail;
            if (catalog->profile_count >= RIN_USB_AUDIO_UAC1_MAX_PROFILES) {
                result = RIN_USB_AUDIO_UAC1_LIMIT_EXCEEDED;
                goto fail;
            }
            uac1_copy(&catalog->profiles[catalog->profile_count], &candidate,
                      sizeof(candidate));
            if ((candidate.flags & RIN_USB_AUDIO_UAC1_PROFILE_PLAYBACK) != 0u)
                catalog->flags |= RIN_USB_AUDIO_UAC1_CATALOG_PLAYBACK;
            else
                catalog->flags |= RIN_USB_AUDIO_UAC1_CATALOG_CAPTURE;
            ++catalog->profile_count;
        }
        offset += descriptor[offset];
    }
    if (catalog->profile_count == 0u) {
        result = RIN_USB_AUDIO_UAC1_UNSUPPORTED;
        goto fail;
    }
    catalog->abi_version = RIN_USB_AUDIO_UAC1_ABI_VERSION;
    catalog->struct_size = sizeof(*catalog);
    catalog->configuration_length = descriptor_size;
    if ((catalog->flags & ~RIN_USB_AUDIO_UAC1_CATALOG_FLAGS) != 0u ||
        !uac1_all_zero(catalog->reserved, sizeof(catalog->reserved))) {
        result = RIN_USB_AUDIO_UAC1_MALFORMED;
        goto fail;
    }
    return RIN_USB_AUDIO_UAC1_OK;

fail:
    uac1_zero(catalog, sizeof(*catalog));
    return result;
}
