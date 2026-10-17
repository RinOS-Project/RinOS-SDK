/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <rin/driver/usb/hid/gamepad_v1.h>

#define HID_ITEM_TYPE_MAIN 0u
#define HID_ITEM_TYPE_GLOBAL 1u
#define HID_ITEM_TYPE_LOCAL 2u

#define HID_MAIN_INPUT 8u
#define HID_MAIN_OUTPUT 9u
#define HID_MAIN_COLLECTION 10u
#define HID_MAIN_FEATURE 11u
#define HID_MAIN_END_COLLECTION 12u

#define HID_GLOBAL_USAGE_PAGE 0u
#define HID_GLOBAL_LOGICAL_MINIMUM 1u
#define HID_GLOBAL_LOGICAL_MAXIMUM 2u
#define HID_GLOBAL_PHYSICAL_MINIMUM 3u
#define HID_GLOBAL_PHYSICAL_MAXIMUM 4u
#define HID_GLOBAL_UNIT_EXPONENT 5u
#define HID_GLOBAL_UNIT 6u
#define HID_GLOBAL_REPORT_SIZE 7u
#define HID_GLOBAL_REPORT_ID 8u
#define HID_GLOBAL_REPORT_COUNT 9u
#define HID_GLOBAL_PUSH 10u
#define HID_GLOBAL_POP 11u

#define HID_LOCAL_USAGE 0u
#define HID_LOCAL_USAGE_MINIMUM 1u
#define HID_LOCAL_USAGE_MAXIMUM 2u

#define HID_COLLECTION_APPLICATION 1u
#define HID_USAGE_PAGE_GENERIC_DESKTOP 0x01u
#define HID_USAGE_PAGE_BUTTON 0x09u
#define HID_USAGE_X 0x30u
#define HID_USAGE_WHEEL 0x38u
#define HID_USAGE_HAT_SWITCH 0x39u

#define HID_INPUT_CONSTANT (1u << 0)
#define HID_INPUT_VARIABLE (1u << 1)
#define HID_INPUT_RELATIVE (1u << 2)
#define HID_INPUT_NULL_STATE (1u << 6)
#define HID_INPUT_SUPPORTED_FLAGS \
    (HID_INPUT_VARIABLE | HID_INPUT_NULL_STATE)

#define HID_MAX_GLOBAL_STACK 4u
#define HID_MAX_COLLECTION_DEPTH 8u
#define HID_MAX_LOCAL_USAGES 32u
#define HID_MAX_REPORT_IDS 8u
#define HID_MAX_REPORT_BITS (RIN_USB_HID_GAMEPAD_MAX_REPORT_BYTES * 8u)

#define HID_INT32_MAX 2147483647

typedef struct HidGlobalState {
    uint32_t usage_page;
    uint32_t report_size;
    uint32_t report_count;
    uint32_t logical_maximum_raw;
    int32_t logical_minimum;
    uint8_t report_id;
    uint8_t logical_maximum_size;
    uint8_t has_usage_page;
    uint8_t has_report_size;
    uint8_t has_report_count;
    uint8_t has_logical_minimum;
    uint8_t has_logical_maximum;
    uint8_t reserved[2];
} HidGlobalState;

typedef struct HidLocalState {
    uint32_t usages[HID_MAX_LOCAL_USAGES];
    uint32_t usage_count;
    uint32_t usage_minimum;
    uint32_t usage_maximum;
    uint8_t has_usage_minimum;
    uint8_t has_usage_maximum;
    uint8_t reserved[2];
} HidLocalState;

typedef struct HidReportBits {
    uint32_t bits;
    uint8_t report_id;
    uint8_t reserved[3];
} HidReportBits;

typedef struct HidParser {
    HidGlobalState global;
    HidGlobalState global_stack[HID_MAX_GLOBAL_STACK];
    HidLocalState local;
    HidReportBits reports[HID_MAX_REPORT_IDS];
    HidReportBits output_reports[HID_MAX_REPORT_IDS];
    uint8_t selected_collections[HID_MAX_COLLECTION_DEPTH];
    uint32_t global_depth;
    uint32_t collection_depth;
    uint32_t report_count;
    uint32_t output_report_count;
    uint8_t selected_report_id;
    uint8_t selected_output_report_id;
    uint8_t selected_report_set;
    uint8_t selected_output_report_set;
    uint8_t selected_found;
    uint8_t selected_closed;
    uint8_t report_ids_declared;
    uint8_t validate_output;
} HidParser;

static void hid_gamepad_zero(void* pointer, uint32_t size) {
    volatile uint8_t* output = (volatile uint8_t*)pointer;
    while (size-- != 0u) *output++ = 0u;
}

static void hid_gamepad_copy(void* destination, const void* source,
                             uint32_t size) {
    volatile uint8_t* output = (volatile uint8_t*)destination;
    const volatile uint8_t* input =
        (const volatile uint8_t*)source;
    while (size-- != 0u) *output++ = *input++;
}

static uint32_t hid_gamepad_all_zero(const void* pointer, uint32_t size) {
    const uint8_t* input = (const uint8_t*)pointer;
    uint8_t combined = 0u;
    while (size-- != 0u) combined |= *input++;
    return combined == 0u;
}

static uint32_t hid_gamepad_ranges_overlap(
    const void* first, uint32_t first_size,
    const void* second, uint32_t second_size) {
    uintptr_t first_start = (uintptr_t)first;
    uintptr_t second_start = (uintptr_t)second;
    uintptr_t first_end;
    uintptr_t second_end;
    if (first_start > UINTPTR_MAX - first_size ||
        second_start > UINTPTR_MAX - second_size) {
        return 1u;
    }
    first_end = first_start + first_size;
    second_end = second_start + second_size;
    return first_start < second_end && second_start < first_end;
}

static uint32_t hid_item_unsigned(const uint8_t* data, uint32_t size) {
    uint32_t value = 0u;
    for (uint32_t index = 0u; index < size; ++index) {
        value |= (uint32_t)data[index] << (index * 8u);
    }
    return value;
}

static int32_t hid_item_signed(const uint8_t* data, uint32_t size) {
    uint32_t value = hid_item_unsigned(data, size);
    if (size != 0u && size < 4u &&
        (value & ((uint32_t)1u << (size * 8u - 1u))) != 0u) {
        value |= UINT32_MAX << (size * 8u);
    }
    return (int32_t)value;
}

