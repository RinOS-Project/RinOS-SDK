#ifndef RIN_SDK_GUI_H
#define RIN_SDK_GUI_H
#include "abi.h"
#include "contract_abi.h"
#include "gui/native_event.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct RinRectV1 { int32_t x; int32_t y; uint32_t width; uint32_t height; } RinRectV1;
enum RinGuiPixelFormatV1 {
    RIN_GUI_PIXEL_FORMAT_BGRA32 = 0u,
    RIN_GUI_PIXEL_FORMAT_RGBA32 = 1u
};
#define RIN_GUI_WINDOW_TITLE_MAX_BYTES 127u
#define RIN_GUI_DIALOG_TEXT_MAX_BYTES 4096u
#define RIN_GUI_SURFACE_MAX_BYTES (64u * 1024u * 1024u)
#define RIN_GUI_DAMAGE_MAX_RECTS 32u

/* Version 1 accepts strict UTF-8 text without embedded NUL. Window-create
 * flags and dialog flags are reserved and must be zero. The window pixel
 * format is validated here; each caller-owned surface selects its actual
 * format when created. Present damage uses at most
 * RIN_GUI_DAMAGE_MAX_RECTS positive local rectangles wholly inside the
 * surface; an empty list requests full damage. Runtime presents require the
 * surface extent to match the live window target and convert its format to
 * the Compositor target as needed. RinRuntime owns window, surface, and dialog
 * calls in RinRuntime.rll; clipboard and notification remain in the SDK GUI
 * library. */
typedef struct RinWindowCreateV1 {
    uint32_t struct_size;
    uint32_t version;
    RinStringV1 title;
    RinRectV1 bounds;
    uint32_t flags;
    uint32_t pixel_format;
    uint64_t reserved[4];
} RinWindowCreateV1;
typedef struct RinGuiEventV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t type;
    uint32_t flags;
    uint64_t timestamp_ns;
    RinWindow window;
    int64_t data[4];
    uint64_t reserved[2];
} RinGuiEventV1;

/* RinGuiEventV1 preserves RinGuiNativeEventV1.type as `type` and its flags.
 * data[0] is native.data; data[1] packs signed screen_x/screen_y as two
 * uint32 bit patterns; data[2] packs key_modifiers in the high word and
 * pointer_buttons in the low word; data[3] is wheel_delta. */

/* A single-process mapping lease for an app-owned packed 32-bit surface.
 * `address` is a process-local virtual address, never a physical address.
 * Do not access it after unmap or destroy.  Present is rejected while mapped,
 * and unmap must return the exact descriptor produced by map. */
typedef struct RinGuiSurfaceMapV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t address;
    uint32_t width;
    uint32_t height;
    uint32_t stride_bytes;
    uint32_t format;
    uint64_t generation;
    uint64_t reserved[2];
} RinGuiSurfaceMapV1;

#if defined(__cplusplus)
static_assert(sizeof(RinGuiSurfaceMapV1) == 56u,
              "RinGuiSurfaceMapV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGuiSurfaceMapV1) == 56u,
               "RinGuiSurfaceMapV1 ABI drift");
#endif

RIN_SDK_API RinResult rin_window_create_v1(const RinWindowCreateV1* request, RinWindow* window);
/* A successful close invalidates the Runtime-owned window handle. */
RIN_SDK_API RinResult rin_window_close_v1(RinWindow window);
RIN_SDK_API RinResult rin_window_next_event_v1(RinWindow window, uint64_t timeout_ns, RinGuiEventV1* event);
RIN_SDK_API RinResult rin_surface_create_v1(uint32_t width, uint32_t height, uint32_t format, RinSurface* surface);
/* At most one mapping is active for a surface. Present/destroy return BUSY
 * until the exact mapping descriptor has been unmapped. */
RIN_SDK_API RinResult rin_surface_map_v1(RinSurface surface, RinGuiSurfaceMapV1* mapping);
RIN_SDK_API RinResult rin_surface_unmap_v1(RinSurface surface, const RinGuiSurfaceMapV1* mapping);
RIN_SDK_API RinResult rin_surface_destroy_v1(RinSurface surface);
RIN_SDK_API RinResult rin_window_present_v1(RinWindow window, RinSurface surface, RinSliceV1 damage_rects);
RIN_SDK_API RinResult rin_clipboard_set_v1(uint32_t format, RinSliceV1 data);
RIN_SDK_API RinResult rin_clipboard_get_v1(uint32_t format, RinSliceV1 data, uint64_t* required);
/* result is required; version 1 currently defines only flags == 0. */
RIN_SDK_API RinResult rin_dialog_show_v1(RinStringV1 title, RinStringV1 message, uint32_t flags, uint32_t* result);
/* flags selects RIN_NOTIFICATION_KIND_INFO/SUCCESS/WARNING/ERROR. The SDK
 * submits the bounded request to the authenticated notification syscall. */
RIN_SDK_API RinResult rin_notification_show_v1(RinStringV1 title, RinStringV1 message, uint32_t flags);

#ifdef __cplusplus
}
#endif
#endif
