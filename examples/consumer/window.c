/* SPDX-License-Identifier: MIT */
#include <rin/base.h>
#include <rin/gui.h>
#include <rin/gui/surface_abi.h>

int main(void)
{
    static const char title[] = "RinOS SDK window example";
    RinWindowCreateV1 request = {0};
    RinGuiEventV1 event = {0};
    RinWindow window = RIN_HANDLE_INVALID;
    RinResult result;

    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.title.address = (uint64_t)(uintptr_t)title;
    request.title.size = sizeof(title) - 1u;
    request.bounds = (RinRectV1){80, 80, 640u, 360u};
    request.pixel_format = RIN_SURFACE_FORMAT_BGRA32;
    result = rin_window_create_v1(&request, &window);
    if (result != RIN_SUCCESS) return 1;

    event.struct_size = sizeof(event);
    event.version = RIN_SDK_STRUCT_VERSION_1;
    result = rin_window_next_event_v1(window, RIN_SDK_INFINITE, &event);
    if (rin_object_close_v1((RinObject)window) != RIN_SUCCESS &&
        result == RIN_SUCCESS)
        result = RIN_ERROR_IO;
    return result == RIN_SUCCESS ? 0 : 1;
}