static uint32_t hid_usage_value(uint32_t value, uint32_t size,
                                const HidGlobalState* global,
                                uint32_t* usage) {
    uint32_t page;
    uint32_t identifier;
    if (size == 4u && (value >> 16u) != 0u) {
        page = value >> 16u;
        identifier = value & 0xffffu;
    } else {
        if (!global->has_usage_page || value > 0xffffu) return 0u;
        page = global->usage_page;
        identifier = value;
    }
    if (page == 0u || page > 0xffffu || identifier == 0u) return 0u;
    *usage = (page << 16u) | identifier;
    return 1u;
}

static void hid_clear_local(HidLocalState* local) {
    hid_gamepad_zero(local, sizeof(*local));
}

static RinUsbHidGamepadResult hid_handle_global(
    HidParser* parser, uint32_t tag, const uint8_t* data,
    uint32_t size) {
    uint32_t value = hid_item_unsigned(data, size);
    switch (tag) {
        case HID_GLOBAL_USAGE_PAGE:
            if (size == 0u || value == 0u || value > 0xffffu) {
                return RIN_USB_HID_GAMEPAD_MALFORMED;
            }
            parser->global.usage_page = value;
            parser->global.has_usage_page = 1u;
            return RIN_USB_HID_GAMEPAD_OK;
        case HID_GLOBAL_LOGICAL_MINIMUM:
            if (size == 0u) return RIN_USB_HID_GAMEPAD_MALFORMED;
            parser->global.logical_minimum = hid_item_signed(data, size);
            parser->global.has_logical_minimum = 1u;
            return RIN_USB_HID_GAMEPAD_OK;
        case HID_GLOBAL_LOGICAL_MAXIMUM:
            if (size == 0u) return RIN_USB_HID_GAMEPAD_MALFORMED;
            parser->global.logical_maximum_raw = value;
            parser->global.logical_maximum_size = (uint8_t)size;
            parser->global.has_logical_maximum = 1u;
            return RIN_USB_HID_GAMEPAD_OK;
        case HID_GLOBAL_PHYSICAL_MINIMUM:
        case HID_GLOBAL_PHYSICAL_MAXIMUM:
        case HID_GLOBAL_UNIT_EXPONENT:
        case HID_GLOBAL_UNIT:
            return size == 0u ? RIN_USB_HID_GAMEPAD_MALFORMED
                              : RIN_USB_HID_GAMEPAD_OK;
        case HID_GLOBAL_REPORT_SIZE:
            if (size == 0u || value == 0u || value > 32u) {
                return RIN_USB_HID_GAMEPAD_MALFORMED;
            }
            parser->global.report_size = value;
            parser->global.has_report_size = 1u;
            return RIN_USB_HID_GAMEPAD_OK;
        case HID_GLOBAL_REPORT_ID:
            if (size != 1u || value == 0u || value > 255u) {
                return RIN_USB_HID_GAMEPAD_MALFORMED;
            }
            parser->global.report_id = (uint8_t)value;
            parser->report_ids_declared = 1u;
            return RIN_USB_HID_GAMEPAD_OK;
        case HID_GLOBAL_REPORT_COUNT:
            if (size == 0u || value == 0u || value > 512u) {
                return RIN_USB_HID_GAMEPAD_MALFORMED;
            }
            parser->global.report_count = value;
            parser->global.has_report_count = 1u;
            return RIN_USB_HID_GAMEPAD_OK;
        case HID_GLOBAL_PUSH:
            if (size != 0u) return RIN_USB_HID_GAMEPAD_MALFORMED;
            if (parser->global_depth == HID_MAX_GLOBAL_STACK) {
                return RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
            }
            parser->global_stack[parser->global_depth++] = parser->global;
            return RIN_USB_HID_GAMEPAD_OK;
        case HID_GLOBAL_POP:
            if (size != 0u || parser->global_depth == 0u) {
                return RIN_USB_HID_GAMEPAD_MALFORMED;
            }
            parser->global = parser->global_stack[--parser->global_depth];
            return RIN_USB_HID_GAMEPAD_OK;
        default:
            return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
    }
}

static RinUsbHidGamepadResult hid_handle_local(
    HidParser* parser, uint32_t tag, const uint8_t* data,
    uint32_t size) {
    uint32_t usage;
    if (tag > HID_LOCAL_USAGE_MAXIMUM) {
        return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
    }
    if (size == 0u ||
        !hid_usage_value(hid_item_unsigned(data, size), size,
                         &parser->global, &usage)) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    if (tag == HID_LOCAL_USAGE) {
        if (parser->local.usage_count == HID_MAX_LOCAL_USAGES) {
            return RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
        }
        parser->local.usages[parser->local.usage_count++] = usage;
    } else if (tag == HID_LOCAL_USAGE_MINIMUM) {
        if (parser->local.has_usage_minimum) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
        parser->local.usage_minimum = usage;
        parser->local.has_usage_minimum = 1u;
    } else {
        if (parser->local.has_usage_maximum) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
        parser->local.usage_maximum = usage;
        parser->local.has_usage_maximum = 1u;
    }
    return RIN_USB_HID_GAMEPAD_OK;
}

static RinUsbHidGamepadResult hid_local_usage_at(
    const HidLocalState* local, uint32_t count, uint32_t index,
    uint32_t* usage) {
    if (local->usage_count != 0u) {
        if (local->has_usage_minimum || local->has_usage_maximum ||
            local->usage_count != count) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
        *usage = local->usages[index];
        return RIN_USB_HID_GAMEPAD_OK;
    }
    if (!local->has_usage_minimum || !local->has_usage_maximum ||
        (local->usage_minimum >> 16u) !=
            (local->usage_maximum >> 16u) ||
        local->usage_maximum < local->usage_minimum ||
        local->usage_maximum - local->usage_minimum + 1u != count) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    *usage = local->usage_minimum + index;
    return RIN_USB_HID_GAMEPAD_OK;
}

static HidReportBits* hid_report_bits(HidParser* parser, uint8_t report_id) {
    for (uint32_t index = 0u; index < parser->report_count; ++index) {
        if (parser->reports[index].report_id == report_id) {
            return &parser->reports[index];
        }
    }
    if (parser->report_count == HID_MAX_REPORT_IDS) return 0;
    parser->reports[parser->report_count].report_id = report_id;
    return &parser->reports[parser->report_count++];
}

