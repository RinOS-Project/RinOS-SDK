/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <rin/driver/usb/hid/gamepad_force_feedback_v1.h>

#include <stddef.h>

#define FF_PID_PAGE UINT32_C(0x0f)
#define FF_SET_EFFECT UINT32_C(0x21)
#define FF_EFFECT_OPERATION UINT32_C(0x77)
#define FF_DEVICE_CONTROL UINT32_C(0x96)
#define FF_SET_CONSTANT UINT32_C(0x73)
#define FF_EFFECT_BLOCK_INDEX UINT32_C(0x22)
#define FF_DURATION UINT32_C(0x50)
#define FF_MAGNITUDE UINT32_C(0x70)
#define FF_OPERATION UINT32_C(0x78)
#define FF_STOP_ALL UINT32_C(0x99)

typedef struct FfParser {
    uint32_t usage_page;
    uint32_t usage;
    uint32_t usage_min;
    uint32_t usage_max;
    uint8_t usage_is_range;
    int32_t logical_minimum;
    int32_t logical_maximum;
    uint32_t report_size;
    uint32_t report_count;
    uint8_t report_id;
    uint32_t collection_depth;
    uint32_t collection_page[8];
    uint32_t collection_usage[8];
    uint32_t output_bits[256];
    uint8_t report_kind[256];
    RinUsbHidGamepadForceFeedbackReportV1 reports[4];
} FfParser;

static void ff_zero(void* pointer, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)pointer;
    if (!pointer) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static void ff_copy(void* destination, const void* source, uint32_t size)
{
    volatile uint8_t* out = (volatile uint8_t*)destination;
    const volatile uint8_t* in = (const volatile uint8_t*)source;
    while (size-- != 0u) *out++ = *in++;
}

static int ff_all_zero(const void* pointer, uint32_t size)
{
    const volatile uint8_t* bytes = (const volatile uint8_t*)pointer;
    uint8_t value = 0u;
    if (!pointer) return 0;
    while (size-- != 0u) value |= *bytes++;
    return value == 0u;
}

static int ff_equal(const void* first, const void* second, uint32_t size)
{
    const volatile uint8_t* left = (const volatile uint8_t*)first;
    const volatile uint8_t* right = (const volatile uint8_t*)second;
    uint8_t difference = 0u;
    if (!first || !second) return 0;
    while (size-- != 0u) difference |= (uint8_t)(*left++ ^ *right++);
    return difference == 0u;
}

static uint32_t ff_read_unsigned(const uint8_t* bytes, uint32_t size)
{
    uint32_t value = 0u;
    for (uint32_t index = 0u; index < size; ++index)
        value |= (uint32_t)bytes[index] << (index * 8u);
    return value;
}

static int32_t ff_read_signed(const uint8_t* bytes, uint32_t size)
{
    uint32_t value = ff_read_unsigned(bytes, size);
    uint32_t bits = size * 8u;
    if (bits != 0u && bits < 32u && (value & (UINT32_C(1) << (bits - 1u))) != 0u)
        value |= ~((UINT32_C(1) << bits) - 1u);
    return (int32_t)value;
}

static uint32_t ff_item_size(uint8_t prefix)
{
    switch (prefix & 3u) {
        case 0u: return 0u;
        case 1u: return 1u;
        case 2u: return 2u;
        default: return 4u;
    }
}

static int ff_report_kind(uint32_t page, uint32_t usage)
{
    if (page != FF_PID_PAGE) return -1;
    if (usage == FF_SET_EFFECT) return 0;
    if (usage == FF_SET_CONSTANT) return 1;
    if (usage == FF_EFFECT_OPERATION) return 2;
    if (usage == FF_DEVICE_CONTROL) return 3;
    return -1;
}

static RinUsbHidGamepadForceFeedbackReportV1* ff_report(
    FfParser* parser, uint32_t kind)
{
    return &parser->reports[kind];
}

