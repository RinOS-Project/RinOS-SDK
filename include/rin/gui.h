#ifndef RIN_SDK_GUI_H
#define RIN_SDK_GUI_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct RinRectV1 { int32_t x; int32_t y; uint32_t width; uint32_t height; } RinRectV1;
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
RIN_SDK_API RinResult rin_dialog_show_v1(RinStringV1 title, RinStringV1 message, uint32_t flags, uint32_t* result);
RIN_SDK_API RinResult rin_notification_show_v1(RinStringV1 title, RinStringV1 message, uint32_t flags);

#ifdef __cplusplus
}
#endif
#endif
