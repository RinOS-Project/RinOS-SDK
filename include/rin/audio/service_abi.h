/* SPDX-License-Identifier: MIT */
/* Versioned control and shared-ring contract for the user-space audio broker.
 *
 * This header is wire ABI.  All integer fields are little-endian, reserved
 * fields must be zero, and handles/generations are opaque values owned by the
 * service.  PCM payloads use the explicit S16LE/F32LE formats below.  The
 * transport and the service's socket path are deliberately not part of this
 * contract.
 */
#ifndef RIN_AUDIO_SERVICE_ABI_H
#define RIN_AUDIO_SERVICE_ABI_H

#include "device_abi.h"
#include "policy_abi.h"
#include <stdint.h>

#pragma pack(push, 1)

#define RIN_AUDIO_SERVICE_PROTOCOL_VERSION UINT32_C(1)
#define RIN_AUDIO_SERVICE_PROTOCOL_MAGIC UINT32_C(0x31534152) /* RAS1 */
#define RIN_AUDIO_SERVICE_MAX_CLIENTS UINT32_C(32)
#define RIN_AUDIO_SERVICE_MAX_STREAMS_PER_CLIENT UINT32_C(8)
#define RIN_AUDIO_SERVICE_MAX_STREAMS UINT32_C(128)
#define RIN_AUDIO_SERVICE_MAX_RING_FRAMES UINT32_C(65536)
#define RIN_AUDIO_SERVICE_MIN_RING_FRAMES UINT32_C(64)
#define RIN_AUDIO_SERVICE_MIN_SAMPLE_RATE UINT32_C(8000)
#define RIN_AUDIO_SERVICE_MAX_SAMPLE_RATE UINT32_C(192000)
#define RIN_AUDIO_SERVICE_MAX_CHANNELS UINT32_C(2)
#define RIN_AUDIO_SERVICE_APPLICATION_ID_BYTES UINT32_C(32)
#define RIN_AUDIO_SERVICE_APPLICATION_NAME_MAX UINT32_C(64)
#define RIN_AUDIO_SERVICE_APPLICATION_IMAGE_PATH_MAX UINT32_C(96)
#define RIN_AUDIO_SERVICE_MAX_APPLICATIONS UINT32_C(16)
#define RIN_AUDIO_SERVICE_DEVICE_MAX RIN_MEDIA_AUDIO_DEVICE_MAX
#define RIN_AUDIO_SERVICE_EVENT_MAX UINT32_C(16)
#define RIN_AUDIO_SERVICE_RING_MAGIC UINT32_C(0x31475241) /* RARG */

enum {
    RIN_AUDIO_SERVICE_OK = 0,
    RIN_AUDIO_SERVICE_INVALID = -1,
    RIN_AUDIO_SERVICE_AUTHENTICATION = -2,
    RIN_AUDIO_SERVICE_LIMIT = -3,
    RIN_AUDIO_SERVICE_NOT_FOUND = -4,
    RIN_AUDIO_SERVICE_STALE = -5,
    RIN_AUDIO_SERVICE_NO_DEVICE = -6,
    RIN_AUDIO_SERVICE_NO_SPACE = -7,
    RIN_AUDIO_SERVICE_UNDERRUN = -8,
    RIN_AUDIO_SERVICE_POLICY = -9,
    RIN_AUDIO_SERVICE_BUSY = -10,
    RIN_AUDIO_SERVICE_PROTOCOL = -11,
    RIN_AUDIO_SERVICE_RANGE = -12,
    RIN_AUDIO_SERVICE_BACKEND = -13,
};

enum {
    RIN_AUDIO_SERVICE_FORMAT_S16LE = 1u,
    RIN_AUDIO_SERVICE_FORMAT_F32LE = 2u,
};

enum {
    RIN_AUDIO_SERVICE_STREAM_STOPPED = 1u,
    RIN_AUDIO_SERVICE_STREAM_RUNNING = 2u,
    RIN_AUDIO_SERVICE_STREAM_PAUSED = 3u,
    RIN_AUDIO_SERVICE_STREAM_DRAINING = 4u,
};

/* Stream categories keep system UX sounds independent from application
 * mixer state.  The category is advisory only after authentication; the
 * service still applies the authenticated client policy to every stream. */