static int ff_field_is_valid(uint32_t offset, uint32_t size,
                             uint32_t total_bits)
{
    return size != 0u && size <= 16u && offset <= total_bits &&
           size <= total_bits - offset;
}

static int ff_assign_field(RinUsbHidGamepadForceFeedbackReportV1* report,
                           uint32_t usage, uint32_t offset, uint32_t size,
                           int32_t logical_minimum, int32_t logical_maximum)
{
    if (!ff_field_is_valid(offset, size, report->report_bit_count)) return 0;
    if (usage == FF_EFFECT_BLOCK_INDEX) {
        if (report->index_bit_size != 0u || logical_minimum > 1 ||
            logical_maximum < 1) return 0;
        report->index_bit_offset = (uint16_t)offset;
        report->index_bit_size = (uint8_t)size;
        return 1;
    }
    if (report->value_bit_size != 0u) return 0;
    report->value_bit_offset = (uint16_t)offset;
    report->value_bit_size = (uint8_t)size;
    report->value_minimum = logical_minimum;
    report->value_maximum = logical_maximum;
    return 1;
}

static int ff_parse_output(FfParser* parser, uint32_t data)
{
    uint32_t offset = parser->output_bits[parser->report_id];
    uint32_t bits;
    int kind;
    RinUsbHidGamepadForceFeedbackReportV1* report;
    uint32_t usage;
    if (parser->report_size == 0u || parser->report_size > 16u ||
        parser->report_count == 0u || parser->report_count > 8u ||
        parser->report_id == 0u || parser->collection_depth == 0u ||
        parser->collection_page[parser->collection_depth - 1u] != FF_PID_PAGE ||
        (data & 3u) != 2u || (data & 4u) != 0u)
        return 0;
    bits = parser->report_size * parser->report_count;
    if (bits > UINT32_MAX - offset || offset + bits > 512u) return 0;
    kind = ff_report_kind(
        parser->collection_page[parser->collection_depth - 1u],
        parser->collection_usage[parser->collection_depth - 1u]);
    if (kind < 0) {
        parser->output_bits[parser->report_id] = offset + bits;
        parser->usage_is_range = 0u;
        return 1;
    }
    if (parser->report_kind[parser->report_id] != 0u &&
        parser->report_kind[parser->report_id] != (uint8_t)(kind + 1u))
        return 0;
    parser->report_kind[parser->report_id] = (uint8_t)(kind + 1u);
    report = ff_report(parser, (uint32_t)kind);
    report->report_bit_count = offset + bits;
    report->report_id = parser->report_id;
    usage = parser->usage;
    for (uint32_t index = 0u; index < parser->report_count; ++index) {
        uint32_t field_usage = parser->usage_is_range
            ? parser->usage_min + index : usage;
        if (parser->usage_is_range && field_usage > parser->usage_max)
            return 0;
        if (field_usage == FF_EFFECT_BLOCK_INDEX ||
            (kind == 0u && field_usage == FF_DURATION) ||
            (kind == 1u && field_usage == FF_MAGNITUDE) ||
            (kind == 2u && field_usage == FF_OPERATION) ||
            (kind == 3u && field_usage == FF_STOP_ALL)) {
            if (!ff_assign_field(report, field_usage,
                                 offset + index * parser->report_size,
                                 parser->report_size,
                                 parser->logical_minimum,
                                 parser->logical_maximum))
                return 0;
            if (kind == 2u && field_usage == FF_OPERATION) {
                if (parser->logical_minimum > 1 ||
                    parser->logical_maximum < 2) return 0;
                report->value_start = 1u;
                report->value_stop = 2u;
            }
            if (kind == 3u && field_usage == FF_STOP_ALL) {
                if (parser->logical_minimum > 1 ||
                    parser->logical_maximum < 1) return 0;
                report->value_start = 1u;
                report->value_stop = 1u;
            }
        }
    }
    parser->output_bits[parser->report_id] = offset + bits;
    parser->usage_is_range = 0u;
    return 1;
}

