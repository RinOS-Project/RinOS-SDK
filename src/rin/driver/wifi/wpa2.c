/* SPDX-License-Identifier: MIT */

#include <rin/driver/wifi/wpa2.h>

#include <libs/rintls/crypto/aes.h>
#include <libs/rintls/crypto/sha1.h>

#define WPA2_SHA1_SIZE 20u
#define WPA2_SHA1_BLOCK_SIZE 64u
#define WPA2_PBKDF2_ITERATIONS 4096u
#define WPA2_EAPOL_KEY_MIN_SIZE 99u
#define WPA2_EAPOL_MIC_OFFSET 81u
#define WPA2_EAPOL_KEY_DATA_LENGTH_OFFSET 97u
#define WPA2_EAPOL_KEY_DESCRIPTOR_RSN 2u
#define WPA2_EAPOL_KEY_MIC UINT16_C(0x0100)
#define WPA2_EAPOL_KEY_DESCRIPTOR_VERSION_MASK UINT16_C(0x0007)
#define WPA2_EAPOL_KEY_DESCRIPTOR_VERSION_HMAC_SHA1_AES 2u
#define WPA2_AES_KEY_WRAP_BLOCK_SIZE 8u

typedef struct WifiWpa2HmacSha1 {
    sha1_ctx_t inner;
    sha1_ctx_t outer;
} WifiWpa2HmacSha1;

static void wpa2_zero(void* destination, uint32_t size) {
    volatile uint8_t* bytes = (volatile uint8_t*)destination;
    while (size-- != 0u) *bytes++ = 0u;
}

static void wpa2_copy(void* destination, const void* source, uint32_t size) {
    uint8_t* output = (uint8_t*)destination;
    const uint8_t* input = (const uint8_t*)source;
    uint32_t index;
    for (index = 0u; index < size; ++index) output[index] = input[index];
}

static int wpa2_equal(const uint8_t* left, const uint8_t* right,
                      uint32_t size) {
    uint8_t difference = 0u;
    uint32_t index;
    for (index = 0u; index < size; ++index)
        difference = (uint8_t)(difference | (left[index] ^ right[index]));
    return difference == 0u;
}

static int wpa2_compare(const uint8_t* left, const uint8_t* right,
                        uint32_t size) {
    uint32_t index;
    for (index = 0u; index < size; ++index) {
        if (left[index] < right[index]) return -1;
        if (left[index] > right[index]) return 1;
    }
    return 0;
}

static int wpa2_ranges_overlap(const void* left, uint32_t left_size,
                               const void* right, uint32_t right_size) {
    uintptr_t left_begin;
    uintptr_t right_begin;
    if (!left || !right || left_size == 0u || right_size == 0u) return 0;
    left_begin = (uintptr_t)left;
    right_begin = (uintptr_t)right;
    if (left_begin > UINTPTR_MAX - left_size ||
        right_begin > UINTPTR_MAX - right_size)
        return 1;
    return left_begin < right_begin + right_size &&
           right_begin < left_begin + left_size;
}

static int wpa2_zero_bytes(const uint8_t* bytes, uint32_t size) {
    uint8_t combined = 0u;
    uint32_t index;
    for (index = 0u; index < size; ++index)
        combined = (uint8_t)(combined | bytes[index]);
    return combined == 0u;
}

static uint16_t wpa2_read_be16(const uint8_t* bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8u) | bytes[1]);
}

static void wpa2_write_be32(uint8_t bytes[4], uint32_t value) {
    bytes[0] = (uint8_t)(value >> 24u);
    bytes[1] = (uint8_t)(value >> 16u);
    bytes[2] = (uint8_t)(value >> 8u);
    bytes[3] = (uint8_t)value;
}

static void wpa2_xor_be64(uint8_t bytes[8], uint64_t value) {
    uint32_t index;
    for (index = 0u; index < 8u; ++index) {
        bytes[7u - index] ^= (uint8_t)value;
        value >>= 8u;
    }
}

static void wpa2_hmac_sha1_init(WifiWpa2HmacSha1* context,
                                const uint8_t* key, uint32_t key_size) {
    uint8_t key_block[WPA2_SHA1_BLOCK_SIZE];
    uint8_t inner_pad[WPA2_SHA1_BLOCK_SIZE];
    uint8_t outer_pad[WPA2_SHA1_BLOCK_SIZE];
    uint32_t index;
    wpa2_zero(key_block, sizeof(key_block));
    wpa2_copy(key_block, key, key_size);
    for (index = 0u; index < sizeof(key_block); ++index) {
        inner_pad[index] = (uint8_t)(key_block[index] ^ UINT8_C(0x36));
        outer_pad[index] = (uint8_t)(key_block[index] ^ UINT8_C(0x5c));
    }
    sha1_init(&context->inner);
    sha1_update(&context->inner, inner_pad, sizeof(inner_pad));
    sha1_init(&context->outer);
    sha1_update(&context->outer, outer_pad, sizeof(outer_pad));
    wpa2_zero(key_block, sizeof(key_block));
    wpa2_zero(inner_pad, sizeof(inner_pad));
    wpa2_zero(outer_pad, sizeof(outer_pad));
}

static void wpa2_hmac_sha1_update(WifiWpa2HmacSha1* context,
                                  const uint8_t* data, uint32_t size) {
    if (size != 0u) sha1_update(&context->inner, data, size);
}

static void wpa2_hmac_sha1_final(WifiWpa2HmacSha1* context,
                                 uint8_t output[WPA2_SHA1_SIZE]) {
    uint8_t inner[WPA2_SHA1_SIZE];
    sha1_final(&context->inner, inner);
    sha1_update(&context->outer, inner, sizeof(inner));
    sha1_final(&context->outer, output);
    wpa2_zero(inner, sizeof(inner));
    wpa2_zero(context, sizeof(*context));
}

static void wpa2_hmac_sha1(const uint8_t* key, uint32_t key_size,
                           const uint8_t* data, uint32_t data_size,
                           uint8_t output[WPA2_SHA1_SIZE]) {
    WifiWpa2HmacSha1 context;
    wpa2_hmac_sha1_init(&context, key, key_size);
    wpa2_hmac_sha1_update(&context, data, data_size);
    wpa2_hmac_sha1_final(&context, output);
}

static int wpa2_passphrase_valid(const uint8_t* passphrase, uint32_t size) {
    uint32_t index;
    if (!passphrase || size < RIN_WIFI_WPA2_PASSPHRASE_MIN ||
        size > RIN_WIFI_WPA2_PASSPHRASE_MAX)
        return 0;
    for (index = 0u; index < size; ++index)
        if (passphrase[index] < UINT8_C(0x20) ||
            passphrase[index] > UINT8_C(0x7e))
            return 0;
    return 1;
}

