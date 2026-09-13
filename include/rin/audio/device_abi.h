/* SPDX-License-Identifier: MIT */
/* Bounded audio input/output device and mixer-control contract.
 *
 * Device identifiers and generations are service-owned opaque values.  The
 * names below retain the existing RinMedia wire spelling for source and wire
 * compatibility; RinAudio* aliases are the public SDK vocabulary.
 */
#ifndef RIN_AUDIO_DEVICE_ABI_H
#define RIN_AUDIO_DEVICE_ABI_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define RIN_MEDIA_AUDIO_SETTINGS_VERSION 1u
#define RIN_MEDIA_AUDIO_DEVICE_NAME_BYTES 64u
#define RIN_MEDIA_AUDIO_DEVICE_MAX 8u

enum {
    RIN_MEDIA_AUDIO_DEVICE_OUTPUT = 1u,
    RIN_MEDIA_AUDIO_DEVICE_INPUT = 2u,
};

enum {
    RIN_MEDIA_AUDIO_CONTROL_VOLUME = 1u,
    RIN_MEDIA_AUDIO_CONTROL_MUTE = 2u,
};

typedef struct RinMediaAudioDeviceV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_id;
    uint64_t generation;
    uint32_t kind;
    uint32_t available;
    uint32_t channels_min;
    uint32_t channels_max;
    uint32_t sample_rate_min;
    uint32_t sample_rate_max;
    char name[RIN_MEDIA_AUDIO_DEVICE_NAME_BYTES];
    uint32_t reserved0;
    uint32_t reserved1;
} RinMediaAudioDeviceV1;

typedef RinMediaAudioDeviceV1 RinAudioDeviceV1;

typedef struct RinMediaAudioSelectionRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t snapshot_generation;
    uint64_t device_id;
    uint32_t kind;
    uint32_t reserved0;
} RinMediaAudioSelectionRequestV1;

typedef struct RinMediaAudioControlRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t snapshot_generation;
    uint64_t device_id;
    uint32_t kind;
    uint32_t control;
    uint32_t value;
    uint32_t reserved0;
} RinMediaAudioControlRequestV1;

typedef RinMediaAudioSelectionRequestV1 RinAudioDeviceSelectionRequestV1;
typedef RinMediaAudioControlRequestV1 RinAudioDeviceControlRequestV1;

#if defined(__cplusplus)
static_assert(sizeof(RinMediaAudioDeviceV1) == 120u,
              "media audio device ABI drift");
static_assert(sizeof(RinMediaAudioSelectionRequestV1) == 32u,
              "media audio selection ABI drift");
static_assert(sizeof(RinMediaAudioControlRequestV1) == 40u,
              "media audio control ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinMediaAudioDeviceV1) == 120u,
               "media audio device ABI drift");
_Static_assert(sizeof(RinMediaAudioSelectionRequestV1) == 32u,
               "media audio selection ABI drift");
_Static_assert(sizeof(RinMediaAudioControlRequestV1) == 40u,
               "media audio control ABI drift");
#endif

static inline int rin_media_audio_device_name_valid_v1(const char* name)
{
    size_t index;
    if (name == NULL || name[0] == '\0') return 0;
    for (index = 0u; index < RIN_MEDIA_AUDIO_DEVICE_NAME_BYTES; ++index) {
        unsigned char value = (unsigned char)name[index];
        if (value == '\0') return 1;
        if (value < 0x20u || value > 0x7eu) return 0;
    }
    return 0;
}

static inline int rin_media_audio_device_valid_v1(
    const RinMediaAudioDeviceV1* device)
{
    if (device == NULL || device->struct_size != sizeof(*device) ||
        device->version != RIN_MEDIA_AUDIO_SETTINGS_VERSION ||
        device->device_id == 0u || device->generation == 0u ||
        (device->kind != RIN_MEDIA_AUDIO_DEVICE_OUTPUT &&
         device->kind != RIN_MEDIA_AUDIO_DEVICE_INPUT) ||
        device->available > 1u || device->channels_min == 0u ||
        device->channels_min > device->channels_max ||
        device->channels_max > 32u || device->sample_rate_min < 8000u ||
        device->sample_rate_min > device->sample_rate_max ||
        device->sample_rate_max > 384000u || device->reserved0 != 0u ||
        device->reserved1 != 0u ||
        !rin_media_audio_device_name_valid_v1(device->name))
        return 0;
    return 1;
}

