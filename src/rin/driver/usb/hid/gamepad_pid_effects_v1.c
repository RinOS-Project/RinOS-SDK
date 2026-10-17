/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <rin/driver/usb/hid/gamepad_pid_effects_v1.h>

static void pid_zero(void* value, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)value;
    if (!value) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static int pid_all_zero(const void* value, uint32_t size)
{
    const volatile uint8_t* bytes = (const volatile uint8_t*)value;
    uint8_t combined = 0u;
    if (!value) return 0;
    while (size-- != 0u) combined |= *bytes++;
    return combined == 0u;
}

static int pid_equal(const void* first, const void* second, uint32_t size)
{
    const volatile uint8_t* left = (const volatile uint8_t*)first;
    const volatile uint8_t* right = (const volatile uint8_t*)second;
    uint8_t difference = 0u;
    if (!first || !second) return 0;
    while (size-- != 0u) difference |= (uint8_t)(*left++ ^ *right++);
    return difference == 0u;
}

static uint32_t pid_item_size(uint8_t prefix)
{
    switch (prefix & 3u) {
        case 0u: return 0u;
        case 1u: return 1u;
        case 2u: return 2u;
        default: return 4u;
    }
}

static uint32_t pid_unsigned(const uint8_t* value, uint32_t size)
{
    uint32_t result = 0u;
    for (uint32_t index = 0u; index < size; ++index)
        result |= (uint32_t)value[index] << (index * 8u);
    return result;
}

static uint32_t pid_capabilities(const uint8_t* descriptor, uint32_t size)
{
    uint32_t page = 0u;
    uint32_t capabilities = 0u;
    uint32_t cursor = 0u;
    uint32_t collection_depth = 0u;
    while (cursor < size) {
        uint8_t prefix = descriptor[cursor++];
        uint32_t item_size;
        uint32_t type;
        uint32_t tag;
        uint32_t value;
        if (prefix == 0xfeu) {
            if (cursor + 2u > size) return 0u;
            item_size = descriptor[cursor++];
            ++cursor;
            if (item_size > size - cursor) return 0u;
            cursor += item_size;
            continue;
        }
        item_size = pid_item_size(prefix);
        if (item_size > size - cursor) return 0u;
        type = (prefix >> 2u) & 3u;
        tag = (prefix >> 4u) & 0x0fu;
        value = pid_unsigned(descriptor + cursor, item_size);
        if (type == 1u && tag == 0u) {
            page = value;
        } else if (type == 2u && tag == 0u && page == 0x0fu &&
                   collection_depth != 0u) {
            if (value == 0x23u) capabilities |= RIN_USB_HID_GAMEPAD_PID_EFFECT_RAMP;
            else if (value == 0x24u) capabilities |= RIN_USB_HID_GAMEPAD_PID_EFFECT_PERIODIC;
            else if (value == 0x25u) capabilities |= RIN_USB_HID_GAMEPAD_PID_EFFECT_CONDITIONAL;
        } else if (type == 0u && tag == 10u) {
            if (item_size != 1u || collection_depth >= 8u) return 0u;
            ++collection_depth;
        } else if (type == 0u && tag == 12u) {
            if (item_size != 0u || collection_depth == 0u) return 0u;
            --collection_depth;
        }
        cursor += item_size;
    }
    return collection_depth == 0u ? capabilities : 0u;
}

