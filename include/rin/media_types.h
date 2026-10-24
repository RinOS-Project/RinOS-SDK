/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_MEDIA_TYPES_H
#define RIN_SDK_MEDIA_TYPES_H

/* Value-only media ABI definitions. Keep this header free of SDK handles,
 * result aliases, and entry-point declarations so kernel dispatch code can
 * consume the same stable enum values as applications. */
typedef enum RinMediaTypeV1 {
    RIN_MEDIA_TYPE_IMAGE = 1,
    RIN_MEDIA_TYPE_AUDIO = 2,
    RIN_MEDIA_TYPE_VIDEO = 3
} RinMediaTypeV1;

typedef enum RinImageDecodeFormatV1 {
    /* Each output pixel is a little-endian uint32_t value 0xAARRGGBB. */
    RIN_IMAGE_DECODE_FORMAT_ARGB8888 = 1
} RinImageDecodeFormatV1;

#if defined(__cplusplus)
static_assert(RIN_MEDIA_TYPE_IMAGE == 1, "RinMedia image type ABI drift");
static_assert(RIN_MEDIA_TYPE_AUDIO == 2, "RinMedia audio type ABI drift");
static_assert(RIN_MEDIA_TYPE_VIDEO == 3, "RinMedia video type ABI drift");
static_assert(RIN_IMAGE_DECODE_FORMAT_ARGB8888 == 1,
              "RinMedia image format ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(RIN_MEDIA_TYPE_IMAGE == 1, "RinMedia image type ABI drift");
_Static_assert(RIN_MEDIA_TYPE_AUDIO == 2, "RinMedia audio type ABI drift");
_Static_assert(RIN_MEDIA_TYPE_VIDEO == 3, "RinMedia video type ABI drift");
_Static_assert(RIN_IMAGE_DECODE_FORMAT_ARGB8888 == 1,
               "RinMedia image format ABI drift");
#endif

#endif /* RIN_SDK_MEDIA_TYPES_H */