static HidReportBits* hid_output_report_bits(HidParser* parser,
                                             uint8_t report_id) {
    for (uint32_t index = 0u; index < parser->output_report_count;
         ++index) {
        if (parser->output_reports[index].report_id == report_id) {
            return &parser->output_reports[index];
        }
    }
    if (parser->output_report_count == HID_MAX_REPORT_IDS) return 0;
    parser->output_reports[parser->output_report_count].report_id =
        report_id;
    return &parser->output_reports[parser->output_report_count++];
}

static RinUsbHidGamepadResult hid_logical_range(
    const HidGlobalState* global, int32_t* minimum, int32_t* maximum) {
    int32_t parsed_maximum;
    if (!global->has_logical_minimum || !global->has_logical_maximum) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    if (global->logical_minimum < 0) {
        uint8_t encoded[4];
        uint32_t raw = global->logical_maximum_raw;
        for (uint32_t index = 0u; index < 4u; ++index) {
            encoded[index] = (uint8_t)(raw >> (index * 8u));
        }
        parsed_maximum = hid_item_signed(
            encoded, global->logical_maximum_size);
    } else {
        if (global->logical_maximum_raw > (uint32_t)HID_INT32_MAX) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
        parsed_maximum = (int32_t)global->logical_maximum_raw;
    }
    if (global->logical_minimum >= parsed_maximum) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    *minimum = global->logical_minimum;
    *maximum = parsed_maximum;
    return RIN_USB_HID_GAMEPAD_OK;
}

static int32_t hid_axis_index(uint32_t usage) {
    uint32_t page = usage >> 16u;
    uint32_t identifier = usage & 0xffffu;
    if (page != HID_USAGE_PAGE_GENERIC_DESKTOP ||
        identifier < HID_USAGE_X || identifier > HID_USAGE_WHEEL) {
        return -1;
    }
    return (int32_t)(identifier - HID_USAGE_X);
}

static RinUsbHidGamepadResult hid_record_known_field(
    RinUsbHidGamepadProfileV1* profile, uint32_t usage,
    uint32_t bit_offset, uint32_t bit_size, uint32_t input_flags,
    int32_t logical_minimum, int32_t logical_maximum) {
    uint32_t page = usage >> 16u;
    uint32_t identifier = usage & 0xffffu;
    int32_t axis = hid_axis_index(usage);
    if (axis >= 0) {
        RinUsbHidGamepadControlV1* control;
        uint32_t mask = (uint32_t)1u << (uint32_t)axis;
        int32_t raw_minimum;
        int32_t raw_maximum;
        if (logical_minimum < 0) {
            raw_minimum = -((int32_t)1 << (bit_size - 1u));
            raw_maximum = ((int32_t)1 << (bit_size - 1u)) - 1;
        } else {
            raw_minimum = 0;
            raw_maximum = ((int32_t)1 << bit_size) - 1;
        }
        if (bit_size < 4u || bit_size > 16u ||
            (input_flags & HID_INPUT_NULL_STATE) != 0u ||
            logical_minimum < -65535 || logical_maximum > 65535 ||
            logical_minimum < raw_minimum ||
            logical_maximum > raw_maximum ||
            (profile->axis_mask & mask) != 0u) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
        control = &profile->axes[axis];
        control->bit_offset = (uint16_t)bit_offset;
        control->bit_size = (uint8_t)bit_size;
        control->flags = logical_minimum < 0
            ? RIN_USB_HID_GAMEPAD_CONTROL_SIGNED : 0u;
        control->logical_minimum = logical_minimum;
        control->logical_maximum = logical_maximum;
        profile->axis_mask |= mask;
        return RIN_USB_HID_GAMEPAD_OK;
    }
    if (page == HID_USAGE_PAGE_GENERIC_DESKTOP &&
        identifier == HID_USAGE_HAT_SWITCH) {
        uint32_t positions = (uint32_t)(logical_maximum - logical_minimum + 1);
        uint32_t raw_maximum = ((uint32_t)1u << bit_size) - 1u;
        if ((positions != 4u && positions != 8u) ||
            bit_size < 2u || bit_size > 8u ||
            (profile->flags & RIN_USB_HID_GAMEPAD_PROFILE_HAS_HAT) != 0u ||
            logical_minimum < 0 || logical_maximum > 255 ||
            (uint32_t)logical_maximum > raw_maximum) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
        profile->hat.bit_offset = (uint16_t)bit_offset;
        profile->hat.bit_size = (uint8_t)bit_size;
        profile->hat.flags = (input_flags & HID_INPUT_NULL_STATE) != 0u
            ? RIN_USB_HID_GAMEPAD_CONTROL_NULL_STATE : 0u;
        profile->hat.logical_minimum = logical_minimum;
        profile->hat.logical_maximum = logical_maximum;
        profile->hat_positions = (uint8_t)positions;
        profile->flags |= RIN_USB_HID_GAMEPAD_PROFILE_HAS_HAT;
        return RIN_USB_HID_GAMEPAD_OK;
    }
    if (page == HID_USAGE_PAGE_BUTTON && identifier >= 1u &&
        identifier <= RIN_USB_HID_GAMEPAD_MAX_BUTTONS) {
        uint32_t index = identifier - 1u;
        if (bit_size != 1u || logical_minimum != 0 ||
            logical_maximum != 1 ||
            (input_flags & HID_INPUT_NULL_STATE) != 0u ||
            profile->button_bit_offsets[index] != UINT16_MAX) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
        profile->button_bit_offsets[index] = (uint16_t)bit_offset;
        ++profile->button_count;
    }
    return RIN_USB_HID_GAMEPAD_OK;
}