static int pid_spec_valid(const RinUsbHidGamepadPidEffectSpecV1* spec,
                          uint32_t supported)
{
    uint32_t known = RIN_USB_HID_GAMEPAD_PID_EFFECT_CONSTANT |
        RIN_USB_HID_GAMEPAD_PID_EFFECT_RAMP |
        RIN_USB_HID_GAMEPAD_PID_EFFECT_PERIODIC |
        RIN_USB_HID_GAMEPAD_PID_EFFECT_CONDITIONAL;
    if (!spec || spec->abi_version != RIN_USB_HID_GAMEPAD_PID_EFFECTS_VERSION ||
        spec->struct_size != sizeof(*spec) || spec->type == 0u ||
        (spec->type & ~known) != 0u || (spec->type & (spec->type - 1u)) != 0u ||
        (spec->type & supported) == 0u || spec->flags != 0u ||
        spec->duration_ms == 0u ||
        spec->duration_ms > RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_DURATION_MS ||
        (spec->type == RIN_USB_HID_GAMEPAD_PID_EFFECT_PERIODIC &&
         (spec->period_ms == 0u || spec->period_ms > spec->duration_ms)) ||
        (spec->type != RIN_USB_HID_GAMEPAD_PID_EFFECT_PERIODIC &&
         (spec->period_ms != 0u || spec->phase_ms != 0u)) ||
        (spec->type != RIN_USB_HID_GAMEPAD_PID_EFFECT_RAMP &&
         spec->end_value != 0) ||
        (spec->type != RIN_USB_HID_GAMEPAD_PID_EFFECT_CONDITIONAL &&
         spec->type != RIN_USB_HID_GAMEPAD_PID_EFFECT_PERIODIC &&
         (spec->positive != 0 || spec->negative != 0)) ||
        spec->reserved0 != 0u || !pid_all_zero(spec->reserved, sizeof(spec->reserved)))
        return 0;
    return 1;
}

static int pid_safety_report_valid(
    const RinUsbHidGamepadForceFeedbackReportV1* report)
{
    return report && report->report_id != 0u && report->report_size >= 2u &&
           report->report_size <= RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_REPORT_BYTES &&
           report->report_bit_count != 0u && report->report_bit_count <= 512u &&
           report->index_bit_size != 0u && report->index_bit_size <= 16u &&
           report->value_bit_size != 0u && report->value_bit_size <= 16u &&
           report->index_bit_offset <= report->report_bit_count &&
           report->value_bit_offset <= report->report_bit_count &&
           report->index_bit_size <= report->report_bit_count - report->index_bit_offset &&
           report->value_bit_size <= report->report_bit_count - report->value_bit_offset &&
           report->value_minimum <= report->value_maximum &&
           report->report_size == (report->report_bit_count + 7u) / 8u + 1u &&
           pid_all_zero(report->reserved0, sizeof(report->reserved0)) &&
           report->reserved1 == 0u && report->reserved2 == 0u;
}

static int pid_profile_valid(const RinUsbHidGamepadPidEffectsProfileV1* profile)
{
    return profile && profile->abi_version == RIN_USB_HID_GAMEPAD_PID_EFFECTS_VERSION &&
           profile->struct_size == sizeof(*profile) &&
           profile->descriptor_size != 0u &&
           profile->descriptor_size <= RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DESCRIPTOR_BYTES &&
           (profile->supported_types & ~UINT32_C(0x0f)) == 0u &&
           profile->supported_types != 0u && profile->max_effects ==
               RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS &&
           profile->safety.abi_version == RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_VERSION &&
           profile->safety.struct_size == sizeof(profile->safety) &&
           pid_safety_report_valid(&profile->safety.set_effect) &&
           pid_safety_report_valid(&profile->safety.set_constant) &&
           pid_safety_report_valid(&profile->safety.effect_operation) &&
           pid_safety_report_valid(&profile->safety.device_control) &&
           profile->safety.set_effect.report_id != profile->safety.set_constant.report_id &&
           profile->safety.set_effect.report_id != profile->safety.effect_operation.report_id &&
           profile->safety.set_effect.report_id != profile->safety.device_control.report_id &&
           profile->safety.set_constant.report_id != profile->safety.effect_operation.report_id &&
           profile->safety.set_constant.report_id != profile->safety.device_control.report_id &&
           profile->safety.effect_operation.report_id != profile->safety.device_control.report_id &&
           pid_all_zero(profile->safety.reserved, sizeof(profile->safety.reserved)) &&
           pid_all_zero(profile->reserved, sizeof(profile->reserved));
}

