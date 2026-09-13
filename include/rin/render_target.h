/* SPDX-License-Identifier: MIT */
/* Stable userspace render-target descriptor shared by public 2D libraries. */

#ifndef RIN_SDK_RENDER_TARGET_H
#define RIN_SDK_RENDER_TARGET_H

#include <stdint.h>

#define RIN_RENDER_TARGET_VERSION 1u

enum RinRenderTargetFormat {
    RIN_RENDER_TARGET_FORMAT_BGRA32 = 0,
    RIN_RENDER_TARGET_FORMAT_RGBA32 = 1,
};

#define RIN_RENDER_TARGET_FLAG_WRITABLE UINT32_C(0x00000001)
#define RIN_RENDER_TARGET_FLAG_EXTERNAL UINT32_C(0x00000002)

typedef struct RinRenderTarget {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    void* pixels;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t format;
    uint64_t generation;
    uint32_t buffer_slot;
    uint32_t flags;
} RinRenderTarget;

#endif /* RIN_SDK_RENDER_TARGET_H */