static RinUsbHidGamepadResult hid_handle_input(
    HidParser* parser, uint32_t input_flags,
    RinUsbHidGamepadProfileV1* profile) {
    HidReportBits* report;
    uint32_t bits;
    uint32_t first_bit;
    uint32_t selected = parser->collection_depth != 0u &&
        parser->selected_collections[parser->collection_depth - 1u] != 0u;
    int32_t logical_minimum = 0;
    int32_t logical_maximum = 0;
    RinUsbHidGamepadResult result;

    if (!parser->global.has_report_size ||
        !parser->global.has_report_count ||
        parser->global.report_count >
            UINT32_MAX / parser->global.report_size) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    bits = parser->global.report_size * parser->global.report_count;
    report = hid_report_bits(parser, parser->global.report_id);
    if (!report) return RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
    first_bit = report->bits;
    if (bits > HID_MAX_REPORT_BITS - report->bits) {
        return RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
    }
    report->bits += bits;
    if (!selected) return RIN_USB_HID_GAMEPAD_OK;

    if (!parser->selected_report_set) {
        parser->selected_report_id = parser->global.report_id;
        parser->selected_report_set = 1u;
    } else if (parser->selected_report_id != parser->global.report_id) {
        return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
    }
    if ((input_flags & HID_INPUT_CONSTANT) != 0u) {
        return RIN_USB_HID_GAMEPAD_OK;
    }
    if ((input_flags & HID_INPUT_VARIABLE) == 0u ||
        (input_flags & HID_INPUT_RELATIVE) != 0u ||
        (input_flags & ~HID_INPUT_SUPPORTED_FLAGS) != 0u ||
        parser->global.report_size > 16u) {
        return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
    }
    result = hid_logical_range(&parser->global, &logical_minimum,
                               &logical_maximum);
    if (result != RIN_USB_HID_GAMEPAD_OK) return result;
    for (uint32_t index = 0u; index < parser->global.report_count; ++index) {
        uint32_t usage;
        result = hid_local_usage_at(&parser->local,
                                    parser->global.report_count,
                                    index, &usage);
        if (result != RIN_USB_HID_GAMEPAD_OK) return result;
        result = hid_record_known_field(
            profile, usage,
            first_bit + index * parser->global.report_size,
            parser->global.report_size, input_flags,
            logical_minimum, logical_maximum);
        if (result != RIN_USB_HID_GAMEPAD_OK) return result;
    }
    return RIN_USB_HID_GAMEPAD_OK;
}

static RinUsbHidGamepadResult hid_handle_output(
    HidParser* parser, uint32_t output_flags) {
    HidReportBits* report;
    uint32_t bits;
    uint32_t selected = parser->collection_depth != 0u &&
        parser->selected_collections[parser->collection_depth - 1u] != 0u;

    if (!parser->validate_output || !selected)
        return RIN_USB_HID_GAMEPAD_OK;
    if (!parser->global.has_report_size ||
        !parser->global.has_report_count ||
        parser->global.report_count >
            UINT32_MAX / parser->global.report_size) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    bits = parser->global.report_size * parser->global.report_count;
    report = hid_output_report_bits(parser, parser->global.report_id);
    if (!report) return RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
    if (bits > HID_MAX_REPORT_BITS - report->bits) {
        return RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
    }
    report->bits += bits;
    if (!parser->selected_output_report_set) {
        parser->selected_output_report_id = parser->global.report_id;
        parser->selected_output_report_set = 1u;
    } else if (parser->selected_output_report_id !=
               parser->global.report_id) {
        return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
    }
    /* An opaque transport may never manufacture constant or relative fields.
     * Semantic PID/vendor mapping stays above this bounded byte protocol. */
    return output_flags == HID_INPUT_VARIABLE
               ? RIN_USB_HID_GAMEPAD_OK
               : RIN_USB_HID_GAMEPAD_UNSUPPORTED;
}

static RinUsbHidGamepadResult hid_handle_collection(
    HidParser* parser, uint32_t collection_type,
    RinUsbHidGamepadProfileV1* profile) {
    uint32_t usage;
    uint32_t parent_selected = parser->collection_depth != 0u &&
        parser->selected_collections[parser->collection_depth - 1u] != 0u;
    uint32_t selected = parent_selected;
    if (collection_type > 6u || parser->local.usage_count != 1u ||
        parser->local.has_usage_minimum || parser->local.has_usage_maximum) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    if (parser->collection_depth == HID_MAX_COLLECTION_DEPTH) {
        return RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
    }
    usage = parser->local.usages[0];
    if (collection_type == HID_COLLECTION_APPLICATION) {
        uint32_t page = usage >> 16u;
        uint32_t identifier = usage & 0xffffu;
        if (parent_selected) return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
        selected = 0u;
        if (page == HID_USAGE_PAGE_GENERIC_DESKTOP &&
            (identifier == RIN_USB_HID_GAMEPAD_APPLICATION_JOYSTICK ||
             identifier == RIN_USB_HID_GAMEPAD_APPLICATION_GAME_PAD)) {
            if (parser->collection_depth != 0u || parser->selected_found) {
                return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
            }
            selected = 1u;
            parser->selected_found = 1u;
            profile->application_usage = identifier;
        }
    }
    parser->selected_collections[parser->collection_depth++] =
        (uint8_t)selected;
    return RIN_USB_HID_GAMEPAD_OK;
}

static RinUsbHidGamepadResult hid_handle_main(
    HidParser* parser, uint32_t tag, const uint8_t* data,
    uint32_t size, RinUsbHidGamepadProfileV1* profile) {
    RinUsbHidGamepadResult result = RIN_USB_HID_GAMEPAD_OK;
    uint32_t value = hid_item_unsigned(data, size);
    if (tag == HID_MAIN_INPUT) {
        if (size == 0u) result = RIN_USB_HID_GAMEPAD_MALFORMED;
        else result = hid_handle_input(parser, value, profile);
    } else if (tag == HID_MAIN_OUTPUT) {
        if (size == 0u) result = RIN_USB_HID_GAMEPAD_MALFORMED;
        else result = hid_handle_output(parser, value);
    } else if (tag == HID_MAIN_FEATURE) {
        if (size == 0u) result = RIN_USB_HID_GAMEPAD_MALFORMED;
    } else if (tag == HID_MAIN_COLLECTION) {
        if (size != 1u) result = RIN_USB_HID_GAMEPAD_MALFORMED;
        else result = hid_handle_collection(parser, value, profile);
    } else if (tag == HID_MAIN_END_COLLECTION) {
        if (size != 0u || parser->collection_depth == 0u) {
            result = RIN_USB_HID_GAMEPAD_MALFORMED;
        } else {
            uint32_t closing_selected =
                parser->selected_collections[parser->collection_depth - 1u];
            uint32_t parent_selected = parser->collection_depth > 1u &&
                parser->selected_collections[parser->collection_depth - 2u];
            --parser->collection_depth;
            if (closing_selected && !parent_selected) {
                parser->selected_closed = 1u;
            }
        }
    } else {
        result = RIN_USB_HID_GAMEPAD_UNSUPPORTED;
    }
    hid_clear_local(&parser->local);
    return result;
}