static int ff_parse_descriptor(const uint8_t* descriptor, uint32_t size,
                               RinUsbHidGamepadForceFeedbackProfileV1* profile)
{
    FfParser parser;
    uint32_t cursor = 0u;
    uint32_t global_depth = 0u;
    uint32_t global_page[4];
    uint32_t global_size[4];
    uint32_t global_count[4];
    int32_t global_min[4];
    int32_t global_max[4];
    uint8_t report_id_stack[4];
    ff_zero(&parser, sizeof(parser));
    parser.report_id = 1u;
    parser.report_kind[0] = 0xffu;
    while (cursor < size) {
        uint8_t prefix = descriptor[cursor++];
        uint32_t item_size;
        uint32_t value;
        uint32_t type;
        uint32_t tag;
        if (prefix == 0xfeu) {
            if (cursor + 2u > size) return 0;
            item_size = descriptor[cursor++];
            cursor++;
            if (item_size > size - cursor) return 0;
            cursor += item_size;
            continue;
        }
        item_size = ff_item_size(prefix);
        if (item_size > size - cursor) return 0;
        value = ff_read_unsigned(descriptor + cursor, item_size);
        type = (prefix >> 2u) & 3u;
        tag = (prefix >> 4u) & 0x0fu;
        if (type == 0u) {
            switch (tag) {
                case 8u: /* Input: it does not occupy an output report. */
                    parser.usage_is_range = 0u;
                    break;
                case 9u:
                    if (!ff_parse_output(&parser, value)) return 0;
                    break;
                case 11u: /* Feature */
                    parser.usage_is_range = 0u;
                    break;
                case 10u: /* Collection */
                    if (parser.collection_depth >= 8u ||
                        item_size == 0u) return 0;
                    parser.collection_page[parser.collection_depth] =
                        parser.usage_page;
                    parser.collection_usage[parser.collection_depth] =
                        parser.usage;
                    ++parser.collection_depth;
                    parser.usage_is_range = 0u;
                    break;
                case 12u: /* End Collection */
                    if (parser.collection_depth == 0u) return 0;
                    --parser.collection_depth;
                    parser.usage_is_range = 0u;
                    break;
                default:
                    parser.usage_is_range = 0u;
                    break;
            }
        } else if (type == 1u) {
            switch (tag) {
                case 0u: parser.usage_page = value; break;
                case 1u:
                    parser.logical_minimum = ff_read_signed(
                        descriptor + cursor, item_size); break;
                case 2u:
                    parser.logical_maximum = ff_read_signed(
                        descriptor + cursor, item_size);
                    if (parser.logical_maximum < 0 &&
                        parser.logical_minimum >= 0)
                        parser.logical_maximum = (int32_t)value;
                    break;
                case 7u: parser.report_size = value; break;
                case 8u: parser.report_id = (uint8_t)value; break;
                case 9u: parser.report_count = value; break;
                case 10u:
                    if (global_depth >= 4u) return 0;
                    global_page[global_depth] = parser.usage_page;
                    global_size[global_depth] = parser.report_size;
                    global_count[global_depth] = parser.report_count;
                    global_min[global_depth] = parser.logical_minimum;
                    global_max[global_depth] = parser.logical_maximum;
                    report_id_stack[global_depth] = parser.report_id;
                    ++global_depth;
                    break;
                case 11u:
                    if (global_depth == 0u) return 0;
                    --global_depth;
                    parser.usage_page = global_page[global_depth];
                    parser.report_size = global_size[global_depth];
                    parser.report_count = global_count[global_depth];
                    parser.logical_minimum = global_min[global_depth];
                    parser.logical_maximum = global_max[global_depth];
                    parser.report_id = report_id_stack[global_depth];
                    break;
                default: break;
            }
        } else if (type == 2u) {
            switch (tag) {
                case 0u: parser.usage = value; parser.usage_is_range = 0u; break;
                case 1u: parser.usage_min = value; parser.usage_is_range = 1u; break;
                case 2u: parser.usage_max = value; break;
                default: break;
            }
        }
        cursor += item_size;
    }
    if (parser.collection_depth != 0u || global_depth != 0u) return 0;
    for (uint32_t index = 0u; index < 4u; ++index) {
        RinUsbHidGamepadForceFeedbackReportV1* report = &parser.reports[index];
        if (report->report_id == 0u || report->report_bit_count == 0u ||
            (report->report_bit_count + 7u) / 8u >
                RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_REPORT_BYTES - 1u ||
            report->index_bit_size == 0u || report->value_bit_size == 0u)
            return 0;
        report->report_size = (report->report_bit_count + 7u) / 8u + 1u;
    }
    profile->abi_version = RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_VERSION;
    profile->struct_size = sizeof(*profile);
    profile->descriptor_size = size;
    profile->set_effect = parser.reports[0];
    profile->set_constant = parser.reports[1];
    profile->effect_operation = parser.reports[2];
    profile->device_control = parser.reports[3];
    return 1;
}