RinUsbHidGamepadPidEffectsResult
rin_usb_hid_gamepad_pid_effects_admit_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidGamepadPidEffectsProfileV1* profile)
{
    RinUsbHidGamepadForceFeedbackProfileV1 safety;
    uint32_t capabilities;
    RinUsbHidGamepadForceFeedbackResult result;
    if (!profile) return RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    pid_zero(profile, sizeof(*profile));
    if (!descriptor || descriptor_size == 0u ||
        descriptor_size > RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DESCRIPTOR_BYTES)
        return descriptor_size > RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DESCRIPTOR_BYTES
            ? RIN_USB_HID_GAMEPAD_PID_EFFECTS_MALFORMED
            : RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    capabilities = pid_capabilities(descriptor, descriptor_size);
    if (capabilities == 0u) return RIN_USB_HID_GAMEPAD_PID_EFFECTS_UNSUPPORTED;
    capabilities |= RIN_USB_HID_GAMEPAD_PID_EFFECT_CONSTANT;
    pid_zero(&safety, sizeof(safety));
    result = rin_usb_hid_gamepad_force_feedback_admit_descriptor(
        descriptor, descriptor_size, &safety);
    if (result != RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_OK)
        return result == RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MALFORMED
            ? RIN_USB_HID_GAMEPAD_PID_EFFECTS_MALFORMED
            : RIN_USB_HID_GAMEPAD_PID_EFFECTS_UNSUPPORTED;
    profile->abi_version = RIN_USB_HID_GAMEPAD_PID_EFFECTS_VERSION;
    profile->struct_size = sizeof(*profile);
    profile->descriptor_size = descriptor_size;
    profile->supported_types = capabilities;
    profile->safety = safety;
    profile->max_effects = RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS;
    pid_zero(&safety, sizeof(safety));
    return RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK;
}

static void pid_lock(RinUsbHidGamepadPidEffectsOwnerV1* owner)
{
    while (__atomic_exchange_n(&owner->lock, 1u, __ATOMIC_ACQUIRE) != 0u) {
#if defined(__i386__) || defined(__x86_64__)
        __asm__ volatile("pause");
#endif
    }
}

static void pid_unlock(RinUsbHidGamepadPidEffectsOwnerV1* owner)
{
    __atomic_store_n(&owner->lock, 0u, __ATOMIC_RELEASE);
}

static int pid_owner_valid(const RinUsbHidGamepadPidEffectsOwnerV1* owner)
{
    return owner && owner->magic == RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAGIC &&
           owner->attached && owner->device_id != 0u &&
           owner->port_generation != 0u && pid_profile_valid(&owner->profile) &&
           owner->authorize && owner->submit;
}

static int pid_find_slot(const RinUsbHidGamepadPidEffectsOwnerV1* owner,
                         uint32_t effect_id)
{
    for (uint32_t index = 0u; index < RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS;
         ++index)
        if (owner->slots[index].effect_id == effect_id) return (int)index;
    return -1;
}

