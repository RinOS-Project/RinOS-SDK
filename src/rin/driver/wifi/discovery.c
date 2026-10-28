/* SPDX-License-Identifier: MIT */

#include <rin/driver/wifi/discovery.h>

#include <stddef.h>

#define WIFI_MANAGEMENT_HEADER_SIZE 24u
#define WIFI_DISCOVERY_FIXED_SIZE 12u
#define WIFI_DISCOVERY_MINIMUM_SIZE \
    (WIFI_MANAGEMENT_HEADER_SIZE + WIFI_DISCOVERY_FIXED_SIZE)

#define WIFI_FRAME_TYPE_MASK UINT16_C(0x000c)
#define WIFI_FRAME_SUBTYPE_SHIFT 4u
#define WIFI_FRAME_SUBTYPE_PROBE_RESPONSE 5u
#define WIFI_FRAME_SUBTYPE_BEACON 8u
#define WIFI_FRAME_REJECTED_FLAGS UINT16_C(0xc700)
#define WIFI_CAPABILITY_PRIVACY UINT16_C(0x0010)

#define WIFI_IE_SSID 0u
#define WIFI_IE_DS_PARAMETER 3u
#define WIFI_IE_RSN 48u
#define WIFI_IE_RSNX 244u
#define WIFI_IE_EXTENSION 255u
#define WIFI_IE_EXTENSION_HE_OPERATION 36u
#define WIFI_IE_EXTENSION_EHT_OPERATION 106u
#define WIFI_IE_EXTENSION_EHT_MULTI_LINK 107u

#define WIFI_HE_OPERATION_BASE_SIZE 7u
#define WIFI_HE_OPERATION_VHT_INFO UINT32_C(0x00004000)
#define WIFI_HE_OPERATION_CO_HOSTED_BSS UINT32_C(0x00008000)
#define WIFI_HE_OPERATION_6GHZ_INFO UINT32_C(0x00020000)
#define WIFI_HE_6GHZ_OPERATION_SIZE 5u
#define WIFI_HE_6GHZ_WIDTH_MASK 0x03u
#define WIFI_HE_6GHZ_DUPLICATE_BEACON 0x04u
#define WIFI_HE_6GHZ_REGULATORY_MASK 0x78u
#define WIFI_HE_6GHZ_RESERVED_MASK 0x80u

#define WIFI_EHT_OPERATION_BASE_SIZE 5u
#define WIFI_EHT_OPERATION_INFO_PRESENT 0x01u
#define WIFI_EHT_OPERATION_DISABLED_SUBCHANNELS 0x02u
#define WIFI_EHT_OPERATION_KNOWN_PARAMETERS 0x7fu
#define WIFI_EHT_OPERATION_INFO_SIZE 3u
#define WIFI_EHT_OPERATION_WIDTH_MASK 0x07u
#define WIFI_EHT_OPERATION_WIDTH_20_MHZ 0u
#define WIFI_EHT_OPERATION_WIDTH_40_MHZ 1u
#define WIFI_EHT_OPERATION_WIDTH_80_MHZ 2u
#define WIFI_EHT_OPERATION_WIDTH_160_MHZ 3u
#define WIFI_EHT_OPERATION_WIDTH_320_MHZ 4u

#define WIFI_MLO_CONTROL_TYPE_MASK UINT16_C(0x0007)
#define WIFI_MLO_CONTROL_RESERVED_LOW UINT16_C(0x0008)
#define WIFI_MLO_CONTROL_LINK_ID_PRESENT UINT16_C(0x0010)
#define WIFI_MLO_CONTROL_BSS_PARAMETER_CHANGE_COUNT UINT16_C(0x0020)
#define WIFI_MLO_CONTROL_MEDIUM_SYNC_DELAY UINT16_C(0x0040)
#define WIFI_MLO_CONTROL_EML_CAPABILITIES UINT16_C(0x0080)
#define WIFI_MLO_CONTROL_MLD_CAPABILITIES UINT16_C(0x0100)
#define WIFI_MLO_CONTROL_MLD_ID UINT16_C(0x0200)
#define WIFI_MLO_CONTROL_EXTENDED_MLD_CAPABILITIES UINT16_C(0x0400)
#define WIFI_MLO_CONTROL_ENHANCED_CRITICAL_UPDATE UINT16_C(0x0800)
#define WIFI_MLO_CONTROL_KNOWN_PRESENCE UINT16_C(0x0ff0)
#define WIFI_MLO_BASIC_COMMON_MINIMUM_SIZE 7u
#define WIFI_MLO_LINK_ID_RESERVED_MASK 0xf0u

#define WIFI_RSNX_FIELD_LENGTH_MASK 0x0fu
#define WIFI_RSNX_SAE_H2E 0x20u

#define WIFI_RSN_SUITE_SIZE 4u
#define WIFI_RSN_MAX_PAIRWISE 4u
#define WIFI_RSN_MAX_AKM 4u
#define WIFI_RSN_MAX_PMKIDS 16u
#define WIFI_RSN_CAPABILITY_MFPR UINT16_C(0x0040)
#define WIFI_RSN_CAPABILITY_MFPC UINT16_C(0x0080)

