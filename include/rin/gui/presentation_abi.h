/* SPDX-License-Identifier: MIT */
/* Public frame submission and presentation token ABI. */
#ifndef RIN_SDK_GUI_PRESENTATION_ABI_H
#define RIN_SDK_GUI_PRESENTATION_ABI_H

#include <stdint.h>
#include "window_abi.h"
#include "surface_abi.h"

#define RIN_PRESENTATION_ABI_VERSION 1u
#define RIN_PRESENTATION_FLAG_FULL_FRAME UINT32_C(0x00000001)
#define RIN_PRESENTATION_FLAG_VSYNC UINT32_C(0x00000002)
#define RIN_PRESENTATION_FLAG_FEEDBACK UINT32_C(0x00000004)
#define RIN_PRESENTATION_FLAG_KNOWN_MASK \
    (RIN_PRESENTATION_FLAG_FULL_FRAME | RIN_PRESENTATION_FLAG_VSYNC | \
     RIN_PRESENTATION_FLAG_FEEDBACK)

typedef struct RinPresentationTokenV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    RinWindowHandle window;
    uint64_t frame_sequence;
    uint64_t surface_generation;
    uint32_t buffer_slot;
    uint32_t damage_count;
    uint32_t reserved[2];
} RinPresentationTokenV1;

static inline int rin_presentation_token_valid(
    const RinPresentationTokenV1* value) {
    return value != 0 && value->struct_size == sizeof(*value) &&
           value->version == RIN_PRESENTATION_ABI_VERSION &&
           (value->flags & ~RIN_PRESENTATION_FLAG_KNOWN_MASK) == 0u &&
           value->window != RIN_WINDOW_HANDLE_INVALID &&
           value->frame_sequence != 0u && value->surface_generation != 0u &&
           value->buffer_slot < 2u && value->reserved[0] == 0u &&
           value->reserved[1] == 0u;
}

#ifdef __cplusplus
static_assert(sizeof(RinPresentationTokenV1) == 48u,
              "RinPresentationTokenV1 ABI drift");
#else
_Static_assert(sizeof(RinPresentationTokenV1) == 48u,
               "RinPresentationTokenV1 ABI drift");
#endif

#endif /* RIN_SDK_GUI_PRESENTATION_ABI_H */