enum {
    RIN_AUDIO_SERVICE_STREAM_CATEGORY_APPLICATION = 0u,
    RIN_AUDIO_SERVICE_STREAM_CATEGORY_NOTIFICATION = 1u,
    RIN_AUDIO_SERVICE_STREAM_CATEGORY_LOGIN = 2u,
    RIN_AUDIO_SERVICE_STREAM_CATEGORY_LOGOUT = 3u,
    RIN_AUDIO_SERVICE_STREAM_CATEGORY_ERROR = 4u,
    RIN_AUDIO_SERVICE_STREAM_CATEGORY_MAX = 5u,
};

enum {
    RIN_AUDIO_SERVICE_OP_CONNECT = 1u,
    RIN_AUDIO_SERVICE_OP_CREATE_STREAM = 2u,
    RIN_AUDIO_SERVICE_OP_DESTROY_STREAM = 3u,
    RIN_AUDIO_SERVICE_OP_START = 4u,
    RIN_AUDIO_SERVICE_OP_PAUSE = 5u,
    RIN_AUDIO_SERVICE_OP_RESUME = 6u,
    RIN_AUDIO_SERVICE_OP_DRAIN = 7u,
    RIN_AUDIO_SERVICE_OP_FLUSH = 8u,
    RIN_AUDIO_SERVICE_OP_SET_STREAM_VOLUME = 9u,
    RIN_AUDIO_SERVICE_OP_SET_STREAM_MUTE = 10u,
    RIN_AUDIO_SERVICE_OP_SET_APPLICATION_VOLUME = 11u,
    RIN_AUDIO_SERVICE_OP_SET_APPLICATION_MUTE = 12u,
    RIN_AUDIO_SERVICE_OP_GET_STATUS = 13u,
    RIN_AUDIO_SERVICE_OP_SET_MASTER_VOLUME = 14u,
    RIN_AUDIO_SERVICE_OP_SET_MASTER_MUTE = 15u,
    RIN_AUDIO_SERVICE_OP_ENUMERATE_DEVICES = 16u,
    RIN_AUDIO_SERVICE_OP_SELECT_DEVICE = 17u,
    RIN_AUDIO_SERVICE_OP_GET_DIAGNOSTICS = 18u,
    RIN_AUDIO_SERVICE_OP_POLL_EVENTS = 19u,
    RIN_AUDIO_SERVICE_OP_SET_STREAM_PAN = 20u,
    RIN_AUDIO_SERVICE_OP_SET_POLICY = 21u,
    RIN_AUDIO_SERVICE_OP_SET_CATEGORY_VOLUME = 22u,
    RIN_AUDIO_SERVICE_OP_SET_CATEGORY_MUTE = 23u,
    RIN_AUDIO_SERVICE_OP_SET_INPUT_VOLUME = 24u,
    RIN_AUDIO_SERVICE_OP_SET_INPUT_MUTE = 25u,
    RIN_AUDIO_SERVICE_OP_ENUMERATE_APPLICATIONS = 26u,
    RIN_AUDIO_SERVICE_OP_SET_APPLICATION_VOLUME_FOR = 27u,
    RIN_AUDIO_SERVICE_OP_SET_APPLICATION_MUTE_FOR = 28u,
};

enum {
    RIN_AUDIO_SERVICE_EVENT_UNDERRUN = 1u,
    RIN_AUDIO_SERVICE_EVENT_DEVICE_REMOVED = 2u,
    RIN_AUDIO_SERVICE_EVENT_POLICY_REJECTED = 3u,
    /* Backend period loss is distinct from an application ring underrun. */
    RIN_AUDIO_SERVICE_EVENT_HARDWARE_UNDERRUN = 4u,
    /* A successfully submitted backend quantum; coalesced while unpolled. */
    RIN_AUDIO_SERVICE_EVENT_PERIOD_COMPLETE = 5u,
    /* Capture producer could not fit the complete input quantum. */
    RIN_AUDIO_SERVICE_EVENT_CAPTURE_OVERRUN = 6u,
};

typedef struct RinAudioServiceMessageHeaderV1 {
    uint32_t magic;
    uint32_t version;
    uint32_t operation;
    uint32_t reserved0;
    uint64_t request_id;
    uint64_t stream_handle;
    uint32_t payload_bytes;
    int32_t status;
} RinAudioServiceMessageHeaderV1;

