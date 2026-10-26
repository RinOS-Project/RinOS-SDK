/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <rin/driver/display/edid.h>

#define EDID_BASE_DTD_OFFSET 54u
#define EDID_DTD_SIZE 18u
#define EDID_BASE_DTD_COUNT 4u
#define EDID_EXTENSION_COUNT_OFFSET 126u
#define EDID_CHECKSUM_OFFSET 127u

#define CTA_EXTENSION_TAG 0x02u
#define CTA_REVISION_SUPPORTED 3u
#define CTA_DATA_BLOCK_OFFSET 4u
#define CTA_TAG_AUDIO 1u
#define CTA_TAG_VIDEO 2u
#define CTA_TAG_VENDOR 3u
#define CTA_TAG_SPEAKER 4u
#define CTA_TAG_EXTENDED 7u
#define CTA_EXT_HDR_STATIC_METADATA 0x06u

#define EDID_MIN_REFRESH_MILLIHZ 1000u
#define EDID_MAX_REFRESH_MILLIHZ 1000000u

static void edid_zero(void* destination, uint32_t size) {
    uint8_t* output = (uint8_t*)destination;
    for (uint32_t index = 0u; index < size; ++index) output[index] = 0u;
}

static void edid_copy(uint8_t* destination, const uint8_t* source,
                      uint32_t size) {
    for (uint32_t index = 0u; index < size; ++index) {
        destination[index] = source[index];
    }
}

static uint32_t edid_all_zero(const uint8_t* bytes, uint32_t size) {
    uint8_t value = 0u;
    for (uint32_t index = 0u; index < size; ++index) value |= bytes[index];
    return value == 0u;
}

static uint32_t edid_block_checksum_valid(const uint8_t* block) {
    uint32_t sum = 0u;
    for (uint32_t index = 0u; index < RIN_DISPLAY_EDID_BLOCK_SIZE; ++index) {
        sum += block[index];
    }
    return (sum & 0xffu) == 0u;
}

static uint64_t edid_divide_u64_u32(uint64_t numerator,
                                    uint32_t denominator) {
    uint64_t quotient = 0u;
    uint64_t remainder = 0u;
    if (denominator == 0u) return 0u;
    for (uint32_t bit = 64u; bit != 0u; --bit) {
        remainder = (remainder << 1u) | ((numerator >> (bit - 1u)) & 1u);
        if (remainder >= denominator) {
            remainder -= denominator;
            quotient |= (uint64_t)1u << (bit - 1u);
        }
    }
    return quotient;
}

