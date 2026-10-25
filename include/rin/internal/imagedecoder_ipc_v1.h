/* SPDX-License-Identifier: MIT */
/* Shared SDK/service wire contract for authenticated imagedecoder IPC.
 * This is an internal RinOS protocol, not an application-facing decoder API. */

#ifndef RIN_INTERNAL_IMAGEDECODER_IPC_V1_H
#define RIN_INTERNAL_IMAGEDECODER_IPC_V1_H

#include <stdint.h>

#if defined(__cplusplus)
#define RIN_IMAGE_DECODER_ALIGNAS_8 alignas(8)
#else
#define RIN_IMAGE_DECODER_ALIGNAS_8 _Alignas(8)
#endif

#define RIN_IMAGE_DECODER_TRANSPORT_VERSION UINT16_C(2)
#define RIN_IMAGE_DECODER_TRANSPORT_MAX_SOURCE_BYTES UINT64_C(67108864)
#define RIN_IMAGE_DECODER_TRANSPORT_MAX_DIMENSION UINT32_C(4096)
#define RIN_IMAGE_DECODER_TRANSPORT_MAX_PIXELS UINT32_C(16777216)
#define RIN_IMAGE_DECODER_TRANSPORT_MAX_FRAMES UINT32_C(1024)
#define RIN_IMAGE_DECODER_TRANSPORT_MAX_OUTPUT_BYTES UINT32_C(67108864)
#define RIN_IMAGE_DECODER_TRANSPORT_MAX_PIXEL_CHUNK UINT32_C(65536)
#define RIN_IMAGE_DECODER_TRANSPORT_PIXEL_FORMAT_ARGB8888 UINT32_C(1)
#define RIN_IMAGE_DECODER_TRANSPORT_FRAME_KIND_STATIC UINT32_C(0)
#define RIN_IMAGE_DECODER_TRANSPORT_FRAME_KIND_ANIMATED UINT32_C(1)

#define RIN_IMAGE_DECODER_SERVICE_ID "imagedecoder"
#define RIN_IMAGE_DECODER_SERVICE_SOCKET_PATH "/run/rin/imagedecoder.sock"
#define RIN_IMAGE_DECODER_SERVICE_PROTOCOL_VERSION UINT16_C(2)
#define RIN_IMAGE_DECODER_SERVICE_PROTOCOL_MAGIC UINT32_C(0x31434449)

typedef enum RinImageDecoderTransportOperation {
    RIN_IMAGE_DECODER_TRANSPORT_DECODE = 1,
    RIN_IMAGE_DECODER_TRANSPORT_READ_PIXELS = 2,
    RIN_IMAGE_DECODER_TRANSPORT_CANCEL = 3,
    RIN_IMAGE_DECODER_TRANSPORT_REGISTER_SOURCE = 4
} RinImageDecoderTransportOperation;

typedef enum RinImageDecoderTransportStatus {
    RIN_IMAGE_DECODER_TRANSPORT_OK = 0,
    RIN_IMAGE_DECODER_TRANSPORT_INVALID_ARGUMENT = -1,
    RIN_IMAGE_DECODER_TRANSPORT_UNAVAILABLE = -2,
    RIN_IMAGE_DECODER_TRANSPORT_UNAUTHORIZED = -3,
    RIN_IMAGE_DECODER_TRANSPORT_TIMED_OUT = -4,
    RIN_IMAGE_DECODER_TRANSPORT_CANCELLED = -5,
    RIN_IMAGE_DECODER_TRANSPORT_NOT_FOUND = -6,
    RIN_IMAGE_DECODER_TRANSPORT_PROTOCOL = -7,
    /* Codec outcomes stay distinct on the service wire.  Callers may map
     * these onto a smaller public result set without losing IPC diagnostics. */
    RIN_IMAGE_DECODER_TRANSPORT_UNSUPPORTED = -8,
    RIN_IMAGE_DECODER_TRANSPORT_LIMIT_EXCEEDED = -9,
    RIN_IMAGE_DECODER_TRANSPORT_OVERFLOW = -10,
    RIN_IMAGE_DECODER_TRANSPORT_MALFORMED = -11
} RinImageDecoderTransportStatus;

typedef enum RinImageDecoderServiceListenerOperation {
    RIN_IMAGE_DECODER_SERVICE_LISTENER_HELLO = 0,
    RIN_IMAGE_DECODER_SERVICE_LISTENER_DECODE = 1,
    RIN_IMAGE_DECODER_SERVICE_LISTENER_READ_PIXELS = 2,
    RIN_IMAGE_DECODER_SERVICE_LISTENER_CANCEL = 3,
    RIN_IMAGE_DECODER_SERVICE_LISTENER_REGISTER_SOURCE = 4
} RinImageDecoderServiceListenerOperation;

typedef struct RinImageDecoderServiceListenerHeaderV1 {
    uint32_t magic;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t request_id;
    uint32_t payload_size;
    int32_t status;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t connection_id;
    uint32_t reserved;
} RinImageDecoderServiceListenerHeaderV1;

