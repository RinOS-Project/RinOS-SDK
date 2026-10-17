/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "gamepad_pid_actuator_v1.h"

static void actuator_zero(void* value, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)value;
    if (!value) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static int actuator_all_zero(const void* value, uint32_t size)
{
    const volatile uint8_t* bytes = (const volatile uint8_t*)value;
    uint8_t combined = 0u;
    if (!value) return 0;
    while (size-- != 0u) combined |= *bytes++;
    return combined == 0u;
}

static int actuator_equal(const void* left, const void* right, uint32_t size)
{
    const volatile uint8_t* a = (const volatile uint8_t*)left;
    const volatile uint8_t* b = (const volatile uint8_t*)right;
    uint8_t difference = 0u;
    if (!left || !right) return 0;
    while (size-- != 0u) difference |= (uint8_t)(*a++ ^ *b++);
    return difference == 0u;
}

static int actuator_report_valid(
    const RinUsbHidGamepadForceFeedbackReportV1* report)
{
    uint32_t payload_bits;
    if (!report || report->report_id == 0u || report->report_size < 2u ||
        report->report_size > RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAX_REPORT_BYTES ||
        report->report_bit_count == 0u || report->report_bit_count > 512u ||
        report->index_bit_size == 0u || report->index_bit_size > 16u ||
        report->value_bit_size == 0u || report->value_bit_size > 16u ||
        report->value_minimum > report->value_maximum ||
        report->index_bit_offset > report->report_bit_count ||
        report->value_bit_offset > report->report_bit_count ||
        report->index_bit_size > report->report_bit_count -
                                     report->index_bit_offset ||
        report->value_bit_size > report->report_bit_count -
                                     report->value_bit_offset ||
        report->report_size != (report->report_bit_count + 7u) / 8u + 1u ||
        !actuator_all_zero(report->reserved0, sizeof(report->reserved0)) ||
        report->reserved1 != 0u || report->reserved2 != 0u)
        return 0;
    payload_bits = (uint32_t)report->report_size - 1u;
    if (payload_bits > 64u) return 0;
    if (report->index_bit_offset < report->value_bit_offset) {
        if (report->index_bit_size > report->value_bit_offset -
                                    report->index_bit_offset)
            return 0;
    } else if (report->value_bit_size > report->index_bit_offset -
                                      report->value_bit_offset) {
        return 0;
    }
    return 1;
}

static int actuator_profile_valid(
    const RinUsbHidGamepadForceFeedbackProfileV1* profile)
{
    const RinUsbHidGamepadForceFeedbackReportV1* reports[4];
    if (!profile || profile->abi_version !=
                       RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_VERSION ||
        profile->struct_size != sizeof(*profile) || profile->descriptor_size == 0u ||
        profile->descriptor_size >
            RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DESCRIPTOR_BYTES ||
        !actuator_all_zero(profile->reserved, sizeof(profile->reserved)))
        return 0;
    reports[0] = &profile->set_effect;
    reports[1] = &profile->set_constant;
    reports[2] = &profile->effect_operation;
    reports[3] = &profile->device_control;
    for (uint32_t i = 0u; i < 4u; ++i) {
        if (!actuator_report_valid(reports[i])) return 0;
        for (uint32_t j = 0u; j < i; ++j)
            if (reports[i]->report_id == reports[j]->report_id) return 0;
    }
    if (profile->effect_operation.value_start != 1u ||
        profile->effect_operation.value_stop != 2u ||
        profile->device_control.value_start != 1u ||
        profile->device_control.value_stop != 1u)
        return 0;
    if ((int32_t)profile->effect_operation.value_start <
            profile->effect_operation.value_minimum ||
        (int32_t)profile->effect_operation.value_start >
            profile->effect_operation.value_maximum ||
        (int32_t)profile->effect_operation.value_stop <
            profile->effect_operation.value_minimum ||
        (int32_t)profile->effect_operation.value_stop >
            profile->effect_operation.value_maximum ||
        (int32_t)profile->device_control.value_stop <
            profile->device_control.value_minimum ||
        (int32_t)profile->device_control.value_stop >
            profile->device_control.value_maximum)
        return 0;
    return 1;
}

