/* SPDX-License-Identifier: MIT */
/* Fixed chunk ABI for privacy-gated camera and microphone capture. */

#ifndef RIN_API_RIN_CAPTURE_H
#define RIN_API_RIN_CAPTURE_H

#include <stdint.h>

#include <rin/resource/capability.h>

#define RIN_CAPTURE_CHUNK_VERSION UINT32_C(0x00010000)
#define RIN_CAPTURE_CHUNK_HEADER_SIZE UINT32_C(64)
#define RIN_CAPTURE_MAX_CHUNK_BYTES UINT32_C(65536)
#define RIN_CAPTURE_MAX_FRAME_BYTES UINT32_C(268435456)

#define RIN_CAPTURE_COMMAND_READ_CHUNK UINT32_C(1)
#define RIN_CAPTURE_COMMAND_GET_PROFILE UINT32_C(2)
#define RIN_CAPTURE_TRANSPORT_PROFILE_VERSION UINT32_C(0x00010000)

#define RIN_CAPTURE_CHUNK_FRAME_START UINT32_C(0x00000001)
#define RIN_CAPTURE_CHUNK_FRAME_END UINT32_C(0x00000002)
#define RIN_CAPTURE_CHUNK_DROPPED_BEFORE UINT32_C(0x00000004)
#define RIN_CAPTURE_CHUNK_KNOWN_FLAGS                                  \
    (RIN_CAPTURE_CHUNK_FRAME_START | RIN_CAPTURE_CHUNK_FRAME_END |     \
     RIN_CAPTURE_CHUNK_DROPPED_BEFORE)

#define RIN_CAPTURE_FORMAT_PCM_S16LE UINT32_C(1)
#define RIN_CAPTURE_FORMAT_PCM_F32LE UINT32_C(2)
#define RIN_CAPTURE_FORMAT_RGBX8888 UINT32_C(0x00000100)
#define RIN_CAPTURE_FORMAT_NV12 UINT32_C(0x00000101)
#define RIN_CAPTURE_FORMAT_MJPEG UINT32_C(0x00000102)

/* Payload bytes immediately follow this header. Large frames are split into
 * chunks with the same sequence, total_size and format description. */
typedef struct RinCaptureChunkV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t sequence;
    uint64_t monotonic_time_ns;
    uint32_t total_size;
    uint32_t offset;
    uint32_t payload_size;
    uint32_t format;
    uint32_t flags;
    uint32_t width;
    uint32_t height;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t reserved0;
} RinCaptureChunkV1;

/* The negotiated format is fixed for the lifetime of one capture session.
 * It is published separately from READ_CHUNK so a consumer never has to
 * infer sample geometry from an untrusted first packet. */
typedef struct RinCaptureTransportProfileV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t stream_id;
    uint32_t format;
    uint32_t max_frame_size;
    uint32_t width;
    uint32_t height;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinCaptureTransportProfileV1;

#if defined(__cplusplus)
static_assert(sizeof(RinCaptureChunkV1) == RIN_CAPTURE_CHUNK_HEADER_SIZE,
              "RinCaptureChunkV1 ABI size changed");
static_assert(sizeof(RinCaptureTransportProfileV1) == 64u,
              "RinCaptureTransportProfileV1 ABI size changed");
#else
_Static_assert(sizeof(RinCaptureChunkV1) == RIN_CAPTURE_CHUNK_HEADER_SIZE,
               "RinCaptureChunkV1 ABI size changed");
_Static_assert(sizeof(RinCaptureTransportProfileV1) == 64u,
               "RinCaptureTransportProfileV1 ABI size changed");
#endif

#endif /* RIN_API_RIN_CAPTURE_H */