static int wpa2_address_valid(const uint8_t address[6]) {
    return address && !wpa2_zero_bytes(address, 6u) &&
           (address[0] & UINT8_C(0x01)) == 0u;
}

static int wpa2_pmk_valid(const RinWifiWpa2PmkV1* pmk) {
    return pmk && pmk->struct_size == sizeof(*pmk) &&
           pmk->version == RIN_WIFI_WPA2_VERSION &&
           !wpa2_zero_bytes(pmk->key, sizeof(pmk->key));
}

static int wpa2_ptk_valid(const RinWifiWpa2PtkV1* ptk) {
    return ptk && ptk->struct_size == sizeof(*ptk) &&
           ptk->version == RIN_WIFI_WPA2_VERSION &&
           !wpa2_zero_bytes(ptk->kck, sizeof(ptk->kck));
}

/* CCMP consumes only the temporal key. The driver-facing view intentionally
 * has no KCK or KEK, so it cannot be repurposed for EAPOL authentication or
 * GTK unwrap. */
static int wpa2_ccmp_key_valid(const RinWifiWpa2PtkV1* ptk) {
    return ptk && ptk->struct_size == sizeof(*ptk) &&
           ptk->version == RIN_WIFI_WPA2_VERSION &&
           !wpa2_zero_bytes(ptk->tk, sizeof(ptk->tk));
}

int rin_wifi_wpa2_derive_pmk(const uint8_t* passphrase,
                             uint32_t passphrase_size,
                             const uint8_t* ssid, uint32_t ssid_size,
                             RinWifiWpa2PmkV1* pmk_out) {
    uint8_t password[RIN_WIFI_WPA2_PASSPHRASE_MAX];
    uint8_t network_name[RIN_WIFI_WPA2_SSID_MAX];
    RinWifiWpa2PmkV1 result;
    uint32_t block;
    if (!pmk_out) return -1;
    wpa2_zero(password, sizeof(password));
    wpa2_zero(network_name, sizeof(network_name));
    wpa2_zero(&result, sizeof(result));
    if (passphrase && passphrase_size <= sizeof(password))
        wpa2_copy(password, passphrase, passphrase_size);
    if (ssid && ssid_size <= sizeof(network_name))
        wpa2_copy(network_name, ssid, ssid_size);
    wpa2_zero(pmk_out, sizeof(*pmk_out));
    if (!wpa2_passphrase_valid(password, passphrase_size) || !ssid ||
        ssid_size == 0u || ssid_size > sizeof(network_name))
        goto fail;

    for (block = 1u; block <= 2u; ++block) {
        WifiWpa2HmacSha1 context;
        uint8_t counter[4];
        uint8_t digest[WPA2_SHA1_SIZE];
        uint8_t accumulator[WPA2_SHA1_SIZE];
        uint32_t iteration;
        uint32_t index;
        uint32_t output_offset = (block - 1u) * WPA2_SHA1_SIZE;
        uint32_t output_size = block == 1u ? WPA2_SHA1_SIZE : 12u;
        wpa2_write_be32(counter, block);
        wpa2_hmac_sha1_init(&context, password, passphrase_size);
        wpa2_hmac_sha1_update(&context, network_name, ssid_size);
        wpa2_hmac_sha1_update(&context, counter, sizeof(counter));
        wpa2_hmac_sha1_final(&context, digest);
        wpa2_copy(accumulator, digest, sizeof(accumulator));
        for (iteration = 1u; iteration < WPA2_PBKDF2_ITERATIONS;
             ++iteration) {
            wpa2_hmac_sha1(password, passphrase_size, digest,
                           sizeof(digest), digest);
            for (index = 0u; index < sizeof(accumulator); ++index)
                accumulator[index] ^= digest[index];
        }
        wpa2_copy(result.key + output_offset, accumulator, output_size);
        wpa2_zero(counter, sizeof(counter));
        wpa2_zero(digest, sizeof(digest));
        wpa2_zero(accumulator, sizeof(accumulator));
    }
    result.struct_size = sizeof(result);
    result.version = RIN_WIFI_WPA2_VERSION;
    wpa2_copy(pmk_out, &result, sizeof(result));
    wpa2_zero(password, sizeof(password));
    wpa2_zero(network_name, sizeof(network_name));
    wpa2_zero(&result, sizeof(result));
    return 0;

fail:
    wpa2_zero(password, sizeof(password));
    wpa2_zero(network_name, sizeof(network_name));
    wpa2_zero(&result, sizeof(result));
    return -1;
}