static RinDisplayEdidResult edid_parse_detailed_timing(
    const uint8_t* timing, RinDisplayEdidModeV1* mode) {
    uint32_t horizontal_active;
    uint32_t horizontal_blank;
    uint32_t vertical_active;
    uint32_t vertical_blank;
    uint32_t horizontal_sync_offset;
    uint32_t horizontal_sync_width;
    uint32_t vertical_sync_offset;
    uint32_t vertical_sync_width;
    uint32_t denominator;
    uint64_t rounded_clock;
    uint64_t refresh;

    edid_zero(mode, sizeof(*mode));
    mode->pixel_clock_khz =
        (((uint32_t)timing[1] << 8u) | timing[0]) * 10u;
    horizontal_active = timing[2] |
        ((uint32_t)(timing[4] & 0xf0u) << 4u);
    horizontal_blank = timing[3] |
        ((uint32_t)(timing[4] & 0x0fu) << 8u);
    vertical_active = timing[5] |
        ((uint32_t)(timing[7] & 0xf0u) << 4u);
    vertical_blank = timing[6] |
        ((uint32_t)(timing[7] & 0x0fu) << 8u);
    horizontal_sync_offset = timing[8] |
        ((uint32_t)(timing[11] & 0xc0u) << 2u);
    horizontal_sync_width = timing[9] |
        ((uint32_t)(timing[11] & 0x30u) << 4u);
    vertical_sync_offset = (timing[10] >> 4u) |
        ((uint32_t)(timing[11] & 0x0cu) << 2u);
    vertical_sync_width = (timing[10] & 0x0fu) |
        ((uint32_t)(timing[11] & 0x03u) << 4u);
    if (mode->pixel_clock_khz == 0u || horizontal_active == 0u ||
        horizontal_blank == 0u || vertical_active == 0u ||
        vertical_blank == 0u || horizontal_sync_width == 0u ||
        vertical_sync_width == 0u ||
        horizontal_sync_offset + horizontal_sync_width > horizontal_blank ||
        vertical_sync_offset + vertical_sync_width > vertical_blank) {
        return RIN_DISPLAY_EDID_MALFORMED;
    }

    mode->width = horizontal_active;
    mode->height = vertical_active;
    mode->horizontal_total = horizontal_active + horizontal_blank;
    mode->vertical_total = vertical_active + vertical_blank;
    denominator = mode->horizontal_total * mode->vertical_total;
    rounded_clock = (uint64_t)mode->pixel_clock_khz * 1000000u +
                    (uint64_t)(denominator / 2u);
    refresh = edid_divide_u64_u32(rounded_clock, denominator);
    if (refresh < EDID_MIN_REFRESH_MILLIHZ ||
        refresh > EDID_MAX_REFRESH_MILLIHZ) {
        edid_zero(mode, sizeof(*mode));
        return RIN_DISPLAY_EDID_MALFORMED;
    }
    mode->refresh_millihz = (uint32_t)refresh;
    if ((timing[17] & 0x80u) != 0u) {
        mode->flags |= RIN_DISPLAY_EDID_MODE_INTERLACED;
    }
    return RIN_DISPLAY_EDID_OK;
}

static RinDisplayEdidResult edid_parse_base_descriptor(
    const uint8_t* descriptor, RinDisplayEdidInfoV1* info,
    uint32_t descriptor_index) {
    RinDisplayEdidModeV1 mode;
    RinDisplayEdidResult result;
    if (descriptor[0] == 0u && descriptor[1] == 0u) {
        if (descriptor[2] != 0u || descriptor[4] != 0u) {
            return RIN_DISPLAY_EDID_MALFORMED;
        }
        return RIN_DISPLAY_EDID_OK;
    }
    result = edid_parse_detailed_timing(descriptor, &mode);
    if (result != RIN_DISPLAY_EDID_OK) return result;
    ++info->detailed_timing_count;
    if (descriptor_index == 0u && (info->flags &
            RIN_DISPLAY_EDID_INFO_HAS_PREFERRED_MODE) != 0u) {
        edid_copy((uint8_t*)&info->preferred_mode,
                  (const uint8_t*)&mode, sizeof(mode));
    }
    return RIN_DISPLAY_EDID_OK;
}

static RinDisplayEdidResult edid_parse_hdr_block(
    const uint8_t* payload, uint32_t length, RinDisplayEdidInfoV1* info) {
    uint32_t eotf;
    uint32_t metadata;
    if (length < 3u || length > 6u ||
        (info->flags & RIN_DISPLAY_EDID_INFO_HAS_HDR_STATIC_METADATA) != 0u) {
        return RIN_DISPLAY_EDID_MALFORMED;
    }
    eotf = payload[1];
    metadata = payload[2];
    if (eotf == 0u || (eotf & ~0x0fu) != 0u ||
        (metadata & ~RIN_DISPLAY_EDID_HDR_STATIC_TYPE_1) != 0u) {
        return RIN_DISPLAY_EDID_MALFORMED;
    }
    info->hdr_eotf_flags = eotf;
    info->hdr_static_metadata_flags = metadata;
    info->flags |= RIN_DISPLAY_EDID_INFO_HAS_HDR_STATIC_METADATA;
    return RIN_DISPLAY_EDID_OK;
}

