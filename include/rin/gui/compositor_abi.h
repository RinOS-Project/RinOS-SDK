/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_COMPOSITOR_ABI_H
#define RIN_SDK_COMPOSITOR_ABI_H

#include <stddef.h>
#include <stdint.h>
#include "native_event.h"
#include "text_input.h"

#define RIN_COMPOSITOR_DISPLAY_VERSION 2u
#define RIN_COMPOSITOR_DISPLAY_VERSION_V1 1u
#define RIN_COMPOSITOR_DISPLAY_VERSION_V2 2u
#define RIN_COMPOSITOR_DISPLAY_GET_OUTPUT 1u
#define RIN_COMPOSITOR_DISPLAY_PRESENT 2u
#define RIN_COMPOSITOR_INPUT_VERSION 2u
#define RIN_COMPOSITOR_INPUT_VERSION_V1 1u
#define RIN_COMPOSITOR_INPUT_VERSION_V2 2u
#define RIN_COMPOSITOR_INPUT_VERSION_V3 3u
#define RIN_COMPOSITOR_INPUT_VERSION_V4 4u
#define RIN_COMPOSITOR_INPUT_POLL 1u
/* WAIT is a non-consuming, spurious-wakeable wait. V1-V3 use a 250 ms
 * scheduler timeout; V4 returns a shorter repeat deadline when one is due.
 * Callers must POLL after every successful return. */
#define RIN_COMPOSITOR_INPUT_WAIT 2u
#define RIN_COMPOSITOR_INPUT_FLAG_TEXT_INPUT 0x00000001u
#define RIN_COMPOSITOR_INPUT_FLAG_RESET_COMPOSITION 0x00000002u
#define RIN_COMPOSITOR_INPUT_FLAG_KNOWN_MASK \
    (RIN_COMPOSITOR_INPUT_FLAG_TEXT_INPUT | \
     RIN_COMPOSITOR_INPUT_FLAG_RESET_COMPOSITION)

/* The compositor owns a private BGRA surface.  The kernel only exposes the
 * output geometry and accepts a checked, read-only frame submission; it never
 * maps the physical framebuffer into an untrusted process. */
#define RIN_COMPOSITOR_PIXEL_BGRA8888 1u

typedef struct RinCompositorDisplayRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t pixel_format;
    uint32_t reserved0;
    uint64_t pixels;
    uint64_t pixels_size;
    uint64_t frame_sequence;
    uint32_t reserved[2];
} RinCompositorDisplayRequestV1;

#if defined(__cplusplus)
static_assert(sizeof(RinCompositorDisplayRequestV1) == 64u,
              "RinCompositorDisplayRequestV1 ABI drift");
#else
_Static_assert(sizeof(RinCompositorDisplayRequestV1) == 64u,
              "RinCompositorDisplayRequestV1 ABI drift");
#endif

typedef struct RinCompositorDisplayDamageV1 {
    int32_t x;
    int32_t y;
    uint32_t w;
    uint32_t h;
} RinCompositorDisplayDamageV1;

#define RIN_COMPOSITOR_DISPLAY_MAX_DAMAGE_RECTS 32u

/* V2 keeps the V1 request as an exact prefix.  A fixed damage array avoids
 * trusting a second userspace pointer while allowing the kernel to copy only
 * rows changed by the compositor. */
typedef struct RinCompositorDisplayRequestV2 {
    RinCompositorDisplayRequestV1 base;
    uint32_t damage_count;
    uint32_t reserved1;
    RinCompositorDisplayDamageV1 damage[RIN_COMPOSITOR_DISPLAY_MAX_DAMAGE_RECTS];
} RinCompositorDisplayRequestV2;

#if defined(__cplusplus)
static_assert(sizeof(RinCompositorDisplayDamageV1) == 16u,
              "RinCompositorDisplayDamageV1 ABI drift");
static_assert(sizeof(RinCompositorDisplayRequestV2) == 584u,
              "RinCompositorDisplayRequestV2 ABI drift");
#else
_Static_assert(sizeof(RinCompositorDisplayDamageV1) == 16u,
               "RinCompositorDisplayDamageV1 ABI drift");
_Static_assert(sizeof(RinCompositorDisplayRequestV2) == 584u,
               "RinCompositorDisplayRequestV2 ABI drift");
#endif

/* Kernel input is exposed only to the compositor capability domain.  The
 * compositor then routes this normalized native event to the owning client;
 * applications never get a driver queue or trusted-input evidence. */
typedef struct RinCompositorInputPollV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t flags;
    uint32_t reserved0;
    RinGuiNativeEventV1 event;
    uint32_t reserved1;
    uint64_t sequence;
} RinCompositorInputPollV1;

#if defined(__cplusplus)
static_assert(sizeof(RinCompositorInputPollV1) == 64u,
              "RinCompositorInputPollV1 ABI drift");
#else
_Static_assert(sizeof(RinCompositorInputPollV1) == 64u,
               "RinCompositorInputPollV1 ABI drift");