int rin_wifi_wpa2_derive_ptk(const RinWifiWpa2PmkV1* pmk,
                             const uint8_t authenticator_address[6],
                             const uint8_t supplicant_address[6],
                             const uint8_t authenticator_nonce[32],
                             const uint8_t supplicant_nonce[32],
                             RinWifiWpa2PtkV1* ptk_out) {
    static const uint8_t label[] = "Pairwise key expansion";
    RinWifiWpa2PmkV1 master;
    RinWifiWpa2PtkV1 result;
    uint8_t addresses[12];
    uint8_t nonces[64];
    uint8_t digest[WPA2_SHA1_SIZE];
    uint8_t expanded[60];
    uint32_t round;
    if (!ptk_out) return -1;
    wpa2_zero(&master, sizeof(master));
    wpa2_zero(&result, sizeof(result));
    wpa2_zero(addresses, sizeof(addresses));
    wpa2_zero(nonces, sizeof(nonces));
    wpa2_zero(digest, sizeof(digest));
    wpa2_zero(expanded, sizeof(expanded));
    if (pmk) wpa2_copy(&master, pmk, sizeof(master));
    if (authenticator_address && supplicant_address) {
        if (wpa2_compare(authenticator_address, supplicant_address, 6u) < 0) {
            wpa2_copy(addresses, authenticator_address, 6u);
            wpa2_copy(addresses + 6u, supplicant_address, 6u);
        } else {
            wpa2_copy(addresses, supplicant_address, 6u);
            wpa2_copy(addresses + 6u, authenticator_address, 6u);
        }
    }
    if (authenticator_nonce && supplicant_nonce) {
        if (wpa2_compare(authenticator_nonce, supplicant_nonce, 32u) < 0) {
            wpa2_copy(nonces, authenticator_nonce, 32u);
            wpa2_copy(nonces + 32u, supplicant_nonce, 32u);
        } else {
            wpa2_copy(nonces, supplicant_nonce, 32u);
            wpa2_copy(nonces + 32u, authenticator_nonce, 32u);
        }
    }
    wpa2_zero(ptk_out, sizeof(*ptk_out));
    if (!wpa2_pmk_valid(&master) ||
        !wpa2_address_valid(addresses) ||
        !wpa2_address_valid(addresses + 6u) ||
        wpa2_equal(addresses, addresses + 6u, 6u) ||
        wpa2_zero_bytes(nonces, 32u) ||
        wpa2_zero_bytes(nonces + 32u, 32u) ||
        wpa2_equal(nonces, nonces + 32u, 32u))
        goto fail;
    for (round = 0u; round < 3u; ++round) {
        WifiWpa2HmacSha1 context;
        uint8_t separator = 0u;
        uint8_t counter = (uint8_t)round;
        wpa2_hmac_sha1_init(&context, master.key, sizeof(master.key));
        wpa2_hmac_sha1_update(&context, label, sizeof(label) - 1u);
        wpa2_hmac_sha1_update(&context, &separator, 1u);
        wpa2_hmac_sha1_update(&context, addresses, sizeof(addresses));
        wpa2_hmac_sha1_update(&context, nonces, sizeof(nonces));
        wpa2_hmac_sha1_update(&context, &counter, 1u);
        wpa2_hmac_sha1_final(&context, digest);
        wpa2_copy(expanded + round * WPA2_SHA1_SIZE, digest,
                  WPA2_SHA1_SIZE);
    }
    result.struct_size = sizeof(result);
    result.version = RIN_WIFI_WPA2_VERSION;
    wpa2_copy(result.kck, expanded, 16u);
    wpa2_copy(result.kek, expanded + 16u, 16u);
    wpa2_copy(result.tk, expanded + 32u, 16u);
    wpa2_copy(ptk_out, &result, sizeof(result));
    wpa2_zero(&master, sizeof(master));
    wpa2_zero(&result, sizeof(result));
    wpa2_zero(addresses, sizeof(addresses));
    wpa2_zero(nonces, sizeof(nonces));
    wpa2_zero(digest, sizeof(digest));
    wpa2_zero(expanded, sizeof(expanded));
    return 0;

fail:
    wpa2_zero(&master, sizeof(master));
    wpa2_zero(&result, sizeof(result));
    wpa2_zero(addresses, sizeof(addresses));
    wpa2_zero(nonces, sizeof(nonces));
    wpa2_zero(digest, sizeof(digest));
    wpa2_zero(expanded, sizeof(expanded));
    return -1;
}

static int wpa2_eapol_valid(const uint8_t* frame, uint32_t size) {
    uint16_t body_size;
    uint16_t key_info;
    uint16_t key_data_size;
    if (!frame || size < WPA2_EAPOL_KEY_MIN_SIZE ||
        size > RIN_WIFI_WPA2_MAX_EAPOL_BYTES ||
        (frame[0] != 1u && frame[0] != 2u) || frame[1] != 3u)
        return 0;
    body_size = wpa2_read_be16(frame + 2u);
    key_info = wpa2_read_be16(frame + 5u);
    key_data_size = wpa2_read_be16(
        frame + WPA2_EAPOL_KEY_DATA_LENGTH_OFFSET);
    return body_size == size - 4u &&
           frame[4] == WPA2_EAPOL_KEY_DESCRIPTOR_RSN &&
           (key_info & WPA2_EAPOL_KEY_DESCRIPTOR_VERSION_MASK) ==
               WPA2_EAPOL_KEY_DESCRIPTOR_VERSION_HMAC_SHA1_AES &&
           (key_info & WPA2_EAPOL_KEY_MIC) != 0u &&
           key_data_size == size - WPA2_EAPOL_KEY_MIN_SIZE;
}

static void wpa2_eapol_mic_unchecked(const RinWifiWpa2PtkV1* ptk,
                                      uint8_t* frame, uint32_t size,
                                      uint8_t output[16]) {
    uint8_t digest[WPA2_SHA1_SIZE];
    wpa2_zero(frame + WPA2_EAPOL_MIC_OFFSET, RIN_WIFI_WPA2_MIC_SIZE);
    wpa2_hmac_sha1(ptk->kck, sizeof(ptk->kck), frame, size, digest);
    wpa2_copy(output, digest, RIN_WIFI_WPA2_MIC_SIZE);
    wpa2_zero(digest, sizeof(digest));
}

int rin_wifi_wpa2_eapol_mic(const RinWifiWpa2PtkV1* ptk,
                            const uint8_t* eapol, uint32_t eapol_size,
                            uint8_t mic_out[RIN_WIFI_WPA2_MIC_SIZE]) {
    RinWifiWpa2PtkV1 key;
    uint8_t frame[RIN_WIFI_WPA2_MAX_EAPOL_BYTES];
    uint8_t mic[RIN_WIFI_WPA2_MIC_SIZE];
    if (!mic_out) return -1;
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(frame, sizeof(frame));
    wpa2_zero(mic, sizeof(mic));
    if (ptk) wpa2_copy(&key, ptk, sizeof(key));
    if (eapol && eapol_size <= sizeof(frame))
        wpa2_copy(frame, eapol, eapol_size);
    wpa2_zero(mic_out, RIN_WIFI_WPA2_MIC_SIZE);
    if (!wpa2_ptk_valid(&key) || !wpa2_eapol_valid(frame, eapol_size))
        goto fail;
    wpa2_eapol_mic_unchecked(&key, frame, eapol_size, mic);
    wpa2_copy(mic_out, mic, sizeof(mic));
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(frame, sizeof(frame));
    wpa2_zero(mic, sizeof(mic));
    return 0;

fail:
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(frame, sizeof(frame));
    wpa2_zero(mic, sizeof(mic));
    return -1;
}

int rin_wifi_wpa2_eapol_verify_mic(const RinWifiWpa2PtkV1* ptk,
                                   const uint8_t* eapol,
                                   uint32_t eapol_size) {
    RinWifiWpa2PtkV1 key;
    uint8_t frame[RIN_WIFI_WPA2_MAX_EAPOL_BYTES];
    uint8_t expected[RIN_WIFI_WPA2_MIC_SIZE];
    uint8_t actual[RIN_WIFI_WPA2_MIC_SIZE];
    int valid = 0;
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(frame, sizeof(frame));
    wpa2_zero(expected, sizeof(expected));
    wpa2_zero(actual, sizeof(actual));
    if (ptk) wpa2_copy(&key, ptk, sizeof(key));
    if (eapol && eapol_size <= sizeof(frame))
        wpa2_copy(frame, eapol, eapol_size);
    if (wpa2_ptk_valid(&key) && wpa2_eapol_valid(frame, eapol_size)) {
        wpa2_copy(expected, frame + WPA2_EAPOL_MIC_OFFSET,
                  sizeof(expected));
        wpa2_eapol_mic_unchecked(&key, frame, eapol_size, actual);
        valid = wpa2_equal(expected, actual, sizeof(expected));
    }
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(frame, sizeof(frame));
    wpa2_zero(expected, sizeof(expected));
    wpa2_zero(actual, sizeof(actual));
    return valid ? 0 : -1;
}