static RinDisplayEdidResult edid_parse_data_block(
    uint32_t tag, const uint8_t* payload, uint32_t length,
    RinDisplayEdidInfoV1* info) {
    if (tag == 0u) return RIN_DISPLAY_EDID_MALFORMED;
    if (tag == CTA_TAG_AUDIO && (length == 0u || (length % 3u) != 0u)) {
        return RIN_DISPLAY_EDID_MALFORMED;
    }
    if (tag == CTA_TAG_VIDEO) {
        if (length == 0u) return RIN_DISPLAY_EDID_MALFORMED;
        for (uint32_t index = 0u; index < length; ++index) {
            uint32_t vic = payload[index] & 0x7fu;
            if (vic == 0u) return RIN_DISPLAY_EDID_MALFORMED;
            ++info->short_video_descriptor_count;
            if ((payload[index] & 0x80u) != 0u && info->native_vic == 0u) {
                info->native_vic = vic;
            }
        }
        return RIN_DISPLAY_EDID_OK;
    }
    if (tag == CTA_TAG_VENDOR && length < 3u) {
        return RIN_DISPLAY_EDID_MALFORMED;
    }
    if (tag == CTA_TAG_SPEAKER && length != 3u) {
        return RIN_DISPLAY_EDID_MALFORMED;
    }
    if (tag == CTA_TAG_EXTENDED) {
        if (length == 0u) return RIN_DISPLAY_EDID_MALFORMED;
        if (payload[0] == CTA_EXT_HDR_STATIC_METADATA) {
            return edid_parse_hdr_block(payload, length, info);
        }
    }
    return RIN_DISPLAY_EDID_OK;
}

static RinDisplayEdidResult edid_parse_cta_extension(
    const uint8_t* block, RinDisplayEdidInfoV1* info) {
    uint32_t data_end = block[2];
    uint32_t offset = CTA_DATA_BLOCK_OFFSET;
    uint32_t cta_detailed_count = 0u;
    uint32_t declared_native_detailed = block[3] & 0x0fu;

    if (block[1] != CTA_REVISION_SUPPORTED) {
        return RIN_DISPLAY_EDID_UNSUPPORTED;
    }
    ++info->cta_extension_count;
    info->flags |= RIN_DISPLAY_EDID_INFO_HAS_CTA;
    if (data_end == 0u) {
        if (declared_native_detailed != 0u ||
            !edid_all_zero(block + CTA_DATA_BLOCK_OFFSET,
                           EDID_CHECKSUM_OFFSET - CTA_DATA_BLOCK_OFFSET)) {
            return RIN_DISPLAY_EDID_MALFORMED;
        }
        return RIN_DISPLAY_EDID_OK;
    }
    if (data_end < CTA_DATA_BLOCK_OFFSET || data_end > EDID_CHECKSUM_OFFSET) {
        return RIN_DISPLAY_EDID_MALFORMED;
    }

    while (offset < data_end) {
        uint32_t header = block[offset++];
        uint32_t tag;
        uint32_t length;
        RinDisplayEdidResult result;
        if (header == 0u) {
            if (!edid_all_zero(block + offset, data_end - offset)) {
                return RIN_DISPLAY_EDID_MALFORMED;
            }
            break;
        }
        tag = header >> 5u;
        length = header & 0x1fu;
        if (length > data_end - offset) return RIN_DISPLAY_EDID_MALFORMED;
        result = edid_parse_data_block(tag, block + offset, length, info);
        if (result != RIN_DISPLAY_EDID_OK) return result;
        offset += length;
    }

    offset = data_end;
    while (offset + EDID_DTD_SIZE <= EDID_CHECKSUM_OFFSET) {
        RinDisplayEdidModeV1 ignored_mode;
        RinDisplayEdidResult result;
        if (edid_all_zero(block + offset, EDID_DTD_SIZE)) {
            offset += EDID_DTD_SIZE;
            continue;
        }
        result = edid_parse_detailed_timing(block + offset, &ignored_mode);
        if (result != RIN_DISPLAY_EDID_OK) return result;
        ++cta_detailed_count;
        ++info->detailed_timing_count;
        offset += EDID_DTD_SIZE;
    }
    if (!edid_all_zero(block + offset, EDID_CHECKSUM_OFFSET - offset) ||
        declared_native_detailed > cta_detailed_count) {
        return RIN_DISPLAY_EDID_MALFORMED;
    }
    return RIN_DISPLAY_EDID_OK;
}

