#ifndef RIN_SDK_MEDIA_H
#define RIN_SDK_MEDIA_H
#include "abi.h"
#include "media_types.h"
#include "syscall_abi.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum RinAudioSampleFormatV1 {
    /* Signed 16-bit little-endian samples, interleaved by channel. */
    RIN_MEDIA_AUDIO_SAMPLE_FORMAT_S16LE = RIN_AUDIO_SAMPLE_FORMAT_S16LE
} RinAudioSampleFormatV1;

typedef struct RinImageInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t row_stride_bytes;
    uint32_t frame_count;
    uint32_t reserved0;
    uint64_t decoded_size_bytes;
    uint64_t reserved[2];
} RinImageInfoV1;

enum {
    RIN_AUDIO_INFO_FRAME_COUNT_KNOWN = 1u << 0
};

typedef struct RinAudioInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t sample_rate_hz;
    uint32_t channels;
    uint32_t sample_format;
    uint32_t bytes_per_frame;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t frame_count;
    uint64_t reserved[2];
} RinAudioInfoV1;

/* V1 image decode returns the first frame in tightly packed ARGB8888 words;
 * row_stride_bytes is width * 4 and decoded_size_bytes is stride * height.
 * Image frame_count is the total frame count (1 for a still image), or zero
 * when the decoder cannot determine it. Audio decode returns interleaved
 * S16LE frames; query its rate and channel count before sizing the output
 * buffer. A clear FRAME_COUNT_KNOWN flag means audio frame_count is zero. */

/* Keep the public values fixed even when enum declarations are reorganized. */
#if defined(__cplusplus)
static_assert(RIN_AUDIO_SAMPLE_FORMAT_S16LE == 1, "RinMedia audio format ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(RIN_AUDIO_SAMPLE_FORMAT_S16LE == 1, "RinMedia audio format ABI drift");
#endif

typedef struct RinMediaOpenV1 {
    uint32_t struct_size;
    uint32_t version;
    RinStringV1 mime_type;
    RinSliceV1 source;
    RinMediaTypeV1 media_type;
    uint32_t flags;
    uint64_t reserved[4];
} RinMediaOpenV1;

#if defined(__cplusplus)
static_assert(sizeof(RinMediaOpenV1) == 80u,
              "RinMediaOpenV1 ABI drift");
static_assert(offsetof(RinMediaOpenV1, media_type) == 40u,
              "RinMediaOpenV1 media type offset drift");
static_assert(offsetof(RinMediaOpenV1, reserved) == 48u,
              "RinMediaOpenV1 reserved offset drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinMediaOpenV1) == 80u,
               "RinMediaOpenV1 ABI drift");
_Static_assert(offsetof(RinMediaOpenV1, media_type) == 40u,
               "RinMediaOpenV1 media type offset drift");
_Static_assert(offsetof(RinMediaOpenV1, reserved) == 48u,
               "RinMediaOpenV1 reserved offset drift");
#endif
/* A failed open writes RIN_HANDLE_INVALID to a non-null output handle. */
RIN_SDK_API RinResult rin_media_open_v1(const RinMediaOpenV1* request, RinMediaObject* object);
RIN_SDK_API RinResult rin_image_get_info_v1(RinMediaObject object, RinImageInfoV1* info);
RIN_SDK_API RinResult rin_image_decode_v1(RinMediaObject object, uint32_t format, RinSliceV1 output, uint64_t* required);
RIN_SDK_API RinResult rin_audio_get_info_v1(RinMediaObject object, RinAudioInfoV1* info);
RIN_SDK_API RinResult rin_audio_decode_v1(RinMediaObject object, RinSliceV1 output, uint64_t* frames);
RIN_SDK_API RinResult rin_video_decode_v1(RinMediaObject object, RinSurface target, uint64_t* timestamp_ns);

#if defined(__cplusplus)
static_assert(sizeof(RinImageInfoV1) == 56u, "RinImageInfoV1 ABI drift");
static_assert(sizeof(RinAudioInfoV1) == 56u, "RinAudioInfoV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinImageInfoV1) == 56u, "RinImageInfoV1 ABI drift");
_Static_assert(sizeof(RinAudioInfoV1) == 56u, "RinAudioInfoV1 ABI drift");
#endif
#ifdef __cplusplus
}
#endif
#endif