static int ff_report_valid(const RinUsbHidGamepadForceFeedbackReportV1* report,
                           int signed_value)
{
    if (!report || report->report_id == 0u || report->report_size < 2u ||
        report->report_size > RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_REPORT_BYTES ||
        report->report_bit_count == 0u || report->report_bit_count > 512u ||
        report->index_bit_size == 0u || report->index_bit_size > 16u ||
        report->value_bit_size == 0u || report->value_bit_size > 16u ||
        report->index_bit_offset > report->report_bit_count ||
        report->value_bit_offset > report->report_bit_count ||
        report->index_bit_size > report->report_bit_count - report->index_bit_offset ||
        report->value_bit_size > report->report_bit_count - report->value_bit_offset ||
        report->value_minimum > report->value_maximum ||
        report->reserved0[0] != 0u || report->reserved0[1] != 0u ||
        report->reserved0[2] != 0u || report->reserved1 != 0u ||
        report->reserved2 != 0u)
        return 0;
    if (signed_value && (report->value_minimum >= 0 ||
                         report->value_maximum <= 0)) return 0;
    if (report->report_size != (report->report_bit_count + 7u) / 8u + 1u)
        return 0;
    return 1;
}

static int ff_profile_valid(const RinUsbHidGamepadForceFeedbackProfileV1* profile)
{
    return profile && profile->abi_version ==
               RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_VERSION &&
           profile->struct_size == sizeof(*profile) &&
           profile->descriptor_size != 0u &&
           profile->descriptor_size <=
               RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DESCRIPTOR_BYTES &&
           ff_report_valid(&profile->set_effect, 0) &&
           ff_report_valid(&profile->set_constant, 1) &&
           ff_report_valid(&profile->effect_operation, 0) &&
           ff_report_valid(&profile->device_control, 0) &&
           profile->set_effect.report_id != profile->set_constant.report_id &&
           profile->set_effect.report_id != profile->effect_operation.report_id &&
           profile->set_effect.report_id != profile->device_control.report_id &&
           profile->set_constant.report_id != profile->effect_operation.report_id &&
           profile->set_constant.report_id != profile->device_control.report_id &&
           profile->effect_operation.report_id != profile->device_control.report_id &&
           profile->effect_operation.value_start == 1u &&
           profile->effect_operation.value_stop == 2u &&
           profile->device_control.value_stop == 1u &&
           profile->set_effect.value_minimum >= 0 &&
           profile->set_effect.value_maximum > 0 &&
           profile->effect_operation.value_maximum >= 2 &&
           profile->device_control.value_maximum >= 1 &&
           profile->device_control.value_minimum >= 0 &&
           ff_all_zero(profile->reserved, sizeof(profile->reserved));
}

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_admit_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidGamepadForceFeedbackProfileV1* profile)
{
    uint8_t snapshot[RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DESCRIPTOR_BYTES];
    RinUsbHidGamepadForceFeedbackProfileV1 candidate;
    if (!profile) return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_INVALID_ARGUMENT;
    ff_zero(profile, sizeof(*profile));
    if (!descriptor || descriptor_size == 0u ||
        descriptor_size > RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DESCRIPTOR_BYTES)
        return descriptor_size > RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DESCRIPTOR_BYTES
            ? RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MALFORMED
            : RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_INVALID_ARGUMENT;
    ff_copy(snapshot, descriptor, descriptor_size);
    ff_zero(&candidate, sizeof(candidate));
    if (!ff_parse_descriptor(snapshot, descriptor_size, &candidate) ||
        !ff_profile_valid(&candidate)) return
        RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_UNSUPPORTED;
    *profile = candidate;
    ff_zero(snapshot, sizeof(snapshot));
    ff_zero(&candidate, sizeof(candidate));
    return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK;
}