#endif

/* V2 keeps the native event prefix and returns the current bounded IME
 * composition alongside a composition event.  This avoids a second pointer
 * dereference from the compositor while retaining the V1 raw-key fallback. */
typedef struct RinCompositorInputPollV2 {
    RinCompositorInputPollV1 base;
    RinTextCompositionV1 composition;
} RinCompositorInputPollV2;

#if defined(__cplusplus)
static_assert(sizeof(RinCompositorInputPollV2) == 504u,
              "RinCompositorInputPollV2 ABI drift");
#else
_Static_assert(sizeof(RinCompositorInputPollV2) == 504u,
              "RinCompositorInputPollV2 ABI drift");
#endif

/* V3 preserves V2 as an exact prefix and exposes the trusted kernel input
 * evidence timestamp/source for end-to-end latency tracing. */
typedef struct RinCompositorInputPollV3 {
    RinCompositorInputPollV2 base;
    uint64_t device_timestamp_ms;
    uint32_t device_source;
    uint32_t reserved[3];
} RinCompositorInputPollV3;

#if defined(__cplusplus)
static_assert(sizeof(RinCompositorInputPollV3) == 528u,
              "RinCompositorInputPollV3 ABI drift");
#else
_Static_assert(sizeof(RinCompositorInputPollV3) == 528u,
               "RinCompositorInputPollV3 ABI drift");
#endif

/* V4 adds the compositor's current keyboard target and a bounded WAIT timeout
 * returned by the kernel. The target binds held-key repeat to focus; the
 * timeout lets WAIT wake for the next repeat without polling while idle. */
typedef struct RinCompositorInputPollV4 {
    RinCompositorInputPollV3 base;
    uint32_t focused_surface_id;
    uint32_t wait_timeout_ms;
} RinCompositorInputPollV4;

#if defined(__cplusplus)
static_assert(sizeof(RinCompositorInputPollV4) == 536u,
              "RinCompositorInputPollV4 ABI drift");
#else
_Static_assert(sizeof(RinCompositorInputPollV4) == 536u,
               "RinCompositorInputPollV4 ABI drift");
#endif

/* Surface effects are copied into compositor-owned state.  The request has
 * no caller pointers, is bound to the live surface generation, and uses a
 * surface-local clip rectangle.  An all-zero clip means the full surface. */
#define RIN_COMPOSITOR_SURFACE_EFFECT_VERSION_V1 UINT16_C(1)
#define RIN_COMPOSITOR_SURFACE_EFFECT_FLAG_ENABLED UINT16_C(0x0001)
#define RIN_COMPOSITOR_SURFACE_EFFECT_FLAG_KNOWN_MASK \
    RIN_COMPOSITOR_SURFACE_EFFECT_FLAG_ENABLED
#define RIN_COMPOSITOR_SURFACE_EFFECT_NONE UINT32_C(0)
#define RIN_COMPOSITOR_SURFACE_EFFECT_BACKDROP_BLUR UINT32_C(1)
#define RIN_COMPOSITOR_SURFACE_EFFECT_MAX_BLUR_RADIUS_PX UINT32_C(32)
#define RIN_COMPOSITOR_SURFACE_EFFECT_MAX_OPACITY UINT16_C(255)
#define RIN_COMPOSITOR_SURFACE_GENERATION_VERSION_V1 UINT16_C(1)

typedef struct RinCompositorSurfaceEffectRectV1 {
    int32_t x;
    int32_t y;
    uint32_t w;
    uint32_t h;
} RinCompositorSurfaceEffectRectV1;

typedef struct RinCompositorSurfaceEffectRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    uint32_t effect_kind;
    uint32_t blur_radius_px;
    uint32_t tint_argb;
    uint16_t opacity;
    uint16_t reserved0;
    uint32_t corner_radius_px;
    RinCompositorSurfaceEffectRectV1 clip;
    uint64_t expected_surface_generation;
    uint32_t reserved[2];
} RinCompositorSurfaceEffectRequestV1;

/* Surface generations are opaque to clients. Query after creating/attaching
 * a surface before issuing generation-bound effect requests. */
typedef struct RinCompositorSurfaceGenerationRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t surface_id;
    uint32_t reserved1;
} RinCompositorSurfaceGenerationRequestV1;

typedef struct RinCompositorSurfaceGenerationInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t surface_id;
    uint32_t reserved1;
    uint64_t generation;
} RinCompositorSurfaceGenerationInfoV1;

static inline int rin_compositor_surface_generation_request_valid(
    const RinCompositorSurfaceGenerationRequestV1* request) {
    return request &&
           request->struct_size == sizeof(*request) &&
           request->version == RIN_COMPOSITOR_SURFACE_GENERATION_VERSION_V1 &&
           request->reserved0 == 0u && request->surface_id != 0u &&
           request->reserved1 == 0u;
}

