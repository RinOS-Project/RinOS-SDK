/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_COMPOSITOR_ABI_H
#define RIN_SDK_COMPOSITOR_ABI_H

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
#define RIN_COMPOSITOR_INPUT_POLL 1u
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

#endif /* RIN_SDK_COMPOSITOR_ABI_H */