typedef struct RinImageDecoderServiceListenerCancelV1 {
    uint64_t request_id;
    uint64_t source_generation;
} RinImageDecoderServiceListenerCancelV1;

/* REGISTER_SOURCE appends exactly source_size encoded bytes after this record;
 * no pathname, shared-memory name, or caller-selected capability is accepted. */
typedef struct RinImageDecoderSourceRegisterRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t request_id;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_size;
    uint32_t max_width;
    uint32_t max_height;
    uint32_t max_pixels;
    uint32_t max_frames;
    uint32_t max_output_bytes;
    uint32_t reserved0;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t reserved1[2];
} RinImageDecoderSourceRegisterRequestV1;

typedef struct RinImageDecoderSourceRegisterResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    int16_t status;
    uint32_t flags;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t request_id;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_size;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_capability;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_generation;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t reserved1[2];
} RinImageDecoderSourceRegisterResponseV1;

typedef struct RinImageDecoderDecodeRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t request_id;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_capability;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_generation;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_size;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t deadline_ms;
    uint32_t max_width;
    uint32_t max_height;
    uint32_t max_pixels;
    uint32_t max_frames;
    uint32_t max_output_bytes;
    uint32_t reserved0;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t reserved1[2];
} RinImageDecoderDecodeRequestV1;

typedef struct RinImageDecoderDecodeResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    int16_t status;
    uint32_t flags;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t request_id;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_generation;
    uint32_t width;
    uint32_t height;
    uint32_t stride_bytes;
    uint32_t pixel_bytes;
    /* Total source frame count; only the first frame is returned by V1. */
    uint32_t frame_count;
    uint32_t pixel_format;
    /* RinImageFrameKind from rinimage/image.h, kept fixed-width on the wire. */
    uint32_t frame_kind;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t reserved1[2];
} RinImageDecoderDecodeResponseV1;

typedef struct RinImageDecoderPixelReadRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t request_id;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_generation;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t offset;
    uint32_t capacity;
    uint32_t reserved0;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t reserved1[2];
} RinImageDecoderPixelReadRequestV1;

typedef struct RinImageDecoderPixelReadResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    int16_t status;
    uint32_t flags;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t request_id;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t source_generation;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t offset;
    uint32_t total_bytes;
    uint32_t bytes_read;
    uint32_t reserved0;
    RIN_IMAGE_DECODER_ALIGNAS_8 uint64_t reserved1[2];
} RinImageDecoderPixelReadResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinImageDecoderServiceListenerHeaderV1) == 48u,
              "ImageDecoder listener header ABI drift");
static_assert(sizeof(RinImageDecoderServiceListenerCancelV1) == 16u,
              "ImageDecoder listener cancel ABI drift");
static_assert(sizeof(RinImageDecoderSourceRegisterRequestV1) == 72u,
              "ImageDecoder source registration ABI drift");
static_assert(sizeof(RinImageDecoderSourceRegisterResponseV1) == 64u,
              "ImageDecoder source registration response ABI drift");
static_assert(sizeof(RinImageDecoderDecodeRequestV1) == 96u,
              "ImageDecoder decode request ABI drift");
static_assert(sizeof(RinImageDecoderDecodeResponseV1) == 80u,
              "ImageDecoder decode response ABI drift");
static_assert(sizeof(RinImageDecoderPixelReadRequestV1) == 64u,
              "ImageDecoder pixel read request ABI drift");
static_assert(sizeof(RinImageDecoderPixelReadResponseV1) == 72u,
              "ImageDecoder pixel read response ABI drift");
#else
_Static_assert(sizeof(RinImageDecoderServiceListenerHeaderV1) == 48u,
               "ImageDecoder listener header ABI drift");
_Static_assert(sizeof(RinImageDecoderServiceListenerCancelV1) == 16u,
               "ImageDecoder listener cancel ABI drift");
_Static_assert(sizeof(RinImageDecoderSourceRegisterRequestV1) == 72u,
               "ImageDecoder source registration ABI drift");
_Static_assert(sizeof(RinImageDecoderSourceRegisterResponseV1) == 64u,
               "ImageDecoder source registration response ABI drift");
_Static_assert(sizeof(RinImageDecoderDecodeRequestV1) == 96u,
               "ImageDecoder decode request ABI drift");
_Static_assert(sizeof(RinImageDecoderDecodeResponseV1) == 80u,
               "ImageDecoder decode response ABI drift");
_Static_assert(sizeof(RinImageDecoderPixelReadRequestV1) == 64u,
               "ImageDecoder pixel read request ABI drift");
_Static_assert(sizeof(RinImageDecoderPixelReadResponseV1) == 72u,
               "ImageDecoder pixel read response ABI drift");
#endif

#endif /* RIN_INTERNAL_IMAGEDECODER_IPC_V1_H */