int rin_wifi_wpa2_unwrap_key_data(
    const RinWifiWpa2PtkV1* ptk, const uint8_t* wrapped,
    uint32_t wrapped_size,
    uint8_t plaintext_out[RIN_WIFI_WPA2_MAX_KEY_DATA_BYTES],
    uint32_t* plaintext_size_out) {
    static const uint8_t initial_value[8] = {
        0xa6u, 0xa6u, 0xa6u, 0xa6u, 0xa6u, 0xa6u, 0xa6u, 0xa6u};
    RinWifiWpa2PtkV1 key;
    aes_ctx aes;
    uint8_t ciphertext[RIN_WIFI_WPA2_MAX_WRAPPED_KEY_DATA_BYTES];
    uint8_t plaintext[RIN_WIFI_WPA2_MAX_KEY_DATA_BYTES];
    uint8_t accumulator[8];
    uint8_t block[16];
    uint32_t plaintext_size = 0u;
    uint32_t block_count;
    uint32_t round;
    uint32_t index;
    int valid = 0;

    if (!plaintext_out || !plaintext_size_out ||
        wpa2_ranges_overlap(plaintext_out,
                            RIN_WIFI_WPA2_MAX_KEY_DATA_BYTES,
                            plaintext_size_out,
                            sizeof(*plaintext_size_out)))
        return -1;
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(&aes, sizeof(aes));
    wpa2_zero(ciphertext, sizeof(ciphertext));
    wpa2_zero(plaintext, sizeof(plaintext));
    wpa2_zero(accumulator, sizeof(accumulator));
    wpa2_zero(block, sizeof(block));
    if (ptk) wpa2_copy(&key, ptk, sizeof(key));
    if (wrapped && wrapped_size <= sizeof(ciphertext))
        wpa2_copy(ciphertext, wrapped, wrapped_size);
    wpa2_zero(plaintext_out, RIN_WIFI_WPA2_MAX_KEY_DATA_BYTES);
    *plaintext_size_out = 0u;
    if (!wpa2_ptk_valid(&key) ||
        wpa2_zero_bytes(key.kek, sizeof(key.kek)) || !wrapped ||
        wrapped_size < 24u || wrapped_size > sizeof(ciphertext) ||
        (wrapped_size % WPA2_AES_KEY_WRAP_BLOCK_SIZE) != 0u)
        goto done;

    plaintext_size = wrapped_size - WPA2_AES_KEY_WRAP_BLOCK_SIZE;
    block_count = plaintext_size / WPA2_AES_KEY_WRAP_BLOCK_SIZE;
    wpa2_copy(accumulator, ciphertext, sizeof(accumulator));
    wpa2_copy(plaintext, ciphertext + sizeof(accumulator), plaintext_size);
    if (aes_init(&aes, key.kek, AES128_KEY_SIZE) != 0) goto done;
    for (round = 6u; round-- != 0u;) {
        for (index = block_count; index != 0u; --index) {
            uint64_t counter = (uint64_t)block_count * round + index;
            wpa2_copy(block, accumulator, 8u);
            wpa2_xor_be64(block, counter);
            wpa2_copy(block + 8u,
                      plaintext + (index - 1u) * 8u, 8u);
            aes_decrypt_block(&aes, block, block);
            wpa2_copy(accumulator, block, 8u);
            wpa2_copy(plaintext + (index - 1u) * 8u,
                      block + 8u, 8u);
        }
    }
    if (!wpa2_equal(accumulator, initial_value, sizeof(initial_value)))
        goto done;
    wpa2_copy(plaintext_out, plaintext, plaintext_size);
    *plaintext_size_out = plaintext_size;
    valid = 1;

done:
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(&aes, sizeof(aes));
    wpa2_zero(ciphertext, sizeof(ciphertext));
    wpa2_zero(plaintext, sizeof(plaintext));
    wpa2_zero(accumulator, sizeof(accumulator));
    wpa2_zero(block, sizeof(block));
    return valid ? 0 : -1;
}

#define WPA2_CCMP_AAD_MAX_SIZE 30u
#define WPA2_CCMP_NONCE_SIZE 13u
#define WPA2_CCMP_BLOCK_SIZE AES_BLOCK_SIZE

static uint16_t wpa2_read_le16(const uint8_t bytes[2]) {
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8u));
}

static int wpa2_ccmp_unicast_address_valid(const uint8_t address[6]) {
    return address && !wpa2_zero_bytes(address, 6u) &&
           (address[0] & UINT8_C(0x01)) == 0u;
}

static int wpa2_ccmp_address_valid(const uint8_t address[6]) {
    return address && !wpa2_zero_bytes(address, 6u);
}