/* CEA-861 VICs that have an unambiguous mode representation in the public
 * pointer-free ABI. Unknown or newer VICs are intentionally omitted instead
 * of being guessed into a mode the hardware may not support. The list bound
 * accounts for four SVD data blocks of up to 30, 30, 30, and 29 entries in
 * each of the three admitted CTA extensions. */
static int edid_mode_from_vic(uint32_t vic, RinDisplayEdidModeV1* mode) {
    uint32_t pixel_clock_khz = 0u;
    uint32_t width = 0u;
    uint32_t height = 0u;
    uint32_t horizontal_total = 0u;
    uint32_t vertical_total = 0u;
    uint32_t refresh_millihz = 0u;
    uint32_t flags = 0u;

    if (!mode) return 0;
    switch (vic) {
        case 1u:
            pixel_clock_khz = 25175u; width = 640u; height = 480u;
            horizontal_total = 800u; vertical_total = 525u;
            refresh_millihz = 59940u;
            break;
        case 2u:
        case 3u:
            pixel_clock_khz = 27000u; width = 720u; height = 480u;
            horizontal_total = 858u; vertical_total = 525u;
            refresh_millihz = 60000u;
            break;
        case 4u:
            pixel_clock_khz = 74250u; width = 1280u; height = 720u;
            horizontal_total = 1650u; vertical_total = 750u;
            refresh_millihz = 60000u;
            break;
        case 5u:
            pixel_clock_khz = 74250u; width = 1920u; height = 1080u;
            horizontal_total = 2200u; vertical_total = 1125u;
            refresh_millihz = 60000u;
            flags = RIN_DISPLAY_EDID_MODE_INTERLACED;
            break;
        case 6u:
            pixel_clock_khz = 13500u; width = 720u; height = 480u;
            horizontal_total = 858u; vertical_total = 525u;
            refresh_millihz = 60000u;
            flags = RIN_DISPLAY_EDID_MODE_INTERLACED;
            break;
        case 16u:
            pixel_clock_khz = 148500u; width = 1920u; height = 1080u;
            horizontal_total = 2200u; vertical_total = 1125u;
            refresh_millihz = 60000u;
            break;
        case 17u:
        case 18u:
            pixel_clock_khz = 27000u; width = 720u; height = 576u;
            horizontal_total = 864u; vertical_total = 625u;
            refresh_millihz = 50000u;
            break;
        case 19u:
            pixel_clock_khz = 74250u; width = 1280u; height = 720u;
            horizontal_total = 1980u; vertical_total = 750u;
            refresh_millihz = 50000u;
            break;
        case 20u:
            pixel_clock_khz = 74250u; width = 1920u; height = 1080u;
            horizontal_total = 2640u; vertical_total = 1125u;
            refresh_millihz = 50000u;
            flags = RIN_DISPLAY_EDID_MODE_INTERLACED;
            break;
        case 31u:
            pixel_clock_khz = 148500u; width = 1920u; height = 1080u;
            horizontal_total = 2640u; vertical_total = 1125u;
            refresh_millihz = 50000u;
            break;
        case 32u:
            pixel_clock_khz = 74250u; width = 1920u; height = 1080u;
            horizontal_total = 2750u; vertical_total = 1125u;
            refresh_millihz = 24000u;
            break;
        case 33u:
            pixel_clock_khz = 74250u; width = 1920u; height = 1080u;
            horizontal_total = 2640u; vertical_total = 1125u;
            refresh_millihz = 25000u;
            break;
        case 34u:
            pixel_clock_khz = 74250u; width = 1920u; height = 1080u;
            horizontal_total = 2200u; vertical_total = 1125u;
            refresh_millihz = 30000u;
            break;
        case 93u:
            pixel_clock_khz = 297000u; width = 3840u; height = 2160u;
            horizontal_total = 5500u; vertical_total = 2250u;
            refresh_millihz = 24000u;
            break;
        case 94u:
            pixel_clock_khz = 297000u; width = 3840u; height = 2160u;
            horizontal_total = 5280u; vertical_total = 2250u;
            refresh_millihz = 25000u;
            break;
        case 95u:
            pixel_clock_khz = 297000u; width = 3840u; height = 2160u;
            horizontal_total = 4400u; vertical_total = 2250u;
            refresh_millihz = 30000u;
            break;
        case 96u:
            pixel_clock_khz = 594000u; width = 3840u; height = 2160u;
            horizontal_total = 5280u; vertical_total = 2250u;
            refresh_millihz = 50000u;
            break;
        case 97u:
            pixel_clock_khz = 594000u; width = 3840u; height = 2160u;
            horizontal_total = 4400u; vertical_total = 2250u;
            refresh_millihz = 60000u;
            break;
        case 98u:
            pixel_clock_khz = 297000u; width = 4096u; height = 2160u;
            horizontal_total = 5500u; vertical_total = 2250u;
            refresh_millihz = 24000u;
            break;
        case 99u:
            pixel_clock_khz = 297000u; width = 4096u; height = 2160u;
            horizontal_total = 5280u; vertical_total = 2250u;
            refresh_millihz = 25000u;
            break;
        case 100u:
            pixel_clock_khz = 297000u; width = 4096u; height = 2160u;
            horizontal_total = 4400u; vertical_total = 2250u;
            refresh_millihz = 30000u;
            break;
        case 101u:
            pixel_clock_khz = 594000u; width = 4096u; height = 2160u;
            horizontal_total = 5280u; vertical_total = 2250u;
            refresh_millihz = 50000u;
            break;
        case 102u:
            pixel_clock_khz = 594000u; width = 4096u; height = 2160u;
            horizontal_total = 4400u; vertical_total = 2250u;
            refresh_millihz = 60000u;
            break;
        case 103u:
            pixel_clock_khz = 297000u; width = 3840u; height = 2160u;
            horizontal_total = 5500u; vertical_total = 2250u;
            refresh_millihz = 24000u;
            break;
        case 104u:
            pixel_clock_khz = 297000u; width = 3840u; height = 2160u;
            horizontal_total = 5280u; vertical_total = 2250u;
            refresh_millihz = 25000u;
            break;
        case 105u:
            pixel_clock_khz = 297000u; width = 3840u; height = 2160u;
            horizontal_total = 4400u; vertical_total = 2250u;
            refresh_millihz = 30000u;
            break;
        default:
            return 0;
    }

    edid_zero(mode, sizeof(*mode));
    mode->pixel_clock_khz = pixel_clock_khz;
    mode->width = width;
    mode->height = height;
    mode->horizontal_total = horizontal_total;
    mode->vertical_total = vertical_total;
    mode->refresh_millihz = refresh_millihz;
    mode->flags = flags;
    return 1;
}