static void ff_lock(RinUsbHidGamepadForceFeedbackOwnerV1* owner)
{
    while (__atomic_exchange_n(&owner->lock, 1u, __ATOMIC_ACQUIRE) != 0u) {
#if defined(__i386__) || defined(__x86_64__)
        __asm__ volatile("pause");
#endif
    }
}

static void ff_unlock(RinUsbHidGamepadForceFeedbackOwnerV1* owner)
{
    __atomic_store_n(&owner->lock, 0u, __ATOMIC_RELEASE);
}

static uint32_t ff_mask(uint32_t bits)
{
    return bits == 32u ? UINT32_MAX : ((UINT32_C(1) << bits) - 1u);
}

static void ff_write_bits(uint8_t* report, uint32_t offset, uint32_t size,
                          uint32_t value)
{
    uint32_t mask = ff_mask(size);
    value &= mask;
    for (uint32_t bit = 0u; bit < size; ++bit) {
        uint32_t absolute = offset + bit;
        uint8_t bit_value = (uint8_t)((value >> bit) & 1u);
        report[1u + absolute / 8u] =
            (uint8_t)((report[1u + absolute / 8u] &
                       (uint8_t)~(UINT8_C(1) << (absolute & 7u))) |
                      (uint8_t)(bit_value << (absolute & 7u)));
    }
}

static uint32_t ff_scale_magnitude(
    const RinUsbHidGamepadForceFeedbackReportV1* report, int16_t magnitude)
{
    int32_t value = (int32_t)magnitude;
    if (value >= 0) {
        uint64_t scaled = (uint64_t)value *
            (uint32_t)report->value_maximum / UINT32_C(32767);
        return (uint32_t)scaled;
    }
    return (uint32_t)(-(int32_t)(((uint64_t)(-value) *
        (uint32_t)(-report->value_minimum)) / UINT32_C(32767)));
}

static uint32_t ff_scale_duration(
    const RinUsbHidGamepadForceFeedbackReportV1* report, uint32_t duration)
{
    uint32_t minimum = report->value_minimum < 0 ? 0u :
        (uint32_t)report->value_minimum;
    uint32_t maximum = report->value_maximum < 0 ? 0u :
        (uint32_t)report->value_maximum;
    if (duration < minimum) return minimum;
    if (duration > maximum) return maximum;
    return duration;
}

static int ff_owner_valid(const RinUsbHidGamepadForceFeedbackOwnerV1* owner);

static void ff_build_report(uint8_t* report,
                            const RinUsbHidGamepadForceFeedbackReportV1* profile,
                            uint32_t index, uint32_t value)
{
    ff_zero(report, RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_REPORT_BYTES);
    report[0] = profile->report_id;
    ff_write_bits(report, profile->index_bit_offset,
                  profile->index_bit_size, index);
    ff_write_bits(report, profile->value_bit_offset,
                  profile->value_bit_size, value);
}