static RinUsbHidGamepadPidEffectsResult pid_authorize(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t token, uint32_t permission,
    uint64_t* device_id_out, uint32_t* generation_out)
{
    uint64_t device_id = owner->device_id;
    uint32_t generation = owner->port_generation;
    RinUsbHidGamepadPidEffectsAuthorizeFn authorize = owner->authorize;
    void* context = owner->authorize_context;
    int result;
    owner->callback_active = 1u;
    pid_unlock(owner);
    result = authorize(context, token, device_id, generation, permission);
    pid_lock(owner);
    owner->callback_active = 0u;
    if (!pid_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != generation || owner->authorize != authorize)
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE;
    *device_id_out = device_id;
    *generation_out = generation;
    return result == 1 ? RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK
                       : RIN_USB_HID_GAMEPAD_PID_EFFECTS_ACCESS_DENIED;
}

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_init(
    RinUsbHidGamepadPidEffectsOwnerV1* owner,
    const RinUsbHidGamepadPidEffectsConfigV1* config)
{
    if (!owner || !config || config->abi_version != RIN_USB_HID_GAMEPAD_PID_EFFECTS_VERSION ||
        config->struct_size != sizeof(*config) || config->device_id == 0u ||
        config->port_generation == 0u || !config->profile ||
        !pid_profile_valid(config->profile) || !config->authorize ||
        !config->submit || !pid_all_zero(config->reserved, sizeof(config->reserved)))
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    pid_zero(owner, sizeof(*owner));
    owner->magic = RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAGIC;
    owner->attached = 1u;
    owner->device_id = config->device_id;
    owner->port_generation = config->port_generation;
    owner->profile = *config->profile;
    owner->authorize = config->authorize;
    owner->authorize_context = config->authorize_context;
    owner->submit = config->submit;
    owner->submit_context = config->submit_context;
    return RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK;
}

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_create(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t application_token,
    const RinUsbHidGamepadPidEffectSpecV1* spec, uint32_t* effect_id_out)
{
    RinUsbHidGamepadPidEffectsResult result;
    uint64_t device_id;
    uint32_t generation;
    int free_slot = -1;
    if (effect_id_out) *effect_id_out = 0u;
    if (!owner || !spec || !effect_id_out || application_token == 0u)
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    pid_lock(owner);
    if (!pid_owner_valid(owner) || owner->recovery_required) {
        pid_unlock(owner);
        return owner->recovery_required
            ? RIN_USB_HID_GAMEPAD_PID_EFFECTS_RECOVERY_REQUIRED
            : RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE;
    }
    if (!pid_spec_valid(spec, owner->profile.supported_types)) {
        pid_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    }
    if (owner->sequence == UINT32_MAX) {
        pid_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_LIMIT_EXCEEDED;
    }
    if (owner->callback_active) {
        pid_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_BUSY;
    }
    for (uint32_t index = 0u; index < RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS;
         ++index) {
        if (owner->slots[index].effect_id == 0u) {
            free_slot = (int)index;
            break;
        }
    }
    if (free_slot < 0) {
        pid_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_BUSY;
    }
    result = pid_authorize(owner, application_token,
                           RIN_USB_HID_GAMEPAD_PID_EFFECT_PERMISSION_USE,
                           &device_id, &generation);
    if (result != RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK) {
        pid_unlock(owner);
        return result;
    }
    owner->sequence++;
    if (owner->sequence == 0u) owner->sequence = 1u;
    owner->slots[free_slot].effect_id = owner->sequence;
    owner->slots[free_slot].spec = *spec;
    *effect_id_out = owner->sequence;
    (void)device_id;
    (void)generation;
    pid_unlock(owner);
    return RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK;
}