static RinDisplayEdidResult edid_mode_list_append(
    RinDisplayEdidModeListV1* list, const RinDisplayEdidModeV1* mode);

static RinDisplayEdidResult edid_collect_cta_video_modes(
    const uint8_t* block, RinDisplayEdidModeListV1* list) {
    uint32_t data_end = block[2];
    uint32_t offset = CTA_DATA_BLOCK_OFFSET;

    while (offset < data_end) {
        uint32_t header = block[offset++];
        uint32_t tag;
        uint32_t length;
        if (header == 0u) break;
        tag = header >> 5u;
        length = header & 0x1fu;
        if (length > data_end - offset) {
            return RIN_DISPLAY_EDID_MALFORMED;
        }
        if (tag == CTA_TAG_VIDEO) {
            for (uint32_t index = 0u; index < length; ++index) {
                RinDisplayEdidModeV1 mode;
                if (edid_mode_from_vic(block[offset + index] & 0x7fu,
                                       &mode)) {
                    RinDisplayEdidResult result =
                        edid_mode_list_append(list, &mode);
                    if (result != RIN_DISPLAY_EDID_OK) return result;
                }
            }
        }
        offset += length;
    }
    return RIN_DISPLAY_EDID_OK;
}

static int edid_modes_equal(const RinDisplayEdidModeV1* left,
                            const RinDisplayEdidModeV1* right) {
    const uint8_t* left_bytes = (const uint8_t*)left;
    const uint8_t* right_bytes = (const uint8_t*)right;
    for (uint32_t index = 0u; index < sizeof(*left); ++index) {
        if (left_bytes[index] != right_bytes[index]) return 0;
    }
    return 1;
}