/* This identity is copied from the kernel-owned authenticated socket option;
 * it is never accepted from an application message. */
typedef struct RinAudioServiceClientIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t owner_uid;
    uint32_t reserved0;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    uint64_t connection_id;
    uint8_t application_id[RIN_AUDIO_SERVICE_APPLICATION_ID_BYTES];
    char application_name[RIN_AUDIO_SERVICE_APPLICATION_NAME_MAX];
    /* Optional authenticated launch path.  Empty means the kernel did not
     * expose a path for this client; consumers must never infer one. */
    char application_image_path[RIN_AUDIO_SERVICE_APPLICATION_IMAGE_PATH_MAX];
} RinAudioServiceClientIdentityV1;

typedef struct RinAudioServiceFormatV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t sample_format;
    uint32_t reserved0;
} RinAudioServiceFormatV1;

static inline int rin_audio_service_format_valid(
    const RinAudioServiceFormatV1* format)
{
    return format != NULL &&
           format->struct_size == sizeof(*format) &&
           format->version == RIN_AUDIO_SERVICE_PROTOCOL_VERSION &&
           format->sample_rate >= RIN_AUDIO_SERVICE_MIN_SAMPLE_RATE &&
           format->sample_rate <= RIN_AUDIO_SERVICE_MAX_SAMPLE_RATE &&
           (format->channels == 1u || format->channels == 2u) &&
           (format->sample_format == RIN_AUDIO_SERVICE_FORMAT_S16LE ||
            format->sample_format == RIN_AUDIO_SERVICE_FORMAT_F32LE) &&
           format->reserved0 == 0u;
}

static inline uint32_t rin_audio_service_format_frame_bytes(
    const RinAudioServiceFormatV1* format)
{
    if (!rin_audio_service_format_valid(format)) return 0u;
    return format->channels *
        (format->sample_format == RIN_AUDIO_SERVICE_FORMAT_F32LE ? 4u : 2u);
}

/* The header and PCM bytes live in one named RinSHM region.  The producer is
 * the application, the consumer is Audio Service, and both counters are
 * monotonically increasing frame positions rather than wrapped indexes. */
#pragma pack(pop)
typedef struct RinAudioServiceSharedRingV1 {
    uint32_t magic;
    uint32_t version;
    uint32_t header_bytes;
    uint32_t frame_bytes;
    uint32_t capacity_frames;
    uint32_t reserved0;
    uint64_t stream_generation;
    volatile uint64_t producer_frames;
    volatile uint64_t consumer_frames;
    uint64_t reserved1[2];
} RinAudioServiceSharedRingV1;
#pragma pack(push, 1)

typedef struct RinAudioServiceCreateStreamRequestV1 {
    RinAudioServiceFormatV1 format;
    uint32_t capacity_frames;
    uint32_t device_kind;
    uint64_t device_id;
    char ring_name[64];
    uint32_t category;
    uint32_t reserved0;
} RinAudioServiceCreateStreamRequestV1;

/* CREATE_STREAM is the only request that allocates a shared-ring mapping.
 * Keep its complete structural admission in the shared protocol header so a
 * client and audiod cannot silently disagree about the format or ring size
 * before any mapping is touched.  device_id==0 selects the current default;
 * non-zero ids remain accepted for generation-bound route extensions. */
static inline int rin_audio_service_ring_name_valid(const char name[64])
{
    uint32_t length = 0u;
    if (name == NULL) return 0;
    for (; length < 64u && name[length] != '\0'; ++length) {
        const uint8_t value = (uint8_t)name[length];
        if (value < 0x21u || value > 0x7eu) return 0;
    }
    if (length == 0u || length == 64u) return 0;
    for (; length < 64u; ++length)
        if (name[length] != '\0') return 0;
    return 1;
}

static inline int rin_audio_service_create_stream_request_valid(
    const RinAudioServiceCreateStreamRequestV1* request)
{
    const uint32_t frame_bytes = request == NULL
        ? 0u : rin_audio_service_format_frame_bytes(&request->format);
    return request != NULL && frame_bytes != 0u &&
           request->capacity_frames >= RIN_AUDIO_SERVICE_MIN_RING_FRAMES &&
           request->capacity_frames <= RIN_AUDIO_SERVICE_MAX_RING_FRAMES &&
           request->device_kind != 0u &&
           (request->device_kind == RIN_MEDIA_AUDIO_DEVICE_OUTPUT ||
            request->device_kind == RIN_MEDIA_AUDIO_DEVICE_INPUT) &&
           request->category < RIN_AUDIO_SERVICE_STREAM_CATEGORY_MAX &&
           request->reserved0 == 0u &&
           rin_audio_service_ring_name_valid(request->ring_name) &&
           request->capacity_frames <= UINT32_MAX / frame_bytes;
}

