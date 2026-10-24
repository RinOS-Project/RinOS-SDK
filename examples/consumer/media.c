/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include <stdio.h>

#include <rin/base.h>
#include <rin/media.h>

/* One complete 1x1 RGBA PNG keeps the sample self-contained and exercises
 * the public byte-slice contract without requesting filesystem authority. */
static const uint8_t sample_png[] = {
    0x89u, 0x50u, 0x4eu, 0x47u, 0x0du, 0x0au, 0x1au, 0x0au,
    0x00u, 0x00u, 0x00u, 0x0du, 0x49u, 0x48u, 0x44u, 0x52u,
    0x00u, 0x00u, 0x00u, 0x01u, 0x00u, 0x00u, 0x00u, 0x01u,
    0x08u, 0x06u, 0x00u, 0x00u, 0x00u, 0x1fu, 0x15u, 0xc4u,
    0x89u, 0x00u, 0x00u, 0x00u, 0x0du, 0x49u, 0x44u, 0x41u,
    0x54u, 0x78u, 0x9cu, 0x63u, 0xf8u, 0xcfu, 0xc0u, 0xf0u,
    0x1fu, 0x00u, 0x05u, 0x00u, 0x01u, 0xffu, 0x89u, 0x99u,
    0x3du, 0x1du, 0x00u, 0x00u, 0x00u, 0x00u, 0x49u, 0x45u,
    0x4eu, 0x44u, 0xaeu, 0x42u, 0x60u, 0x82u,
};

int main(void)
{
    static const char mime_type[] = "image/png";
    RinMediaOpenV1 request = {0};
    RinMediaObject object = RIN_HANDLE_INVALID;
    RinResult result;

    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.mime_type.address = (uint64_t)(uintptr_t)mime_type;
    request.mime_type.size = sizeof(mime_type) - 1u;
    request.source.address = (uint64_t)(uintptr_t)sample_png;
    request.source.size = sizeof(sample_png);
    request.media_type = RIN_MEDIA_TYPE_IMAGE;

    result = rin_media_open_v1(&request, &object);
    if (result == RIN_ERROR_NOT_SUPPORTED) {
        puts("image media pipeline is not available on this target");
        return 3;
    }
    if (result != RIN_SUCCESS || object == RIN_HANDLE_INVALID) {
        printf("media open failed: %d\n", (int)result);
        return 1;
    }

    result = rin_object_close_v1((RinObject)object);
    if (result != RIN_SUCCESS) {
        printf("media object close failed: %d\n", (int)result);
        return 1;
    }
    puts("typed PNG media object opened and closed");
    return 0;
}