static void actuator_lock(RinUsbHidGamepadPidActuatorOwnerV1* owner)
{
    while (__atomic_exchange_n(&owner->lock, 1u, __ATOMIC_ACQUIRE) != 0u) {
#if defined(__i386__) || defined(__x86_64__)
        __asm__ volatile("pause");
#endif
    }
}

static void actuator_unlock(RinUsbHidGamepadPidActuatorOwnerV1* owner)
{
    __atomic_store_n(&owner->lock, 0u, __ATOMIC_RELEASE);
}

static int actuator_owner_valid(
    const RinUsbHidGamepadPidActuatorOwnerV1* owner)
{
    return owner && owner->magic == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAGIC &&
           owner->attached != 0u && owner->device_id != 0u &&
           owner->port_generation != 0u && actuator_profile_valid(&owner->profile) &&
           owner->authorize && owner->submit;
}

static RinUsbHidGamepadPidActuatorResult actuator_authorize(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t token,
    uint32_t permission, uint64_t* device_id_out, uint32_t* generation_out)
{
    RinUsbHidGamepadPidActuatorAuthorizeFn authorize = owner->authorize;
    void* context = owner->authorize_context;
    uint64_t device_id = owner->device_id;
    uint32_t generation = owner->port_generation;
    int result;
    owner->callback_active = 1u;
    actuator_unlock(owner);
    result = authorize(context, token, device_id, generation, permission);
    actuator_lock(owner);
    owner->callback_active = 0u;
    if (!actuator_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != generation || owner->authorize != authorize)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE;
    *device_id_out = device_id;
    *generation_out = generation;
    return result == 1 ? RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK
                       : RIN_USB_HID_GAMEPAD_PID_ACTUATOR_ACCESS_DENIED;
}

/* Called with owner->lock held and returns with it held. */
static RinUsbHidGamepadPidActuatorResult actuator_send_locked(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, const uint8_t* report,
    uint32_t report_size)
{
    RinUsbHidGamepadPidActuatorSubmitFn submit = owner->submit;
    void* submit_context = owner->submit_context;
    uint64_t device_id = owner->device_id;
    uint32_t generation = owner->port_generation;
    int callback_result;
    if (!report || report_size == 0u || report_size > sizeof(owner->report))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    actuator_zero(owner->report, sizeof(owner->report));
    for (uint32_t i = 0u; i < report_size; ++i) owner->report[i] = report[i];
    owner->callback_active = 1u;
    actuator_unlock(owner);
    callback_result = submit(submit_context, device_id, generation, report,
                             report_size);
    actuator_lock(owner);
    owner->callback_active = 0u;
    if (!actuator_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != generation || owner->submit != submit ||
        owner->submit_context != submit_context ||
        !actuator_equal(owner->report, report, report_size))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE;
    if (callback_result != 1) {
        owner->recovery_required = 1u;
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_RECOVERY_REQUIRED;
    }
    return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK;
}

static int actuator_write_bits(uint8_t* report, uint32_t report_size,
                               uint16_t bit_offset, uint8_t bit_size,
                               uint32_t value)
{
    uint32_t total_bits;
    if (!report || report_size < 2u || report_size > 64u || bit_size == 0u ||
        bit_size > 16u) return 0;
    total_bits = (report_size - 1u) * 8u;
    if (bit_offset > total_bits || bit_size > total_bits - bit_offset)
        return 0;
    for (uint32_t bit = 0u; bit < bit_size; ++bit) {
        uint32_t position = (uint32_t)bit_offset + bit;
        uint8_t* byte = &report[1u + position / 8u];
        uint8_t mask = (uint8_t)(1u << (position % 8u));
        if ((value & (UINT32_C(1) << bit)) != 0u) *byte |= mask;
        else *byte &= (uint8_t)~mask;
    }
    return 1;
}