typedef struct RinAudioServiceCategoryValueRequestV1 {
    uint32_t category;
    uint32_t value;
} RinAudioServiceCategoryValueRequestV1;

typedef struct RinAudioServiceConnectReplyV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t connection_id;
    uint32_t max_streams;
    uint32_t max_ring_bytes;
    uint64_t reserved0;
} RinAudioServiceConnectReplyV1;

static inline int rin_audio_service_connect_reply_valid(
    const RinAudioServiceConnectReplyV1* reply)
{
    return reply != NULL &&
           reply->struct_size == sizeof(*reply) &&
           reply->version == RIN_AUDIO_SERVICE_PROTOCOL_VERSION &&
           reply->connection_id != 0u &&
           reply->max_streams == RIN_AUDIO_SERVICE_MAX_STREAMS_PER_CLIENT &&
           reply->max_ring_bytes >=
               RIN_AUDIO_SERVICE_MIN_RING_FRAMES * 2u &&
           reply->max_ring_bytes <= RIN_AUDIO_SERVICE_MAX_RING_FRAMES * 8u &&
           reply->reserved0 == 0u;
}

typedef struct RinAudioServiceStreamReplyV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t stream_handle;
    uint64_t stream_generation;
    uint32_t capacity_frames;
    uint32_t frame_bytes;
    uint64_t reserved0;
} RinAudioServiceStreamReplyV1;

static inline int rin_audio_service_stream_reply_valid(
    const RinAudioServiceStreamReplyV1* reply, uint32_t expected_capacity,
    uint32_t expected_frame_bytes)
{
    return reply != NULL &&
           reply->struct_size == sizeof(*reply) &&
           reply->version == RIN_AUDIO_SERVICE_PROTOCOL_VERSION &&
           reply->stream_handle != 0u &&
           reply->stream_generation != 0u &&
           reply->capacity_frames >= RIN_AUDIO_SERVICE_MIN_RING_FRAMES &&
           reply->capacity_frames <= RIN_AUDIO_SERVICE_MAX_RING_FRAMES &&
           reply->capacity_frames == expected_capacity &&
           (reply->frame_bytes == 2u || reply->frame_bytes == 4u ||
            reply->frame_bytes == 8u) &&
           reply->frame_bytes == expected_frame_bytes &&
           reply->reserved0 == 0u;
}

typedef struct RinAudioServiceValueRequestV1 {
    uint32_t value;
    uint32_t reserved0;
} RinAudioServiceValueRequestV1;

typedef struct RinAudioServicePanRequestV1 {
    int32_t pan;
    uint32_t reserved0;
} RinAudioServicePanRequestV1;

typedef struct RinAudioServiceSelectDeviceRequestV1 {
    uint64_t snapshot_generation;
    uint64_t device_id;
    uint32_t kind;
    uint32_t reserved0;
} RinAudioServiceSelectDeviceRequestV1;

typedef struct RinAudioServiceStatusV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t sample_format;
    uint32_t capacity_frames;
    uint32_t queued_frames;
    uint64_t submitted_frames;
    uint64_t played_frames;
    uint64_t application_underrun_count;
    uint64_t hardware_underrun_count;
    uint64_t frames_dropped;
    uint64_t deadline_misses;
    uint64_t device_latency_frames;
    uint64_t total_latency_frames;
    uint32_t stream_volume;
    uint32_t application_volume;
    uint32_t master_volume;
    uint32_t stream_muted;
    uint32_t application_muted;
    uint32_t master_muted;
    uint32_t input_volume;
    uint32_t input_muted;
} RinAudioServiceStatusV1;

typedef struct RinAudioServiceDeviceListV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t snapshot_generation;
    uint32_t device_count;
    uint32_t default_output_index;
    uint32_t default_input_index;
    uint32_t reserved0;
    RinMediaAudioDeviceV1 devices[RIN_AUDIO_SERVICE_DEVICE_MAX];
} RinAudioServiceDeviceListV1;