typedef struct WifiEhtMloEvidence {
    uint16_t eht_channel_width_mhz;
    uint16_t eht_disabled_subchannel_bitmap;
    uint8_t eht_present;
    uint8_t eht_center_channel0;
    uint8_t eht_center_channel1;
    uint8_t mlo_present;
    uint8_t mlo_link_id;
    uint8_t mld_address[6];
} WifiEhtMloEvidence;

static void wifi_zero(void* destination, size_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)destination;
    if (destination == NULL) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static uint16_t wifi_read_u16(const uint8_t* bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8u));
}

static uint32_t wifi_read_u32(const uint8_t* bytes)
{
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8u) |
           ((uint32_t)bytes[2] << 16u) | ((uint32_t)bytes[3] << 24u);
}

static uint64_t wifi_read_u64(const uint8_t* bytes)
{
    uint64_t value = 0u;
    uint32_t index;
    for (index = 0u; index < 8u; ++index)
        value |= (uint64_t)bytes[index] << (index * 8u);
    return value;
}

static int wifi_equal(const uint8_t* left, const uint8_t* right,
                      uint32_t size)
{
    uint8_t difference = 0u;
    uint32_t index;
    if (left == NULL || right == NULL) return 0;
    for (index = 0u; index < size; ++index)
        difference |= (uint8_t)(left[index] ^ right[index]);
    return difference == 0u;
}

static int wifi_ranges_overlap(const void* left, size_t left_size,
                               const void* right, size_t right_size)
{
    uintptr_t left_first;
    uintptr_t right_first;
    uintptr_t left_last;
    uintptr_t right_last;
    if (left == NULL || right == NULL || left_size == 0u || right_size == 0u)
        return 0;
    left_first = (uintptr_t)left;
    right_first = (uintptr_t)right;
    if (left_first > UINTPTR_MAX - (left_size - 1u) ||
        right_first > UINTPTR_MAX - (right_size - 1u))
        return 1;
    left_last = left_first + left_size - 1u;
    right_last = right_first + right_size - 1u;
    return left_first <= right_last && right_first <= left_last;
}

static int wifi_bssid_valid(const uint8_t bssid[6])
{
    uint8_t observed = 0u;
    uint32_t index;
    if (bssid == NULL || (bssid[0] & 1u) != 0u) return 0;
    for (index = 0u; index < 6u; ++index) observed |= bssid[index];
    return observed != 0u;
}

static int wifi_5ghz_channel_valid(uint8_t channel)
{
    if (channel >= 36u && channel <= 64u)
        return (channel & 3u) == 0u;
    if (channel >= 100u && channel <= 144u)
        return (channel & 3u) == 0u;
    if (channel >= 149u && channel <= 177u)
        return (uint8_t)(channel - 149u) % 4u == 0u;
    return 0;
}

static int wifi_6ghz_primary_valid(uint8_t channel)
{
    return channel >= 1u && channel <= 233u &&
           (uint8_t)(channel - 1u) % 4u == 0u;
}

static int wifi_6ghz_channel_plan_valid(
    uint8_t primary, uint8_t width_mhz, uint8_t center0, uint8_t center1)
{
    uint8_t distance;
    if (!wifi_6ghz_primary_valid(primary)) return 0;
    if (width_mhz == RIN_WIFI_CHANNEL_WIDTH_20_MHZ)
        return center0 == primary && center1 == 0u;
    if (width_mhz == RIN_WIFI_CHANNEL_WIDTH_40_MHZ) {
        if (center0 < 3u || center0 > 227u ||
            (uint8_t)(center0 - 3u) % 8u != 0u || center1 != 0u)
            return 0;
        distance = primary > center0 ? (uint8_t)(primary - center0)
                                     : (uint8_t)(center0 - primary);
        return distance == 2u;
    }
    if (width_mhz == RIN_WIFI_CHANNEL_WIDTH_80_MHZ) {
        if (center0 < 7u || center0 > 215u ||
            (uint8_t)(center0 - 7u) % 16u != 0u || center1 != 0u)
            return 0;
        distance = primary > center0 ? (uint8_t)(primary - center0)
                                     : (uint8_t)(center0 - primary);
        return distance == 2u || distance == 6u;
    }
    if (width_mhz == RIN_WIFI_CHANNEL_WIDTH_160_MHZ) {
        uint8_t expected_center0;
        if (center1 < 15u || center1 > 207u ||
            (uint8_t)(center1 - 15u) % 32u != 0u)
            return 0;
        expected_center0 = primary < center1
                               ? (uint8_t)(center1 - 8u)
                               : (uint8_t)(center1 + 8u);
        if (center0 != expected_center0) return 0;
        distance = primary > center0 ? (uint8_t)(primary - center0)
                                     : (uint8_t)(center0 - primary);
        return distance == 2u || distance == 6u;
    }
    return 0;
}

/* In an EHT 320 MHz operation, CCFS0 names the primary 160 MHz segment and
 * CCFS1 names one of the six 320 MHz channel centres. */
static int wifi_6ghz_eht_320_channel_plan_valid(
    uint8_t primary, uint8_t center0, uint8_t center1)
{
    uint8_t distance;
    if (!wifi_6ghz_primary_valid(primary) || center1 < 31u ||
        center1 > 191u || (uint8_t)(center1 - 31u) % 32u != 0u ||
        (center0 != (uint8_t)(center1 - 16u) &&
         center0 != (uint8_t)(center1 + 16u))) {
        return 0;
    }
    distance = primary > center0 ? (uint8_t)(primary - center0)
                                 : (uint8_t)(center0 - primary);
    return distance == 2u || distance == 6u || distance == 10u ||
           distance == 14u;
}