static int wpa2_ccmp_build_aad_nonce(
    const uint8_t* header, uint32_t header_size, uint64_t packet_number,
    uint8_t aad[WPA2_CCMP_AAD_MAX_SIZE], uint32_t* aad_size_out,
    uint8_t nonce[WPA2_CCMP_NONCE_SIZE]) {
    uint16_t frame_control;
    uint32_t subtype;
    uint32_t address_4_present;
    uint32_t qos_present;
    uint32_t expected_size;
    uint32_t qos_offset;
    uint32_t aad_size = 22u;
    uint32_t index;

    if (!header || !aad || !aad_size_out || !nonce ||
        packet_number == 0u || packet_number > RIN_WIFI_WPA2_CCMP_PN_MAX ||
        header_size < 24u) {
        return -1;
    }
    frame_control = wpa2_read_le16(header);
    subtype = (frame_control >> 4u) & UINT16_C(0x000f);
    address_4_present = (frame_control & UINT16_C(0x0300)) ==
                        UINT16_C(0x0300);
    qos_present = subtype == 8u;
    expected_size = 24u + (address_4_present ? 6u : 0u) +
                    (qos_present ? 2u : 0u);
    if ((frame_control & UINT16_C(0x0003)) != 0u ||
        (frame_control & UINT16_C(0x000c)) != UINT16_C(0x0008) ||
        (subtype != 0u && subtype != 8u) ||
        (frame_control & UINT16_C(0x8000)) != 0u ||
        (frame_control & UINT16_C(0x4000)) == 0u ||
        header_size != expected_size ||
        !wpa2_ccmp_address_valid(header + 4u) ||
        !wpa2_ccmp_unicast_address_valid(header + 10u) ||
        !wpa2_ccmp_address_valid(header + 16u) ||
        (address_4_present && !wpa2_ccmp_unicast_address_valid(header + 24u))) {
        return -1;
    }

    wpa2_zero(aad, WPA2_CCMP_AAD_MAX_SIZE);
    wpa2_zero(nonce, WPA2_CCMP_NONCE_SIZE);
    aad[0] = (uint8_t)(header[0] & UINT8_C(0x8f));
    aad[1] = (uint8_t)(header[1] & UINT8_C(0xc7));
    wpa2_copy(aad + 2u, header + 4u, 18u);
    aad[20] = (uint8_t)(header[22] & UINT8_C(0x0f));
    if (address_4_present) {
        wpa2_copy(aad + aad_size, header + 24u, 6u);
        aad_size += 6u;
    }
    qos_offset = 24u + (address_4_present ? 6u : 0u);
    if (qos_present) {
        aad[aad_size] = (uint8_t)(header[qos_offset] & UINT8_C(0x0f));
        aad_size += 2u;
        nonce[0] = (uint8_t)(header[qos_offset] & UINT8_C(0x0f));
    }
    wpa2_copy(nonce + 1u, header + 10u, 6u);
    for (index = 0u; index < 6u; ++index) {
        nonce[12u - index] = (uint8_t)packet_number;
        packet_number >>= 8u;
    }
    *aad_size_out = aad_size;
    return 0;
}

static void wpa2_ccmp_mac_block(const aes_ctx* aes,
                                uint8_t authentication[WPA2_CCMP_BLOCK_SIZE],
                                const uint8_t* input, uint32_t input_size) {
    uint8_t block[WPA2_CCMP_BLOCK_SIZE];
    uint32_t index;
    wpa2_zero(block, sizeof(block));
    if (input_size != 0u) wpa2_copy(block, input, input_size);
    for (index = 0u; index < sizeof(block); ++index)
        block[index] ^= authentication[index];
    aes_encrypt_block(aes, block, authentication);
    wpa2_zero(block, sizeof(block));
}

static void wpa2_ccmp_authenticate(
    const aes_ctx* aes, const uint8_t nonce[WPA2_CCMP_NONCE_SIZE],
    const uint8_t* aad, uint32_t aad_size, const uint8_t* plaintext,
    uint32_t plaintext_size, uint8_t tag[RIN_WIFI_WPA2_CCMP_MIC_SIZE]) {
    uint8_t authentication[WPA2_CCMP_BLOCK_SIZE];
    uint8_t block[WPA2_CCMP_BLOCK_SIZE];
    uint32_t offset;
    uint32_t size;
    uint32_t index;

    wpa2_zero(authentication, sizeof(authentication));
    wpa2_zero(block, sizeof(block));
    block[0] = UINT8_C(0x59); /* AAD present, M=8, L=2. */
    wpa2_copy(block + 1u, nonce, WPA2_CCMP_NONCE_SIZE);
    block[14] = (uint8_t)(plaintext_size >> 8u);
    block[15] = (uint8_t)plaintext_size;
    wpa2_ccmp_mac_block(aes, authentication, block, sizeof(block));

    wpa2_zero(block, sizeof(block));
    block[0] = (uint8_t)(aad_size >> 8u);
    block[1] = (uint8_t)aad_size;
    wpa2_copy(block + 2u, aad,
              aad_size < sizeof(block) - 2u ? aad_size : sizeof(block) - 2u);
    wpa2_ccmp_mac_block(aes, authentication, block, sizeof(block));
    offset = sizeof(block) - 2u;
    while (offset < aad_size) {
        size = aad_size - offset;
        if (size > sizeof(block)) size = sizeof(block);
        wpa2_ccmp_mac_block(aes, authentication, aad + offset, size);
        offset += size;
    }

    offset = 0u;
    while (offset < plaintext_size) {
        size = plaintext_size - offset;
        if (size > sizeof(block)) size = sizeof(block);
        wpa2_ccmp_mac_block(aes, authentication, plaintext + offset, size);
        offset += size;
    }
    wpa2_zero(block, sizeof(block));
    block[0] = UINT8_C(0x01); /* L=2 counter block. */
    wpa2_copy(block + 1u, nonce, WPA2_CCMP_NONCE_SIZE);
    aes_encrypt_block(aes, block, block);
    for (index = 0u; index < RIN_WIFI_WPA2_CCMP_MIC_SIZE; ++index)
        tag[index] = (uint8_t)(authentication[index] ^ block[index]);
    wpa2_zero(authentication, sizeof(authentication));
    wpa2_zero(block, sizeof(block));
}

static void wpa2_ccmp_crypt(const aes_ctx* aes,
                            const uint8_t nonce[WPA2_CCMP_NONCE_SIZE],
                            const uint8_t* input, uint32_t input_size,
                            uint8_t* output) {
    uint8_t counter[WPA2_CCMP_BLOCK_SIZE];
    uint8_t stream[WPA2_CCMP_BLOCK_SIZE];
    uint32_t offset = 0u;
    uint32_t counter_value = 1u;
    uint32_t size;
    uint32_t index;

    while (offset < input_size) {
        wpa2_zero(counter, sizeof(counter));
        counter[0] = UINT8_C(0x01); /* L=2 counter block. */
        wpa2_copy(counter + 1u, nonce, WPA2_CCMP_NONCE_SIZE);
        counter[14] = (uint8_t)(counter_value >> 8u);
        counter[15] = (uint8_t)counter_value;
        aes_encrypt_block(aes, counter, stream);
        size = input_size - offset;
        if (size > sizeof(stream)) size = sizeof(stream);
        for (index = 0u; index < size; ++index)
            output[offset + index] = (uint8_t)(input[offset + index] ^
                                               stream[index]);
        offset += size;
        ++counter_value;
    }
    wpa2_zero(counter, sizeof(counter));
    wpa2_zero(stream, sizeof(stream));
}