static inline int rin_media_audio_selection_request_valid_v1(
    const RinMediaAudioSelectionRequestV1* request)
{
    return request != NULL && request->struct_size == sizeof(*request) &&
           request->version == RIN_MEDIA_AUDIO_SETTINGS_VERSION &&
           request->snapshot_generation != 0u && request->device_id != 0u &&
           (request->kind == RIN_MEDIA_AUDIO_DEVICE_OUTPUT ||
            request->kind == RIN_MEDIA_AUDIO_DEVICE_INPUT) &&
           request->reserved0 == 0u;
}

static inline int rin_media_audio_control_request_valid_v1(
    const RinMediaAudioControlRequestV1* request)
{
    return request != NULL && request->struct_size == sizeof(*request) &&
           request->version == RIN_MEDIA_AUDIO_SETTINGS_VERSION &&
           request->snapshot_generation != 0u && request->device_id != 0u &&
           (request->kind == RIN_MEDIA_AUDIO_DEVICE_OUTPUT ||
            request->kind == RIN_MEDIA_AUDIO_DEVICE_INPUT) &&
           (request->control == RIN_MEDIA_AUDIO_CONTROL_VOLUME ||
            request->control == RIN_MEDIA_AUDIO_CONTROL_MUTE) &&
           request->value <= 100u && request->reserved0 == 0u;
}

/* Return 1 for a selected device, 0 for a valid but unavailable/missing one,
 * and -1 when the complete provider snapshot is malformed. */
static inline int rin_media_audio_select_device_v1(
    const RinMediaAudioDeviceV1* devices, size_t device_count,
    uint64_t snapshot_generation, uint32_t kind, uint64_t device_id,
    RinMediaAudioSelectionRequestV1* output, size_t output_size)
{
    size_t index;
    if (output == NULL || output_size < sizeof(*output) ||
        devices == NULL || device_count == 0u ||
        device_count > RIN_MEDIA_AUDIO_DEVICE_MAX ||
        snapshot_generation == 0u || device_id == 0u ||
        (kind != RIN_MEDIA_AUDIO_DEVICE_OUTPUT &&
         kind != RIN_MEDIA_AUDIO_DEVICE_INPUT))
        return -1;
#if defined(__cplusplus)
    *output = RinMediaAudioSelectionRequestV1{};
#else
    *output = (RinMediaAudioSelectionRequestV1){0};
#endif
    for (index = 0u; index < device_count; ++index) {
        const RinMediaAudioDeviceV1* device = &devices[index];
        size_t duplicate;
        if (!rin_media_audio_device_valid_v1(device) ||
            device->generation != snapshot_generation)
            return -1;
        for (duplicate = 0u; duplicate < index; ++duplicate)
            if (devices[duplicate].device_id == device->device_id)
                return -1;
    }
    for (index = 0u; index < device_count; ++index) {
        const RinMediaAudioDeviceV1* device = &devices[index];
        if (device->device_id == device_id && device->kind == kind &&
            device->available != 0u) {
            output->struct_size = sizeof(*output);
            output->version = RIN_MEDIA_AUDIO_SETTINGS_VERSION;
            output->snapshot_generation = snapshot_generation;
            output->device_id = device_id;
            output->kind = kind;
            return 1;
        }
    }
    return 0;
}

static inline int rin_media_audio_build_control_v1(
    const RinMediaAudioSelectionRequestV1* selection, uint32_t control,
    uint32_t value, RinMediaAudioControlRequestV1* output, size_t output_size)
{
    if (output == NULL || output_size < sizeof(*output) ||
        !rin_media_audio_selection_request_valid_v1(selection) ||
        (control != RIN_MEDIA_AUDIO_CONTROL_VOLUME &&
         control != RIN_MEDIA_AUDIO_CONTROL_MUTE) ||
        value > 100u)
        return -1;
#if defined(__cplusplus)
    *output = RinMediaAudioControlRequestV1{};
#else
    *output = (RinMediaAudioControlRequestV1){0};
#endif
    output->struct_size = sizeof(*output);
    output->version = RIN_MEDIA_AUDIO_SETTINGS_VERSION;
    output->snapshot_generation = selection->snapshot_generation;
    output->device_id = selection->device_id;
    output->kind = selection->kind;
    output->control = control;
    output->value = value;
    return 1;
}

/* Authenticated audio providers and Settings communicate through this
 * callback owner.  The owner retains a bounded copy of the latest device
 * snapshot and never forwards a caller-owned array after the callback
 * returns.  Selection and control callbacks receive only fixed requests. */
typedef int (*RinMediaAudioSnapshotPublishV1)(
    void* context, uint64_t generation, const RinMediaAudioDeviceV1* devices,
    size_t device_count);
typedef int (*RinMediaAudioSelectionApplyV1)(
    void* context, const RinMediaAudioSelectionRequestV1* request);