static RinUsbHidGamepadResult hid_parse_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidGamepadProfileV1* profile,
    RinUsbHidGamepadOutputProfileV1* output_profile) {
    HidParser parser;
    uint32_t offset = 0u;
    HidReportBits* selected_report = 0;
    hid_gamepad_zero(&parser, sizeof(parser));
    parser.validate_output = output_profile != 0;

    while (offset < descriptor_size) {
        uint8_t prefix = descriptor[offset++];
        uint32_t item_size;
        uint32_t item_type;
        uint32_t item_tag;
        RinUsbHidGamepadResult result;
        if (prefix == 0xfeu) return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
        item_size = prefix & 3u;
        if (item_size == 3u) item_size = 4u;
        item_type = (prefix >> 2u) & 3u;
        item_tag = prefix >> 4u;
        if (item_size > descriptor_size - offset) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
        if (item_type == HID_ITEM_TYPE_GLOBAL) {
            result = hid_handle_global(&parser, item_tag,
                                       descriptor + offset, item_size);
        } else if (item_type == HID_ITEM_TYPE_LOCAL) {
            result = hid_handle_local(&parser, item_tag,
                                      descriptor + offset, item_size);
        } else if (item_type == HID_ITEM_TYPE_MAIN) {
            result = hid_handle_main(&parser, item_tag,
                                     descriptor + offset, item_size,
                                     profile);
        } else {
            result = RIN_USB_HID_GAMEPAD_UNSUPPORTED;
        }
        if (result != RIN_USB_HID_GAMEPAD_OK) return result;
        offset += item_size;
    }
    if (parser.collection_depth != 0u || parser.global_depth != 0u ||
        !hid_gamepad_all_zero(&parser.local, sizeof(parser.local)) ||
        !parser.selected_found || !parser.selected_closed ||
        !parser.selected_report_set ||
        (parser.report_ids_declared && parser.selected_report_id == 0u) ||
        (!parser.report_ids_declared && parser.selected_report_id != 0u)) {
        return parser.selected_found ? RIN_USB_HID_GAMEPAD_MALFORMED
                                     : RIN_USB_HID_GAMEPAD_UNSUPPORTED;
    }
    for (uint32_t index = 0u; index < parser.report_count; ++index) {
        if (parser.reports[index].report_id == parser.selected_report_id) {
            selected_report = &parser.reports[index];
            break;
        }
    }
    if (!selected_report || selected_report->bits == 0u) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    profile->report_id = parser.selected_report_id;
    profile->report_bit_count = selected_report->bits;
    profile->report_size = (selected_report->bits + 7u) / 8u;
    if (profile->report_id != 0u) {
        profile->flags |= RIN_USB_HID_GAMEPAD_PROFILE_HAS_REPORT_ID;
        ++profile->report_size;
    }
    if (profile->report_size > RIN_USB_HID_GAMEPAD_MAX_REPORT_BYTES) {
        return RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
    }
    if ((profile->axis_mask &
         (((uint32_t)1u << RIN_USB_HID_GAMEPAD_AXIS_X) |
          ((uint32_t)1u << RIN_USB_HID_GAMEPAD_AXIS_Y))) !=
        (((uint32_t)1u << RIN_USB_HID_GAMEPAD_AXIS_X) |
         ((uint32_t)1u << RIN_USB_HID_GAMEPAD_AXIS_Y))) {
        return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
    }
    if ((profile->application_usage ==
             RIN_USB_HID_GAMEPAD_APPLICATION_GAME_PAD &&
         profile->button_count < 4u) ||
        (profile->application_usage ==
             RIN_USB_HID_GAMEPAD_APPLICATION_JOYSTICK &&
         profile->button_count == 0u)) {
        return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
    }
    if (output_profile) {
        HidReportBits* selected_output = 0;
        uint32_t prefix_size;
        if (!parser.selected_output_report_set) {
            return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
        }
        for (uint32_t index = 0u; index < parser.output_report_count;
             ++index) {
            if (parser.output_reports[index].report_id ==
                parser.selected_output_report_id) {
                selected_output = &parser.output_reports[index];
                break;
            }
        }
        if (!selected_output || selected_output->bits == 0u ||
            selected_output->bits > HID_MAX_REPORT_BITS ||
            (selected_output->bits & 7u) != 0u) {
            return RIN_USB_HID_GAMEPAD_UNSUPPORTED;
        }
        prefix_size = parser.selected_output_report_id != 0u;
        output_profile->abi_version = RIN_USB_HID_GAMEPAD_ABI_VERSION;
        output_profile->struct_size = sizeof(*output_profile);
        output_profile->descriptor_size = descriptor_size;
        output_profile->report_bit_count = selected_output->bits;
        output_profile->report_size = selected_output->bits / 8u +
                                      prefix_size;
        output_profile->report_id = parser.selected_output_report_id;
        if (prefix_size != 0u) {
            output_profile->flags =
                RIN_USB_HID_GAMEPAD_OUTPUT_HAS_REPORT_ID;
        }
        if (output_profile->report_size == 0u ||
            output_profile->report_size >
                RIN_USB_HID_GAMEPAD_MAX_REPORT_BYTES) {
            return RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
        }
    }
    return RIN_USB_HID_GAMEPAD_OK;
}

RinUsbHidGamepadResult rin_usb_hid_gamepad_admit_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidGamepadProfileV1* profile) {
    uint8_t snapshot[RIN_USB_HID_GAMEPAD_MAX_DESCRIPTOR_BYTES];
    RinUsbHidGamepadProfileV1 candidate;
    RinUsbHidGamepadResult result;
    if (!profile) return RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    if (!descriptor || descriptor_size == 0u ||
        descriptor_size > RIN_USB_HID_GAMEPAD_MAX_DESCRIPTOR_BYTES ||
        hid_gamepad_ranges_overlap(descriptor, descriptor_size,
                                   profile, sizeof(*profile))) {
        hid_gamepad_zero(profile, sizeof(*profile));
        return descriptor_size > RIN_USB_HID_GAMEPAD_MAX_DESCRIPTOR_BYTES
            ? RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED
            : RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    }
    hid_gamepad_copy(snapshot, descriptor, descriptor_size);
    hid_gamepad_zero(profile, sizeof(*profile));
    hid_gamepad_zero(&candidate, sizeof(candidate));
    for (uint32_t index = 0u;
         index < RIN_USB_HID_GAMEPAD_MAX_BUTTONS; ++index) {
        candidate.button_bit_offsets[index] = UINT16_MAX;
    }
    candidate.abi_version = RIN_USB_HID_GAMEPAD_ABI_VERSION;
    candidate.struct_size = sizeof(candidate);
    candidate.descriptor_size = descriptor_size;
    result = hid_parse_descriptor(snapshot, descriptor_size, &candidate,
                                  0);
    if (result != RIN_USB_HID_GAMEPAD_OK) return result;
    hid_gamepad_copy(profile, &candidate, sizeof(candidate));
    return RIN_USB_HID_GAMEPAD_OK;
}

RinUsbHidGamepadResult rin_usb_hid_gamepad_admit_output_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidGamepadOutputProfileV1* profile) {
    uint8_t snapshot[RIN_USB_HID_GAMEPAD_MAX_DESCRIPTOR_BYTES];
    RinUsbHidGamepadProfileV1 input;
    RinUsbHidGamepadOutputProfileV1 candidate;
    RinUsbHidGamepadResult result;
    if (!profile) return RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    if (!descriptor || descriptor_size == 0u ||
        descriptor_size > RIN_USB_HID_GAMEPAD_MAX_DESCRIPTOR_BYTES ||
        hid_gamepad_ranges_overlap(descriptor, descriptor_size,
                                   profile, sizeof(*profile))) {
        hid_gamepad_zero(profile, sizeof(*profile));
        return descriptor_size > RIN_USB_HID_GAMEPAD_MAX_DESCRIPTOR_BYTES
            ? RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED
            : RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    }
    hid_gamepad_copy(snapshot, descriptor, descriptor_size);
    hid_gamepad_zero(profile, sizeof(*profile));
    hid_gamepad_zero(&input, sizeof(input));
    hid_gamepad_zero(&candidate, sizeof(candidate));
    for (uint32_t index = 0u;
         index < RIN_USB_HID_GAMEPAD_MAX_BUTTONS; ++index) {
        input.button_bit_offsets[index] = UINT16_MAX;
    }
    input.abi_version = RIN_USB_HID_GAMEPAD_ABI_VERSION;
    input.struct_size = sizeof(input);
    input.descriptor_size = descriptor_size;
    result = hid_parse_descriptor(snapshot, descriptor_size, &input,
                                  &candidate);
    hid_gamepad_zero(&input, sizeof(input));
    if (result != RIN_USB_HID_GAMEPAD_OK) {
        hid_gamepad_zero(&candidate, sizeof(candidate));
        return result;
    }
    hid_gamepad_copy(profile, &candidate, sizeof(candidate));
    hid_gamepad_zero(&candidate, sizeof(candidate));
    return RIN_USB_HID_GAMEPAD_OK;
}

static uint32_t hid_output_profile_valid(
    const RinUsbHidGamepadOutputProfileV1* profile) {
    uint32_t prefix_size;
    if (!profile ||
        profile->abi_version != RIN_USB_HID_GAMEPAD_ABI_VERSION ||
        profile->struct_size != sizeof(*profile) ||
        profile->descriptor_size == 0u ||
        profile->descriptor_size >
            RIN_USB_HID_GAMEPAD_MAX_DESCRIPTOR_BYTES ||
        profile->report_bit_count == 0u ||
        profile->report_bit_count > HID_MAX_REPORT_BITS ||
        (profile->report_bit_count & 7u) != 0u ||
        profile->report_size == 0u ||
        profile->report_size > RIN_USB_HID_GAMEPAD_MAX_REPORT_BYTES ||
        (profile->flags & ~RIN_USB_HID_GAMEPAD_OUTPUT_HAS_REPORT_ID) !=
            0u ||
        !hid_gamepad_all_zero(profile->reserved0,
                              sizeof(profile->reserved0)) ||
        profile->reserved1 != 0u ||
        !hid_gamepad_all_zero(profile->reserved, sizeof(profile->reserved))) {
        return 0u;
    }
    prefix_size = (profile->flags &
                   RIN_USB_HID_GAMEPAD_OUTPUT_HAS_REPORT_ID) != 0u;
    return (prefix_size != 0u && profile->report_id != 0u) ||
           (prefix_size == 0u && profile->report_id == 0u)
        ? profile->report_size == profile->report_bit_count / 8u +
              prefix_size
        : 0u;
}

RinUsbHidGamepadResult rin_usb_hid_gamepad_validate_output_report(
    const RinUsbHidGamepadOutputProfileV1* profile,
    const uint8_t* report, uint32_t report_size) {
    uint32_t prefix_size;
    if (!profile || !report ||
        hid_gamepad_ranges_overlap(profile, sizeof(*profile), report,
                                   report_size)) {
        return RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    }
    if (!hid_output_profile_valid(profile))
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    if (report_size != profile->report_size)
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    prefix_size = (profile->flags &
                   RIN_USB_HID_GAMEPAD_OUTPUT_HAS_REPORT_ID) != 0u;
    if (prefix_size != 0u && report[0] != profile->report_id)
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    return RIN_USB_HID_GAMEPAD_OK;
}

static uint32_t hid_extract_bits(const uint8_t* payload,
                                 uint32_t bit_offset,
                                 uint32_t bit_size) {
    uint32_t value = 0u;
    for (uint32_t bit = 0u; bit < bit_size; ++bit) {
        uint32_t source_bit = bit_offset + bit;
        if ((payload[source_bit / 8u] &
             ((uint8_t)1u << (source_bit % 8u))) != 0u) {
            value |= (uint32_t)1u << bit;
        }
    }
    return value;
}