static RinUsbHidGamepadPidEffectsResult pid_submit(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t token, uint32_t effect_id,
    uint32_t running, uint32_t permission)
{
    RinUsbHidGamepadPidEffectsResult result;
    uint64_t device_id;
    uint32_t generation;
    RinUsbHidGamepadPidEffectsSubmitFn submit = owner->submit;
    void* context = owner->submit_context;
    RinUsbHidGamepadPidEffectSpecV1 spec;
    uint32_t callback_generation;
    int current_index;
    int index = pid_find_slot(owner, effect_id);
    if (index < 0) return RIN_USB_HID_GAMEPAD_PID_EFFECTS_NOT_FOUND;
    result = pid_authorize(owner, token, permission, &device_id, &generation);
    if (result != RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK) return result;
    spec = owner->slots[index].spec;
    callback_generation = generation;
    owner->callback_active = 1u;
    pid_unlock(owner);
    result = submit(context, device_id, callback_generation, effect_id, &spec,
                    running) == 1 ? RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK
                                  : RIN_USB_HID_GAMEPAD_PID_EFFECTS_IO;
    pid_lock(owner);
    owner->callback_active = 0u;
    current_index = pid_find_slot(owner, effect_id);
    if (!pid_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != callback_generation ||
        owner->submit != submit || current_index < 0 ||
        !pid_equal(&owner->slots[current_index].spec, &spec, sizeof(spec)))
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE;
    return result;
}

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_start(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t application_token,
    uint32_t effect_id, uint64_t now_ms)
{
    RinUsbHidGamepadPidEffectsResult result;
    int index;
    if (!owner || effect_id == 0u || application_token == 0u) return
        RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    pid_lock(owner);
    if (!pid_owner_valid(owner) || owner->recovery_required) {
        pid_unlock(owner);
        return owner->recovery_required ? RIN_USB_HID_GAMEPAD_PID_EFFECTS_RECOVERY_REQUIRED
                                        : RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE;
    }
    if (owner->callback_active) { pid_unlock(owner); return RIN_USB_HID_GAMEPAD_PID_EFFECTS_BUSY; }
    index = pid_find_slot(owner, effect_id);
    if (index < 0) { pid_unlock(owner); return RIN_USB_HID_GAMEPAD_PID_EFFECTS_NOT_FOUND; }
    if (owner->slots[index].running) { pid_unlock(owner); return RIN_USB_HID_GAMEPAD_PID_EFFECTS_BUSY; }
    owner->slots[index].running = 1u;
    owner->slots[index].started_ms = now_ms;
    result = pid_submit(owner, application_token, effect_id, 1u,
                        RIN_USB_HID_GAMEPAD_PID_EFFECT_PERMISSION_USE);
    if (result != RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK) {
        owner->slots[index].running = 0u;
        owner->recovery_required = 1u;
        pid_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_RECOVERY_REQUIRED;
    }
    pid_unlock(owner);
    return result;
}

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_stop(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t application_token,
    uint32_t effect_id)
{
    RinUsbHidGamepadPidEffectsResult result;
    int index;
    if (!owner || effect_id == 0u || application_token == 0u) return
        RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    pid_lock(owner);
    if (!pid_owner_valid(owner) || owner->recovery_required) {
        pid_unlock(owner);
        return owner->recovery_required ? RIN_USB_HID_GAMEPAD_PID_EFFECTS_RECOVERY_REQUIRED
                                        : RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE;
    }
    if (owner->callback_active) { pid_unlock(owner); return RIN_USB_HID_GAMEPAD_PID_EFFECTS_BUSY; }
    index = pid_find_slot(owner, effect_id);
    if (index < 0) { pid_unlock(owner); return RIN_USB_HID_GAMEPAD_PID_EFFECTS_NOT_FOUND; }
    if (!owner->slots[index].running) { pid_unlock(owner); return RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK; }
    result = pid_submit(owner, application_token, effect_id, 0u,
                        RIN_USB_HID_GAMEPAD_PID_EFFECT_PERMISSION_STOP);
    if (result != RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK) {
        owner->recovery_required = 1u;
        pid_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_RECOVERY_REQUIRED;
    }
    owner->slots[index].running = 0u;
    pid_unlock(owner);
    return result;
}

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_destroy(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t application_token,
    uint32_t effect_id)
{
    RinUsbHidGamepadPidEffectsResult result;
    uint64_t device_id;
    uint32_t generation;
    int index;
    if (!owner || effect_id == 0u || application_token == 0u) return
        RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    pid_lock(owner);
    if (!pid_owner_valid(owner) || owner->recovery_required) {
        pid_unlock(owner);
        return owner->recovery_required ? RIN_USB_HID_GAMEPAD_PID_EFFECTS_RECOVERY_REQUIRED
                                        : RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE;
    }
    index = pid_find_slot(owner, effect_id);
    if (index < 0) { pid_unlock(owner); return RIN_USB_HID_GAMEPAD_PID_EFFECTS_NOT_FOUND; }
    if (owner->slots[index].running) { pid_unlock(owner); return RIN_USB_HID_GAMEPAD_PID_EFFECTS_BUSY; }
    result = pid_authorize(owner, application_token,
                           RIN_USB_HID_GAMEPAD_PID_EFFECT_PERMISSION_USE,
                           &device_id, &generation);
    if (result == RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK) {
        pid_zero(&owner->slots[index], sizeof(owner->slots[index]));
    }
    (void)device_id;
    (void)generation;
    pid_unlock(owner);
    return result;
}

static int32_t pid_saturate(int32_t value)
{
    if (value < -32767) return -32767;
    if (value > 32767) return 32767;
    return value;
}