typedef int (*RinMediaAudioSelectionRevokeV1)(
    void* context, const RinMediaAudioSelectionRequestV1* request);
typedef int (*RinMediaAudioControlApplyV1)(
    void* context, const RinMediaAudioControlRequestV1* request);

typedef struct RinMediaAudioSettingsOwnerOpsV1 {
    uint32_t struct_size;
    uint32_t version;
    void* context;
    RinMediaAudioSnapshotPublishV1 publish_snapshot;
    RinMediaAudioSelectionApplyV1 apply_selection;
    RinMediaAudioSelectionRevokeV1 revoke_selection;
    RinMediaAudioControlApplyV1 apply_control;
} RinMediaAudioSettingsOwnerOpsV1;

typedef struct RinMediaAudioSettingsOwnerV1 {
    uint32_t struct_size;
    uint32_t version;
    RinMediaAudioSettingsOwnerOpsV1 ops;
    RinMediaAudioDeviceV1 devices[RIN_MEDIA_AUDIO_DEVICE_MAX];
    uint64_t snapshot_generation;
    uint32_t device_count;
    uint32_t has_snapshot;
    RinMediaAudioSelectionRequestV1 selected_output;
    RinMediaAudioSelectionRequestV1 selected_input;
    uint32_t has_output;
    uint32_t has_input;
    uint32_t callback_active;
    uint32_t reserved0;
} RinMediaAudioSettingsOwnerV1;

#define RIN_MEDIA_AUDIO_SETTINGS_OWNER_VERSION 1u

static inline int rin_media_audio_settings_owner_ops_valid_v1(
    const RinMediaAudioSettingsOwnerOpsV1* operations)
{
    return operations != NULL &&
           operations->struct_size == sizeof(*operations) &&
           operations->version == RIN_MEDIA_AUDIO_SETTINGS_OWNER_VERSION &&
           operations->context != NULL && operations->publish_snapshot != NULL &&
           operations->apply_selection != NULL &&
           operations->revoke_selection != NULL &&
           operations->apply_control != NULL;
}

static inline int rin_media_audio_settings_owner_valid_v1(
    const RinMediaAudioSettingsOwnerV1* owner)
{
    return owner != NULL && owner->struct_size == sizeof(*owner) &&
           owner->version == RIN_MEDIA_AUDIO_SETTINGS_OWNER_VERSION &&
           rin_media_audio_settings_owner_ops_valid_v1(&owner->ops) &&
           owner->device_count <= RIN_MEDIA_AUDIO_DEVICE_MAX &&
           owner->has_snapshot <= 1u && owner->has_output <= 1u &&
           owner->has_input <= 1u && owner->callback_active <= 1u &&
           owner->reserved0 == 0u &&
           (!owner->has_snapshot || (owner->device_count != 0u &&
                                     owner->snapshot_generation != 0u)) &&
           (!owner->has_output ||
            rin_media_audio_selection_request_valid_v1(&owner->selected_output)) &&
           (!owner->has_input ||
            rin_media_audio_selection_request_valid_v1(&owner->selected_input));
}

static inline int rin_media_audio_settings_owner_init_v1(
    RinMediaAudioSettingsOwnerV1* owner,
    const RinMediaAudioSettingsOwnerOpsV1* operations)
{
    if (owner == NULL) return -1;
    memset(owner, 0, sizeof(*owner));
    if (!rin_media_audio_settings_owner_ops_valid_v1(operations)) return -1;
    owner->struct_size = sizeof(*owner);
    owner->version = RIN_MEDIA_AUDIO_SETTINGS_OWNER_VERSION;
    owner->ops = *operations;
    return 0;
}

static inline int rin_media_audio_settings_owner_revoke_one_v1(
    RinMediaAudioSettingsOwnerV1* owner,
    RinMediaAudioSelectionRequestV1* selection, uint32_t* present)
{
    int result;
    if (*present == 0u) return 0;
    owner->callback_active = 1u;
    result = owner->ops.revoke_selection(owner->ops.context, selection);
    owner->callback_active = 0u;
    if (result != 0) return -1;
    memset(selection, 0, sizeof(*selection));
    *present = 0u;
    return 0;
}

static inline int rin_media_audio_settings_owner_clear_v1(
    RinMediaAudioSettingsOwnerV1* owner)
{
    if (!rin_media_audio_settings_owner_valid_v1(owner) ||
        owner->callback_active != 0u)
        return -1;
    if (rin_media_audio_settings_owner_revoke_one_v1(
            owner, &owner->selected_output, &owner->has_output) != 0)
        return -1;
    if (rin_media_audio_settings_owner_revoke_one_v1(
            owner, &owner->selected_input, &owner->has_input) != 0)
        return -1;
    return 0;
}