/* Device snapshots cross an authenticated IPC boundary.  Keep the complete
 * topology check in the shared protocol header so every client rejects the
 * same malformed generation/default/duplicate combination before exposing it
 * to Settings or an application.  A zero-device snapshot is valid: it is the
 * generation-bound representation of a last-device removal. */
static inline int rin_audio_service_device_list_valid(
    const RinAudioServiceDeviceListV1* devices)
{
    if (devices == NULL ||
        devices->struct_size != sizeof(*devices) ||
        devices->version != RIN_AUDIO_SERVICE_PROTOCOL_VERSION ||
        devices->snapshot_generation == 0u ||
        devices->device_count > RIN_AUDIO_SERVICE_DEVICE_MAX ||
        devices->reserved0 != 0u)
        return 0;
    for (uint32_t index = 0u; index < devices->device_count; ++index) {
        if (!rin_media_audio_device_valid_v1(&devices->devices[index]) ||
            devices->devices[index].generation !=
                devices->snapshot_generation)
            return 0;
        for (uint32_t previous = 0u; previous < index; ++previous)
            if (devices->devices[previous].device_id ==
                devices->devices[index].device_id)
                return 0;
    }
    for (uint32_t index = devices->device_count;
         index < RIN_AUDIO_SERVICE_DEVICE_MAX; ++index) {
        const uint8_t* bytes = (const uint8_t*)&devices->devices[index];
        for (uint32_t byte = 0u; byte < sizeof(devices->devices[index]);
             ++byte)
            if (bytes[byte] != 0u) return 0;
    }
    if (devices->default_output_index != UINT32_MAX &&
        (devices->default_output_index >= devices->device_count ||
         devices->devices[devices->default_output_index].kind !=
             RIN_MEDIA_AUDIO_DEVICE_OUTPUT ||
         devices->devices[devices->default_output_index].available == 0u))
        return 0;
    if (devices->default_input_index != UINT32_MAX &&
        (devices->default_input_index >= devices->device_count ||
         devices->devices[devices->default_input_index].kind !=
             RIN_MEDIA_AUDIO_DEVICE_INPUT ||
         devices->devices[devices->default_input_index].available == 0u))
        return 0;
    return 1;
}

typedef struct RinAudioServiceDiagnosticsV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t active_clients;
    uint32_t active_streams;
    uint32_t active_capture_streams;
    uint32_t master_volume;
    uint32_t master_muted;
    uint32_t device_available;
    uint64_t frames_submitted;
    uint64_t frames_played;
    uint64_t application_underruns;
    uint64_t hardware_underruns;
    uint64_t frames_dropped;
    uint64_t mixer_deadline_misses;
    uint64_t resampler_frames;
    uint64_t current_queue_frames;
    uint32_t capture_peak_percent;
    uint32_t reserved1;
} RinAudioServiceDiagnosticsV1;

static inline int rin_audio_service_status_valid(
    const RinAudioServiceStatusV1* status)
{
    return status != NULL &&
           status->struct_size == sizeof(*status) &&
           status->version == RIN_AUDIO_SERVICE_PROTOCOL_VERSION &&
           (status->state == RIN_AUDIO_SERVICE_STREAM_STOPPED ||
            status->state == RIN_AUDIO_SERVICE_STREAM_RUNNING ||
            status->state == RIN_AUDIO_SERVICE_STREAM_PAUSED ||
            status->state == RIN_AUDIO_SERVICE_STREAM_DRAINING) &&
           status->sample_rate >= RIN_AUDIO_SERVICE_MIN_SAMPLE_RATE &&
           status->sample_rate <= RIN_AUDIO_SERVICE_MAX_SAMPLE_RATE &&
           (status->channels == 1u || status->channels == 2u) &&
           (status->sample_format == RIN_AUDIO_SERVICE_FORMAT_S16LE ||
            status->sample_format == RIN_AUDIO_SERVICE_FORMAT_F32LE) &&
           status->capacity_frames >= RIN_AUDIO_SERVICE_MIN_RING_FRAMES &&
           status->capacity_frames <= RIN_AUDIO_SERVICE_MAX_RING_FRAMES &&
           status->queued_frames <= status->capacity_frames &&
           status->stream_volume <= 100u &&
           status->application_volume <= 100u &&
           status->master_volume <= 100u && status->stream_muted <= 1u &&
           status->application_muted <= 1u && status->master_muted <= 1u &&
           status->input_volume <= 100u && status->input_muted <= 1u;
}

