/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVERS_DISPLAY_EDID_H
#define RIN_DRIVERS_DISPLAY_EDID_H

#include <stdint.h>

#define RIN_DISPLAY_EDID_INFO_ABI_VERSION 1u
#define RIN_DISPLAY_EDID_BLOCK_SIZE 128u
#define RIN_DISPLAY_EDID_MAX_BLOCKS 4u
#define RIN_DISPLAY_EDID_MAX_BYTES \
    (RIN_DISPLAY_EDID_BLOCK_SIZE * RIN_DISPLAY_EDID_MAX_BLOCKS)
#define RIN_DISPLAY_EDID_MAX_MODES 361u
#define RIN_DISPLAY_EDID_NO_MODE_INDEX UINT32_MAX

#define RIN_DISPLAY_EDID_INFO_HAS_PREFERRED_MODE (1u << 0)
#define RIN_DISPLAY_EDID_INFO_HAS_CTA (1u << 1)
#define RIN_DISPLAY_EDID_INFO_HAS_HDR_STATIC_METADATA (1u << 2)

#define RIN_DISPLAY_EDID_MODE_INTERLACED (1u << 0)

#define RIN_DISPLAY_EDID_HDR_TRADITIONAL_SDR (1u << 0)
#define RIN_DISPLAY_EDID_HDR_TRADITIONAL_HDR (1u << 1)
#define RIN_DISPLAY_EDID_HDR_PQ (1u << 2)
#define RIN_DISPLAY_EDID_HDR_HLG (1u << 3)
#define RIN_DISPLAY_EDID_HDR_STATIC_TYPE_1 (1u << 0)

typedef enum RinDisplayEdidResult {
    RIN_DISPLAY_EDID_OK = 0,
    RIN_DISPLAY_EDID_INVALID_ARGUMENT = -1,
    RIN_DISPLAY_EDID_MALFORMED = -2,
    RIN_DISPLAY_EDID_UNSUPPORTED = -3
} RinDisplayEdidResult;

typedef struct RinDisplayEdidModeV1 {
    uint32_t pixel_clock_khz;
    uint32_t width;
    uint32_t height;
    uint32_t horizontal_total;
    uint32_t vertical_total;
    uint32_t refresh_millihz;
    uint32_t flags;
} RinDisplayEdidModeV1;

typedef struct RinDisplayEdidInfoV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t manufacturer_id;
    uint32_t product_code;
    uint32_t serial_number;
    uint32_t edid_version;
    uint32_t edid_revision;
    uint32_t physical_width_mm;
    uint32_t physical_height_mm;
    uint32_t block_count;
    uint32_t cta_extension_count;
    uint32_t detailed_timing_count;
    uint32_t short_video_descriptor_count;
    uint32_t native_vic;
    uint32_t hdr_eotf_flags;
    uint32_t hdr_static_metadata_flags;
    uint32_t flags;
    RinDisplayEdidModeV1 preferred_mode;
    uint32_t reserved[4];
} RinDisplayEdidInfoV1;

/* A validated, pointer-free list of EDID detailed timings and known CTA SVD
 * modes. Four base descriptors, eighteen CTA detailed descriptors, and up to
 * three CTA extensions' worth of 119 SVD entries fit under the block limit. */
typedef struct RinDisplayEdidModeListV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t mode_count;
    uint32_t preferred_mode_index;
    RinDisplayEdidModeV1 modes[RIN_DISPLAY_EDID_MAX_MODES];
    uint32_t reserved[4];
} RinDisplayEdidModeListV1;

RinDisplayEdidResult rin_display_edid_admit(
    const uint8_t* bytes, uint32_t length, RinDisplayEdidInfoV1* info);

RinDisplayEdidResult rin_display_edid_collect_modes(
    const uint8_t* bytes, uint32_t length, RinDisplayEdidModeListV1* modes);

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDisplayEdidModeV1) == 28u,
               "display EDID mode ABI drift");
_Static_assert(sizeof(RinDisplayEdidInfoV1) == 112u,
               "display EDID info ABI drift");
_Static_assert(sizeof(RinDisplayEdidModeListV1) == 10140u,
               "display EDID mode list ABI drift");
#endif

#endif