/* Publish a new provider snapshot.  A caller must explicitly clear selected
 * devices first, so a provider failure can never silently discard an active
 * Settings selection. */
static inline int rin_media_audio_settings_owner_bind_snapshot_v1(
    RinMediaAudioSettingsOwnerV1* owner,
    const RinMediaAudioDeviceV1* devices, size_t device_count,
    uint64_t snapshot_generation)
{
    size_t index;
    size_t duplicate;
    if (!rin_media_audio_settings_owner_valid_v1(owner) ||
        owner->callback_active != 0u || devices == NULL ||
        device_count == 0u || device_count > RIN_MEDIA_AUDIO_DEVICE_MAX ||
        snapshot_generation == 0u)
        return -1;
    for (index = 0u; index < device_count; ++index) {
        if (!rin_media_audio_device_valid_v1(&devices[index]) ||
            devices[index].generation != snapshot_generation)
            return -1;
        for (duplicate = 0u; duplicate < index; ++duplicate)
            if (devices[duplicate].device_id == devices[index].device_id)
                return -1;
    }
    if (owner->has_snapshot != 0u &&
        owner->snapshot_generation == snapshot_generation &&
        owner->device_count == device_count &&
        memcmp(owner->devices, devices,
               device_count * sizeof(owner->devices[0])) == 0)
        return 1;
    if (owner->has_output != 0u || owner->has_input != 0u) return -1;
    owner->callback_active = 1u;
    if (owner->ops.publish_snapshot(owner->ops.context, snapshot_generation,
                                    devices, device_count) != 0) {
        owner->callback_active = 0u;
        return -1;
    }
    owner->callback_active = 0u;
    memset(owner->devices, 0, sizeof(owner->devices));
    memcpy(owner->devices, devices, device_count * sizeof(owner->devices[0]));
    owner->snapshot_generation = snapshot_generation;
    owner->device_count = (uint32_t)device_count;
    owner->has_snapshot = 1u;
    return 1;
}

static inline int rin_media_audio_settings_owner_select_v1(
    RinMediaAudioSettingsOwnerV1* owner, uint32_t kind, uint64_t device_id)
{
    RinMediaAudioSelectionRequestV1 candidate;
    RinMediaAudioSelectionRequestV1* previous;
    uint32_t* present;
    int result;
    if (!rin_media_audio_settings_owner_valid_v1(owner) ||
        owner->callback_active != 0u || owner->has_snapshot == 0u)
        return -1;
    result = rin_media_audio_select_device_v1(
        owner->devices, owner->device_count, owner->snapshot_generation, kind,
        device_id, &candidate, sizeof(candidate));
    if (result <= 0) return result;
    previous = kind == RIN_MEDIA_AUDIO_DEVICE_OUTPUT
        ? &owner->selected_output : &owner->selected_input;
    present = kind == RIN_MEDIA_AUDIO_DEVICE_OUTPUT
        ? &owner->has_output : &owner->has_input;
    if (*present != 0u && previous->device_id == candidate.device_id) return 1;
    if (*present != 0u &&
        rin_media_audio_settings_owner_revoke_one_v1(
            owner, previous, present) != 0)
        return -1;
    owner->callback_active = 1u;
    result = owner->ops.apply_selection(owner->ops.context, &candidate);
    owner->callback_active = 0u;
    if (result != 0) return -1;
    *previous = candidate;
    *present = 1u;
    return 1;
}

static inline int rin_media_audio_settings_owner_control_v1(
    RinMediaAudioSettingsOwnerV1* owner, uint32_t kind, uint32_t control,
    uint32_t value)
{
    const RinMediaAudioSelectionRequestV1* selection;
    RinMediaAudioControlRequestV1 request;
    if (!rin_media_audio_settings_owner_valid_v1(owner) ||
        owner->callback_active != 0u || owner->has_snapshot == 0u)
        return -1;
    selection = kind == RIN_MEDIA_AUDIO_DEVICE_OUTPUT
        ? (owner->has_output != 0u ? &owner->selected_output : NULL)
        : (kind == RIN_MEDIA_AUDIO_DEVICE_INPUT && owner->has_input != 0u
               ? &owner->selected_input : NULL);
    if (selection == NULL ||
        rin_media_audio_build_control_v1(selection, control, value, &request,
                                         sizeof(request)) != 1)
        return -1;
    owner->callback_active = 1u;
    if (owner->ops.apply_control(owner->ops.context, &request) != 0) {
        owner->callback_active = 0u;
        return -1;
    }
    owner->callback_active = 0u;
    return 1;
}

#endif /* RIN_AUDIO_DEVICE_ABI_H */