static RinDisplayEdidResult edid_mode_list_append(
    RinDisplayEdidModeListV1* list, const RinDisplayEdidModeV1* mode) {
    for (uint32_t index = 0u; index < list->mode_count; ++index) {
        if (edid_modes_equal(&list->modes[index], mode)) {
            return RIN_DISPLAY_EDID_OK;
        }
    }
    if (list->mode_count >= RIN_DISPLAY_EDID_MAX_MODES) {
        return RIN_DISPLAY_EDID_UNSUPPORTED;
    }
    edid_copy((uint8_t*)&list->modes[list->mode_count],
              (const uint8_t*)mode, sizeof(*mode));
    ++list->mode_count;
    return RIN_DISPLAY_EDID_OK;
}

RinDisplayEdidResult rin_display_edid_admit(
    const uint8_t* bytes, uint32_t length, RinDisplayEdidInfoV1* info) {
    static const uint8_t header[8] = {
        0x00u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu, 0x00u
    };
    uint8_t snapshot[RIN_DISPLAY_EDID_MAX_BYTES];
    RinDisplayEdidInfoV1 candidate;
    uint32_t expected_length;

    if (!info) return RIN_DISPLAY_EDID_INVALID_ARGUMENT;
    if (!bytes || length < RIN_DISPLAY_EDID_BLOCK_SIZE ||
        length > RIN_DISPLAY_EDID_MAX_BYTES ||
        (length % RIN_DISPLAY_EDID_BLOCK_SIZE) != 0u) {
        edid_zero(info, sizeof(*info));
        return RIN_DISPLAY_EDID_INVALID_ARGUMENT;
    }
    edid_copy(snapshot, bytes, length);
    edid_zero(info, sizeof(*info));
    for (uint32_t index = 0u; index < sizeof(header); ++index) {
        if (snapshot[index] != header[index]) {
            return RIN_DISPLAY_EDID_MALFORMED;
        }
    }
    expected_length = ((uint32_t)snapshot[EDID_EXTENSION_COUNT_OFFSET] + 1u) *
                      RIN_DISPLAY_EDID_BLOCK_SIZE;
    if (expected_length > RIN_DISPLAY_EDID_MAX_BYTES) {
        return RIN_DISPLAY_EDID_UNSUPPORTED;
    }
    if (length != expected_length) return RIN_DISPLAY_EDID_MALFORMED;
    for (uint32_t offset = 0u; offset < length;
         offset += RIN_DISPLAY_EDID_BLOCK_SIZE) {
        if (!edid_block_checksum_valid(snapshot + offset)) {
            return RIN_DISPLAY_EDID_MALFORMED;
        }
    }
    if (snapshot[18] != 1u || snapshot[19] < 3u || snapshot[19] > 4u) {
        return RIN_DISPLAY_EDID_UNSUPPORTED;
    }
    {
        uint32_t manufacturer =
            ((uint32_t)snapshot[8] << 8u) | snapshot[9];
        uint32_t first = (manufacturer >> 10u) & 0x1fu;
        uint32_t second = (manufacturer >> 5u) & 0x1fu;
        uint32_t third = manufacturer & 0x1fu;
        if ((manufacturer & 0x8000u) != 0u || first == 0u || first > 26u ||
            second == 0u || second > 26u || third == 0u || third > 26u) {
            return RIN_DISPLAY_EDID_MALFORMED;
        }
    }

    edid_zero(&candidate, sizeof(candidate));
    candidate.abi_version = RIN_DISPLAY_EDID_INFO_ABI_VERSION;
    candidate.struct_size = sizeof(candidate);
    candidate.manufacturer_id = ((uint32_t)snapshot[8] << 8u) | snapshot[9];
    candidate.product_code = snapshot[10] | ((uint32_t)snapshot[11] << 8u);
    candidate.serial_number = snapshot[12] |
        ((uint32_t)snapshot[13] << 8u) |
        ((uint32_t)snapshot[14] << 16u) |
        ((uint32_t)snapshot[15] << 24u);
    candidate.edid_version = snapshot[18];
    candidate.edid_revision = snapshot[19];
    candidate.physical_width_mm = (uint32_t)snapshot[21] * 10u;
    candidate.physical_height_mm = (uint32_t)snapshot[22] * 10u;
    candidate.block_count = length / RIN_DISPLAY_EDID_BLOCK_SIZE;
    if ((snapshot[24] & 0x02u) != 0u) {
        candidate.flags |= RIN_DISPLAY_EDID_INFO_HAS_PREFERRED_MODE;
    }

    for (uint32_t index = 0u; index < EDID_BASE_DTD_COUNT; ++index) {
        RinDisplayEdidResult result = edid_parse_base_descriptor(
            snapshot + EDID_BASE_DTD_OFFSET + index * EDID_DTD_SIZE,
            &candidate, index);
        if (result != RIN_DISPLAY_EDID_OK) return result;
    }
    if ((candidate.flags & RIN_DISPLAY_EDID_INFO_HAS_PREFERRED_MODE) != 0u &&
        candidate.preferred_mode.pixel_clock_khz == 0u) {
        return RIN_DISPLAY_EDID_MALFORMED;
    }

    for (uint32_t block_index = 1u; block_index < candidate.block_count;
         ++block_index) {
        const uint8_t* block = snapshot +
            block_index * RIN_DISPLAY_EDID_BLOCK_SIZE;
        if (block[0] == CTA_EXTENSION_TAG) {
            RinDisplayEdidResult result =
                edid_parse_cta_extension(block, &candidate);
            if (result != RIN_DISPLAY_EDID_OK) return result;
        } else if (block[0] == 0u) {
            return RIN_DISPLAY_EDID_MALFORMED;
        }
    }
    edid_copy((uint8_t*)info, (const uint8_t*)&candidate, sizeof(candidate));
    return RIN_DISPLAY_EDID_OK;
}

