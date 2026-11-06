/* SPDX-License-Identifier: MIT */
/* Public, bounded Text to Speech provider wire ABI. */
#ifndef RIN_SDK_GUI_TEXT_TO_SPEECH_PROVIDER_H
#define RIN_SDK_GUI_TEXT_TO_SPEECH_PROVIDER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_TTS_WIRE_MAGIC UINT32_C(0x31535454) /* "TTS1", little-endian */
#define RIN_TTS_WIRE_VERSION_MAJOR 1u
#define RIN_TTS_WIRE_VERSION_MINOR 0u
#define RIN_TTS_WIRE_MAX_MESSAGE_BYTES 8192u
#define RIN_TTS_WIRE_MAX_TEXT_BYTES (4u * 1024u * 1024u)
#define RIN_TTS_WIRE_TEXT_CHUNK_BYTES 1024u
#define RIN_TTS_WIRE_MAX_PCM_FRAMES 2048u
#define RIN_TTS_WIRE_SAMPLE_RATE 22050u
#define RIN_TTS_SERVICE_ID "tts"
#define RIN_TTS_SERVICE_SOCKET_NAME_FORMAT "tts-%u"
#define RIN_TTS_SERVICE_SOCKET_PATH_FORMAT "/run/rin/tts-%u.sock"

#define RIN_TTS_FEATURE_STREAM_PCM UINT32_C(0x00000001)
#define RIN_TTS_FEATURE_CANCEL UINT32_C(0x00000002)
#define RIN_TTS_FEATURE_RATE_VOLUME UINT32_C(0x00000004)
#define RIN_TTS_FEATURE_VOICE_SELECTION UINT32_C(0x00000008)
#define RIN_TTS_FEATURE_KNOWN_MASK UINT32_C(0x0000000f)

#define RIN_TTS_SAMPLE_FORMAT_S16LE 1u
#define RIN_TTS_CHANNELS_MONO 1u

enum RinTtsWireMessageTypeV1 {
    RIN_TTS_WIRE_HELLO_V1 = 1,
    RIN_TTS_WIRE_HELLO_ACK_V1 = 2,
    RIN_TTS_WIRE_SPEAK_V1 = 3,
    RIN_TTS_WIRE_TEXT_CHUNK_V1 = 4,
    RIN_TTS_WIRE_SPEAK_END_V1 = 5,
    RIN_TTS_WIRE_PCM_V1 = 6,
    RIN_TTS_WIRE_DONE_V1 = 7,
    RIN_TTS_WIRE_CANCEL_V1 = 8,
    RIN_TTS_WIRE_ERROR_V1 = 9
};

enum RinTtsProviderStatusV1 {
    RIN_TTS_PROVIDER_OK = 0,
    RIN_TTS_PROVIDER_REJECTED = 1,
    RIN_TTS_PROVIDER_UNSUPPORTED = 2,
    RIN_TTS_PROVIDER_BUSY = 3,
    RIN_TTS_PROVIDER_FAILED = 4
};

/* Every frame is little-endian, bounded by MAX_MESSAGE_BYTES and contains no
 * pointers or native handles. A request ID is nonzero and is not reused on a
 * connection. The provider runs as the selected user service `tts` and binds
 * /run/rin/tts-<session-id>.sock. The client authenticates the socket's
 * kernel-owned service, application, and session identities. Text arrives in
 * ordered UTF-8 chunks after SPEAK and ends with SPEAK_END. The provider then
 * returns bounded PCM frames and one DONE or ERROR. Closing the connection
 * cancels the active synthesis request. */
typedef struct RinTtsWireHeaderV1 {
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t message_type;
    uint32_t header_bytes;
    uint32_t message_bytes;
    uint32_t flags;
    uint64_t request_id;
    uint32_t reserved[2];
} RinTtsWireHeaderV1;

typedef struct RinTtsProviderHelloV1 {
    RinTtsWireHeaderV1 header;
    uint32_t min_version;
    uint32_t max_version;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t sample_format;
    uint32_t max_pcm_frames;
    uint32_t max_text_bytes;
    uint32_t features;
    uint32_t reserved[2];
} RinTtsProviderHelloV1;

typedef struct RinTtsSpeakV1 {
    RinTtsWireHeaderV1 header;
    uint32_t voice_id;
    uint32_t language;
    uint32_t rate_percent;
    uint32_t volume_percent;
    uint32_t text_bytes;
    uint32_t reserved[3];
} RinTtsSpeakV1;

typedef struct RinTtsTextChunkV1 {
    RinTtsWireHeaderV1 header;
    uint32_t offset;
    uint32_t text_bytes;
    uint8_t text[RIN_TTS_WIRE_TEXT_CHUNK_BYTES];
} RinTtsTextChunkV1;

typedef struct RinTtsPcmV1 {
    RinTtsWireHeaderV1 header;
    uint32_t sample_rate;
    uint16_t channels;
    uint16_t sample_format;
    uint32_t frame_count;
    uint32_t reserved;
    uint8_t samples[RIN_TTS_WIRE_MAX_PCM_FRAMES * sizeof(int16_t)];
} RinTtsPcmV1;

