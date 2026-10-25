/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_MEDIA_TYPES_H
#define RIN_SDK_MEDIA_TYPES_H

#include <stdint.h>

/* Value-only media ABI definitions. Keep this header free of SDK handles,
 * result aliases, and entry-point declarations so kernel dispatch code can
 * consume the same stable values as applications. The public field type is
 * explicitly uint32_t so compiler enum-width options cannot change the ABI. */
typedef uint32_t RinMediaTypeV1;
enum {
    RIN_MEDIA_TYPE_IMAGE = 1u,
    RIN_MEDIA_TYPE_AUDIO = 2u,
    RIN_MEDIA_TYPE_VIDEO = 3u
};

/* Bound every v1 caller-owned source span before it is copied or decoded. */
#define RIN_MEDIA_MAX_SOURCE_BYTES_V1 UINT64_C(1073741824)
/* The first authenticated VIDEO backend keeps an indexed AVI snapshot in
 * bounded userspace memory. Other media types retain the generic 1 GiB cap. */
#define RIN_MEDIA_MAX_VIDEO_SOURCE_BYTES_V1 UINT64_C(268435456)

/* MIME metadata is small and bounded independently of the media payload. */
#define RIN_MEDIA_MAX_MIME_TYPE_BYTES_V1 UINT64_C(4096)

typedef enum RinImageDecodeFormatV1 {
    /* Each output pixel is a little-endian uint32_t value 0xAARRGGBB. */
    RIN_IMAGE_DECODE_FORMAT_ARGB8888 = 1
} RinImageDecodeFormatV1;

#if defined(__cplusplus)
static_assert(sizeof(RinMediaTypeV1) == 4u,
              "RinMedia type width ABI drift");
static_assert(RIN_MEDIA_TYPE_IMAGE == 1, "RinMedia image type ABI drift");
static_assert(RIN_MEDIA_TYPE_AUDIO == 2, "RinMedia audio type ABI drift");
static_assert(RIN_MEDIA_TYPE_VIDEO == 3, "RinMedia video type ABI drift");
static_assert(RIN_IMAGE_DECODE_FORMAT_ARGB8888 == 1,
              "RinMedia image format ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinMediaTypeV1) == 4u,
               "RinMedia type width ABI drift");
_Static_assert(RIN_MEDIA_TYPE_IMAGE == 1, "RinMedia image type ABI drift");
_Static_assert(RIN_MEDIA_TYPE_AUDIO == 2, "RinMedia audio type ABI drift");
_Static_assert(RIN_MEDIA_TYPE_VIDEO == 3, "RinMedia video type ABI drift");
_Static_assert(RIN_IMAGE_DECODE_FORMAT_ARGB8888 == 1,
               "RinMedia image format ABI drift");
#endif

#endif /* RIN_SDK_MEDIA_TYPES_H */