static inline int rin_audio_service_diagnostics_valid(
    const RinAudioServiceDiagnosticsV1* diagnostics)
{
    return diagnostics != NULL &&
           diagnostics->struct_size == sizeof(*diagnostics) &&
           diagnostics->version == RIN_AUDIO_SERVICE_PROTOCOL_VERSION &&
           diagnostics->active_clients <= RIN_AUDIO_SERVICE_MAX_CLIENTS &&
           diagnostics->active_streams <= RIN_AUDIO_SERVICE_MAX_STREAMS &&
           diagnostics->active_capture_streams <= diagnostics->active_streams &&
           diagnostics->master_volume <= 100u &&
           diagnostics->master_muted <= 1u &&
           diagnostics->device_available <= 1u &&
           diagnostics->capture_peak_percent <= 100u &&
           diagnostics->reserved1 == 0u;
}

/* The application mixer exposes authenticated identity generation, the
 * kernel-provided signed package/catalog display name, and an optional
 * authenticated launch path for signed icon resource lookup. */
typedef struct RinAudioServiceApplicationSnapshotV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t application_id;
    uint64_t application_generation;
    uint32_t output_stream_count;
    uint32_t active_output_stream_count;
    uint32_t application_volume;
    uint32_t application_muted;
    uint32_t active;
    uint32_t reserved0;
    char application_name[RIN_AUDIO_SERVICE_APPLICATION_NAME_MAX];
    char application_image_path[RIN_AUDIO_SERVICE_APPLICATION_IMAGE_PATH_MAX];
} RinAudioServiceApplicationSnapshotV1;

typedef struct RinAudioServiceApplicationSnapshotListV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t snapshot_generation;
    uint32_t application_count;
    uint32_t reserved0;
    RinAudioServiceApplicationSnapshotV1 applications[
        RIN_AUDIO_SERVICE_MAX_APPLICATIONS];
} RinAudioServiceApplicationSnapshotListV1;

static inline int rin_audio_service_application_image_path_valid(
    const char path[RIN_AUDIO_SERVICE_APPLICATION_IMAGE_PATH_MAX])
{
    uint32_t index;
    if (path == NULL || path[0] == '\0') return 1;
    if (path[0] != '/') return 0;
    for (index = 1u; index < RIN_AUDIO_SERVICE_APPLICATION_IMAGE_PATH_MAX;
         ++index)
        if (path[index] == '\0') return 1;
    return 0;
}

/* Keep every policy-owner consumer on the same strict wire contract.  The
 * service never publishes an application row without an output stream, and
 * an active count cannot exceed the total count.  Treating these invariants
 * as a shared validator prevents a client/UI pair from accepting different
 * malformed snapshots. */
static inline int rin_audio_service_application_snapshot_valid(
    const RinAudioServiceApplicationSnapshotV1* application)
{
    uint32_t index;
    int name_terminated = application != NULL &&
        application->application_name[0] == '\0';
    if (application != NULL && !name_terminated) {
        name_terminated = 0;
        for (index = 1u; index < RIN_AUDIO_SERVICE_APPLICATION_NAME_MAX;
             ++index) {
            if (application->application_name[index] == '\0') {
                name_terminated = 1;
                break;
            }
        }
    }
    return application != NULL &&
           application->struct_size == sizeof(*application) &&
           application->version == RIN_AUDIO_SERVICE_PROTOCOL_VERSION &&
           application->application_id != 0u &&
           application->application_generation != 0u &&
           application->output_stream_count != 0u &&
           application->output_stream_count <= RIN_AUDIO_SERVICE_MAX_STREAMS &&
           application->active_output_stream_count <=
               application->output_stream_count &&
           application->application_volume <= 100u &&
           application->application_muted <= 1u &&
           application->active <= 1u &&
           application->active ==
               (application->active_output_stream_count != 0u ? 1u : 0u) &&
           application->reserved0 == 0u && name_terminated &&
           rin_audio_service_application_image_path_valid(
               application->application_image_path);
}