typedef struct RinTtsDoneV1 {
    RinTtsWireHeaderV1 header;
    uint64_t total_frames;
} RinTtsDoneV1;

typedef struct RinTtsErrorV1 {
    RinTtsWireHeaderV1 header;
    uint32_t status;
    uint32_t reserved;
} RinTtsErrorV1;

static inline int rin_tts_wire_all_zero_v1(const uint8_t* bytes,
                                            uint32_t count) {
    uint32_t index;
    if (bytes == NULL && count != 0u) return 0;
    for (index = 0u; index < count; ++index)
        if (bytes[index] != 0u) return 0;
    return 1;
}

static inline int rin_tts_wire_utf8_v1(const uint8_t* text, uint32_t bytes) {
    uint32_t index = 0u;
    if (text == NULL && bytes != 0u) return 0;
    while (index < bytes) {
        const uint8_t first = text[index++];
        uint32_t scalar;
        uint32_t minimum;
        uint32_t continuation;
        if (first == 0u) return 0;
        if (first < 0x80u) continue;
        if (first >= 0xc2u && first <= 0xdfu) {
            scalar = first & 0x1fu;
            minimum = 0x80u;
            continuation = 1u;
        } else if (first >= 0xe0u && first <= 0xefu) {
            scalar = first & 0x0fu;
            minimum = 0x800u;
            continuation = 2u;
        } else if (first >= 0xf0u && first <= 0xf4u) {
            scalar = first & 0x07u;
            minimum = 0x10000u;
            continuation = 3u;
        } else {
            return 0;
        }
        if (continuation > bytes - index) return 0;
        while (continuation-- != 0u) {
            const uint8_t next = text[index++];
            if ((next & 0xc0u) != 0x80u) return 0;
            scalar = (scalar << 6u) | (next & 0x3fu);
        }
        if (scalar < minimum || (scalar >= 0xd800u && scalar <= 0xdfffu) ||
            scalar > 0x10ffffu)
            return 0;
    }
    return 1;
}

static inline uint32_t rin_tts_wire_fixed_size_v1(uint32_t type) {
    switch (type) {
    case RIN_TTS_WIRE_HELLO_V1:
    case RIN_TTS_WIRE_HELLO_ACK_V1:
        return (uint32_t)sizeof(RinTtsProviderHelloV1);
    case RIN_TTS_WIRE_SPEAK_V1:
        return (uint32_t)sizeof(RinTtsSpeakV1);
    case RIN_TTS_WIRE_SPEAK_END_V1:
    case RIN_TTS_WIRE_CANCEL_V1:
        return (uint32_t)sizeof(RinTtsWireHeaderV1);
    case RIN_TTS_WIRE_DONE_V1:
        return (uint32_t)sizeof(RinTtsDoneV1);
    case RIN_TTS_WIRE_ERROR_V1:
        return (uint32_t)sizeof(RinTtsErrorV1);
    default:
        return 0u;
    }
}

static inline int rin_tts_wire_header_v1(const RinTtsWireHeaderV1* header,
                                          uint32_t bytes,
                                          uint32_t expected_type) {
    if (header == NULL || bytes < sizeof(*header) ||
        bytes > RIN_TTS_WIRE_MAX_MESSAGE_BYTES ||
        header->magic != RIN_TTS_WIRE_MAGIC ||
        header->version_major != RIN_TTS_WIRE_VERSION_MAJOR ||
        header->version_minor != RIN_TTS_WIRE_VERSION_MINOR ||
        header->message_type != expected_type ||
        header->header_bytes != sizeof(*header) ||
        header->message_bytes != bytes || header->flags != 0u ||
        header->request_id == 0u ||
        !rin_tts_wire_all_zero_v1((const uint8_t*)header->reserved,
                                  (uint32_t)sizeof(header->reserved)))
        return 0;
    return 1;
}