static int wifi_6ghz_regulatory_info_valid(uint8_t regulatory_info)
{
    return regulatory_info <= 4u || regulatory_info == 7u ||
           regulatory_info == 8u;
}

static RinWifiDiscoveryStatus wifi_parse_he_operation(
    const uint8_t* bytes, uint32_t size, uint8_t* has_6ghz_out,
    uint8_t* primary_out, uint8_t* width_out, uint8_t* center0_out,
    uint8_t* center1_out, uint8_t* regulatory_info_out,
    uint8_t* minimum_rate_out, uint8_t* duplicate_beacon_out)
{
    uint32_t parameters;
    uint32_t expected_size = WIFI_HE_OPERATION_BASE_SIZE;
    uint32_t cursor = WIFI_HE_OPERATION_BASE_SIZE;
    uint8_t control;
    uint8_t width_code;
    uint8_t width_mhz;
    uint8_t regulatory_info;

    if (bytes == NULL || has_6ghz_out == NULL || primary_out == NULL ||
        width_out == NULL || center0_out == NULL || center1_out == NULL ||
        regulatory_info_out == NULL || minimum_rate_out == NULL ||
        duplicate_beacon_out == NULL ||
        size < WIFI_HE_OPERATION_BASE_SIZE ||
        bytes[0] != WIFI_IE_EXTENSION_HE_OPERATION)
        return RIN_WIFI_DISCOVERY_MALFORMED;
    parameters = wifi_read_u32(bytes + 1u);
    if ((parameters & WIFI_HE_OPERATION_VHT_INFO) != 0u)
        expected_size += 3u;
    if ((parameters & WIFI_HE_OPERATION_CO_HOSTED_BSS) != 0u)
        expected_size += 1u;
    if ((parameters & WIFI_HE_OPERATION_6GHZ_INFO) != 0u)
        expected_size += WIFI_HE_6GHZ_OPERATION_SIZE;
    if (size != expected_size) return RIN_WIFI_DISCOVERY_MALFORMED;
    if ((parameters & WIFI_HE_OPERATION_6GHZ_INFO) == 0u) {
        *has_6ghz_out = 0u;
        return RIN_WIFI_DISCOVERY_OK;
    }
    if ((parameters & WIFI_HE_OPERATION_VHT_INFO) != 0u) cursor += 3u;
    if ((parameters & WIFI_HE_OPERATION_CO_HOSTED_BSS) != 0u) ++cursor;
    control = bytes[cursor + 1u];
    if ((control & WIFI_HE_6GHZ_RESERVED_MASK) != 0u)
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    width_code = control & WIFI_HE_6GHZ_WIDTH_MASK;
    width_mhz = width_code == 0u
                    ? RIN_WIFI_CHANNEL_WIDTH_20_MHZ
                    : (width_code == 1u
                           ? RIN_WIFI_CHANNEL_WIDTH_40_MHZ
                           : (width_code == 2u
                                  ? RIN_WIFI_CHANNEL_WIDTH_80_MHZ
                                  : RIN_WIFI_CHANNEL_WIDTH_160_MHZ));
    regulatory_info =
        (uint8_t)((control & WIFI_HE_6GHZ_REGULATORY_MASK) >> 3u);
    if (!wifi_6ghz_regulatory_info_valid(regulatory_info) ||
        !wifi_6ghz_primary_valid(bytes[cursor]))
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    *has_6ghz_out = 1u;
    *primary_out = bytes[cursor];
    *regulatory_info_out = regulatory_info;
    *minimum_rate_out = bytes[cursor + 4u];
    if ((control & WIFI_HE_6GHZ_DUPLICATE_BEACON) != 0u) {
        if (width_code != 0u) return RIN_WIFI_DISCOVERY_UNSUPPORTED;
        *width_out = RIN_WIFI_CHANNEL_WIDTH_UNKNOWN;
        *center0_out = 0u;
        *center1_out = 0u;
        *duplicate_beacon_out = 1u;
        return RIN_WIFI_DISCOVERY_OK;
    }
    if (!wifi_6ghz_channel_plan_valid(bytes[cursor], width_mhz,
                                      bytes[cursor + 2u],
                                      bytes[cursor + 3u]))
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    *width_out = width_mhz;
    *center0_out = bytes[cursor + 2u];
    *center1_out = bytes[cursor + 3u];
    return RIN_WIFI_DISCOVERY_OK;
}

static RinWifiDiscoveryStatus wifi_parse_eht_operation(
    const uint8_t* bytes, uint32_t size, WifiEhtMloEvidence* evidence_out)
{
    uint8_t parameters;
    uint32_t expected_size = 1u + WIFI_EHT_OPERATION_BASE_SIZE;
    uint32_t cursor = expected_size;
    uint8_t width_code;

    if (bytes == NULL || evidence_out == NULL ||
        size < 1u + WIFI_EHT_OPERATION_BASE_SIZE ||
        bytes[0] != WIFI_IE_EXTENSION_EHT_OPERATION)
        return RIN_WIFI_DISCOVERY_MALFORMED;
    parameters = bytes[1];
    if ((parameters & ~WIFI_EHT_OPERATION_KNOWN_PARAMETERS) != 0u)
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    if ((parameters & WIFI_EHT_OPERATION_INFO_PRESENT) != 0u)
        expected_size += WIFI_EHT_OPERATION_INFO_SIZE;
    if ((parameters & WIFI_EHT_OPERATION_DISABLED_SUBCHANNELS) != 0u) {
        if ((parameters & WIFI_EHT_OPERATION_INFO_PRESENT) == 0u)
            return RIN_WIFI_DISCOVERY_MALFORMED;
        expected_size += 2u;
    }
    if (size != expected_size) return RIN_WIFI_DISCOVERY_MALFORMED;

    evidence_out->eht_present = 1u;
    if ((parameters & WIFI_EHT_OPERATION_INFO_PRESENT) == 0u)
        return RIN_WIFI_DISCOVERY_OK;
    width_code = bytes[cursor] & WIFI_EHT_OPERATION_WIDTH_MASK;
    if ((bytes[cursor] & ~WIFI_EHT_OPERATION_WIDTH_MASK) != 0u ||
        width_code > WIFI_EHT_OPERATION_WIDTH_320_MHZ)
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    evidence_out->eht_channel_width_mhz =
        width_code == WIFI_EHT_OPERATION_WIDTH_20_MHZ
            ? 20u
            : (width_code == WIFI_EHT_OPERATION_WIDTH_40_MHZ
                   ? 40u
                   : (width_code == WIFI_EHT_OPERATION_WIDTH_80_MHZ
                          ? 80u
                          : (width_code == WIFI_EHT_OPERATION_WIDTH_160_MHZ
                                 ? 160u
                                 : 320u)));
    evidence_out->eht_center_channel0 = bytes[cursor + 1u];
    evidence_out->eht_center_channel1 = bytes[cursor + 2u];
    cursor += WIFI_EHT_OPERATION_INFO_SIZE;
    if ((parameters & WIFI_EHT_OPERATION_DISABLED_SUBCHANNELS) != 0u)
        evidence_out->eht_disabled_subchannel_bitmap =
            wifi_read_u16(bytes + cursor);
    if (evidence_out->eht_channel_width_mhz == 320u &&
        evidence_out->eht_disabled_subchannel_bitmap != 0u)
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    return RIN_WIFI_DISCOVERY_OK;
}

static RinWifiDiscoveryStatus wifi_parse_eht_multi_link(
    const uint8_t* bytes, uint32_t size, WifiEhtMloEvidence* evidence_out)
{
    uint16_t control;
    uint8_t common_length;
    uint32_t required_common = WIFI_MLO_BASIC_COMMON_MINIMUM_SIZE;
    uint32_t cursor;

    if (bytes == NULL || evidence_out == NULL || size < 1u + 2u +
            WIFI_MLO_BASIC_COMMON_MINIMUM_SIZE ||
        bytes[0] != WIFI_IE_EXTENSION_EHT_MULTI_LINK)
        return RIN_WIFI_DISCOVERY_MALFORMED;
    control = wifi_read_u16(bytes + 1u);
    if ((control & WIFI_MLO_CONTROL_TYPE_MASK) != 0u ||
        (control & WIFI_MLO_CONTROL_RESERVED_LOW) != 0u ||
        (control & (uint16_t)~WIFI_MLO_CONTROL_KNOWN_PRESENCE) != 0u ||
        (control & WIFI_MLO_CONTROL_LINK_ID_PRESENT) == 0u)
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    if ((control & WIFI_MLO_CONTROL_LINK_ID_PRESENT) != 0u)
        ++required_common;
    if ((control & WIFI_MLO_CONTROL_BSS_PARAMETER_CHANGE_COUNT) != 0u)
        ++required_common;
    if ((control & WIFI_MLO_CONTROL_MEDIUM_SYNC_DELAY) != 0u)
        required_common += 2u;
    if ((control & WIFI_MLO_CONTROL_EML_CAPABILITIES) != 0u)
        required_common += 2u;
    if ((control & WIFI_MLO_CONTROL_MLD_CAPABILITIES) != 0u)
        required_common += 2u;
    if ((control & WIFI_MLO_CONTROL_MLD_ID) != 0u)
        ++required_common;
    if ((control & WIFI_MLO_CONTROL_EXTENDED_MLD_CAPABILITIES) != 0u)
        required_common += 2u;
    if ((control & WIFI_MLO_CONTROL_ENHANCED_CRITICAL_UPDATE) != 0u)
        ++required_common;
    common_length = bytes[3];
    if (common_length != required_common ||
        size != 3u + (uint32_t)common_length ||
        !wifi_bssid_valid(bytes + 4u))
        return RIN_WIFI_DISCOVERY_MALFORMED;

    cursor = 10u;
    if ((bytes[cursor] & WIFI_MLO_LINK_ID_RESERVED_MASK) != 0u ||
        bytes[cursor] > RIN_WIFI_DISCOVERY_MAX_MLO_LINK_ID)
        return RIN_WIFI_DISCOVERY_MALFORMED;
    evidence_out->mlo_present = 1u;
    evidence_out->mlo_link_id = bytes[cursor];
    for (cursor = 0u; cursor < 6u; ++cursor)
        evidence_out->mld_address[cursor] = bytes[4u + cursor];
    return RIN_WIFI_DISCOVERY_OK;
}

static RinWifiDiscoveryStatus wifi_parse_rsnx(
    const uint8_t* bytes, uint32_t size, uint8_t* sae_h2e_out)
{
    if (bytes == NULL || sae_h2e_out == NULL || size == 0u || size > 16u)
        return RIN_WIFI_DISCOVERY_MALFORMED;
    if (size != (uint32_t)(bytes[0] & WIFI_RSNX_FIELD_LENGTH_MASK) + 1u)
        return RIN_WIFI_DISCOVERY_MALFORMED;
    *sae_h2e_out = (bytes[0] & WIFI_RSNX_SAE_H2E) != 0u ? 1u : 0u;
    return RIN_WIFI_DISCOVERY_OK;
}

static int wifi_rsn_suite(const uint8_t* suite, uint8_t* type_out)
{
    if (suite == NULL || type_out == NULL || suite[0] != 0x00u ||
        suite[1] != 0x0fu || suite[2] != 0xacu)
        return 0;
    *type_out = suite[3];
    return 1;
}

static int wifi_rsn_data_cipher_supported(uint8_t type)
{
    return type == 4u || type == 8u || type == 9u || type == 10u;
}

static int wifi_rsn_group_management_cipher_supported(uint8_t type)
{
    return type == 6u || type == 11u || type == 12u || type == 13u;
}

static RinWifiDiscoveryStatus wifi_parse_rsn(
    const uint8_t* bytes, uint32_t size, uint8_t* security_out,
    uint8_t* pmf_capable_out, uint8_t* pmf_required_out)
{
    uint32_t cursor = 0u;
    uint16_t count;
    uint16_t capabilities = 0u;
    uint8_t suite_type = 0u;
    uint8_t has_psk = 0u;
    uint8_t has_sae = 0u;
    uint32_t index;

    if (bytes == NULL || security_out == NULL || pmf_capable_out == NULL ||
        pmf_required_out == NULL || size < 18u)
        return RIN_WIFI_DISCOVERY_MALFORMED;
    if (wifi_read_u16(bytes) != 1u)
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    cursor = 2u;
    if (!wifi_rsn_suite(bytes + cursor, &suite_type) ||
        !wifi_rsn_data_cipher_supported(suite_type))
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    cursor += WIFI_RSN_SUITE_SIZE;

    if (size - cursor < 2u) return RIN_WIFI_DISCOVERY_MALFORMED;
    count = wifi_read_u16(bytes + cursor);
    cursor += 2u;
    if (count == 0u || count > WIFI_RSN_MAX_PAIRWISE ||
        (uint32_t)count > (size - cursor) / WIFI_RSN_SUITE_SIZE)
        return count > WIFI_RSN_MAX_PAIRWISE
                   ? RIN_WIFI_DISCOVERY_UNSUPPORTED
                   : RIN_WIFI_DISCOVERY_MALFORMED;
    for (index = 0u; index < count; ++index) {
        if (!wifi_rsn_suite(bytes + cursor, &suite_type) ||
            !wifi_rsn_data_cipher_supported(suite_type))
            return RIN_WIFI_DISCOVERY_UNSUPPORTED;
        cursor += WIFI_RSN_SUITE_SIZE;
    }

    if (size - cursor < 2u) return RIN_WIFI_DISCOVERY_MALFORMED;
    count = wifi_read_u16(bytes + cursor);
    cursor += 2u;
    if (count == 0u || count > WIFI_RSN_MAX_AKM ||
        (uint32_t)count > (size - cursor) / WIFI_RSN_SUITE_SIZE)
        return count > WIFI_RSN_MAX_AKM
                   ? RIN_WIFI_DISCOVERY_UNSUPPORTED
                   : RIN_WIFI_DISCOVERY_MALFORMED;
    for (index = 0u; index < count; ++index) {
        if (!wifi_rsn_suite(bytes + cursor, &suite_type))
            return RIN_WIFI_DISCOVERY_UNSUPPORTED;
        if (suite_type == 2u || suite_type == 4u) has_psk = 1u;
        else if (suite_type == 8u || suite_type == 9u) has_sae = 1u;
        else return RIN_WIFI_DISCOVERY_UNSUPPORTED;
        cursor += WIFI_RSN_SUITE_SIZE;
    }

    if (cursor != size) {
        uint16_t pmkid_count;
        if (size - cursor < 2u) return RIN_WIFI_DISCOVERY_MALFORMED;
        capabilities = wifi_read_u16(bytes + cursor);
        cursor += 2u;
        if ((capabilities & WIFI_RSN_CAPABILITY_MFPR) != 0u &&
            (capabilities & WIFI_RSN_CAPABILITY_MFPC) == 0u)
            return RIN_WIFI_DISCOVERY_MALFORMED;
        if (cursor != size) {
            if (size - cursor < 2u) return RIN_WIFI_DISCOVERY_MALFORMED;
            pmkid_count = wifi_read_u16(bytes + cursor);
            cursor += 2u;
            if (pmkid_count > WIFI_RSN_MAX_PMKIDS ||
                (uint32_t)pmkid_count > (size - cursor) / 16u)
                return pmkid_count > WIFI_RSN_MAX_PMKIDS
                           ? RIN_WIFI_DISCOVERY_UNSUPPORTED
                           : RIN_WIFI_DISCOVERY_MALFORMED;
            cursor += (uint32_t)pmkid_count * 16u;
            if (cursor != size) {
                if (size - cursor != WIFI_RSN_SUITE_SIZE ||
                    !wifi_rsn_suite(bytes + cursor, &suite_type) ||
                    !wifi_rsn_group_management_cipher_supported(suite_type))
                    return RIN_WIFI_DISCOVERY_UNSUPPORTED;
                cursor += WIFI_RSN_SUITE_SIZE;
            }
        }
    }
    if (cursor != size || (!has_psk && !has_sae))
        return RIN_WIFI_DISCOVERY_MALFORMED;
    *pmf_capable_out =
        (capabilities & WIFI_RSN_CAPABILITY_MFPC) != 0u ? 1u : 0u;
    *pmf_required_out =
        (capabilities & WIFI_RSN_CAPABILITY_MFPR) != 0u ? 1u : 0u;
    if (has_sae && *pmf_capable_out == 0u)
        return RIN_WIFI_DISCOVERY_UNSUPPORTED;
    *security_out = has_sae
        ? (has_psk ? RIN_WIFI_SECURITY_WPA2_WPA3_TRANSITION
                   : RIN_WIFI_SECURITY_WPA3_SAE)
        : RIN_WIFI_SECURITY_WPA2_PSK;
    return RIN_WIFI_DISCOVERY_OK;
}

static RinWifiDiscoveryStatus wifi_discovery_parse(
    const uint8_t* frame, uint32_t frame_size, int32_t signal_dbm,
    RinWifiDiscoveredNetworkV1* network_out,
    WifiEhtMloEvidence* evidence_out)
{
    RinWifiDiscoveredNetworkV1 candidate;
    WifiEhtMloEvidence evidence;
    uint16_t frame_control;
    uint16_t sequence_control;
    uint32_t cursor;
    uint32_t ie_count = 0u;
    uint8_t saw_ssid = 0u;
    uint8_t saw_channel = 0u;
    uint8_t saw_rsn = 0u;
    uint8_t saw_rsnx = 0u;
    uint8_t saw_he_operation = 0u;
    uint8_t saw_eht_operation = 0u;
    uint8_t saw_eht_multi_link = 0u;
    uint8_t saw_6ghz = 0u;
    uint8_t saw_6ghz_duplicate_beacon = 0u;
    RinWifiDiscoveryStatus status = RIN_WIFI_DISCOVERY_OK;

    if (network_out == NULL || evidence_out == NULL || frame == NULL ||
        frame_size < WIFI_DISCOVERY_MINIMUM_SIZE ||
        frame_size > RIN_WIFI_DISCOVERY_MAX_FRAME_SIZE ||
        signal_dbm < -127 || signal_dbm > 0)
        return RIN_WIFI_DISCOVERY_INVALID_ARGUMENT;
    wifi_zero(&candidate, sizeof(candidate));
    wifi_zero(&evidence, sizeof(evidence));
    candidate.struct_size = sizeof(candidate);
    candidate.version = RIN_WIFI_DISCOVERY_VERSION;
    candidate.signal_dbm = (int8_t)signal_dbm;

    frame_control = wifi_read_u16(frame);
    candidate.frame_subtype =
        (uint8_t)((frame_control >> WIFI_FRAME_SUBTYPE_SHIFT) & 0x0fu);
    if ((frame_control & WIFI_FRAME_TYPE_MASK) != 0u ||
        (candidate.frame_subtype != WIFI_FRAME_SUBTYPE_BEACON &&
         candidate.frame_subtype != WIFI_FRAME_SUBTYPE_PROBE_RESPONSE) ||
        (frame_control & WIFI_FRAME_REJECTED_FLAGS) != 0u) {
        status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
        goto done;
    }
    if (!wifi_bssid_valid(frame + 16u) ||
        !wifi_equal(frame + 10u, frame + 16u, 6u)) {
        status = RIN_WIFI_DISCOVERY_MALFORMED;
        goto done;
    }
    for (cursor = 0u; cursor < 6u; ++cursor)
        candidate.bssid[cursor] = frame[16u + cursor];
    sequence_control = wifi_read_u16(frame + 22u);
    if ((sequence_control & 0x000fu) != 0u) {
        status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
        goto done;
    }
    candidate.sequence_number = (uint16_t)(sequence_control >> 4u);
    candidate.timestamp_tsf = wifi_read_u64(frame + 24u);
    candidate.beacon_interval_tu = wifi_read_u16(frame + 32u);
    candidate.capability = wifi_read_u16(frame + 34u);
    if (candidate.beacon_interval_tu == 0u) {
        status = RIN_WIFI_DISCOVERY_MALFORMED;
        goto done;
    }

    cursor = WIFI_DISCOVERY_MINIMUM_SIZE;
    while (cursor < frame_size) {
        uint8_t id;
        uint8_t length;
        const uint8_t* value;
        if (frame_size - cursor < 2u) {
            status = RIN_WIFI_DISCOVERY_MALFORMED;
            goto done;
        }
        if (++ie_count > RIN_WIFI_DISCOVERY_MAX_INFORMATION_ELEMENTS) {
            status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
            goto done;
        }
        id = frame[cursor++];
        length = frame[cursor++];
        if ((uint32_t)length > frame_size - cursor) {
            status = RIN_WIFI_DISCOVERY_MALFORMED;
            goto done;
        }
        value = frame + cursor;
        if (id == WIFI_IE_SSID) {
            uint32_t index;
            if (saw_ssid != 0u) {
                status = RIN_WIFI_DISCOVERY_AMBIGUOUS;
                goto done;
            }
            if (length > RIN_WIFI_DISCOVERY_MAX_SSID_SIZE) {
                status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
                goto done;
            }
            saw_ssid = 1u;
            candidate.ssid_length = length;
            candidate.hidden = length == 0u ? 1u : 0u;
            for (index = 0u; index < length; ++index)
                candidate.ssid[index] = value[index];
        } else if (id == WIFI_IE_DS_PARAMETER) {
            if (saw_channel != 0u) {
                status = RIN_WIFI_DISCOVERY_AMBIGUOUS;
                goto done;
            }
            if (length != 1u || value[0] == 0u || value[0] > 233u) {
                status = RIN_WIFI_DISCOVERY_MALFORMED;
                goto done;
            }
            saw_channel = 1u;
            candidate.channel = value[0];
        } else if (id == WIFI_IE_RSN) {
            if (saw_rsn != 0u) {
                status = RIN_WIFI_DISCOVERY_AMBIGUOUS;
                goto done;
            }
            saw_rsn = 1u;
            status = wifi_parse_rsn(value, length, &candidate.security,
                                    &candidate.pmf_capable,
                                    &candidate.pmf_required);
            if (status != RIN_WIFI_DISCOVERY_OK) goto done;
        } else if (id == WIFI_IE_RSNX) {
            if (saw_rsnx != 0u) {
                status = RIN_WIFI_DISCOVERY_AMBIGUOUS;
                goto done;
            }
            saw_rsnx = 1u;
            status = wifi_parse_rsnx(
                value, length, &candidate.sae_h2e);
            if (status != RIN_WIFI_DISCOVERY_OK) goto done;
        } else if (id == WIFI_IE_EXTENSION) {
            if (length == 0u) {
                status = RIN_WIFI_DISCOVERY_MALFORMED;
                goto done;
            }
            if (value[0] == WIFI_IE_EXTENSION_HE_OPERATION) {
                if (saw_he_operation != 0u) {
                    status = RIN_WIFI_DISCOVERY_AMBIGUOUS;
                    goto done;
                }
                saw_he_operation = 1u;
                status = wifi_parse_he_operation(
                    value, length, &saw_6ghz, &candidate.channel,
                    &candidate.channel_width_mhz,
                    &candidate.center_channel0,
                    &candidate.center_channel1, &candidate.regulatory_info,
                    &candidate.minimum_rate_mbps,
                    &saw_6ghz_duplicate_beacon);
                if (status != RIN_WIFI_DISCOVERY_OK) goto done;
            } else if (value[0] == WIFI_IE_EXTENSION_EHT_OPERATION) {
                if (saw_eht_operation != 0u) {
                    status = RIN_WIFI_DISCOVERY_AMBIGUOUS;
                    goto done;
                }
                saw_eht_operation = 1u;
                status = wifi_parse_eht_operation(value, length, &evidence);
                if (status != RIN_WIFI_DISCOVERY_OK) goto done;
            } else if (value[0] == WIFI_IE_EXTENSION_EHT_MULTI_LINK) {
                if (saw_eht_multi_link != 0u) {
                    status = RIN_WIFI_DISCOVERY_AMBIGUOUS;
                    goto done;
                }
                saw_eht_multi_link = 1u;
                status = wifi_parse_eht_multi_link(value, length, &evidence);
                if (status != RIN_WIFI_DISCOVERY_OK) goto done;
            }
        }
        cursor += length;
    }
    if (cursor != frame_size || saw_ssid == 0u ||
        (saw_channel == 0u && saw_6ghz == 0u)) {
        status = RIN_WIFI_DISCOVERY_MALFORMED;
        goto done;
    }
    if (saw_channel != 0u && saw_6ghz != 0u) {
        status = RIN_WIFI_DISCOVERY_AMBIGUOUS;
        goto done;
    }
    if (saw_rsn != 0u) {
        if ((candidate.capability & WIFI_CAPABILITY_PRIVACY) == 0u) {
            status = RIN_WIFI_DISCOVERY_MALFORMED;
            goto done;
        }
    } else {
        if ((candidate.capability & WIFI_CAPABILITY_PRIVACY) != 0u) {
            status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
            goto done;
        }
        candidate.security = RIN_WIFI_SECURITY_OPEN;
    }
    if (saw_rsnx != 0u && saw_rsn == 0u) {
        status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
        goto done;
    }
    if (candidate.sae_h2e != 0u &&
        candidate.security != RIN_WIFI_SECURITY_WPA3_SAE &&
        candidate.security != RIN_WIFI_SECURITY_WPA2_WPA3_TRANSITION) {
        status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
        goto done;
    }
    if ((saw_6ghz_duplicate_beacon != 0u &&
         (saw_eht_operation == 0u ||
          evidence.eht_channel_width_mhz != 320u)) ||
        (evidence.eht_channel_width_mhz == 320u &&
         (saw_6ghz == 0u || saw_6ghz_duplicate_beacon == 0u ||
          !wifi_6ghz_eht_320_channel_plan_valid(
              candidate.channel, evidence.eht_center_channel0,
              evidence.eht_center_channel1))) ||
        (saw_eht_multi_link != 0u && saw_eht_operation == 0u)) {
        status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
        goto done;
    }
    if (saw_6ghz != 0u) {
        candidate.band = RIN_WIFI_BAND_6_GHZ;
        if (candidate.security != RIN_WIFI_SECURITY_WPA3_SAE ||
            candidate.pmf_capable == 0u || candidate.pmf_required == 0u ||
            saw_rsnx == 0u || candidate.sae_h2e == 0u) {
            status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
            goto done;
        }
    } else if (candidate.channel >= 1u && candidate.channel <= 14u) {
        candidate.band = RIN_WIFI_BAND_2_4_GHZ;
    } else if (wifi_5ghz_channel_valid(candidate.channel)) {
        candidate.band = RIN_WIFI_BAND_5_GHZ;
    } else {
        status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
        goto done;
    }
    candidate.information_element_count = ie_count;
    *network_out = candidate;
    *evidence_out = evidence;
done:
    wifi_zero(&candidate, sizeof(candidate));
    wifi_zero(&evidence, sizeof(evidence));
    return status;
}

RinWifiDiscoveryStatus rin_wifi_discovery_admit(
    const uint8_t* frame, uint32_t frame_size, int32_t signal_dbm,
    RinWifiDiscoveredNetworkV1* network_out)
{
    RinWifiDiscoveredNetworkV1 candidate;
    WifiEhtMloEvidence evidence;
    RinWifiDiscoveryStatus status;

    if (network_out == NULL) return RIN_WIFI_DISCOVERY_INVALID_ARGUMENT;
    if (wifi_ranges_overlap(frame, frame_size, network_out,
                            sizeof(*network_out)))
        return RIN_WIFI_DISCOVERY_INVALID_ARGUMENT;
    wifi_zero(network_out, sizeof(*network_out));
    if (frame == NULL || frame_size < WIFI_DISCOVERY_MINIMUM_SIZE ||
        frame_size > RIN_WIFI_DISCOVERY_MAX_FRAME_SIZE ||
        signal_dbm < -127 || signal_dbm > 0)
        return RIN_WIFI_DISCOVERY_INVALID_ARGUMENT;

    wifi_zero(&candidate, sizeof(candidate));
    wifi_zero(&evidence, sizeof(evidence));
    status = wifi_discovery_parse(frame, frame_size, signal_dbm, &candidate,
                                  &evidence);
    if (status == RIN_WIFI_DISCOVERY_OK &&
        (evidence.eht_channel_width_mhz == 320u ||
         evidence.mlo_present != 0u))
        status = RIN_WIFI_DISCOVERY_UNSUPPORTED;
    if (status == RIN_WIFI_DISCOVERY_OK)
        *network_out = candidate;
    else
        wifi_zero(network_out, sizeof(*network_out));
    wifi_zero(&candidate, sizeof(candidate));
    wifi_zero(&evidence, sizeof(evidence));
    return status;
}

RinWifiDiscoveryStatus rin_wifi_discovery_admit_v2(
    const uint8_t* frame, uint32_t frame_size, int32_t signal_dbm,
    RinWifiDiscoveredNetworkV2* network_out)
{
    RinWifiDiscoveredNetworkV1 candidate;
    WifiEhtMloEvidence evidence;
    RinWifiDiscoveryStatus status;
    uint32_t index;

    if (network_out == NULL) return RIN_WIFI_DISCOVERY_INVALID_ARGUMENT;
    if (wifi_ranges_overlap(frame, frame_size, network_out,
                            sizeof(*network_out)))
        return RIN_WIFI_DISCOVERY_INVALID_ARGUMENT;
    wifi_zero(network_out, sizeof(*network_out));
    if (frame == NULL || frame_size < WIFI_DISCOVERY_MINIMUM_SIZE ||
        frame_size > RIN_WIFI_DISCOVERY_MAX_FRAME_SIZE ||
        signal_dbm < -127 || signal_dbm > 0)
        return RIN_WIFI_DISCOVERY_INVALID_ARGUMENT;

    wifi_zero(&candidate, sizeof(candidate));
    wifi_zero(&evidence, sizeof(evidence));
    status = wifi_discovery_parse(frame, frame_size, signal_dbm, &candidate,
                                  &evidence);
    if (status == RIN_WIFI_DISCOVERY_OK) {
        network_out->struct_size = sizeof(*network_out);
        network_out->version = RIN_WIFI_DISCOVERY_V2_VERSION;
        network_out->network = candidate;
        network_out->eht_channel_width_mhz = evidence.eht_channel_width_mhz;
        network_out->eht_disabled_subchannel_bitmap =
            evidence.eht_disabled_subchannel_bitmap;
        network_out->eht_present = evidence.eht_present;
        network_out->eht_center_channel0 = evidence.eht_center_channel0;
        network_out->eht_center_channel1 = evidence.eht_center_channel1;
        network_out->mlo_present = evidence.mlo_present;
        network_out->mlo_link_id = evidence.mlo_link_id;
        for (index = 0u; index < 6u; ++index)
            network_out->mld_address[index] = evidence.mld_address[index];
    } else {
        wifi_zero(network_out, sizeof(*network_out));
    }
    wifi_zero(&candidate, sizeof(candidate));
    wifi_zero(&evidence, sizeof(evidence));
    return status;
}