static int wpa2_ccmp_encrypt_outputs_valid(
    const RinWifiWpa2PtkV1* ptk, const uint8_t* mac_header,
    uint32_t mac_header_size, const uint8_t* plaintext,
    uint32_t plaintext_size, uint8_t* ciphertext_out,
    uint8_t mic_out[RIN_WIFI_WPA2_CCMP_MIC_SIZE]) {
    return ptk && mac_header && plaintext && ciphertext_out && mic_out &&
           plaintext_size != 0u &&
           plaintext_size <= RIN_WIFI_WPA2_CCMP_MAX_PAYLOAD_BYTES &&
           !wpa2_ranges_overlap(ciphertext_out, plaintext_size, plaintext,
                                plaintext_size) &&
           !wpa2_ranges_overlap(ciphertext_out, plaintext_size, mac_header,
                                mac_header_size) &&
           !wpa2_ranges_overlap(ciphertext_out, plaintext_size, ptk,
                                sizeof(*ptk)) &&
           !wpa2_ranges_overlap(ciphertext_out, plaintext_size, mic_out,
                                RIN_WIFI_WPA2_CCMP_MIC_SIZE) &&
           !wpa2_ranges_overlap(mic_out, RIN_WIFI_WPA2_CCMP_MIC_SIZE,
                                plaintext, plaintext_size) &&
           !wpa2_ranges_overlap(mic_out, RIN_WIFI_WPA2_CCMP_MIC_SIZE,
                                mac_header, mac_header_size) &&
           !wpa2_ranges_overlap(mic_out, RIN_WIFI_WPA2_CCMP_MIC_SIZE, ptk,
                                sizeof(*ptk));
}

int rin_wifi_wpa2_ccmp_encrypt(
    const RinWifiWpa2PtkV1* ptk, const uint8_t* mac_header,
    uint32_t mac_header_size, uint64_t packet_number,
    const uint8_t* plaintext, uint32_t plaintext_size,
    uint8_t* ciphertext_out,
    uint8_t mic_out[RIN_WIFI_WPA2_CCMP_MIC_SIZE]) {
    RinWifiWpa2PtkV1 key;
    aes_ctx aes;
    uint8_t aad[WPA2_CCMP_AAD_MAX_SIZE];
    uint8_t nonce[WPA2_CCMP_NONCE_SIZE];
    uint8_t tag[RIN_WIFI_WPA2_CCMP_MIC_SIZE];
    uint32_t aad_size = 0u;
    int valid = 0;

    if (!wpa2_ccmp_encrypt_outputs_valid(
            ptk, mac_header, mac_header_size, plaintext, plaintext_size,
            ciphertext_out, mic_out)) {
        return -1;
    }
    wpa2_zero(ciphertext_out, plaintext_size);
    wpa2_zero(mic_out, RIN_WIFI_WPA2_CCMP_MIC_SIZE);
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(&aes, sizeof(aes));
    wpa2_zero(aad, sizeof(aad));
    wpa2_zero(nonce, sizeof(nonce));
    wpa2_zero(tag, sizeof(tag));
    wpa2_copy(&key, ptk, sizeof(key));
    if (!wpa2_ccmp_key_valid(&key) ||
        wpa2_ccmp_build_aad_nonce(mac_header, mac_header_size,
                                  packet_number, aad, &aad_size, nonce) != 0 ||
        aes_init(&aes, key.tk, AES128_KEY_SIZE) != 0) {
        goto done;
    }
    wpa2_ccmp_authenticate(&aes, nonce, aad, aad_size, plaintext,
                           plaintext_size, tag);
    wpa2_ccmp_crypt(&aes, nonce, plaintext, plaintext_size, ciphertext_out);
    wpa2_copy(mic_out, tag, sizeof(tag));
    valid = 1;

done:
    if (!valid) {
        wpa2_zero(ciphertext_out, plaintext_size);
        wpa2_zero(mic_out, RIN_WIFI_WPA2_CCMP_MIC_SIZE);
    }
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(&aes, sizeof(aes));
    wpa2_zero(aad, sizeof(aad));
    wpa2_zero(nonce, sizeof(nonce));
    wpa2_zero(tag, sizeof(tag));
    return valid ? 0 : -1;
}

int rin_wifi_wpa2_ccmp_decrypt(
    const RinWifiWpa2PtkV1* ptk, const uint8_t* mac_header,
    uint32_t mac_header_size, uint64_t packet_number,
    const uint8_t* ciphertext, uint32_t ciphertext_size,
    const uint8_t mic[RIN_WIFI_WPA2_CCMP_MIC_SIZE],
    uint8_t* plaintext_out) {
    RinWifiWpa2PtkV1 key;
    aes_ctx aes;
    uint8_t aad[WPA2_CCMP_AAD_MAX_SIZE];
    uint8_t nonce[WPA2_CCMP_NONCE_SIZE];
    uint8_t plaintext[RIN_WIFI_WPA2_CCMP_MAX_PAYLOAD_BYTES];
    uint8_t expected[RIN_WIFI_WPA2_CCMP_MIC_SIZE];
    uint8_t received[RIN_WIFI_WPA2_CCMP_MIC_SIZE];
    uint32_t aad_size = 0u;
    int valid = 0;

    if (!ptk || !mac_header || !ciphertext || !mic || !plaintext_out ||
        ciphertext_size == 0u ||
        ciphertext_size > RIN_WIFI_WPA2_CCMP_MAX_PAYLOAD_BYTES ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, ciphertext,
                            ciphertext_size) ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, mic,
                            RIN_WIFI_WPA2_CCMP_MIC_SIZE) ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, mac_header,
                            mac_header_size) ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, ptk,
                            sizeof(*ptk))) {
        return -1;
    }
    wpa2_zero(plaintext_out, ciphertext_size);
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(&aes, sizeof(aes));
    wpa2_zero(aad, sizeof(aad));
    wpa2_zero(nonce, sizeof(nonce));
    wpa2_zero(plaintext, sizeof(plaintext));
    wpa2_zero(expected, sizeof(expected));
    wpa2_zero(received, sizeof(received));
    wpa2_copy(&key, ptk, sizeof(key));
    wpa2_copy(received, mic, sizeof(received));
    if (!wpa2_ccmp_key_valid(&key) ||
        wpa2_ccmp_build_aad_nonce(mac_header, mac_header_size,
                                  packet_number, aad, &aad_size, nonce) != 0 ||
        aes_init(&aes, key.tk, AES128_KEY_SIZE) != 0) {
        goto done;
    }
    wpa2_ccmp_crypt(&aes, nonce, ciphertext, ciphertext_size, plaintext);
    wpa2_ccmp_authenticate(&aes, nonce, aad, aad_size, plaintext,
                           ciphertext_size, expected);
    if (!wpa2_equal(expected, received, sizeof(expected))) goto done;
    wpa2_copy(plaintext_out, plaintext, ciphertext_size);
    valid = 1;