RinDisplayEdidResult rin_display_edid_collect_modes(
    const uint8_t* bytes, uint32_t length, RinDisplayEdidModeListV1* modes) {
    uint8_t snapshot[RIN_DISPLAY_EDID_MAX_BYTES];
    RinDisplayEdidInfoV1 info;
    RinDisplayEdidModeListV1 candidate;
    RinDisplayEdidResult result;

    if (!modes) return RIN_DISPLAY_EDID_INVALID_ARGUMENT;
    if (!bytes || length < RIN_DISPLAY_EDID_BLOCK_SIZE ||
        length > RIN_DISPLAY_EDID_MAX_BYTES ||
        (length % RIN_DISPLAY_EDID_BLOCK_SIZE) != 0u) {
        edid_zero(modes, sizeof(*modes));
        return RIN_DISPLAY_EDID_INVALID_ARGUMENT;
    }

    /* Snapshot before touching the caller's output. This keeps the API safe
     * when a caller uses storage that overlaps the EDID input. */
    edid_copy(snapshot, bytes, length);
    edid_zero(modes, sizeof(*modes));
    edid_zero(&info, sizeof(info));
    result = rin_display_edid_admit(snapshot, length, &info);
    if (result != RIN_DISPLAY_EDID_OK) return result;

    edid_zero(&candidate, sizeof(candidate));
    candidate.abi_version = RIN_DISPLAY_EDID_INFO_ABI_VERSION;
    candidate.struct_size = sizeof(candidate);
    candidate.preferred_mode_index = RIN_DISPLAY_EDID_NO_MODE_INDEX;

    for (uint32_t index = 0u; index < EDID_BASE_DTD_COUNT; ++index) {
        const uint8_t* descriptor = snapshot + EDID_BASE_DTD_OFFSET +
            index * EDID_DTD_SIZE;
        RinDisplayEdidModeV1 mode;
        if (descriptor[0] == 0u && descriptor[1] == 0u) continue;
        result = edid_parse_detailed_timing(descriptor, &mode);
        if (result != RIN_DISPLAY_EDID_OK) {
            edid_zero(modes, sizeof(*modes));
            return RIN_DISPLAY_EDID_MALFORMED;
        }
        result = edid_mode_list_append(&candidate, &mode);
        if (result != RIN_DISPLAY_EDID_OK) {
            edid_zero(modes, sizeof(*modes));
            return result;
        }
    }

    for (uint32_t block_index = 1u; block_index < info.block_count;
         ++block_index) {
        const uint8_t* block = snapshot +
            block_index * RIN_DISPLAY_EDID_BLOCK_SIZE;
        uint32_t offset;
        if (block[0] != CTA_EXTENSION_TAG) continue;
        result = edid_collect_cta_video_modes(block, &candidate);
        if (result != RIN_DISPLAY_EDID_OK) {
            edid_zero(modes, sizeof(*modes));
            return result;
        }
        offset = block[2];
        while (offset + EDID_DTD_SIZE <= EDID_CHECKSUM_OFFSET) {
            RinDisplayEdidModeV1 mode;
            if (edid_all_zero(block + offset, EDID_DTD_SIZE)) {
                offset += EDID_DTD_SIZE;
                continue;
            }
            result = edid_parse_detailed_timing(block + offset, &mode);
            if (result != RIN_DISPLAY_EDID_OK) {
                edid_zero(modes, sizeof(*modes));
                return RIN_DISPLAY_EDID_MALFORMED;
            }
            result = edid_mode_list_append(&candidate, &mode);
            if (result != RIN_DISPLAY_EDID_OK) {
                edid_zero(modes, sizeof(*modes));
                return result;
            }
            offset += EDID_DTD_SIZE;
        }
    }

    if ((info.flags & RIN_DISPLAY_EDID_INFO_HAS_PREFERRED_MODE) != 0u) {
        for (uint32_t index = 0u; index < candidate.mode_count; ++index) {
            if (edid_modes_equal(&candidate.modes[index],
                                 &info.preferred_mode)) {
                candidate.preferred_mode_index = index;
                break;
            }
        }
        if (candidate.preferred_mode_index == RIN_DISPLAY_EDID_NO_MODE_INDEX) {
            edid_zero(modes, sizeof(*modes));
            return RIN_DISPLAY_EDID_MALFORMED;
        }
    } else if (info.native_vic != 0u) {
        RinDisplayEdidModeV1 native_mode;
        if (edid_mode_from_vic(info.native_vic, &native_mode)) {
            for (uint32_t index = 0u; index < candidate.mode_count; ++index) {
                if (edid_modes_equal(&candidate.modes[index], &native_mode)) {
                    candidate.preferred_mode_index = index;
                    break;
                }
            }
            if (candidate.preferred_mode_index ==
                    RIN_DISPLAY_EDID_NO_MODE_INDEX) {
                edid_zero(modes, sizeof(*modes));
                return RIN_DISPLAY_EDID_MALFORMED;
            }
        }
    }

    edid_copy((uint8_t*)modes, (const uint8_t*)&candidate,
              sizeof(candidate));
    return RIN_DISPLAY_EDID_OK;
}