static int32_t hid_control_value(const uint8_t* payload,
                                 const RinUsbHidGamepadControlV1* control) {
    uint32_t raw = hid_extract_bits(payload, control->bit_offset,
                                    control->bit_size);
    if ((control->flags & RIN_USB_HID_GAMEPAD_CONTROL_SIGNED) != 0u &&
        control->bit_size != 0u && control->bit_size < 32u &&
        (raw & ((uint32_t)1u << (control->bit_size - 1u))) != 0u) {
        raw |= UINT32_MAX << control->bit_size;
    }
    return (int32_t)raw;
}

static uint64_t hid_divide_u64_u32(uint64_t numerator,
                                   uint32_t denominator) {
    uint64_t quotient = 0u;
    uint64_t remainder = 0u;
    if (denominator == 0u) return 0u;
    for (uint32_t bit = 64u; bit != 0u; --bit) {
        remainder = (remainder << 1u) |
                    ((numerator >> (bit - 1u)) & 1u);
        if (remainder >= denominator) {
            remainder -= denominator;
            quotient |= (uint64_t)1u << (bit - 1u);
        }
    }
    return quotient;
}

static int16_t hid_normalize_axis(
    int32_t value, const RinUsbHidGamepadControlV1* control) {
    uint32_t delta = (uint32_t)(value - control->logical_minimum);
    uint32_t span = (uint32_t)(control->logical_maximum -
                               control->logical_minimum);
    uint64_t scaled = ((uint64_t)delta << 16u) -
                      ((uint64_t)delta << 1u);
    scaled += span / 2u;
    return (int16_t)((int32_t)hid_divide_u64_u32(scaled, span) - 32767);
}

static uint32_t hid_mark_control_bits(
    uint8_t used[RIN_USB_HID_GAMEPAD_MAX_REPORT_BYTES],
    const RinUsbHidGamepadControlV1* control, uint32_t report_bits) {
    if (control->bit_size == 0u || control->bit_size > 16u ||
        control->bit_offset > report_bits ||
        control->bit_size > report_bits - control->bit_offset) {
        return 0u;
    }
    for (uint32_t bit = 0u; bit < control->bit_size; ++bit) {
        uint32_t position = control->bit_offset + bit;
        uint8_t mask = (uint8_t)((uint8_t)1u << (position % 8u));
        if ((used[position / 8u] & mask) != 0u) return 0u;
        used[position / 8u] |= mask;
    }
    return 1u;
}

static uint32_t hid_control_range_valid(
    const RinUsbHidGamepadControlV1* control) {
    int32_t raw_minimum;
    int32_t raw_maximum;
    if (control->bit_size == 0u || control->bit_size > 16u ||
        control->logical_minimum >= control->logical_maximum ||
        (control->flags & ~(RIN_USB_HID_GAMEPAD_CONTROL_SIGNED |
                            RIN_USB_HID_GAMEPAD_CONTROL_NULL_STATE)) != 0u) {
        return 0u;
    }
    if ((control->flags & RIN_USB_HID_GAMEPAD_CONTROL_SIGNED) != 0u) {
        raw_minimum = -((int32_t)1 << (control->bit_size - 1u));
        raw_maximum = ((int32_t)1 << (control->bit_size - 1u)) - 1;
    } else {
        raw_minimum = 0;
        raw_maximum = ((int32_t)1 << control->bit_size) - 1;
    }
    return control->logical_minimum >= raw_minimum &&
           control->logical_maximum <= raw_maximum;
}

static uint32_t hid_profile_valid(const RinUsbHidGamepadProfileV1* profile) {
    uint8_t used[RIN_USB_HID_GAMEPAD_MAX_REPORT_BYTES];
    uint32_t observed_buttons = 0u;
    uint32_t allowed_axis_mask =
        ((uint32_t)1u << RIN_USB_HID_GAMEPAD_MAX_AXES) - 1u;
    uint32_t prefix_size;
    hid_gamepad_zero(used, sizeof(used));
    if (profile->abi_version != RIN_USB_HID_GAMEPAD_ABI_VERSION ||
        profile->struct_size != sizeof(*profile) ||
        profile->descriptor_size == 0u ||
        profile->descriptor_size >
            RIN_USB_HID_GAMEPAD_MAX_DESCRIPTOR_BYTES ||
        profile->report_bit_count == 0u ||
        profile->report_bit_count > HID_MAX_REPORT_BITS ||
        profile->report_size == 0u ||
        profile->report_size > RIN_USB_HID_GAMEPAD_MAX_REPORT_BYTES ||
        (profile->application_usage !=
             RIN_USB_HID_GAMEPAD_APPLICATION_GAME_PAD &&
         profile->application_usage !=
             RIN_USB_HID_GAMEPAD_APPLICATION_JOYSTICK) ||
        (profile->flags & ~(RIN_USB_HID_GAMEPAD_PROFILE_HAS_REPORT_ID |
                            RIN_USB_HID_GAMEPAD_PROFILE_HAS_HAT)) != 0u ||
        (profile->axis_mask & ~allowed_axis_mask) != 0u ||
        (profile->axis_mask & 3u) != 3u ||
        profile->button_count > RIN_USB_HID_GAMEPAD_MAX_BUTTONS ||
        profile->reserved0 != 0u ||
        !hid_gamepad_all_zero(profile->reserved,
                              sizeof(profile->reserved))) {
        return 0u;
    }
    prefix_size = (profile->flags &
                   RIN_USB_HID_GAMEPAD_PROFILE_HAS_REPORT_ID) != 0u;
    if ((prefix_size != 0u && profile->report_id == 0u) ||
        (prefix_size == 0u && profile->report_id != 0u) ||
        profile->report_size !=
            (profile->report_bit_count + 7u) / 8u + prefix_size) {
        return 0u;
    }
    for (uint32_t axis = 0u;
         axis < RIN_USB_HID_GAMEPAD_MAX_AXES; ++axis) {
        const RinUsbHidGamepadControlV1* control = &profile->axes[axis];
        if ((profile->axis_mask & ((uint32_t)1u << axis)) != 0u) {
            if ((control->flags &
                 RIN_USB_HID_GAMEPAD_CONTROL_NULL_STATE) != 0u ||
                !hid_control_range_valid(control) ||
                !hid_mark_control_bits(used, control,
                                       profile->report_bit_count)) {
                return 0u;
            }
        } else if (!hid_gamepad_all_zero(control, sizeof(*control))) {
            return 0u;
        }
    }
    for (uint32_t button = 0u;
         button < RIN_USB_HID_GAMEPAD_MAX_BUTTONS; ++button) {
        uint16_t bit_offset = profile->button_bit_offsets[button];
        if (bit_offset == UINT16_MAX) continue;
        if (bit_offset >= profile->report_bit_count ||
            (used[bit_offset / 8u] &
             ((uint8_t)1u << (bit_offset % 8u))) != 0u) {
            return 0u;
        }
        used[bit_offset / 8u] |=
            (uint8_t)((uint8_t)1u << (bit_offset % 8u));
        ++observed_buttons;
    }
    if (observed_buttons != profile->button_count ||
        (profile->application_usage ==
             RIN_USB_HID_GAMEPAD_APPLICATION_GAME_PAD &&
         observed_buttons < 4u) ||
        (profile->application_usage ==
             RIN_USB_HID_GAMEPAD_APPLICATION_JOYSTICK &&
         observed_buttons == 0u)) {
        return 0u;
    }
    if ((profile->flags & RIN_USB_HID_GAMEPAD_PROFILE_HAS_HAT) != 0u) {
        if ((profile->hat_positions != 4u &&
             profile->hat_positions != 8u) ||
            (profile->hat.flags & RIN_USB_HID_GAMEPAD_CONTROL_SIGNED) != 0u ||
            !hid_control_range_valid(&profile->hat) ||
            (uint32_t)(profile->hat.logical_maximum -
                       profile->hat.logical_minimum + 1) !=
                profile->hat_positions ||
            !hid_mark_control_bits(used, &profile->hat,
                                   profile->report_bit_count)) {
            return 0u;
        }
    } else if (profile->hat_positions != 0u ||
               !hid_gamepad_all_zero(&profile->hat,
                                     sizeof(profile->hat))) {
        return 0u;
    }
    return 1u;
}