static RinUsbHidGamepadForceFeedbackResult ff_submit_report(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    const RinUsbHidGamepadForceFeedbackReportV1* profile,
    uint32_t value, uint32_t index)
{
    uint8_t report[RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_REPORT_BYTES];
    RinUsbHidGamepadForceFeedbackProfileV1 profile_snapshot;
    RinUsbHidGamepadForceFeedbackSubmitFn submit = owner->submit;
    void* context = owner->submit_context;
    uint64_t device_id = owner->device_id;
    uint32_t generation = owner->port_generation;
    uint32_t report_size = profile->report_size;
    int callback_result;
    ff_zero(report, sizeof(report));
    ff_build_report(report, profile, index, value);
    profile_snapshot = owner->profile;
    owner->callback_active = 1u;
    ff_unlock(owner);
    callback_result = submit(context, device_id, generation, report,
                             report_size);
    ff_lock(owner);
    owner->callback_active = 0u;
    ff_zero(report, sizeof(report));
    if (!ff_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != generation || owner->submit != submit ||
        !ff_equal(&owner->profile, &profile_snapshot,
                  sizeof(profile_snapshot)))
        return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_DEVICE_LOST;
    return callback_result == 0 ? RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK :
        RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_IO;
}

static RinUsbHidGamepadForceFeedbackResult ff_stop_reports(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner)
{
    RinUsbHidGamepadForceFeedbackResult result = ff_submit_report(
        owner, &owner->profile.effect_operation,
        owner->profile.effect_operation.value_stop, 1u);
    if (result == RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK)
        result = ff_submit_report(owner, &owner->profile.device_control,
                                  owner->profile.device_control.value_stop, 1u);
    return result;
}

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_init(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    const RinUsbHidGamepadForceFeedbackConfigV1* config)
{
    if (!owner || !config || config->abi_version !=
            RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_VERSION ||
        config->struct_size != sizeof(*config) || config->device_id == 0u ||
        config->port_generation == 0u || !ff_profile_valid(config->profile) ||
        !config->authorize || !config->submit || config->reserved != 0u)
        return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_INVALID_ARGUMENT;
    ff_zero(owner, sizeof(*owner));
    owner->magic = RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAGIC;
    owner->device_id = config->device_id;
    owner->port_generation = config->port_generation;
    owner->profile = *config->profile;
    owner->authorize = config->authorize;
    owner->authorize_context = config->authorize_context;
    owner->submit = config->submit;
    owner->submit_context = config->submit_context;
    owner->attached = 1u;
    return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK;
}

static int ff_owner_valid(const RinUsbHidGamepadForceFeedbackOwnerV1* owner)
{
    return owner && owner->magic == RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAGIC &&
           owner->attached && owner->device_id != 0u &&
           owner->port_generation != 0u && ff_profile_valid(&owner->profile) &&
           owner->authorize && owner->submit;
}

