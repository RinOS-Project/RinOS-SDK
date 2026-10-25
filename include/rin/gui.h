#ifndef RIN_SDK_GUI_H
#define RIN_SDK_GUI_H
#include "abi.h"
#include "contract_abi.h"
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
 * flags and dialog flags are reserved and must be zero. Present damage uses
 * at most RIN_GUI_DAMAGE_MAX_RECTS positive, nonnegative local rectangles; an
 * empty list requests full damage. A surface uses one packed 32-bit pixel per
 * pixel and is bounded by RIN_GUI_SURFACE_MAX_BYTES. */
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

RIN_SDK_API RinResult rin_window_create_v1(const RinWindowCreateV1* request, RinWindow* window);
RIN_SDK_API RinResult rin_window_next_event_v1(RinWindow window, uint64_t timeout_ns, RinGuiEventV1* event);
RIN_SDK_API RinResult rin_surface_create_v1(uint32_t width, uint32_t height, uint32_t format, RinSurface* surface);
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