static int32_t pid_effect_value(const RinUsbHidGamepadPidEffectSlotV1* slot,
                                uint64_t elapsed, int16_t condition)
{
    const RinUsbHidGamepadPidEffectSpecV1* spec = &slot->spec;
    if (elapsed >= spec->duration_ms) return 0;
    if (spec->type == RIN_USB_HID_GAMEPAD_PID_EFFECT_CONSTANT) return spec->start_value;
    if (spec->type == RIN_USB_HID_GAMEPAD_PID_EFFECT_RAMP) {
        int64_t delta = (int64_t)spec->end_value - spec->start_value;
        return (int32_t)spec->start_value +
            (int32_t)(delta * (int64_t)elapsed / spec->duration_ms);
    }
    if (spec->type == RIN_USB_HID_GAMEPAD_PID_EFFECT_CONDITIONAL)
        return (int32_t)spec->offset + (condition >= 0 ? spec->positive : spec->negative);
    {
        uint32_t phase = (spec->phase_ms + (uint32_t)(elapsed % spec->period_ms)) %
            spec->period_ms;
        uint32_t half = spec->period_ms / 2u;
        int32_t amplitude = (int32_t)spec->positive - spec->negative;
        int32_t value = half == 0u ? spec->positive :
            (phase < half ? spec->positive -
                (int32_t)((int64_t)amplitude * phase / half) :
             spec->negative + (int32_t)((int64_t)amplitude *
                (phase - half) / (spec->period_ms - half)));
        return (int32_t)spec->offset + value;
    }
}

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_tick(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t now_ms,
    int16_t condition, RinUsbHidGamepadPidEffectOutputV1* output)
{
    int32_t total = 0;
    uint16_t active = 0u;
    if (output) pid_zero(output, sizeof(*output));
    if (!owner || !output) return RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    pid_lock(owner);
    if (!pid_owner_valid(owner) || owner->recovery_required) {
        pid_unlock(owner);
        return owner->recovery_required ? RIN_USB_HID_GAMEPAD_PID_EFFECTS_RECOVERY_REQUIRED
                                        : RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE;
    }
    if (owner->sequence == UINT32_MAX) {
        pid_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_LIMIT_EXCEEDED;
    }
    for (uint32_t index = 0u; index < RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS;
         ++index) {
        const RinUsbHidGamepadPidEffectSlotV1* slot = &owner->slots[index];
        if (slot->effect_id != 0u && slot->running) {
            total += pid_effect_value(slot, now_ms - slot->started_ms, condition);
            if (now_ms - slot->started_ms < slot->spec.duration_ms) ++active;
        }
    }
    output->abi_version = RIN_USB_HID_GAMEPAD_PID_EFFECTS_VERSION;
    output->struct_size = sizeof(*output);
    output->force = (int16_t)pid_saturate(total);
    output->active_count = active;
    output->sequence = ++owner->sequence;
    pid_unlock(owner);
    return RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK;
}

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_recover(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t device_id,
    uint32_t port_generation)
{
    if (!owner || device_id == 0u || port_generation == 0u) return
        RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    pid_lock(owner);
    if (!pid_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != port_generation || owner->callback_active) {
        pid_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE;
    }
    for (uint32_t index = 0u; index < RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS;
         ++index) owner->slots[index].running = 0u;
    owner->recovery_required = 0u;
    pid_unlock(owner);
    return RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK;
}

RinUsbHidGamepadPidEffectsResult rin_usb_hid_gamepad_pid_effects_detach(
    RinUsbHidGamepadPidEffectsOwnerV1* owner, uint64_t device_id,
    uint32_t port_generation)
{
    if (!owner || device_id == 0u || port_generation == 0u) return
        RIN_USB_HID_GAMEPAD_PID_EFFECTS_INVALID_ARGUMENT;
    pid_lock(owner);
    if (!pid_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != port_generation || owner->callback_active) {
        pid_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_EFFECTS_STALE;
    }
    owner->attached = 0u;
    owner->magic = 0u;
    pid_zero(owner->slots, sizeof(owner->slots));
    pid_unlock(owner);
    return RIN_USB_HID_GAMEPAD_PID_EFFECTS_OK;
}