static RinUsbHidGamepadPidActuatorResult actuator_serialize(
    const RinUsbHidGamepadForceFeedbackReportV1* report_info,
    uint32_t effect_index, uint32_t value, uint8_t* report,
    uint32_t report_capacity, uint32_t* report_size_out)
{
    uint32_t index_limit;
    uint32_t value_limit;
    if (report_size_out) *report_size_out = 0u;
    if (!report_info || !report || !report_size_out ||
        !actuator_report_valid(report_info) ||
        report_capacity < report_info->report_size)
        return !report_info || !report || !report_size_out
                   ? RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT
                   : RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MALFORMED;
    index_limit = report_info->index_bit_size == 16u
                      ? UINT32_C(0xffff)
                      : (UINT32_C(1) << report_info->index_bit_size) - 1u;
    value_limit = report_info->value_bit_size == 16u
                      ? UINT32_C(0xffff)
                      : (UINT32_C(1) << report_info->value_bit_size) - 1u;
    if (effect_index == 0u || effect_index > index_limit || value > value_limit)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    actuator_zero(report, report_info->report_size);
    report[0] = report_info->report_id;
    if (!actuator_write_bits(report, report_info->report_size,
                             report_info->index_bit_offset,
                             report_info->index_bit_size, effect_index) ||
        !actuator_write_bits(report, report_info->report_size,
                             report_info->value_bit_offset,
                             report_info->value_bit_size, value)) {
        actuator_zero(report, report_info->report_size);
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MALFORMED;
    }
    *report_size_out = report_info->report_size;
    return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK;
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_serialize_set_effect(
    const RinUsbHidGamepadForceFeedbackProfileV1* profile,
    uint32_t effect_index, uint32_t duration_ms, uint8_t* report,
    uint32_t report_capacity, uint32_t* report_size_out)
{
    if (report_size_out) *report_size_out = 0u;
    if (!profile || !actuator_profile_valid(profile))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MALFORMED;
    if (duration_ms == 0u ||
        duration_ms > RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DURATION_MS ||
        (int32_t)duration_ms < profile->set_effect.value_minimum ||
        (int32_t)duration_ms > profile->set_effect.value_maximum)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    return actuator_serialize(&profile->set_effect, effect_index, duration_ms,
                              report, report_capacity, report_size_out);
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_serialize_constant(
    const RinUsbHidGamepadForceFeedbackProfileV1* profile,
    uint32_t effect_index, int16_t magnitude, uint8_t* report,
    uint32_t report_capacity, uint32_t* report_size_out)
{
    uint32_t encoded;
    if (!profile || !actuator_profile_valid(profile))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MALFORMED;
    if ((int32_t)magnitude < profile->set_constant.value_minimum ||
        (int32_t)magnitude > profile->set_constant.value_maximum)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    encoded = (uint32_t)(uint16_t)magnitude;
    if (profile->set_constant.value_bit_size < 16u)
        encoded &= (UINT32_C(1) << profile->set_constant.value_bit_size) - 1u;
    return actuator_serialize(&profile->set_constant, effect_index, encoded,
                              report, report_capacity, report_size_out);
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_serialize_operation(
    const RinUsbHidGamepadForceFeedbackProfileV1* profile,
    uint32_t effect_index, uint32_t operation, uint8_t* report,
    uint32_t report_capacity, uint32_t* report_size_out)
{
    if (!profile || !actuator_profile_valid(profile))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MALFORMED;
    if (operation != profile->effect_operation.value_start &&
        operation != profile->effect_operation.value_stop)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    return actuator_serialize(&profile->effect_operation, effect_index,
                              operation, report, report_capacity,
                              report_size_out);
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_serialize_stop_all(
    const RinUsbHidGamepadForceFeedbackProfileV1* profile,
    uint32_t effect_index, uint8_t* report, uint32_t report_capacity,
    uint32_t* report_size_out)
{
    if (!profile || !actuator_profile_valid(profile))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MALFORMED;
    return actuator_serialize(&profile->device_control, effect_index,
                              profile->device_control.value_stop, report,
                              report_capacity, report_size_out);
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_init(
    RinUsbHidGamepadPidActuatorOwnerV1* owner,
    const RinUsbHidGamepadPidActuatorConfigV1* config)
{
    if (!owner || !config || config->abi_version !=
                       RIN_USB_HID_GAMEPAD_PID_ACTUATOR_VERSION ||
        config->struct_size != sizeof(*config) || config->device_id == 0u ||
        config->port_generation == 0u || !config->profile ||
        !actuator_profile_valid(config->profile) || !config->authorize ||
        !config->submit || !actuator_all_zero(config->reserved,
                                               sizeof(config->reserved)))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    actuator_zero(owner, sizeof(*owner));
    owner->magic = RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAGIC;
    owner->attached = 1u;
    owner->device_id = config->device_id;
    owner->port_generation = config->port_generation;
    owner->profile = *config->profile;
    owner->authorize = config->authorize;
    owner->authorize_context = config->authorize_context;
    owner->submit = config->submit;
    owner->submit_context = config->submit_context;
    return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK;
}

static RinUsbHidGamepadPidActuatorResult actuator_submit_report(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t token,
    uint32_t permission, const uint8_t* report, uint32_t report_size)
{
    uint64_t device_id;
    uint32_t generation;
    RinUsbHidGamepadPidActuatorResult result;
    if (!owner || !report || report_size == 0u || report_size >
                                             sizeof(owner->report))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    result = actuator_authorize(owner, token, permission, &device_id,
                                &generation);
    if (result != RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK) return result;
    (void)device_id;
    (void)generation;
    return actuator_send_locked(owner, report, report_size);
}

static RinUsbHidGamepadPidActuatorResult actuator_stop_reports_locked(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint32_t effect_index)
{
    uint8_t report[RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAX_REPORT_BYTES];
    uint32_t report_size = 0u;
    RinUsbHidGamepadPidActuatorResult result;
    actuator_zero(report, sizeof(report));
    result = rin_usb_hid_gamepad_pid_actuator_serialize_operation(
        &owner->profile, effect_index, owner->profile.effect_operation.value_stop,
        report, sizeof(report), &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_send_locked(owner, report, report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = rin_usb_hid_gamepad_pid_actuator_serialize_stop_all(
            &owner->profile, effect_index, report, sizeof(report), &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_send_locked(owner, report, report_size);
    actuator_zero(report, sizeof(report));
    return result;
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_start(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index, uint32_t duration_ms, int16_t magnitude)
{
    uint8_t report[RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAX_REPORT_BYTES];
    uint32_t report_size = 0u;
    uint64_t authorized_device_id = 0u;
    uint32_t authorized_generation = 0u;
    uint32_t cleanup_required = 0u;
    RinUsbHidGamepadPidActuatorResult result;
    if (!owner || application_token == 0u || effect_index == 0u ||
        duration_ms == 0u ||
        duration_ms > RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DURATION_MS)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    actuator_zero(report, sizeof(report));
    actuator_lock(owner);
    if (!actuator_owner_valid(owner) || owner->recovery_required) {
        result = owner && owner->recovery_required
                     ? RIN_USB_HID_GAMEPAD_PID_ACTUATOR_RECOVERY_REQUIRED
                     : RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE;
        actuator_unlock(owner);
        return result;
    }
    if (owner->callback_active || owner->active_effect != 0u) {
        actuator_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_BUSY;
    }
    result = actuator_authorize(owner, application_token,
                                RIN_USB_HID_GAMEPAD_PID_ACTUATOR_PERMISSION_USE,
                                &authorized_device_id, &authorized_generation);
    (void)authorized_device_id;
    (void)authorized_generation;
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = rin_usb_hid_gamepad_pid_actuator_serialize_set_effect(
            &owner->profile, effect_index, duration_ms, report, sizeof(report),
            &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_send_locked(owner, report, report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK ||
        result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_RECOVERY_REQUIRED ||
        result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE)
        cleanup_required = 1u;
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = rin_usb_hid_gamepad_pid_actuator_serialize_constant(
            &owner->profile, effect_index, magnitude, report, sizeof(report),
            &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_send_locked(owner, report, report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = rin_usb_hid_gamepad_pid_actuator_serialize_operation(
            &owner->profile, effect_index,
            owner->profile.effect_operation.value_start, report,
            sizeof(report), &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_send_locked(owner, report, report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK) {
        owner->active_effect = effect_index;
    } else if (cleanup_required) {
        (void)actuator_stop_reports_locked(owner, effect_index);
        owner->recovery_required = 1u;
        result = RIN_USB_HID_GAMEPAD_PID_ACTUATOR_RECOVERY_REQUIRED;
    }
    actuator_unlock(owner);
    actuator_zero(report, sizeof(report));
    return result;
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_submit(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index, int16_t magnitude)
{
    uint8_t report[RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAX_REPORT_BYTES];
    uint32_t report_size = 0u;
    RinUsbHidGamepadPidActuatorResult result;
    if (!owner || application_token == 0u || effect_index == 0u)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    actuator_lock(owner);
    if (!actuator_owner_valid(owner) || owner->recovery_required) {
        result = owner && owner->recovery_required
                     ? RIN_USB_HID_GAMEPAD_PID_ACTUATOR_RECOVERY_REQUIRED
                     : RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE;
        actuator_unlock(owner);
        return result;
    }
    if (owner->callback_active) {
        actuator_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_BUSY;
    }
    if (owner->active_effect != 0u && owner->active_effect != effect_index) {
        actuator_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_BUSY;
    }
    result = rin_usb_hid_gamepad_pid_actuator_serialize_constant(
        &owner->profile, effect_index, magnitude, report, sizeof(report),
        &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_submit_report(owner, application_token,
                                        RIN_USB_HID_GAMEPAD_PID_ACTUATOR_PERMISSION_USE,
                                        report, report_size);
    actuator_unlock(owner);
    actuator_zero(report, sizeof(report));
    return result;
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_submit_output(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index, const RinUsbHidGamepadPidEffectOutputV1* output)
{
    if (!output || output->abi_version !=
                       RIN_USB_HID_GAMEPAD_PID_EFFECTS_VERSION ||
        output->struct_size != sizeof(*output) || output->active_count >
            RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS ||
        !actuator_all_zero(output->reserved, sizeof(output->reserved)))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    return rin_usb_hid_gamepad_pid_actuator_submit(
        owner, application_token, effect_index, output->force);
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_sync_output(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index, uint32_t duration_ms,
    const RinUsbHidGamepadPidEffectOutputV1* output)
{
    uint32_t active_effect;
    RinUsbHidGamepadPidActuatorResult result;
    if (!owner || application_token == 0u || effect_index == 0u ||
        duration_ms == 0u ||
        duration_ms > RIN_USB_HID_GAMEPAD_FORCE_FEEDBACK_MAX_DURATION_MS ||
        !output || output->abi_version != RIN_USB_HID_GAMEPAD_PID_EFFECTS_VERSION ||
        output->struct_size != sizeof(*output) ||
        output->active_count > RIN_USB_HID_GAMEPAD_PID_EFFECTS_MAX_EFFECTS ||
        (output->active_count == 0u && output->force != 0) ||
        !actuator_all_zero(output->reserved, sizeof(output->reserved)))
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    actuator_lock(owner);
    if (!actuator_owner_valid(owner) || owner->recovery_required) {
        result = owner && owner->recovery_required
                     ? RIN_USB_HID_GAMEPAD_PID_ACTUATOR_RECOVERY_REQUIRED
                     : RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE;
        actuator_unlock(owner);
        return result;
    }
    if (owner->callback_active) {
        actuator_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_BUSY;
    }
    active_effect = owner->active_effect;
    actuator_unlock(owner);
    if (output->active_count != 0u) {
        if (active_effect == 0u)
            return rin_usb_hid_gamepad_pid_actuator_start(
                owner, application_token, effect_index, duration_ms,
                output->force);
        if (active_effect != effect_index)
            return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_BUSY;
        return rin_usb_hid_gamepad_pid_actuator_submit(
            owner, application_token, effect_index, output->force);
    }
    if (active_effect == 0u) return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK;
    if (active_effect != effect_index)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_BUSY;
    return rin_usb_hid_gamepad_pid_actuator_stop(
        owner, application_token, effect_index);
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_stop(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t application_token,
    uint32_t effect_index)
{
    uint8_t report[RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAX_REPORT_BYTES];
    uint32_t report_size = 0u;
    RinUsbHidGamepadPidActuatorResult result;
    if (!owner || application_token == 0u || effect_index == 0u)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    actuator_lock(owner);
    if (!actuator_owner_valid(owner) || owner->recovery_required) {
        result = owner && owner->recovery_required
                     ? RIN_USB_HID_GAMEPAD_PID_ACTUATOR_RECOVERY_REQUIRED
                     : RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE;
        actuator_unlock(owner);
        return result;
    }
    if (owner->callback_active) {
        actuator_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_BUSY;
    }
    if (owner->active_effect != 0u && owner->active_effect != effect_index) {
        actuator_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_BUSY;
    }
    result = rin_usb_hid_gamepad_pid_actuator_serialize_operation(
        &owner->profile, effect_index,
        owner->profile.effect_operation.value_stop, report, sizeof(report),
        &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_submit_report(owner, application_token,
                                        RIN_USB_HID_GAMEPAD_PID_ACTUATOR_PERMISSION_STOP,
                                        report, report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = rin_usb_hid_gamepad_pid_actuator_serialize_stop_all(
            &owner->profile, effect_index, report, sizeof(report), &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_submit_report(owner, application_token,
                                        RIN_USB_HID_GAMEPAD_PID_ACTUATOR_PERMISSION_STOP,
                                        report, report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        owner->active_effect = 0u;
    actuator_unlock(owner);
    actuator_zero(report, sizeof(report));
    return result;
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_recover(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t device_id,
    uint32_t port_generation)
{
    if (!owner || device_id == 0u || port_generation == 0u)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    uint8_t report[RIN_USB_HID_GAMEPAD_PID_ACTUATOR_MAX_REPORT_BYTES];
    uint32_t report_size = 0u;
    RinUsbHidGamepadPidActuatorResult result;
    if (!owner) return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    actuator_lock(owner);
    if (!actuator_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != port_generation || owner->callback_active) {
        actuator_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE;
    }
    result = rin_usb_hid_gamepad_pid_actuator_serialize_operation(
        &owner->profile, 1u, owner->profile.effect_operation.value_stop,
        report, sizeof(report), &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_send_locked(owner, report, report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = rin_usb_hid_gamepad_pid_actuator_serialize_stop_all(
            &owner->profile, 1u, report, sizeof(report), &report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK)
        result = actuator_send_locked(owner, report, report_size);
    if (result == RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK) {
        owner->recovery_required = 0u;
        owner->active_effect = 0u;
        actuator_zero(owner->report, sizeof(owner->report));
    }
    actuator_unlock(owner);
    actuator_zero(report, sizeof(report));
    return result;
}

RinUsbHidGamepadPidActuatorResult
rin_usb_hid_gamepad_pid_actuator_detach(
    RinUsbHidGamepadPidActuatorOwnerV1* owner, uint64_t device_id,
    uint32_t port_generation)
{
    if (!owner || device_id == 0u || port_generation == 0u)
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_INVALID_ARGUMENT;
    actuator_lock(owner);
    if (!actuator_owner_valid(owner) || owner->device_id != device_id ||
        owner->port_generation != port_generation || owner->callback_active) {
        actuator_unlock(owner);
        return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_STALE;
    }
    owner->attached = 0u;
    owner->magic = 0u;
    actuator_zero(owner, sizeof(*owner));
    actuator_unlock(owner);
    return RIN_USB_HID_GAMEPAD_PID_ACTUATOR_OK;
}
