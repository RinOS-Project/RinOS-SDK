/* SPDX-License-Identifier: MIT */

#include <rin/driver/legacy_image_v1.h>

#include <stddef.h>

#define RIN_SIGNATURE_HEADER_SIZE_LOCAL UINT32_C(48)
#define RIN_SIGNATURE_MIN_RSA_SIZE_LOCAL UINT32_C(256)
#define RIN_SIGNATURE_MAX_RSA_SIZE_LOCAL UINT32_C(512)

static void image_zero(void* destination, uint32_t size)
{
    uint8_t* bytes = (uint8_t*)destination;
    if (!destination) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static int range_valid(uint32_t offset, uint32_t size, uint32_t limit)
{
    return offset <= limit && size <= limit - offset;
}

static int runtime_range_valid(uint32_t offset, uint32_t size,
                               uint32_t image_size)
{
    return size != 0u && range_valid(offset, size, image_size);
}

static int range_inside_section(uint32_t offset, uint32_t size,
                                uint32_t section_offset,
                                uint32_t section_size)
{
    uint32_t relative;
    if (offset < section_offset) return 0;
    relative = offset - section_offset;
    return relative <= section_size && size <= section_size - relative;
}

static int bytes_zero(const uint8_t* bytes, uint32_t size)
{
    uint32_t index;
    for (index = 0u; index < size; ++index) {
        if (bytes[index] != 0u) return 0;
    }
    return 1;
}

static int name_canonical(const char name[16])
{
    uint32_t index;
    uint32_t terminator = 16u;
    for (index = 0u; index < 16u; ++index) {
        uint8_t value = (uint8_t)name[index];
        if (value == 0u) {
            terminator = index;
            break;
        }
        if (value < 0x21u || value > 0x7eu || value == '/' || value == '\\')
            return 0;
    }
    if (terminator == 0u || terminator == 16u) return 0;
    return bytes_zero((const uint8_t*)name + terminator,
                      16u - terminator);
}

RinLegacyDriverV1ImageStatus rin_legacy_driver_v1_image_validate(
    const void* image, uint32_t image_size,
    RinLegacyDriverV1ImageView* view)
{
    const uint8_t* bytes = (const uint8_t*)image;
    const RinLegacyDriverV1Header* header;
    uint32_t code_runtime_offset;
    uint32_t rodata_runtime_offset;
    uint32_t data_runtime_offset;
    uint32_t relocation_bytes;
    uint32_t index;

    if (view) image_zero(view, (uint32_t)sizeof(*view));
    if (!image || !view || image_size < RIN_LEGACY_DRIVER_V1_HEADER_SIZE)
        return RIN_LEGACY_DRIVER_V1_IMAGE_INVALID_ARGUMENT;

    header = (const RinLegacyDriverV1Header*)image;
    if (header->magic != RIN_LEGACY_DRIVER_V1_MAGIC ||
        header->version != RIN_LEGACY_DRIVER_V1_VERSION ||
        header->header_size != RIN_LEGACY_DRIVER_V1_HEADER_SIZE ||
        header->flags != RIN_LEGACY_DRIVER_V1_FLAG_SIGNED ||
        header->prefix_reserved != 0u || header->reserved != 0u ||
        !name_canonical(header->name)) {
        return RIN_LEGACY_DRIVER_V1_IMAGE_BAD_HEADER;
    }
    if (header->image_size == 0u ||
        header->image_size > RIN_LEGACY_DRIVER_V1_MAX_IMAGE_SIZE ||
        header->code_offset != header->header_size ||
        header->rodata_offset < header->header_size ||
        header->data_offset < header->header_size) {
        return RIN_LEGACY_DRIVER_V1_IMAGE_BAD_LAYOUT;
    }
    code_runtime_offset = header->code_offset - header->header_size;
    rodata_runtime_offset = header->rodata_offset - header->header_size;
    data_runtime_offset = header->data_offset - header->header_size;
    if (code_runtime_offset != 0u || header->code_size == 0u ||
        (rodata_runtime_offset &
         (RIN_LEGACY_DRIVER_V1_PAGE_SIZE - 1u)) != 0u ||
        (data_runtime_offset &
         (RIN_LEGACY_DRIVER_V1_PAGE_SIZE - 1u)) != 0u ||
        !runtime_range_valid(code_runtime_offset, header->code_size,
                             header->image_size) ||
        header->rodata_size == 0u ||
        !runtime_range_valid(rodata_runtime_offset, header->rodata_size,
                             header->image_size) ||
        header->data_size == 0u ||
        !runtime_range_valid(data_runtime_offset, header->data_size,
                             header->image_size) ||
        header->code_size > rodata_runtime_offset ||
        rodata_runtime_offset + header->rodata_size > data_runtime_offset ||
        data_runtime_offset + header->data_size != header->image_size ||
        header->rodata_offset != header->header_size + rodata_runtime_offset ||
        header->data_offset != header->header_size + data_runtime_offset ||
        !range_valid(header->code_offset, header->code_size, image_size) ||
        !range_valid(header->rodata_offset, header->rodata_size, image_size) ||
        !range_valid(header->data_offset, header->data_size, image_size)) {
        return RIN_LEGACY_DRIVER_V1_IMAGE_BAD_LAYOUT;
    }
    if (!bytes_zero(bytes + header->header_size + header->code_size,
                    rodata_runtime_offset - header->code_size) ||
        !bytes_zero(bytes + header->header_size + rodata_runtime_offset +
                        header->rodata_size,
                    data_runtime_offset - rodata_runtime_offset -
                        header->rodata_size)) {
        return RIN_LEGACY_DRIVER_V1_IMAGE_BAD_LAYOUT;
    }
    if (!range_inside_section(header->entry_offset, 1u,
                              code_runtime_offset, header->code_size) ||
        (header->exit_offset != 0u &&
         !range_inside_section(header->exit_offset, 1u,
                               code_runtime_offset, header->code_size)) ||
        (header->api_offset & 3u) != 0u ||
        !range_inside_section(header->api_offset, 23u * sizeof(uint32_t),
                              data_runtime_offset, header->data_size)) {
        return RIN_LEGACY_DRIVER_V1_IMAGE_BAD_LAYOUT;
    }
    if (header->relocation_count > UINT32_MAX /
            (uint32_t)sizeof(RinLegacyDriverV1Relocation)) {
        return RIN_LEGACY_DRIVER_V1_IMAGE_BAD_RELOCATION;
    }
    relocation_bytes = header->relocation_count *
                       (uint32_t)sizeof(RinLegacyDriverV1Relocation);
    if (header->relocation_offset != header->header_size +
            header->image_size ||
        !range_valid(header->relocation_offset, relocation_bytes,
                     image_size) ||
        header->signature_offset != header->relocation_offset +
            relocation_bytes ||
        header->signature_size < RIN_SIGNATURE_HEADER_SIZE_LOCAL +
            RIN_SIGNATURE_MIN_RSA_SIZE_LOCAL ||
        header->signature_size > RIN_SIGNATURE_HEADER_SIZE_LOCAL +
            RIN_SIGNATURE_MAX_RSA_SIZE_LOCAL ||
        header->signature_offset > image_size ||
        header->signature_size != image_size - header->signature_offset) {
        return RIN_LEGACY_DRIVER_V1_IMAGE_BAD_LAYOUT;
    }
    for (index = 0u; index < header->relocation_count; ++index) {
        const RinLegacyDriverV1Relocation* relocation =
            (const RinLegacyDriverV1Relocation*)(
                bytes + header->relocation_offset) + index;
        int in_section;
        if (relocation->reserved != 0u ||
            (relocation->type != RIN_LEGACY_DRIVER_V1_RELOC_ABS32 &&
             relocation->type != RIN_LEGACY_DRIVER_V1_RELOC_REL32)) {
            return RIN_LEGACY_DRIVER_V1_IMAGE_BAD_RELOCATION;
        }
        in_section = range_inside_section(
                         relocation->offset, sizeof(uint32_t),
                         code_runtime_offset, header->code_size) ||
                     range_inside_section(
                         relocation->offset, sizeof(uint32_t),
                         rodata_runtime_offset, header->rodata_size) ||
                     range_inside_section(
                         relocation->offset, sizeof(uint32_t),
                         data_runtime_offset, header->data_size);
        if (!in_section) return RIN_LEGACY_DRIVER_V1_IMAGE_BAD_RELOCATION;
    }

    view->header = header;
    view->relocations = (const RinLegacyDriverV1Relocation*)(
        bytes + header->relocation_offset);
    view->code_runtime_offset = code_runtime_offset;
    view->rodata_runtime_offset = rodata_runtime_offset;
    view->data_runtime_offset = data_runtime_offset;
    return RIN_LEGACY_DRIVER_V1_IMAGE_OK;
}