done:
    if (!valid) wpa2_zero(plaintext_out, ciphertext_size);
    wpa2_zero(&key, sizeof(key));
    wpa2_zero(&aes, sizeof(aes));
    wpa2_zero(aad, sizeof(aad));
    wpa2_zero(nonce, sizeof(nonce));
    wpa2_zero(plaintext, sizeof(plaintext));
    wpa2_zero(expected, sizeof(expected));
    wpa2_zero(received, sizeof(received));
    return valid ? 0 : -1;
}

int rin_wifi_wpa2_ccmp_prepare_key(
    RinWifiWpa2PtkV1* ccmp_key_out,
    const uint8_t temporal_key[RIN_WIFI_WPA2_CCMP_TK_SIZE]) {
    RinWifiWpa2PtkV1 candidate;
    if (!ccmp_key_out || !temporal_key ||
        wpa2_ranges_overlap(ccmp_key_out, sizeof(*ccmp_key_out),
                            temporal_key, RIN_WIFI_WPA2_CCMP_TK_SIZE)) {
        return -1;
    }
    wpa2_zero(ccmp_key_out, sizeof(*ccmp_key_out));
    wpa2_zero(&candidate, sizeof(candidate));
    if (wpa2_zero_bytes(temporal_key, RIN_WIFI_WPA2_CCMP_TK_SIZE)) {
        return -1;
    }
    candidate.struct_size = sizeof(candidate);
    candidate.version = RIN_WIFI_WPA2_VERSION;
    wpa2_copy(candidate.tk, temporal_key, sizeof(candidate.tk));
    wpa2_copy(ccmp_key_out, &candidate, sizeof(candidate));
    wpa2_zero(&candidate, sizeof(candidate));
    return 0;
}

static int wpa2_ccmp_state_valid(const RinWifiWpa2CcmpStateV1* state) {
    uint32_t index;
    if (!state || state->struct_size != sizeof(*state) ||
        state->version != RIN_WIFI_WPA2_VERSION ||
        state->association_generation == 0u ||
        state->association_generation == UINT64_MAX ||
        state->association_id == 0u || state->key_id > 3u ||
        !wpa2_ccmp_unicast_address_valid(state->local_address) ||
        !wpa2_ccmp_unicast_address_valid(state->peer_address) ||
        wpa2_equal(state->local_address, state->peer_address, 6u) ||
        !wpa2_zero_bytes(state->reserved0, sizeof(state->reserved0))) {
        return 0;
    }
    for (index = 0u; index < 16u; ++index) {
        if (state->transmit_next_packet_number[index] >
                RIN_WIFI_WPA2_CCMP_PN_MAX ||
            state->receive_last_packet_number[index] >
                RIN_WIFI_WPA2_CCMP_PN_MAX) {
            return 0;
        }
    }
    return 1;
}

static int wpa2_ccmp_state_header_identity(
    const RinWifiWpa2CcmpStateV1* state, const uint8_t* mac_header,
    uint32_t mac_header_size, int transmit, uint32_t* priority_out) {
    uint8_t aad[WPA2_CCMP_AAD_MAX_SIZE];
    uint8_t nonce[WPA2_CCMP_NONCE_SIZE];
    uint32_t aad_size = 0u;
    int valid = 0;

    wpa2_zero(aad, sizeof(aad));
    wpa2_zero(nonce, sizeof(nonce));
    if (state && mac_header && priority_out &&
        wpa2_ccmp_build_aad_nonce(mac_header, mac_header_size, 1u, aad,
                                  &aad_size, nonce) == 0 &&
        (transmit
             ? (wpa2_equal(mac_header + 4u, state->peer_address, 6u) &&
                wpa2_equal(mac_header + 10u, state->local_address, 6u))
             : (wpa2_equal(mac_header + 4u, state->local_address, 6u) &&
                wpa2_equal(mac_header + 10u, state->peer_address, 6u)))) {
        *priority_out = nonce[0];
        valid = 1;
    }
    wpa2_zero(aad, sizeof(aad));
    wpa2_zero(nonce, sizeof(nonce));
    return valid ? 0 : -1;
}

static void wpa2_ccmp_header_encode(uint64_t packet_number,
                                    uint32_t key_id,
                                    uint8_t output[RIN_WIFI_WPA2_CCMP_HEADER_SIZE]) {
    output[0] = (uint8_t)packet_number;
    output[1] = (uint8_t)(packet_number >> 8u);
    output[2] = 0u;
    output[3] = (uint8_t)(UINT8_C(0x20) | (key_id << 6u));
    output[4] = (uint8_t)(packet_number >> 16u);
    output[5] = (uint8_t)(packet_number >> 24u);
    output[6] = (uint8_t)(packet_number >> 32u);
    output[7] = (uint8_t)(packet_number >> 40u);
}

static int wpa2_ccmp_header_decode(
    const uint8_t input[RIN_WIFI_WPA2_CCMP_HEADER_SIZE], uint32_t key_id,
    uint64_t* packet_number_out) {
    uint64_t packet_number;
    if (!input || !packet_number_out || key_id > 3u || input[2] != 0u ||
        input[3] != (uint8_t)(UINT8_C(0x20) | (key_id << 6u))) {
        return -1;
    }
    packet_number = (uint64_t)input[0] | ((uint64_t)input[1] << 8u) |
                    ((uint64_t)input[4] << 16u) |
                    ((uint64_t)input[5] << 24u) |
                    ((uint64_t)input[6] << 32u) |
                    ((uint64_t)input[7] << 40u);
    if (packet_number == 0u || packet_number > RIN_WIFI_WPA2_CCMP_PN_MAX)
        return -1;
    *packet_number_out = packet_number;
    return 0;
}

int rin_wifi_wpa2_ccmp_state_init(
    RinWifiWpa2CcmpStateV1* state, uint64_t association_generation,
    uint32_t association_id, uint32_t key_id,
    const uint8_t local_address[6], const uint8_t peer_address[6]) {
    uint32_t index;
    if (!state) return -1;
    wpa2_zero(state, sizeof(*state));
    if (association_generation == 0u || association_generation == UINT64_MAX ||
        association_id == 0u || key_id > 3u ||
        !wpa2_ccmp_unicast_address_valid(local_address) ||
        !wpa2_ccmp_unicast_address_valid(peer_address) ||
        wpa2_equal(local_address, peer_address, 6u)) {
        return -1;
    }
    state->struct_size = sizeof(*state);
    state->version = RIN_WIFI_WPA2_VERSION;
    state->association_generation = association_generation;
    state->association_id = association_id;
    state->key_id = key_id;
    wpa2_copy(state->local_address, local_address, sizeof(state->local_address));
    wpa2_copy(state->peer_address, peer_address, sizeof(state->peer_address));
    for (index = 0u; index < 16u; ++index)
        state->transmit_next_packet_number[index] = 1u;
    return 0;
}