static inline int rin_compositor_surface_generation_info_valid(
    const RinCompositorSurfaceGenerationInfoV1* info,
    uint32_t expected_surface_id) {
    return info &&
           info->struct_size == sizeof(*info) &&
           info->version == RIN_COMPOSITOR_SURFACE_GENERATION_VERSION_V1 &&
           info->reserved0 == 0u &&
           info->surface_id == expected_surface_id && expected_surface_id != 0u &&
           info->reserved1 == 0u && info->generation != 0u;
}

static inline int rin_compositor_surface_effect_request_valid(
    const RinCompositorSurfaceEffectRequestV1* request,
    uint32_t surface_width, uint32_t surface_height,
    uint64_t surface_generation) {
    uint32_t clip_width;
    uint32_t clip_height;
    if (!request || request->struct_size != sizeof(*request) ||
        request->version != RIN_COMPOSITOR_SURFACE_EFFECT_VERSION_V1 ||
        (request->flags &
         (uint16_t)~RIN_COMPOSITOR_SURFACE_EFFECT_FLAG_KNOWN_MASK) != 0u ||
        request->surface_id == 0u || surface_width == 0u ||
        surface_height == 0u || surface_generation == 0u ||
        request->expected_surface_generation != surface_generation ||
        request->reserved0 != 0u || request->reserved[0] != 0u ||
        request->reserved[1] != 0u) {
        return 0;
    }
    if ((request->flags & RIN_COMPOSITOR_SURFACE_EFFECT_FLAG_ENABLED) == 0u) {
        return request->effect_kind == RIN_COMPOSITOR_SURFACE_EFFECT_NONE &&
               request->blur_radius_px == 0u && request->tint_argb == 0u &&
               request->opacity == 0u && request->corner_radius_px == 0u &&
               request->clip.x == 0 && request->clip.y == 0 &&
               request->clip.w == 0u && request->clip.h == 0u;
    }
    if (request->effect_kind != RIN_COMPOSITOR_SURFACE_EFFECT_BACKDROP_BLUR ||
        request->opacity > RIN_COMPOSITOR_SURFACE_EFFECT_MAX_OPACITY ||
        request->blur_radius_px >
            RIN_COMPOSITOR_SURFACE_EFFECT_MAX_BLUR_RADIUS_PX) {
        return 0;
    }
    if (request->clip.x == 0 && request->clip.y == 0 &&
        request->clip.w == 0u && request->clip.h == 0u) {
        clip_width = surface_width;
        clip_height = surface_height;
    } else {
        if (request->clip.x < 0 || request->clip.y < 0 ||
            request->clip.w == 0u || request->clip.h == 0u ||
            (uint32_t)request->clip.x >= surface_width ||
            (uint32_t)request->clip.y >= surface_height ||
            request->clip.w > surface_width - (uint32_t)request->clip.x ||
            request->clip.h > surface_height - (uint32_t)request->clip.y) {
            return 0;
        }
        clip_width = request->clip.w;
        clip_height = request->clip.h;
    }
    return request->corner_radius_px <=
           (clip_width < clip_height ? clip_width : clip_height) / 2u;
}

#if defined(__cplusplus)
static_assert(sizeof(RinCompositorSurfaceEffectRectV1) == 16u,
              "RinCompositorSurfaceEffectRectV1 ABI drift");
static_assert(sizeof(RinCompositorSurfaceEffectRequestV1) == 64u,
              "RinCompositorSurfaceEffectRequestV1 ABI drift");
static_assert(offsetof(RinCompositorSurfaceEffectRequestV1,
                       expected_surface_generation) == 48u,
              "RinCompositorSurfaceEffectRequestV1 generation offset drift");
static_assert(sizeof(RinCompositorSurfaceGenerationRequestV1) == 16u,
              "RinCompositorSurfaceGenerationRequestV1 ABI drift");
static_assert(sizeof(RinCompositorSurfaceGenerationInfoV1) == 24u,
              "RinCompositorSurfaceGenerationInfoV1 ABI drift");
#else
_Static_assert(sizeof(RinCompositorSurfaceEffectRectV1) == 16u,
               "RinCompositorSurfaceEffectRectV1 ABI drift");
_Static_assert(sizeof(RinCompositorSurfaceEffectRequestV1) == 64u,
               "RinCompositorSurfaceEffectRequestV1 ABI drift");
_Static_assert(offsetof(RinCompositorSurfaceEffectRequestV1,
                        expected_surface_generation) == 48u,
               "RinCompositorSurfaceEffectRequestV1 generation offset drift");
_Static_assert(sizeof(RinCompositorSurfaceGenerationRequestV1) == 16u,
               "RinCompositorSurfaceGenerationRequestV1 ABI drift");
_Static_assert(sizeof(RinCompositorSurfaceGenerationInfoV1) == 24u,
               "RinCompositorSurfaceGenerationInfoV1 ABI drift");
#endif

#endif /* RIN_SDK_COMPOSITOR_ABI_H */