static RinUsbHidGamepadForceFeedbackResult ff_authorize(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner, uint64_t token,
    uint32_t permission)
{
    uint64_t device_id = owner->device_id;
    uint32_t generation = owner->port_generation;
    RinUsbHidGamepadForceFeedbackAuthorizeFn authorize = owner->authorize;
    void* context = owner->authorize_context;
    int result;
    owner->callback_active = 1u;
    ff_unlock(owner);
    result = authorize(context, token, device_id, generation, permission);
    ff_lock(owner);
    owner->callback_active = 0u;
    if (!ff_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != generation || owner->authorize != authorize)
        return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_DEVICE_LOST;
    return result == 0 ? RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK :
        RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_ACCESS_DENIED;
}

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_start(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    const RinUsbHidGamepadForceFeedbackRequestV1* request)
{
    RinUsbHidGamepadForceFeedbackResult result;
    if (!owner || !request || request->abi_version !=
            RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_VERSION ||
        request->struct_size != sizeof(*request) || request->duration_ms == 0u ||
        request->duration_ms > RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DURATION_MS ||
        !ff_all_zero(request->reserved, sizeof(request->reserved)))
        return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_INVALID_ARGUMENT;
    ff_lock(owner);
    if (!ff_owner_valid(owner)) { ff_unlock(owner); return
        RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_DEVICE_LOST; }
    if (owner->callback_active || owner->recovery_required || owner->active) {
        ff_unlock(owner); return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_BUSY;
    }
    result = ff_authorize(owner, request->application_token,
                          RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_PERMISSION_USE);
    if (result != RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK) {
        ff_unlock(owner); return result;
    }
    result = ff_submit_report(owner, &owner->profile.set_effect,
                              ff_scale_duration(&owner->profile.set_effect,
                                                request->duration_ms), 1u);
    if (result == RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK)
        result = ff_submit_report(owner, &owner->profile.set_constant,
                                  ff_scale_magnitude(&owner->profile.set_constant,
                                                     request->magnitude), 1u);
    if (result == RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK)
        result = ff_submit_report(owner, &owner->profile.effect_operation,
                                  owner->profile.effect_operation.value_start, 1u);
    if (result != RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK) {
        (void)ff_stop_reports(owner);
        owner->recovery_required = 1u;
        ff_unlock(owner);
        return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_RECOVERY_REQUIRED;
    }
    owner->active = 1u;
    ff_unlock(owner);
    return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK;
}

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_stop(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner, uint64_t application_token)
{
    RinUsbHidGamepadForceFeedbackResult result;
    if (!owner) return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_INVALID_ARGUMENT;
    ff_lock(owner);
    if (!ff_owner_valid(owner)) { ff_unlock(owner); return
        RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_DEVICE_LOST; }
    if (owner->callback_active || owner->recovery_required) {
        ff_unlock(owner); return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_BUSY;
    }
    result = ff_authorize(owner, application_token,
                          RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_PERMISSION_STOP);
    if (result != RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK) {
        ff_unlock(owner); return result;
    }
    result = ff_stop_reports(owner);
    if (result != RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK) {
        owner->recovery_required = 1u;
        ff_unlock(owner);
        return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_RECOVERY_REQUIRED;
    }
    owner->active = 0u;
    ff_unlock(owner);
    return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK;
}

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_recover(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    uint64_t device_id, uint32_t port_generation)
{
    RinUsbHidGamepadForceFeedbackResult result;
    if (!owner || device_id == 0u || port_generation == 0u)
        return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_INVALID_ARGUMENT;
    ff_lock(owner);
    if (!ff_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != port_generation) {
        ff_unlock(owner); return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_DEVICE_LOST;
    }
    if (owner->callback_active) {
        ff_unlock(owner); return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_BUSY;
    }
    result = ff_stop_reports(owner);
    if (result != RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK) {
        owner->recovery_required = 1u;
        ff_unlock(owner);
        return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_RECOVERY_REQUIRED;
    }
    owner->active = 0u;
    owner->recovery_required = 0u;
    ff_unlock(owner);
    return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK;
}

RinUsbHidGamepadForceFeedbackResult
rin_usb_hid_gamepad_force_feedback_detach(
    RinUsbHidGamepadForceFeedbackOwnerV1* owner,
    uint64_t device_id, uint32_t port_generation)
{
    RinUsbHidGamepadForceFeedbackResult result;
    if (!owner || device_id == 0u || port_generation == 0u)
        return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_INVALID_ARGUMENT;
    ff_lock(owner);
    if (!ff_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != port_generation) {
        ff_unlock(owner); return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_DEVICE_LOST;
    }
    if (owner->callback_active) {
        ff_unlock(owner); return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_BUSY;
    }
    if (owner->active) {
        result = ff_stop_reports(owner);
        if (result != RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK) {
            owner->recovery_required = 1u;
            ff_unlock(owner);
            return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_RECOVERY_REQUIRED;
        }
        owner->active = 0u;
    }
    owner->attached = 0u;
    owner->magic = 0u;
    ff_zero(owner->report, sizeof(owner->report));
    ff_unlock(owner);
    return RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK;
}