int rin_wifi_wpa2_ccmp_encrypt_next(
    RinWifiWpa2CcmpStateV1* state, const RinWifiWpa2PtkV1* ptk,
    const uint8_t* mac_header, uint32_t mac_header_size,
    const uint8_t* plaintext, uint32_t plaintext_size,
    uint8_t ccmp_header_out[RIN_WIFI_WPA2_CCMP_HEADER_SIZE],
    uint8_t* ciphertext_out,
    uint8_t mic_out[RIN_WIFI_WPA2_CCMP_MIC_SIZE]) {
    uint64_t packet_number;
    uint32_t priority = 0u;
    int result;

    if (!wpa2_ccmp_encrypt_outputs_valid(
            ptk, mac_header, mac_header_size, plaintext, plaintext_size,
            ciphertext_out, mic_out) || !state || !ccmp_header_out ||
        wpa2_ranges_overlap(ccmp_header_out, RIN_WIFI_WPA2_CCMP_HEADER_SIZE,
                            state, sizeof(*state)) ||
        wpa2_ranges_overlap(ccmp_header_out, RIN_WIFI_WPA2_CCMP_HEADER_SIZE,
                            ptk, sizeof(*ptk)) ||
        wpa2_ranges_overlap(ccmp_header_out, RIN_WIFI_WPA2_CCMP_HEADER_SIZE,
                            mac_header, mac_header_size) ||
        wpa2_ranges_overlap(ccmp_header_out, RIN_WIFI_WPA2_CCMP_HEADER_SIZE,
                            plaintext, plaintext_size) ||
        wpa2_ranges_overlap(ccmp_header_out, RIN_WIFI_WPA2_CCMP_HEADER_SIZE,
                            ciphertext_out, plaintext_size) ||
        wpa2_ranges_overlap(ccmp_header_out, RIN_WIFI_WPA2_CCMP_HEADER_SIZE,
                            mic_out, RIN_WIFI_WPA2_CCMP_MIC_SIZE)) {
        return -1;
    }
    if (wpa2_ranges_overlap(ciphertext_out, plaintext_size, state,
                            sizeof(*state)) ||
        wpa2_ranges_overlap(mic_out, RIN_WIFI_WPA2_CCMP_MIC_SIZE, state,
                            sizeof(*state))) {
        return -1;
    }
    wpa2_zero(ccmp_header_out, RIN_WIFI_WPA2_CCMP_HEADER_SIZE);
    wpa2_zero(ciphertext_out, plaintext_size);
    wpa2_zero(mic_out, RIN_WIFI_WPA2_CCMP_MIC_SIZE);
    if (!wpa2_ccmp_state_valid(state) ||
        wpa2_ccmp_state_header_identity(state, mac_header, mac_header_size,
                                        1, &priority) != 0) {
        return -1;
    }
    packet_number = state->transmit_next_packet_number[priority];
    if (packet_number == 0u) return -1;
    result = rin_wifi_wpa2_ccmp_encrypt(
        ptk, mac_header, mac_header_size, packet_number, plaintext,
        plaintext_size, ciphertext_out, mic_out);
    if (result != 0) return -1;
    wpa2_ccmp_header_encode(packet_number, state->key_id, ccmp_header_out);
    state->transmit_next_packet_number[priority] =
        packet_number == RIN_WIFI_WPA2_CCMP_PN_MAX ? 0u : packet_number + 1u;
    return 0;
}

int rin_wifi_wpa2_ccmp_decrypt_next(
    RinWifiWpa2CcmpStateV1* state, const RinWifiWpa2PtkV1* ptk,
    const uint8_t* mac_header, uint32_t mac_header_size,
    const uint8_t ccmp_header[RIN_WIFI_WPA2_CCMP_HEADER_SIZE],
    const uint8_t* ciphertext, uint32_t ciphertext_size,
    const uint8_t mic[RIN_WIFI_WPA2_CCMP_MIC_SIZE],
    uint8_t* plaintext_out) {
    uint64_t packet_number = 0u;
    uint32_t priority = 0u;
    int result;

    if (!state || !ptk || !mac_header || !ccmp_header || !ciphertext || !mic ||
        !plaintext_out || ciphertext_size == 0u ||
        ciphertext_size > RIN_WIFI_WPA2_CCMP_MAX_PAYLOAD_BYTES ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, state,
                            sizeof(*state)) ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, ptk,
                            sizeof(*ptk)) ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, mac_header,
                            mac_header_size) ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, ccmp_header,
                            RIN_WIFI_WPA2_CCMP_HEADER_SIZE) ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, ciphertext,
                            ciphertext_size) ||
        wpa2_ranges_overlap(plaintext_out, ciphertext_size, mic,
                            RIN_WIFI_WPA2_CCMP_MIC_SIZE)) {
        return -1;
    }
    wpa2_zero(plaintext_out, ciphertext_size);
    if (!wpa2_ccmp_state_valid(state) ||
        wpa2_ccmp_state_header_identity(state, mac_header, mac_header_size,
                                        0, &priority) != 0 ||
        wpa2_ccmp_header_decode(ccmp_header, state->key_id,
                                &packet_number) != 0 ||
        packet_number <= state->receive_last_packet_number[priority]) {
        return -1;
    }
    result = rin_wifi_wpa2_ccmp_decrypt(
        ptk, mac_header, mac_header_size, packet_number, ciphertext,
        ciphertext_size, mic, plaintext_out);
    if (result != 0) return -1;
    state->receive_last_packet_number[priority] = packet_number;
    return 0;
}

void rin_wifi_wpa2_ccmp_state_forget(RinWifiWpa2CcmpStateV1* state) {
    if (state) wpa2_zero(state, sizeof(*state));
}

void rin_wifi_wpa2_forget_pmk(RinWifiWpa2PmkV1* pmk) {
    if (pmk) wpa2_zero(pmk, sizeof(*pmk));
}

void rin_wifi_wpa2_forget_ptk(RinWifiWpa2PtkV1* ptk) {
    if (ptk) wpa2_zero(ptk, sizeof(*ptk));
}