static inline int rin_tts_wire_payload_v1(const void* message,
                                           uint32_t bytes) {
    const RinTtsWireHeaderV1* header =
        (const RinTtsWireHeaderV1*)message;
    uint32_t fixed_size;
    if (message == NULL || bytes < sizeof(*header) ||
        bytes > RIN_TTS_WIRE_MAX_MESSAGE_BYTES)
        return 0;
    fixed_size = rin_tts_wire_fixed_size_v1(header->message_type);
    if (fixed_size != 0u && bytes != fixed_size) return 0;
    switch (header->message_type) {
    case RIN_TTS_WIRE_HELLO_V1:
    case RIN_TTS_WIRE_HELLO_ACK_V1: {
        const RinTtsProviderHelloV1* hello =
            (const RinTtsProviderHelloV1*)message;
        return rin_tts_wire_header_v1(header, bytes, header->message_type) &&
            hello->min_version != 0u &&
            hello->min_version <= hello->max_version &&
            hello->sample_rate == RIN_TTS_WIRE_SAMPLE_RATE &&
            hello->channels == RIN_TTS_CHANNELS_MONO &&
            hello->sample_format == RIN_TTS_SAMPLE_FORMAT_S16LE &&
            hello->max_pcm_frames != 0u &&
            hello->max_pcm_frames <= RIN_TTS_WIRE_MAX_PCM_FRAMES &&
            hello->max_text_bytes != 0u &&
            hello->max_text_bytes <= RIN_TTS_WIRE_MAX_TEXT_BYTES &&
            (hello->features & ~RIN_TTS_FEATURE_KNOWN_MASK) == 0u &&
            (hello->features & RIN_TTS_FEATURE_STREAM_PCM) != 0u &&
            hello->reserved[0] == 0u && hello->reserved[1] == 0u;
    }
    case RIN_TTS_WIRE_SPEAK_V1: {
        const RinTtsSpeakV1* speak = (const RinTtsSpeakV1*)message;
        return rin_tts_wire_header_v1(header, bytes, header->message_type) &&
            speak->voice_id <= 255u && speak->language <= 2u &&
            speak->rate_percent >= 50u && speak->rate_percent <= 200u &&
            speak->volume_percent <= 100u && speak->text_bytes != 0u &&
            speak->text_bytes <= RIN_TTS_WIRE_MAX_TEXT_BYTES &&
            speak->reserved[0] == 0u && speak->reserved[1] == 0u &&
            speak->reserved[2] == 0u;
    }
    case RIN_TTS_WIRE_TEXT_CHUNK_V1: {
        const RinTtsTextChunkV1* chunk = (const RinTtsTextChunkV1*)message;
        return bytes >= offsetof(RinTtsTextChunkV1, text) + 1u &&
            bytes <= offsetof(RinTtsTextChunkV1, text) +
                         RIN_TTS_WIRE_TEXT_CHUNK_BYTES &&
            rin_tts_wire_header_v1(header, bytes, header->message_type) &&
            chunk->text_bytes != 0u &&
            chunk->text_bytes <= RIN_TTS_WIRE_TEXT_CHUNK_BYTES &&
            bytes == offsetof(RinTtsTextChunkV1, text) + chunk->text_bytes &&
            rin_tts_wire_utf8_v1(chunk->text, chunk->text_bytes);
    }
    case RIN_TTS_WIRE_SPEAK_END_V1:
    case RIN_TTS_WIRE_CANCEL_V1:
        return rin_tts_wire_header_v1(header, bytes, header->message_type);
    case RIN_TTS_WIRE_PCM_V1: {
        const RinTtsPcmV1* pcm = (const RinTtsPcmV1*)message;
        return bytes >= offsetof(RinTtsPcmV1, samples) + sizeof(int16_t) &&
            bytes <= sizeof(RinTtsPcmV1) &&
            rin_tts_wire_header_v1(header, bytes, header->message_type) &&
            pcm->sample_rate == RIN_TTS_WIRE_SAMPLE_RATE &&
            pcm->channels == RIN_TTS_CHANNELS_MONO &&
            pcm->sample_format == RIN_TTS_SAMPLE_FORMAT_S16LE &&
            pcm->frame_count != 0u &&
            pcm->frame_count <= RIN_TTS_WIRE_MAX_PCM_FRAMES &&
            bytes == offsetof(RinTtsPcmV1, samples) +
                         pcm->frame_count * sizeof(int16_t) &&
            pcm->reserved == 0u;
    }
    case RIN_TTS_WIRE_DONE_V1: {
        const RinTtsDoneV1* done = (const RinTtsDoneV1*)message;
        return rin_tts_wire_header_v1(header, bytes, header->message_type) &&
            done->total_frames != 0u;
    }
    case RIN_TTS_WIRE_ERROR_V1: {
        const RinTtsErrorV1* error = (const RinTtsErrorV1*)message;
        return rin_tts_wire_header_v1(header, bytes, header->message_type) &&
            error->status >= RIN_TTS_PROVIDER_REJECTED &&
            error->status <= RIN_TTS_PROVIDER_FAILED && error->reserved == 0u;
    }
    default:
        return 0;
    }
}

#if defined(__cplusplus)
static_assert(sizeof(RinTtsWireHeaderV1) == 40u,
              "TTS wire header ABI drift");
static_assert(sizeof(RinTtsProviderHelloV1) == 80u,
              "TTS hello ABI drift");
static_assert(sizeof(RinTtsSpeakV1) == 72u, "TTS speak ABI drift");
static_assert(offsetof(RinTtsTextChunkV1, text) == 48u,
              "TTS text ABI drift");
static_assert(offsetof(RinTtsPcmV1, samples) == 56u,
              "TTS PCM ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinTtsWireHeaderV1) == 40u,
               "TTS wire header ABI drift");
_Static_assert(sizeof(RinTtsProviderHelloV1) == 80u,
               "TTS hello ABI drift");
_Static_assert(sizeof(RinTtsSpeakV1) == 72u, "TTS speak ABI drift");
_Static_assert(offsetof(RinTtsTextChunkV1, text) == 48u,
               "TTS text ABI drift");
_Static_assert(offsetof(RinTtsPcmV1, samples) == 56u,
               "TTS PCM ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif
