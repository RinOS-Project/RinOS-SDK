/* SPDX-License-Identifier: MIT */
#ifndef RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_H
#define RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_H

#include <stddef.h>
#include <stdint.h>

/* Versioned, API-neutral process graphics binding contract. Numeric values
 * for image formats, usages, layouts, and resource kinds intentionally match
 * the corresponding stable RinGPU/RinShader V1 enums. Adapters must convert
 * API-native values explicitly; no Vulkan type or enum is part of this ABI. */
#define RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_VERSION UINT32_C(2)
#define RIN_GPU_PROCESS_IMAGE_VIEW_V1_VERSION UINT32_C(1)
#define RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_MAX_COUNT UINT32_C(64)

#define RIN_GPU_PROCESS_GRAPHICS_ACCESS_READ UINT32_C(0x00000001)
#define RIN_GPU_PROCESS_GRAPHICS_ACCESS_WRITE UINT32_C(0x00000002)
#define RIN_GPU_PROCESS_GRAPHICS_ACCESS_KNOWN \
    (RIN_GPU_PROCESS_GRAPHICS_ACCESS_READ | \
     RIN_GPU_PROCESS_GRAPHICS_ACCESS_WRITE)

#define RIN_GPU_PROCESS_GRAPHICS_BINDING_SAMPLED_MIP_CHAIN UINT32_C(0x1)
#define RIN_GPU_PROCESS_GRAPHICS_BINDING_KNOWN_FLAGS \
    RIN_GPU_PROCESS_GRAPHICS_BINDING_SAMPLED_MIP_CHAIN

#define RIN_GPU_PROCESS_IMAGE_ASPECT_COLOR UINT32_C(0x1)
#define RIN_GPU_PROCESS_IMAGE_ASPECT_DEPTH UINT32_C(0x2)
#define RIN_GPU_PROCESS_IMAGE_ASPECT_STENCIL UINT32_C(0x4)

typedef enum RinGpuProcessGraphicsResourceKindV2 {
    RIN_GPU_PROCESS_GRAPHICS_RESOURCE_STORAGE_BUFFER_V2 = 1u,
    RIN_GPU_PROCESS_GRAPHICS_RESOURCE_SAMPLED_IMAGE_V2 = 2u,
    RIN_GPU_PROCESS_GRAPHICS_RESOURCE_SAMPLER_V2 = 3u,
    RIN_GPU_PROCESS_GRAPHICS_RESOURCE_SAMPLED_DEPTH_IMAGE_V2 = 4u,
    RIN_GPU_PROCESS_GRAPHICS_RESOURCE_COMPARISON_SAMPLER_V2 = 5u,
    RIN_GPU_PROCESS_GRAPHICS_RESOURCE_STORAGE_IMAGE_V2 = 6u
} RinGpuProcessGraphicsResourceKindV2;

typedef enum RinGpuProcessImageLayoutV1 {
    RIN_GPU_PROCESS_IMAGE_LAYOUT_UNDEFINED_V1 = 0u,
    RIN_GPU_PROCESS_IMAGE_LAYOUT_COPY_SOURCE_V1 = 1u,
    RIN_GPU_PROCESS_IMAGE_LAYOUT_COPY_DESTINATION_V1 = 2u,
    RIN_GPU_PROCESS_IMAGE_LAYOUT_COLOR_TARGET_V1 = 3u,
    RIN_GPU_PROCESS_IMAGE_LAYOUT_PRESENT_V1 = 4u,
    RIN_GPU_PROCESS_IMAGE_LAYOUT_DEPTH_TARGET_V1 = 5u,
    RIN_GPU_PROCESS_IMAGE_LAYOUT_SHADER_READ_V1 = 6u,
    RIN_GPU_PROCESS_IMAGE_LAYOUT_GENERAL_V1 = 7u
} RinGpuProcessImageLayoutV1;

typedef enum RinGpuProcessImageDimensionV1 {
    RIN_GPU_PROCESS_IMAGE_DIMENSION_1D_V1 = 1u,
    RIN_GPU_PROCESS_IMAGE_DIMENSION_2D_V1 = 2u,
    RIN_GPU_PROCESS_IMAGE_DIMENSION_3D_V1 = 3u
} RinGpuProcessImageDimensionV1;

/* Stable values equal the matching RinGPU image-format values. */
typedef enum RinGpuProcessImageFormatV1 {
    RIN_GPU_PROCESS_IMAGE_FORMAT_R8_UNORM_V1 = 1u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA8_UNORM_V1 = 2u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_BGRA8_UNORM_V1 = 3u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_D32_FLOAT_V1 = 4u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_D32_FLOAT_S8_UINT_V1 = 5u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_RGB565_UNORM_V1 = 6u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA4_UNORM_V1 = 7u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_RGB5_A1_UNORM_V1 = 8u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_S8_UINT_V1 = 9u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA32_FLOAT_V1 = 10u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA16_FLOAT_V1 = 11u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA8_SRGB_V1 = 12u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_BGRA8_SRGB_V1 = 13u,
    RIN_GPU_PROCESS_IMAGE_FORMAT_BC1_RGBA_UNORM_V1 = 14u
} RinGpuProcessImageFormatV1;

#define RIN_GPU_PROCESS_IMAGE_USAGE_COPY_SOURCE_V1 UINT32_C(0x01)
#define RIN_GPU_PROCESS_IMAGE_USAGE_COPY_DESTINATION_V1 UINT32_C(0x02)
#define RIN_GPU_PROCESS_IMAGE_USAGE_SAMPLED_V1 UINT32_C(0x04)
#define RIN_GPU_PROCESS_IMAGE_USAGE_STORAGE_V1 UINT32_C(0x08)
#define RIN_GPU_PROCESS_IMAGE_USAGE_COLOR_TARGET_V1 UINT32_C(0x10)
#define RIN_GPU_PROCESS_IMAGE_USAGE_DEPTH_STENCIL_V1 UINT32_C(0x20)
#define RIN_GPU_PROCESS_IMAGE_USAGE_PRESENT_V1 UINT32_C(0x40)
#define RIN_GPU_PROCESS_IMAGE_USAGE_KNOWN_V1 UINT32_C(0x7f)

/* `logical_layout` is an API-independent access state, not a storage layout,
 * tiling mode, modifier, row pitch, or device address. One view range must be
 * in one logical state. Physical import must obtain its storage description
 * from the image owner/provider and may not infer it from allocation size. */
typedef struct RinGpuProcessImageViewV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t image_token;
    uint64_t resource_cookie;
    uint64_t allocation_offset;
    uint64_t allocation_size_bytes;
    uint32_t format;
    uint32_t dimension;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mip_levels;
    uint32_t array_layers;
    uint32_t sample_count;
    uint32_t usage;
    uint32_t logical_layout;
    uint32_t aspect_mask;
    uint32_t base_mip_level;
    uint32_t level_count;
    uint32_t base_array_layer;
    uint32_t layer_count;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinGpuProcessImageViewV1;

/* Buffer entries use resource_cookie/offset/size_bytes. Image entries use
 * image_view and its process-owned image/resource tokens. Sampler entries use
 * sampler_token. Sampled image and sampler remain separate shader resources;
 * this record does not invent a combined-image-sampler binding. */
typedef struct RinGpuProcessGraphicsBindingV2 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t kind;
    uint32_t binding;
    uint32_t access;
    uint32_t flags;
    uint64_t resource_cookie;
    uint64_t offset;
    uint64_t size_bytes;
    uint64_t sampler_token;
    RinGpuProcessImageViewV1 image_view;
    uint64_t reserved[2];
} RinGpuProcessGraphicsBindingV2;

static inline int rin_gpu_process_image_format_aspects_v1(uint32_t format,
                                                           uint32_t* aspects) {
    if (!aspects) return 0;
    switch (format) {
    case RIN_GPU_PROCESS_IMAGE_FORMAT_R8_UNORM_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA8_UNORM_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_BGRA8_UNORM_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_RGB565_UNORM_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA4_UNORM_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_RGB5_A1_UNORM_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA32_FLOAT_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA16_FLOAT_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_RGBA8_SRGB_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_BGRA8_SRGB_V1:
    case RIN_GPU_PROCESS_IMAGE_FORMAT_BC1_RGBA_UNORM_V1:
        *aspects = RIN_GPU_PROCESS_IMAGE_ASPECT_COLOR;
        return 1;
    case RIN_GPU_PROCESS_IMAGE_FORMAT_D32_FLOAT_V1:
        *aspects = RIN_GPU_PROCESS_IMAGE_ASPECT_DEPTH;
        return 1;
    case RIN_GPU_PROCESS_IMAGE_FORMAT_D32_FLOAT_S8_UINT_V1:
        *aspects = RIN_GPU_PROCESS_IMAGE_ASPECT_DEPTH |
                   RIN_GPU_PROCESS_IMAGE_ASPECT_STENCIL;
        return 1;
    case RIN_GPU_PROCESS_IMAGE_FORMAT_S8_UINT_V1:
        *aspects = RIN_GPU_PROCESS_IMAGE_ASPECT_STENCIL;
        return 1;
    default:
        return 0;
    }
}

static inline int rin_gpu_process_image_view_v1_valid(
    const RinGpuProcessImageViewV1* view) {
    uint32_t format_aspects;
    uint32_t max_dimension;
    uint32_t max_mips = 1u;
    if (!view || view->struct_size != sizeof(*view) ||
        view->version != RIN_GPU_PROCESS_IMAGE_VIEW_V1_VERSION ||
        view->image_token == 0u || view->resource_cookie == 0u ||
        view->allocation_size_bytes == 0u ||
        view->allocation_offset > UINT64_MAX - view->allocation_size_bytes ||
        view->width == 0u || view->height == 0u || view->depth == 0u ||
        (view->dimension != RIN_GPU_PROCESS_IMAGE_DIMENSION_1D_V1 &&
         view->dimension != RIN_GPU_PROCESS_IMAGE_DIMENSION_2D_V1 &&
         view->dimension != RIN_GPU_PROCESS_IMAGE_DIMENSION_3D_V1) ||
        view->mip_levels == 0u || view->array_layers == 0u ||
        view->sample_count != 1u || view->usage == 0u ||
        (view->usage & ~RIN_GPU_PROCESS_IMAGE_USAGE_KNOWN_V1) != 0u ||
        view->aspect_mask == 0u ||
        (view->aspect_mask & ~UINT32_C(0x7)) != 0u ||
        view->base_mip_level >= view->mip_levels || view->level_count == 0u ||
        view->level_count > view->mip_levels - view->base_mip_level ||
        view->base_array_layer >= view->array_layers ||
        view->layer_count == 0u ||
        view->layer_count > view->array_layers - view->base_array_layer ||
        view->logical_layout > RIN_GPU_PROCESS_IMAGE_LAYOUT_GENERAL_V1 ||
        view->reserved0 != 0u ||
        view->reserved[0] != 0u || view->reserved[1] != 0u ||
        !rin_gpu_process_image_format_aspects_v1(view->format,
                                                 &format_aspects) ||
        (view->aspect_mask & format_aspects) != view->aspect_mask)
        return 0;
    if (view->dimension == RIN_GPU_PROCESS_IMAGE_DIMENSION_1D_V1 &&
        (view->height != 1u || view->depth != 1u))
        return 0;
    if (view->dimension == RIN_GPU_PROCESS_IMAGE_DIMENSION_2D_V1 &&
        view->depth != 1u)
        return 0;
    max_dimension = view->width;
    if (view->height > max_dimension) max_dimension = view->height;
    if (view->depth > max_dimension) max_dimension = view->depth;
    while (max_dimension > 1u) {
        max_dimension >>= 1u;
        ++max_mips;
    }
    return view->mip_levels <= max_mips;
}

static inline int rin_gpu_process_image_view_v1_zero(
    const RinGpuProcessImageViewV1* view) {
    return view && view->struct_size == 0u && view->version == 0u &&
           view->image_token == 0u && view->resource_cookie == 0u &&
           view->allocation_offset == 0u &&
           view->allocation_size_bytes == 0u && view->format == 0u &&
           view->dimension == 0u && view->width == 0u && view->height == 0u &&
           view->depth == 0u && view->mip_levels == 0u &&
           view->array_layers == 0u && view->sample_count == 0u &&
           view->usage == 0u && view->logical_layout == 0u &&
           view->aspect_mask == 0u && view->base_mip_level == 0u &&
           view->level_count == 0u && view->base_array_layer == 0u &&
           view->layer_count == 0u && view->reserved0 == 0u &&
           view->reserved[0] == 0u && view->reserved[1] == 0u;
}

static inline int rin_gpu_process_graphics_binding_v2_valid(
    const RinGpuProcessGraphicsBindingV2* binding) {
    const RinGpuProcessImageViewV1* image;
    if (!binding || binding->struct_size != sizeof(*binding) ||
        binding->version != RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_VERSION ||
        binding->binding >= RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_MAX_COUNT ||
        binding->reserved[0] != 0u || binding->reserved[1] != 0u)
        return 0;
    image = &binding->image_view;
    if (binding->kind == RIN_GPU_PROCESS_GRAPHICS_RESOURCE_STORAGE_BUFFER_V2)
        return binding->resource_cookie != 0u &&
               binding->sampler_token == 0u && binding->size_bytes != 0u &&
               binding->offset <= UINT64_MAX - binding->size_bytes &&
               (binding->offset & 3u) == 0u &&
               (binding->size_bytes & 3u) == 0u && binding->access != 0u &&
               (binding->access & ~RIN_GPU_PROCESS_GRAPHICS_ACCESS_KNOWN) ==
                   0u && binding->flags == 0u &&
               rin_gpu_process_image_view_v1_zero(image);
    if (binding->kind == RIN_GPU_PROCESS_GRAPHICS_RESOURCE_SAMPLER_V2 ||
        binding->kind ==
            RIN_GPU_PROCESS_GRAPHICS_RESOURCE_COMPARISON_SAMPLER_V2)
        return binding->sampler_token != 0u && binding->resource_cookie == 0u &&
               binding->offset == 0u && binding->size_bytes == 0u &&
               binding->access == RIN_GPU_PROCESS_GRAPHICS_ACCESS_READ &&
               binding->flags == 0u &&
               rin_gpu_process_image_view_v1_zero(image);
    if (binding->kind != RIN_GPU_PROCESS_GRAPHICS_RESOURCE_SAMPLED_IMAGE_V2 &&
        binding->kind !=
            RIN_GPU_PROCESS_GRAPHICS_RESOURCE_SAMPLED_DEPTH_IMAGE_V2 &&
        binding->kind != RIN_GPU_PROCESS_GRAPHICS_RESOURCE_STORAGE_IMAGE_V2)
        return 0;
    if (binding->resource_cookie != 0u || binding->offset != 0u ||
        binding->size_bytes != 0u || binding->sampler_token != 0u ||
        !rin_gpu_process_image_view_v1_valid(image))
        return 0;
    if (binding->kind == RIN_GPU_PROCESS_GRAPHICS_RESOURCE_STORAGE_IMAGE_V2)
        return (image->usage & RIN_GPU_PROCESS_IMAGE_USAGE_STORAGE_V1) != 0u &&
               image->logical_layout ==
                   RIN_GPU_PROCESS_IMAGE_LAYOUT_GENERAL_V1 &&
               binding->access != 0u &&
               (binding->access & ~RIN_GPU_PROCESS_GRAPHICS_ACCESS_KNOWN) ==
                   0u && binding->flags == 0u;
    if (binding->access != RIN_GPU_PROCESS_GRAPHICS_ACCESS_READ ||
        (image->usage & RIN_GPU_PROCESS_IMAGE_USAGE_SAMPLED_V1) == 0u ||
        (image->logical_layout != RIN_GPU_PROCESS_IMAGE_LAYOUT_SHADER_READ_V1 &&
         image->logical_layout != RIN_GPU_PROCESS_IMAGE_LAYOUT_GENERAL_V1) ||
        (binding->flags & ~RIN_GPU_PROCESS_GRAPHICS_BINDING_KNOWN_FLAGS) != 0u)
        return 0;
    if ((binding->flags &
         RIN_GPU_PROCESS_GRAPHICS_BINDING_SAMPLED_MIP_CHAIN) != 0u &&
        image->level_count != image->mip_levels - image->base_mip_level)
        return 0;
    if (binding->kind ==
        RIN_GPU_PROCESS_GRAPHICS_RESOURCE_SAMPLED_DEPTH_IMAGE_V2)
        return (image->aspect_mask & RIN_GPU_PROCESS_IMAGE_ASPECT_DEPTH) != 0u &&
               (binding->flags &
                RIN_GPU_PROCESS_GRAPHICS_BINDING_SAMPLED_MIP_CHAIN) == 0u;
    return (image->aspect_mask & RIN_GPU_PROCESS_IMAGE_ASPECT_COLOR) != 0u;
}

#if defined(__cplusplus)
static_assert(sizeof(RinGpuProcessImageViewV1) == 120u,
              "RinGpuProcessImageViewV1 ABI drift");
static_assert(offsetof(RinGpuProcessImageViewV1, reserved0) == 100u,
              "RinGpuProcessImageViewV1 reserved offset drift");
static_assert(offsetof(RinGpuProcessImageViewV1, reserved) == 104u,
              "RinGpuProcessImageViewV1 reserved-array offset drift");
static_assert(sizeof(RinGpuProcessGraphicsBindingV2) == 192u,
              "RinGpuProcessGraphicsBindingV2 ABI drift");
static_assert(offsetof(RinGpuProcessGraphicsBindingV2, image_view) == 56u,
              "RinGpuProcessGraphicsBindingV2 image offset drift");
#else
_Static_assert(sizeof(RinGpuProcessImageViewV1) == 120u,
               "RinGpuProcessImageViewV1 ABI drift");
_Static_assert(offsetof(RinGpuProcessImageViewV1, reserved0) == 100u,
               "RinGpuProcessImageViewV1 reserved offset drift");
_Static_assert(offsetof(RinGpuProcessImageViewV1, reserved) == 104u,
               "RinGpuProcessImageViewV1 reserved-array offset drift");
_Static_assert(sizeof(RinGpuProcessGraphicsBindingV2) == 192u,
               "RinGpuProcessGraphicsBindingV2 ABI drift");
_Static_assert(offsetof(RinGpuProcessGraphicsBindingV2, image_view) == 56u,
               "RinGpuProcessGraphicsBindingV2 image offset drift");
#endif

#endif /* RIN_GPU_PROCESS_GRAPHICS_BINDING_V2_H */
