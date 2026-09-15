/* SPDX-License-Identifier: MIT */
/* Public render-surface descriptor ABI. */
#ifndef RIN_SDK_GUI_SURFACE_ABI_H
#define RIN_SDK_GUI_SURFACE_ABI_H

#include <stdint.h>
#include "../render_target.h"

#define RIN_SURFACE_ABI_VERSION RIN_RENDER_TARGET_VERSION
typedef RinRenderTarget RinSurfaceDescriptorV1;

#define RIN_SURFACE_FLAG_WRITABLE RIN_RENDER_TARGET_FLAG_WRITABLE
#define RIN_SURFACE_FLAG_EXTERNAL RIN_RENDER_TARGET_FLAG_EXTERNAL
#define RIN_SURFACE_FORMAT_BGRA32 RIN_RENDER_TARGET_FORMAT_BGRA32
#define RIN_SURFACE_FORMAT_RGBA32 RIN_RENDER_TARGET_FORMAT_RGBA32

/* pixels is a borrowed mapping for the duration of the acquired frame.
 * generation and buffer_slot identify the exact compositor buffer and must be
 * returned unchanged at release/present.  The descriptor never exposes a
 * physical framebuffer address or a renderer-private pointer. */
static inline int rin_surface_descriptor_valid(
    const RinSurfaceDescriptorV1* value) {
    return value != 0 && value->struct_size == sizeof(*value) &&
           value->version == RIN_SURFACE_ABI_VERSION &&
           value->reserved0 == 0u && value->pixels != 0 &&
           value->width != 0u && value->height != 0u &&
           (uint64_t)value->pitch >= (uint64_t)value->width * 4u &&
           value->generation != 0u &&
           value->buffer_slot < 2u &&
           (value->flags & ~(RIN_SURFACE_FLAG_WRITABLE |
                             RIN_SURFACE_FLAG_EXTERNAL)) == 0u;
}

#ifdef __cplusplus
#if UINTPTR_MAX > UINT32_MAX
static_assert(sizeof(RinSurfaceDescriptorV1) == 48u,
              "RinSurfaceDescriptorV1 ABI drift");
#else
static_assert(sizeof(RinSurfaceDescriptorV1) == 44u,
              "RinSurfaceDescriptorV1 ABI drift");
#endif
#else
#if UINTPTR_MAX > UINT32_MAX
_Static_assert(sizeof(RinSurfaceDescriptorV1) == 48u,
               "RinSurfaceDescriptorV1 ABI drift");
#else
_Static_assert(sizeof(RinSurfaceDescriptorV1) == 44u,
               "RinSurfaceDescriptorV1 ABI drift");
#endif
#endif

#endif /* RIN_SDK_GUI_SURFACE_ABI_H */