RinUsbHidGamepadResult rin_usb_hid_gamepad_decode_report(
    const RinUsbHidGamepadProfileV1* profile,
    const uint8_t* report, uint32_t report_size,
    RinUsbHidGamepadSampleV1* sample) {
    uint8_t snapshot[RIN_USB_HID_GAMEPAD_MAX_REPORT_BYTES];
    RinUsbHidGamepadSampleV1 candidate;
    const uint8_t* payload;
    uint32_t prefix_size;
    if (!sample) return RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    if (!profile || !report ||
        hid_gamepad_ranges_overlap(profile, sizeof(*profile),
                                   sample, sizeof(*sample)) ||
        hid_gamepad_ranges_overlap(report, report_size,
                                   sample, sizeof(*sample))) {
        hid_gamepad_zero(sample, sizeof(*sample));
        return RIN_USB_HID_GAMEPAD_INVALID_ARGUMENT;
    }
    if (report_size == 0u || report_size > sizeof(snapshot)) {
        hid_gamepad_zero(sample, sizeof(*sample));
        return report_size == 0u ? RIN_USB_HID_GAMEPAD_MALFORMED
                                 : RIN_USB_HID_GAMEPAD_LIMIT_EXCEEDED;
    }
    hid_gamepad_copy(snapshot, report, report_size);
    hid_gamepad_zero(sample, sizeof(*sample));
    if (!hid_profile_valid(profile)) return RIN_USB_HID_GAMEPAD_MALFORMED;
    if (report_size != profile->report_size) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    prefix_size = (profile->flags &
                   RIN_USB_HID_GAMEPAD_PROFILE_HAS_REPORT_ID) != 0u;
    if (prefix_size != 0u && snapshot[0] != profile->report_id) {
        return RIN_USB_HID_GAMEPAD_MALFORMED;
    }
    payload = snapshot + prefix_size;
    if ((profile->report_bit_count & 7u) != 0u) {
        uint32_t last = profile->report_bit_count / 8u;
        uint8_t allowed = (uint8_t)(((uint32_t)1u <<
            (profile->report_bit_count & 7u)) - 1u);
        if ((payload[last] & (uint8_t)~allowed) != 0u) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
    }

    hid_gamepad_zero(&candidate, sizeof(candidate));
    candidate.abi_version = RIN_USB_HID_GAMEPAD_ABI_VERSION;
    candidate.struct_size = sizeof(candidate);
    candidate.axis_mask = profile->axis_mask;
    candidate.report_id = profile->report_id;
    candidate.hat = RIN_USB_HID_GAMEPAD_HAT_NEUTRAL;
    for (uint32_t axis = 0u;
         axis < RIN_USB_HID_GAMEPAD_MAX_AXES; ++axis) {
        const RinUsbHidGamepadControlV1* control = &profile->axes[axis];
        int32_t value;
        if ((profile->axis_mask & ((uint32_t)1u << axis)) == 0u) continue;
        value = hid_control_value(payload, control);
        if (value < control->logical_minimum ||
            value > control->logical_maximum) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
        candidate.axes[axis] = hid_normalize_axis(value, control);
    }
    for (uint32_t button = 0u;
         button < RIN_USB_HID_GAMEPAD_MAX_BUTTONS; ++button) {
        uint16_t bit_offset = profile->button_bit_offsets[button];
        if (bit_offset != UINT16_MAX &&
            hid_extract_bits(payload, bit_offset, 1u) != 0u) {
            candidate.buttons |= (uint32_t)1u << button;
        }
    }
    if ((profile->flags & RIN_USB_HID_GAMEPAD_PROFILE_HAS_HAT) != 0u) {
        int32_t value = hid_control_value(payload, &profile->hat);
        candidate.flags |= RIN_USB_HID_GAMEPAD_SAMPLE_HAS_HAT;
        if (value >= profile->hat.logical_minimum &&
            value <= profile->hat.logical_maximum) {
            uint32_t position = (uint32_t)(value -
                                           profile->hat.logical_minimum);
            candidate.hat = (uint8_t)(profile->hat_positions == 4u
                ? position * 2u : position);
        } else if ((profile->hat.flags &
                    RIN_USB_HID_GAMEPAD_CONTROL_NULL_STATE) == 0u) {
            return RIN_USB_HID_GAMEPAD_MALFORMED;
        }
    }
    hid_gamepad_copy(sample, &candidate, sizeof(candidate));
    return RIN_USB_HID_GAMEPAD_OK;
}
