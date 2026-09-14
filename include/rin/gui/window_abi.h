/* SPDX-License-Identifier: MIT */
/* Public window identity, role, flag and geometry ABI. */
#ifndef RIN_SDK_GUI_WINDOW_ABI_H
#define RIN_SDK_GUI_WINDOW_ABI_H

#include <stdint.h>
#include "../abi.h"
#include "compositor_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_WINDOW_ABI_VERSION 1u
#define RIN_WINDOW_HANDLE_INVALID RIN_HANDLE_INVALID

/* RinWindowHandle is the canonical application-facing handle type.  It is
 * deliberately identical to RinHandle so C and C++ clients cannot drift into
 * architecture-specific duplicate typedefs. */
typedef RinHandle RinWindowHandle;

#define RIN_WINDOW_ROLE_NORMAL       RIN_COMPOSITOR_ROLE_NORMAL
#define RIN_WINDOW_ROLE_DESKTOP      RIN_COMPOSITOR_ROLE_DESKTOP
#define RIN_WINDOW_ROLE_PANEL        RIN_COMPOSITOR_ROLE_PANEL
#define RIN_WINDOW_ROLE_POPUP        RIN_COMPOSITOR_ROLE_POPUP
#define RIN_WINDOW_ROLE_MENU         RIN_COMPOSITOR_ROLE_MENU
#define RIN_WINDOW_ROLE_DIALOG       RIN_COMPOSITOR_ROLE_DIALOG
#define RIN_WINDOW_ROLE_TOOLTIP      RIN_COMPOSITOR_ROLE_TOOLTIP
#define RIN_WINDOW_ROLE_NOTIFICATION RIN_COMPOSITOR_ROLE_NOTIFICATION
#define RIN_WINDOW_ROLE_CURSOR       RIN_COMPOSITOR_ROLE_CURSOR

#define RIN_WINDOW_FLAG_CURSOR  RIN_COMPOSITOR_SURFACE_FLAG_CURSOR
#define RIN_WINDOW_FLAG_OVERLAY RIN_COMPOSITOR_SURFACE_FLAG_OVERLAY
#define RIN_WINDOW_FLAG_OPAQUE RIN_COMPOSITOR_SURFACE_FLAG_OPAQUE
#define RIN_WINDOW_FLAG_KNOWN_MASK \
    (RIN_WINDOW_FLAG_CURSOR | RIN_WINDOW_FLAG_OVERLAY | RIN_WINDOW_FLAG_OPAQUE)

#define RIN_WINDOW_STATE_MINIMIZED  RIN_COMPOSITOR_WINDOW_STATE_MINIMIZED
#define RIN_WINDOW_STATE_MAXIMIZED  RIN_COMPOSITOR_WINDOW_STATE_MAXIMIZED
#define RIN_WINDOW_STATE_FULLSCREEN RIN_COMPOSITOR_WINDOW_STATE_FULLSCREEN
#define RIN_WINDOW_STATE_ALWAYS_TOP RIN_COMPOSITOR_WINDOW_STATE_ALWAYS_TOP
#define RIN_WINDOW_STATE_KNOWN_MASK RIN_COMPOSITOR_WINDOW_STATE_KNOWN_MASK

typedef struct RinWindowGeometryV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t scale_numerator;
    uint32_t scale_denominator;
    uint32_t reserved[2];
} RinWindowGeometryV1;

/* Window handles are borrowed capabilities owned by the creating process.
 * Closing a window invalidates the handle; clients must not retain it for a
 * later window or infer authority from its numeric value.  Geometry is a
 * snapshot and is only valid for the call that returned it. */
static inline int rin_window_geometry_valid(const RinWindowGeometryV1* value) {
    return value != 0 && value->struct_size == sizeof(*value) &&
           value->version == RIN_WINDOW_ABI_VERSION && value->flags == 0u &&
           value->width != 0u && value->height != 0u &&
           value->scale_numerator != 0u && value->scale_denominator != 0u &&
           value->reserved[0] == 0u && value->reserved[1] == 0u;
}

#ifdef __cplusplus
static_assert(sizeof(RinWindowGeometryV1) == 40u,
              "RinWindowGeometryV1 ABI drift");
#else
_Static_assert(sizeof(RinWindowGeometryV1) == 40u,
               "RinWindowGeometryV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_GUI_WINDOW_ABI_H */