static inline int rin_audio_service_application_snapshot_list_valid(
    const RinAudioServiceApplicationSnapshotListV1* applications)
{
    if (applications == NULL ||
        applications->struct_size != sizeof(*applications) ||
        applications->version != RIN_AUDIO_SERVICE_PROTOCOL_VERSION ||
        applications->snapshot_generation == 0u ||
        applications->application_count > RIN_AUDIO_SERVICE_MAX_APPLICATIONS ||
        applications->reserved0 != 0u)
        return 0;
    for (uint32_t index = 0u; index < applications->application_count; ++index) {
        if (!rin_audio_service_application_snapshot_valid(
                &applications->applications[index]))
            return 0;
        for (uint32_t previous = 0u; previous < index; ++previous)
            if (applications->applications[previous].application_id ==
                    applications->applications[index].application_id &&
                applications->applications[previous].application_generation ==
                    applications->applications[index].application_generation)
                return 0;
    }
    for (uint32_t index = applications->application_count;
         index < RIN_AUDIO_SERVICE_MAX_APPLICATIONS; ++index) {
        const uint8_t* bytes =
            (const uint8_t*)&applications->applications[index];
        for (uint32_t byte = 0u;
             byte < sizeof(applications->applications[index]); ++byte)
            if (bytes[byte] != 0u) return 0;
    }
    return 1;
}

typedef struct RinAudioServiceApplicationValueRequestV1 {
    uint64_t application_id;
    uint64_t application_generation;
    uint32_t value;
    uint32_t reserved0;
} RinAudioServiceApplicationValueRequestV1;

typedef struct RinAudioServiceEventV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t type;
    uint32_t reserved0;
    uint64_t sequence;
    uint64_t stream_handle;
    uint64_t object_id;
    uint64_t generation;
    int32_t status;
    uint32_t reserved1;
} RinAudioServiceEventV1;

typedef struct RinAudioServiceEventBatchV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t event_count;
    uint32_t reserved0;
    RinAudioServiceEventV1 events[RIN_AUDIO_SERVICE_EVENT_MAX];
} RinAudioServiceEventBatchV1;

static inline int rin_audio_service_event_valid(
    const RinAudioServiceEventV1* event)
{
    return event != NULL &&
           event->struct_size == sizeof(*event) &&
           event->version == RIN_AUDIO_SERVICE_PROTOCOL_VERSION &&
           event->type >= RIN_AUDIO_SERVICE_EVENT_UNDERRUN &&
           event->type <= RIN_AUDIO_SERVICE_EVENT_CAPTURE_OVERRUN &&
           event->reserved0 == 0u && event->sequence != 0u &&
           event->status <= 0 && event->reserved1 == 0u;
}

static inline int rin_audio_service_event_batch_valid(
    const RinAudioServiceEventBatchV1* events)
{
    if (events == NULL || events->struct_size != sizeof(*events) ||
        events->version != RIN_AUDIO_SERVICE_PROTOCOL_VERSION ||
        events->event_count > RIN_AUDIO_SERVICE_EVENT_MAX ||
        events->reserved0 != 0u)
        return 0;
    for (uint32_t index = 0u; index < events->event_count; ++index) {
        if (!rin_audio_service_event_valid(&events->events[index])) return 0;
        for (uint32_t previous = 0u; previous < index; ++previous)
            if (events->events[previous].sequence ==
                events->events[index].sequence)
                return 0;
    }
    for (uint32_t index = events->event_count;
         index < RIN_AUDIO_SERVICE_EVENT_MAX; ++index) {
        const uint8_t* bytes = (const uint8_t*)&events->events[index];
        for (uint32_t byte = 0u; byte < sizeof(events->events[index]); ++byte)
            if (bytes[byte] != 0u) return 0;
    }
    return 1;
}

/* Validate the shared-memory header before calculating any data offset.  A
 * producer or consumer must reject a ring whose monotonic counters have an
 * impossible distance; this prevents wrap/index arithmetic from becoming an
 * out-of-bounds access. */
static inline int rin_audio_service_shared_ring_valid(
    const RinAudioServiceSharedRingV1* ring)
{
    uint64_t producer;
    uint64_t consumer;
    if (ring == NULL || ring->magic != RIN_AUDIO_SERVICE_RING_MAGIC ||
        ring->version != RIN_AUDIO_SERVICE_PROTOCOL_VERSION ||
        ring->header_bytes != sizeof(*ring) || ring->frame_bytes == 0u ||
        ring->capacity_frames < RIN_AUDIO_SERVICE_MIN_RING_FRAMES ||
        ring->capacity_frames > RIN_AUDIO_SERVICE_MAX_RING_FRAMES ||
        ring->reserved0 != 0u || ring->reserved1[0] != 0u ||
        ring->reserved1[1] != 0u || ring->stream_generation == 0u)
        return 0;
    producer = __atomic_load_n(&ring->producer_frames, __ATOMIC_ACQUIRE);
    consumer = __atomic_load_n(&ring->consumer_frames, __ATOMIC_ACQUIRE);
    return producer >= consumer &&
           producer - consumer <= ring->capacity_frames;
}

#pragma pack(pop)

#if defined(__cplusplus)
static_assert(sizeof(RinAudioServiceMessageHeaderV1) == 40u,
              "audio service message ABI drift");
static_assert(sizeof(RinAudioServiceClientIdentityV1) == 232u,
              "audio service identity ABI drift");
static_assert(sizeof(RinAudioServiceFormatV1) == 24u,
              "audio service format ABI drift");
static_assert(sizeof(RinAudioServiceSharedRingV1) == 64u,
              "audio service ring ABI drift");
static_assert(sizeof(RinAudioServiceCreateStreamRequestV1) == 112u,
              "audio service create ABI drift");
static_assert(sizeof(RinAudioServiceConnectReplyV1) == 32u,
              "audio service connect ABI drift");
static_assert(sizeof(RinAudioServiceStreamReplyV1) == 40u,
              "audio service stream reply ABI drift");
static_assert(sizeof(RinAudioServiceStatusV1) == 128u,
              "audio service status ABI drift");
static_assert(sizeof(RinAudioServiceDeviceListV1) == 992u,
              "audio service device list ABI drift");
static_assert(sizeof(RinAudioServiceDiagnosticsV1) == 104u,
              "audio service diagnostics ABI drift");
static_assert(sizeof(RinAudioServiceApplicationSnapshotV1) == 208u,
              "audio service application ABI drift");
static_assert(sizeof(RinAudioServiceApplicationSnapshotListV1) == 3352u,
              "audio service application list ABI drift");
static_assert(sizeof(RinAudioServiceEventV1) == 56u,
              "audio service event ABI drift");
static_assert(sizeof(RinAudioServiceEventBatchV1) == 912u,
              "audio service event batch ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinAudioServiceMessageHeaderV1) == 40u,
               "audio service message ABI drift");
_Static_assert(sizeof(RinAudioServiceClientIdentityV1) == 232u,
               "audio service identity ABI drift");
_Static_assert(sizeof(RinAudioServiceFormatV1) == 24u,
               "audio service format ABI drift");
_Static_assert(sizeof(RinAudioServiceSharedRingV1) == 64u,
               "audio service ring ABI drift");
_Static_assert(sizeof(RinAudioServiceCreateStreamRequestV1) == 112u,
               "audio service create ABI drift");
_Static_assert(sizeof(RinAudioServiceConnectReplyV1) == 32u,
               "audio service connect ABI drift");
_Static_assert(sizeof(RinAudioServiceStreamReplyV1) == 40u,
               "audio service stream reply ABI drift");
_Static_assert(sizeof(RinAudioServiceStatusV1) == 128u,
               "audio service status ABI drift");
_Static_assert(sizeof(RinAudioServiceDeviceListV1) == 992u,
               "audio service device list ABI drift");
_Static_assert(sizeof(RinAudioServiceDiagnosticsV1) == 104u,
               "audio service diagnostics ABI drift");
_Static_assert(sizeof(RinAudioServiceApplicationSnapshotV1) == 208u,
               "audio service application ABI drift");
_Static_assert(sizeof(RinAudioServiceApplicationSnapshotListV1) == 3352u,
               "audio service application list ABI drift");
_Static_assert(sizeof(RinAudioServiceEventV1) == 56u,
               "audio service event ABI drift");
_Static_assert(sizeof(RinAudioServiceEventBatchV1) == 912u,
               "audio service event batch ABI drift");
#endif

#endif /* RIN_AUDIO_SERVICE_ABI_H */
