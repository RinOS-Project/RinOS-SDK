/* SPDX-License-Identifier: MIT */

#include <rin/driver/wifi/wpa2_supplicant.h>

#include <stddef.h>
#include <stdint.h>

#define WPA2_KEY_DESCRIPTOR_RSN 2u
#define WPA2_KEY_LENGTH_CCMP 16u
#define WPA2_MESSAGE_1_KEY_INFO UINT16_C(0x008a)
#define WPA2_MESSAGE_2_KEY_INFO UINT16_C(0x010a)
#define WPA2_MESSAGE_3_KEY_INFO UINT16_C(0x13ca)
#define WPA2_MESSAGE_4_KEY_INFO UINT16_C(0x030a)
#define WPA2_EAPOL_BASE_SIZE 99u
#define WPA2_EAPOL_NONCE_OFFSET 17u
#define WPA2_EAPOL_IV_OFFSET 49u
#define WPA2_EAPOL_RSC_OFFSET 65u
#define WPA2_EAPOL_KEY_ID_OFFSET 73u
#define WPA2_EAPOL_MIC_OFFSET 81u
#define WPA2_EAPOL_KEY_DATA_LENGTH_OFFSET 97u
#define WPA2_EAPOL_KEY_DATA_OFFSET 99u

static void supplicant_zero(void* destination, uint32_t size) {
    volatile uint8_t* bytes = (volatile uint8_t*)destination;
    while (size-- != 0u) *bytes++ = 0u;
}

static void supplicant_copy(void* destination, const void* source,
                            uint32_t size) {
    uint8_t* output = (uint8_t*)destination;
    const uint8_t* input = (const uint8_t*)source;
    uint32_t index;
    for (index = 0u; index < size; ++index) output[index] = input[index];
}

static int supplicant_equal(const void* left, const void* right,
                            uint32_t size) {
    const uint8_t* left_bytes = (const uint8_t*)left;
    const uint8_t* right_bytes = (const uint8_t*)right;
    uint8_t difference = 0u;
    uint32_t index;
    for (index = 0u; index < size; ++index)
        difference = (uint8_t)(difference |
                               (left_bytes[index] ^ right_bytes[index]));
    return difference == 0u;
}

static int supplicant_zero_bytes(const uint8_t* bytes, uint32_t size) {
    uint8_t combined = 0u;
    uint32_t index;
    for (index = 0u; index < size; ++index)
        combined = (uint8_t)(combined | bytes[index]);
    return combined == 0u;
}

static int supplicant_ranges_overlap(const void* left, uint32_t left_size,
                                     const void* right,
                                     uint32_t right_size) {
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

static uint16_t supplicant_read_be16(const uint8_t* bytes) {
    return (uint16_t)(((uint16_t)bytes[0] << 8u) | bytes[1]);
}

static uint16_t supplicant_read_le16(const uint8_t* bytes) {
    return (uint16_t)(bytes[0] | ((uint16_t)bytes[1] << 8u));
}

static uint64_t supplicant_read_be64(const uint8_t* bytes) {
    uint64_t value = 0u;
    uint32_t index;
    for (index = 0u; index < 8u; ++index)
        value = (value << 8u) | bytes[index];
    return value;
}

static void supplicant_write_be16(uint8_t* bytes, uint16_t value) {
    bytes[0] = (uint8_t)(value >> 8u);
    bytes[1] = (uint8_t)value;
}

static void supplicant_write_be64(uint8_t* bytes, uint64_t value) {
    uint32_t index;
    for (index = 0u; index < 8u; ++index) {
        bytes[7u - index] = (uint8_t)value;
        value >>= 8u;
    }
}

static int supplicant_address_valid(const uint8_t address[6]) {
    return address && (address[0] & UINT8_C(0x01)) == 0u &&
           !supplicant_zero_bytes(address, 6u);
}

static int supplicant_rsn_valid(const uint8_t rsn[22]) {
    static const uint8_t ccmp[4] = {0x00u, 0x0fu, 0xacu, 0x04u};
    static const uint8_t psk[4] = {0x00u, 0x0fu, 0xacu, 0x02u};
    return rsn && rsn[0] == 48u && rsn[1] == 20u &&
           supplicant_read_le16(rsn + 2u) == 1u &&
           supplicant_equal(rsn + 4u, ccmp, sizeof(ccmp)) &&
           supplicant_read_le16(rsn + 8u) == 1u &&
           supplicant_equal(rsn + 10u, ccmp, sizeof(ccmp)) &&
           supplicant_read_le16(rsn + 14u) == 1u &&
           supplicant_equal(rsn + 16u, psk, sizeof(psk)) &&
           supplicant_read_le16(rsn + 20u) == 0u;
}

static int supplicant_installed_keys_valid(
    const RinWifiWpa2InstalledKeysV1* keys) {
    return keys && keys->struct_size == sizeof(*keys) &&
           keys->version == RIN_WIFI_WPA2_SUPPLICANT_VERSION &&
           keys->action == RIN_WIFI_WPA2_KEY_INSTALL &&
           keys->key_id <= 3u && keys->association_generation != 0u &&
           keys->association_generation != UINT64_MAX &&
           keys->association_id != 0u && keys->reserved0 == 0u &&
           keys->replay_counter != 0u &&
           supplicant_address_valid(keys->authenticator_address) &&
           supplicant_address_valid(keys->supplicant_address) &&
           !supplicant_equal(keys->authenticator_address,
                             keys->supplicant_address, 6u) &&
           supplicant_zero_bytes(keys->reserved1, sizeof(keys->reserved1));
}

static int supplicant_valid(const RinWifiWpa2SupplicantV1* supplicant) {
    return supplicant && supplicant->struct_size == sizeof(*supplicant) &&
           supplicant->version == RIN_WIFI_WPA2_SUPPLICANT_VERSION &&
           supplicant->state >= RIN_WIFI_WPA2_SUPPLICANT_WAIT_MESSAGE_1 &&
           supplicant->state <= RIN_WIFI_WPA2_SUPPLICANT_FAILED &&
           supplicant->transport.struct_size ==
               sizeof(supplicant->transport) &&
           supplicant->transport.version ==
               RIN_WIFI_WPA2_SUPPLICANT_VERSION &&
           supplicant->transport.random_bytes &&
           supplicant->transport.send_eapol &&
           supplicant->transport.set_keys;
}

static int supplicant_callback_session_unchanged(
    RinWifiWpa2SupplicantV1* supplicant,
    const RinWifiWpa2SupplicantV1* snapshot) {
    if (supplicant_equal(supplicant, snapshot, sizeof(*snapshot))) return 1;
    supplicant_copy(supplicant, snapshot, sizeof(*snapshot));
    return 0;
}

static int supplicant_random(RinWifiWpa2SupplicantV1* supplicant,
                             uint8_t output[32]) {
    RinWifiWpa2SupplicantV1 snapshot;
    RinWifiWpa2RandomFn random_bytes = supplicant->transport.random_bytes;
    void* context = supplicant->transport.context;
    int result;
    supplicant_copy(&snapshot, supplicant, sizeof(snapshot));
    supplicant_zero(output, 32u);
    supplicant->callback_active = 1u;
    result = random_bytes(context, output, 32u);
    supplicant->callback_active = 0u;
    if (!supplicant_callback_session_unchanged(supplicant, &snapshot)) {
        supplicant_zero(output, 32u);
        supplicant_zero(&snapshot, sizeof(snapshot));
        return RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;
    }
    supplicant_zero(&snapshot, sizeof(snapshot));
    if (result != 0) {
        supplicant_zero(output, 32u);
        return RIN_WIFI_WPA2_SUPPLICANT_TRANSPORT_FAILURE;
    }
    if (supplicant_zero_bytes(output, 32u))
        return RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;
    return RIN_WIFI_WPA2_SUPPLICANT_OK;
}

static int supplicant_send(RinWifiWpa2SupplicantV1* supplicant,
                           uint8_t* frame, uint32_t frame_size) {
    RinWifiWpa2SupplicantV1 session_snapshot;
    uint8_t frame_snapshot[RIN_WIFI_WPA2_MAX_EAPOL_BYTES];
    RinWifiWpa2SendEapolFn send_eapol =
        supplicant->transport.send_eapol;
    void* context = supplicant->transport.context;
    int result;
    supplicant_copy(&session_snapshot, supplicant, sizeof(session_snapshot));
    supplicant_zero(frame_snapshot, sizeof(frame_snapshot));
    supplicant_copy(frame_snapshot, frame, frame_size);
    supplicant->callback_active = 1u;
    result = send_eapol(context, frame, frame_size);
    supplicant->callback_active = 0u;
    if (!supplicant_callback_session_unchanged(supplicant,
                                               &session_snapshot) ||
        !supplicant_equal(frame, frame_snapshot, frame_size)) {
        supplicant_copy(frame, frame_snapshot, frame_size);
        supplicant_zero(&session_snapshot, sizeof(session_snapshot));
        supplicant_zero(frame_snapshot, sizeof(frame_snapshot));
        return RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;
    }
    supplicant_zero(&session_snapshot, sizeof(session_snapshot));
    supplicant_zero(frame_snapshot, sizeof(frame_snapshot));
    return result == 0 ? RIN_WIFI_WPA2_SUPPLICANT_OK
                       : RIN_WIFI_WPA2_SUPPLICANT_TRANSPORT_FAILURE;
}

static int supplicant_set_keys(
    RinWifiWpa2SupplicantV1* supplicant,
    RinWifiWpa2InstalledKeysV1* keys) {
    RinWifiWpa2SupplicantV1 session_snapshot;
    RinWifiWpa2InstalledKeysV1 keys_snapshot;
    RinWifiWpa2SetKeysFn set_keys = supplicant->transport.set_keys;
    void* context = supplicant->transport.context;
    int result;
    supplicant_copy(&session_snapshot, supplicant, sizeof(session_snapshot));
    supplicant_copy(&keys_snapshot, keys, sizeof(keys_snapshot));
    supplicant->callback_active = 1u;
    result = set_keys(context, keys);
    supplicant->callback_active = 0u;
    if (!supplicant_callback_session_unchanged(supplicant,
                                               &session_snapshot) ||
        !supplicant_equal(keys, &keys_snapshot, sizeof(keys_snapshot))) {
        supplicant_copy(keys, &keys_snapshot, sizeof(keys_snapshot));
        supplicant_zero(&session_snapshot, sizeof(session_snapshot));
        supplicant_zero(&keys_snapshot, sizeof(keys_snapshot));
        return RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;
    }
    supplicant_zero(&session_snapshot, sizeof(session_snapshot));
    supplicant_zero(&keys_snapshot, sizeof(keys_snapshot));
    return result == 0 ? RIN_WIFI_WPA2_SUPPLICANT_OK
                       : RIN_WIFI_WPA2_SUPPLICANT_TRANSPORT_FAILURE;
}

static void supplicant_key_request_base(
    const RinWifiWpa2SupplicantV1* supplicant,
    RinWifiWpa2InstalledKeysV1* request, uint32_t action) {
    supplicant_zero(request, sizeof(*request));
    request->struct_size = sizeof(*request);
    request->version = RIN_WIFI_WPA2_SUPPLICANT_VERSION;
    request->action = action;
    request->association_generation = supplicant->association_generation;
    request->association_id = supplicant->association_id;
    request->replay_counter = supplicant->message_3_replay_counter;
    supplicant_copy(request->authenticator_address,
                    supplicant->authenticator_address, 6u);
    supplicant_copy(request->supplicant_address,
                    supplicant->supplicant_address, 6u);
}

static int supplicant_remove_keys(RinWifiWpa2SupplicantV1* supplicant) {
    RinWifiWpa2InstalledKeysV1 request;
    int status;
    if (!supplicant->keys_installed) return RIN_WIFI_WPA2_SUPPLICANT_OK;
    supplicant_key_request_base(supplicant, &request,
                                RIN_WIFI_WPA2_KEY_REMOVE);
    status = supplicant_set_keys(supplicant, &request);
    supplicant_zero(&request, sizeof(request));
    if (status == RIN_WIFI_WPA2_SUPPLICANT_OK)
        supplicant->keys_installed = 0u;
    return status;
}

static int supplicant_fail(RinWifiWpa2SupplicantV1* supplicant,
                           int status) {
    int remove_status = supplicant_remove_keys(supplicant);
    rin_wifi_wpa2_forget_pmk(&supplicant->pmk);
    rin_wifi_wpa2_forget_ptk(&supplicant->ptk);
    supplicant_zero(supplicant->authenticator_nonce, 32u);
    supplicant_zero(supplicant->supplicant_nonce, 32u);
    supplicant->state = RIN_WIFI_WPA2_SUPPLICANT_FAILED;
    return remove_status == RIN_WIFI_WPA2_SUPPLICANT_OK ? status
                                                        : remove_status;
}

static int supplicant_frame_base_valid(const uint8_t* frame,
                                       uint32_t frame_size) {
    if (!frame || frame_size < WPA2_EAPOL_BASE_SIZE ||
        frame_size > RIN_WIFI_WPA2_MAX_EAPOL_BYTES ||
        (frame[0] != 1u && frame[0] != 2u) || frame[1] != 3u ||
        supplicant_read_be16(frame + 2u) != frame_size - 4u ||
        frame[4] != WPA2_KEY_DESCRIPTOR_RSN ||
        supplicant_read_be16(frame + 7u) != WPA2_KEY_LENGTH_CCMP ||
        supplicant_read_be16(frame + WPA2_EAPOL_KEY_DATA_LENGTH_OFFSET) !=
            frame_size - WPA2_EAPOL_BASE_SIZE)
        return 0;
    return 1;
}

static int supplicant_message_1_valid(const uint8_t* frame,
                                      uint32_t frame_size) {
    return supplicant_frame_base_valid(frame, frame_size) &&
           frame_size == WPA2_EAPOL_BASE_SIZE &&
           supplicant_read_be16(frame + 5u) ==
               WPA2_MESSAGE_1_KEY_INFO &&
           !supplicant_zero_bytes(frame + WPA2_EAPOL_NONCE_OFFSET, 32u) &&
           supplicant_zero_bytes(frame + WPA2_EAPOL_IV_OFFSET, 16u) &&
           supplicant_zero_bytes(frame + WPA2_EAPOL_RSC_OFFSET, 8u) &&
           supplicant_zero_bytes(frame + WPA2_EAPOL_KEY_ID_OFFSET, 8u) &&
           supplicant_zero_bytes(frame + WPA2_EAPOL_MIC_OFFSET, 16u);
}

static int supplicant_message_3_valid(
    const RinWifiWpa2SupplicantV1* supplicant, const uint8_t* frame,
    uint32_t frame_size) {
    uint32_t key_data_size;
    uint64_t replay_counter;
    if (!supplicant_frame_base_valid(frame, frame_size) ||
        supplicant_read_be16(frame + 5u) != WPA2_MESSAGE_3_KEY_INFO ||
        !supplicant_equal(frame + WPA2_EAPOL_NONCE_OFFSET,
                          supplicant->authenticator_nonce, 32u) ||
        !supplicant_zero_bytes(frame + WPA2_EAPOL_IV_OFFSET, 16u) ||
        !supplicant_zero_bytes(frame + WPA2_EAPOL_KEY_ID_OFFSET, 8u))
        return 0;
    replay_counter = supplicant_read_be64(frame + 9u);
    key_data_size = frame_size - WPA2_EAPOL_BASE_SIZE;
    return replay_counter > supplicant->message_1_replay_counter &&
           key_data_size >= 24u &&
           key_data_size <= RIN_WIFI_WPA2_MAX_WRAPPED_KEY_DATA_BYTES &&
           (key_data_size % 8u) == 0u;
}

static int supplicant_build_response(
    RinWifiWpa2SupplicantV1* supplicant, uint8_t protocol_version,
    uint16_t key_info, uint64_t replay_counter, const uint8_t* nonce,
    const uint8_t* key_data, uint32_t key_data_size, uint8_t* frame,
    uint32_t* frame_size) {
    uint8_t mic[16];
    uint32_t size = WPA2_EAPOL_BASE_SIZE + key_data_size;
    supplicant_zero(frame, RIN_WIFI_WPA2_MAX_EAPOL_BYTES);
    supplicant_zero(mic, sizeof(mic));
    frame[0] = protocol_version;
    frame[1] = 3u;
    supplicant_write_be16(frame + 2u, (uint16_t)(size - 4u));
    frame[4] = WPA2_KEY_DESCRIPTOR_RSN;
    supplicant_write_be16(frame + 5u, key_info);
    supplicant_write_be16(frame + 7u, WPA2_KEY_LENGTH_CCMP);
    supplicant_write_be64(frame + 9u, replay_counter);
    if (nonce) supplicant_copy(frame + WPA2_EAPOL_NONCE_OFFSET, nonce, 32u);
    supplicant_write_be16(frame + WPA2_EAPOL_KEY_DATA_LENGTH_OFFSET,
                          (uint16_t)key_data_size);
    if (key_data_size != 0u)
        supplicant_copy(frame + WPA2_EAPOL_KEY_DATA_OFFSET, key_data,
                        key_data_size);
    if (rin_wifi_wpa2_eapol_mic(&supplicant->ptk, frame, size, mic) != 0) {
        supplicant_zero(frame, RIN_WIFI_WPA2_MAX_EAPOL_BYTES);
        return RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;
    }
    supplicant_copy(frame + WPA2_EAPOL_MIC_OFFSET, mic, sizeof(mic));
    supplicant_zero(mic, sizeof(mic));
    *frame_size = size;
    return RIN_WIFI_WPA2_SUPPLICANT_OK;
}

static int supplicant_send_message_2(
    RinWifiWpa2SupplicantV1* supplicant, uint8_t protocol_version) {
    uint8_t frame[RIN_WIFI_WPA2_MAX_EAPOL_BYTES];
    uint32_t frame_size = 0u;
    int status = supplicant_build_response(
        supplicant, protocol_version, WPA2_MESSAGE_2_KEY_INFO,
        supplicant->message_1_replay_counter, supplicant->supplicant_nonce,
        supplicant->rsn_information_element,
        RIN_WIFI_WPA2_SUPPLICANT_RSN_IE_SIZE, frame, &frame_size);
    if (status == RIN_WIFI_WPA2_SUPPLICANT_OK)
        status = supplicant_send(supplicant, frame, frame_size);
    supplicant_zero(frame, sizeof(frame));
    return status;
}

static int supplicant_send_message_4(
    RinWifiWpa2SupplicantV1* supplicant, uint8_t protocol_version) {
    uint8_t frame[RIN_WIFI_WPA2_MAX_EAPOL_BYTES];
    uint32_t frame_size = 0u;
    int status = supplicant_build_response(
        supplicant, protocol_version, WPA2_MESSAGE_4_KEY_INFO,
        supplicant->message_3_replay_counter, NULL, NULL, 0u, frame,
        &frame_size);
    if (status == RIN_WIFI_WPA2_SUPPLICANT_OK)
        status = supplicant_send(supplicant, frame, frame_size);
    supplicant_zero(frame, sizeof(frame));
    return status;
}

static int supplicant_extract_gtk(
    const RinWifiWpa2SupplicantV1* supplicant, const uint8_t* plaintext,
    uint32_t plaintext_size, uint8_t gtk[16], uint32_t* key_id) {
    uint32_t offset = 0u;
    int found = 0;
    supplicant_zero(gtk, 16u);
    *key_id = 0u;
    while (offset < plaintext_size) {
        uint32_t element_size;
        uint8_t id = plaintext[offset];
        if (id == 0xddu &&
            supplicant_zero_bytes(plaintext + offset + 1u,
                                  plaintext_size - offset - 1u))
            break;
        if (plaintext_size - offset < 2u) goto fail;
        element_size = 2u + plaintext[offset + 1u];
        if (element_size > plaintext_size - offset) goto fail;
        if (id == 48u) {
            if (element_size != RIN_WIFI_WPA2_SUPPLICANT_RSN_IE_SIZE ||
                !supplicant_equal(plaintext + offset,
                                  supplicant->rsn_information_element,
                                  element_size))
                goto fail;
        } else if (id == 0xddu && element_size == 24u &&
                   plaintext[offset + 2u] == 0x00u &&
                   plaintext[offset + 3u] == 0x0fu &&
                   plaintext[offset + 4u] == 0xacu &&
                   plaintext[offset + 5u] == 0x01u) {
            uint8_t key_info = plaintext[offset + 6u];
            if (found || (key_info & UINT8_C(0xf8)) != 0u ||
                plaintext[offset + 7u] != 0u)
                goto fail;
            *key_id = key_info & UINT8_C(0x03);
            supplicant_copy(gtk, plaintext + offset + 8u, 16u);
            if (supplicant_zero_bytes(gtk, 16u)) goto fail;
            found = 1;
        } else {
            goto fail;
        }
        offset += element_size;
    }
    return found ? RIN_WIFI_WPA2_SUPPLICANT_OK
                 : RIN_WIFI_WPA2_SUPPLICANT_UNSUPPORTED;
fail:
    supplicant_zero(gtk, 16u);
    *key_id = 0u;
    return RIN_WIFI_WPA2_SUPPLICANT_MALFORMED;
}

static int supplicant_process_message_1(
    RinWifiWpa2SupplicantV1* supplicant, const uint8_t* frame) {
    RinWifiWpa2PtkV1 ptk;
    uint8_t nonce[32];
    uint64_t replay_counter = supplicant_read_be64(frame + 9u);
    int status;
    supplicant_zero(&ptk, sizeof(ptk));
    supplicant_zero(nonce, sizeof(nonce));
    status = supplicant_random(supplicant, nonce);
    if (status != RIN_WIFI_WPA2_SUPPLICANT_OK) goto fail;
    if (supplicant_equal(nonce, frame + WPA2_EAPOL_NONCE_OFFSET, 32u)) {
        status = RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;
        goto fail;
    }
    if (rin_wifi_wpa2_derive_ptk(
            &supplicant->pmk, supplicant->authenticator_address,
            supplicant->supplicant_address,
            frame + WPA2_EAPOL_NONCE_OFFSET, nonce, &ptk) != 0) {
        status = RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;
        goto fail;
    }
    supplicant_copy(supplicant->authenticator_nonce,
                    frame + WPA2_EAPOL_NONCE_OFFSET, 32u);
    supplicant_copy(supplicant->supplicant_nonce, nonce, 32u);
    supplicant_copy(&supplicant->ptk, &ptk, sizeof(ptk));
    supplicant->message_1_replay_counter = replay_counter;
    supplicant->eapol_protocol_version = frame[0];
    status = supplicant_send_message_2(supplicant, frame[0]);
    if (status != RIN_WIFI_WPA2_SUPPLICANT_OK) goto fail;
    supplicant->state = RIN_WIFI_WPA2_SUPPLICANT_WAIT_MESSAGE_3;
    rin_wifi_wpa2_forget_ptk(&ptk);
    supplicant_zero(nonce, sizeof(nonce));
    return RIN_WIFI_WPA2_SUPPLICANT_OK;
fail:
    rin_wifi_wpa2_forget_ptk(&ptk);
    supplicant_zero(nonce, sizeof(nonce));
    return supplicant_fail(supplicant, status);
}

static int supplicant_process_message_3(
    RinWifiWpa2SupplicantV1* supplicant, const uint8_t* frame,
    uint32_t frame_size, int retransmission) {
    RinWifiWpa2InstalledKeysV1 request;
    uint8_t plaintext[RIN_WIFI_WPA2_MAX_KEY_DATA_BYTES];
    uint8_t gtk[16];
    uint32_t plaintext_size = 0u;
    uint32_t key_id = 0u;
    uint32_t key_data_size = frame_size - WPA2_EAPOL_BASE_SIZE;
    uint64_t replay_counter = supplicant_read_be64(frame + 9u);
    int status;
    supplicant_zero(&request, sizeof(request));
    supplicant_zero(plaintext, sizeof(plaintext));
    supplicant_zero(gtk, sizeof(gtk));
    if ((retransmission &&
         replay_counter != supplicant->message_3_replay_counter) ||
        (!retransmission &&
         replay_counter <= supplicant->message_1_replay_counter) ||
        rin_wifi_wpa2_eapol_verify_mic(&supplicant->ptk, frame,
                                       frame_size) != 0) {
        status = RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;
        goto fail;
    }
    if (retransmission) {
        status = supplicant_send_message_4(supplicant, frame[0]);
        if (status != RIN_WIFI_WPA2_SUPPLICANT_OK) goto fail;
        goto done;
    }
    if (rin_wifi_wpa2_unwrap_key_data(
            &supplicant->ptk, frame + WPA2_EAPOL_KEY_DATA_OFFSET,
            key_data_size, plaintext, &plaintext_size) != 0) {
        status = RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;
        goto fail;
    }
    status = supplicant_extract_gtk(supplicant, plaintext, plaintext_size,
                                    gtk, &key_id);
    if (status != RIN_WIFI_WPA2_SUPPLICANT_OK) goto fail;
    supplicant->message_3_replay_counter = replay_counter;
    supplicant_key_request_base(supplicant, &request,
                                RIN_WIFI_WPA2_KEY_INSTALL);
    request.key_id = key_id;
    supplicant_copy(request.receive_sequence_counter,
                    frame + WPA2_EAPOL_RSC_OFFSET, 8u);
    supplicant_copy(request.pairwise_key, supplicant->ptk.tk, 16u);
    supplicant_copy(request.group_key, gtk, 16u);
    /* The callback contract is transactional, but mark the association as
     * possibly installed before entering it so any reported or ambiguous
     * failure is followed by an idempotent removal request. */
    supplicant->keys_installed = 1u;
    status = supplicant_set_keys(supplicant, &request);
    if (status != RIN_WIFI_WPA2_SUPPLICANT_OK) goto fail;
    status = supplicant_send_message_4(supplicant, frame[0]);
    if (status != RIN_WIFI_WPA2_SUPPLICANT_OK) goto fail;
    supplicant->state = RIN_WIFI_WPA2_SUPPLICANT_COMPLETE;
done:
    supplicant_zero(&request, sizeof(request));
    supplicant_zero(plaintext, sizeof(plaintext));
    supplicant_zero(gtk, sizeof(gtk));
    return RIN_WIFI_WPA2_SUPPLICANT_OK;
fail:
    supplicant_zero(&request, sizeof(request));
    supplicant_zero(plaintext, sizeof(plaintext));
    supplicant_zero(gtk, sizeof(gtk));
    return supplicant_fail(supplicant, status);
}

int rin_wifi_wpa2_supplicant_init(
    RinWifiWpa2SupplicantV1* supplicant,
    const RinWifiWpa2SupplicantTransportV1* transport,
    uint64_t association_generation, uint32_t association_id,
    const uint8_t authenticator_address[6],
    const uint8_t supplicant_address[6], const uint8_t* passphrase,
    uint32_t passphrase_size, const uint8_t* ssid, uint32_t ssid_size,
    const uint8_t rsn_information_element[22]) {
    RinWifiWpa2SupplicantTransportV1 transport_snapshot;
    RinWifiWpa2PmkV1 pmk;
    uint8_t authenticator[6];
    uint8_t station[6];
    uint8_t rsn[22];
    if (!supplicant || !transport || !authenticator_address ||
        !supplicant_address || !passphrase || !ssid ||
        !rsn_information_element || passphrase_size < 8u ||
        passphrase_size > 63u || ssid_size == 0u || ssid_size > 32u ||
        association_generation == 0u || association_id == 0u ||
        supplicant_ranges_overlap(supplicant, sizeof(*supplicant), transport,
                                  sizeof(*transport)) ||
        supplicant_ranges_overlap(supplicant, sizeof(*supplicant),
                                  authenticator_address, 6u) ||
        supplicant_ranges_overlap(supplicant, sizeof(*supplicant),
                                  supplicant_address, 6u) ||
        supplicant_ranges_overlap(supplicant, sizeof(*supplicant), passphrase,
                                  passphrase_size) ||
        supplicant_ranges_overlap(supplicant, sizeof(*supplicant), ssid,
                                  ssid_size) ||
        supplicant_ranges_overlap(supplicant, sizeof(*supplicant),
                                  rsn_information_element, 22u) ||
        transport->struct_size != sizeof(*transport) ||
        transport->version != RIN_WIFI_WPA2_SUPPLICANT_VERSION ||
        !transport->random_bytes || !transport->send_eapol ||
        !transport->set_keys)
        return RIN_WIFI_WPA2_SUPPLICANT_INVALID_ARGUMENT;
    supplicant_copy(&transport_snapshot, transport, sizeof(transport_snapshot));
    supplicant_copy(authenticator, authenticator_address, 6u);
    supplicant_copy(station, supplicant_address, 6u);
    supplicant_copy(rsn, rsn_information_element, sizeof(rsn));
    supplicant_zero(&pmk, sizeof(pmk));
    supplicant_zero(supplicant, sizeof(*supplicant));
    if (!supplicant_address_valid(authenticator) ||
        !supplicant_address_valid(station) ||
        supplicant_equal(authenticator, station, 6u) ||
        !supplicant_rsn_valid(rsn) ||
        rin_wifi_wpa2_derive_pmk(passphrase, passphrase_size, ssid,
                                 ssid_size, &pmk) != 0) {
        rin_wifi_wpa2_forget_pmk(&pmk);
        return RIN_WIFI_WPA2_SUPPLICANT_UNSUPPORTED;
    }
    supplicant->struct_size = sizeof(*supplicant);
    supplicant->version = RIN_WIFI_WPA2_SUPPLICANT_VERSION;
    supplicant->state = RIN_WIFI_WPA2_SUPPLICANT_WAIT_MESSAGE_1;
    supplicant->association_generation = association_generation;
    supplicant->association_id = association_id;
    supplicant_copy(supplicant->authenticator_address, authenticator, 6u);
    supplicant_copy(supplicant->supplicant_address, station, 6u);
    supplicant_copy(supplicant->rsn_information_element, rsn, sizeof(rsn));
    supplicant_copy(&supplicant->pmk, &pmk, sizeof(pmk));
    supplicant_copy(&supplicant->transport, &transport_snapshot,
                    sizeof(transport_snapshot));
    rin_wifi_wpa2_forget_pmk(&pmk);
    supplicant_zero(authenticator, sizeof(authenticator));
    supplicant_zero(station, sizeof(station));
    supplicant_zero(rsn, sizeof(rsn));
    return RIN_WIFI_WPA2_SUPPLICANT_OK;
}

int rin_wifi_wpa2_supplicant_process(
    RinWifiWpa2SupplicantV1* supplicant, const uint8_t* eapol,
    uint32_t eapol_size) {
    uint8_t frame[RIN_WIFI_WPA2_MAX_EAPOL_BYTES];
    int message_1;
    int message_3;
    int status;
    if (!supplicant_valid(supplicant) || !eapol ||
        supplicant->callback_active != 0u ||
        eapol_size < WPA2_EAPOL_BASE_SIZE ||
        eapol_size > sizeof(frame) ||
        supplicant_ranges_overlap(supplicant, sizeof(*supplicant), eapol,
                                  eapol_size))
        return RIN_WIFI_WPA2_SUPPLICANT_INVALID_ARGUMENT;
    if (supplicant->state == RIN_WIFI_WPA2_SUPPLICANT_FAILED)
        return RIN_WIFI_WPA2_SUPPLICANT_INVALID_STATE;
    supplicant_zero(frame, sizeof(frame));
    supplicant_copy(frame, eapol, eapol_size);
    message_1 = supplicant_message_1_valid(frame, eapol_size);
    message_3 = supplicant_message_3_valid(supplicant, frame, eapol_size);
    if (supplicant->state == RIN_WIFI_WPA2_SUPPLICANT_WAIT_MESSAGE_1 &&
        message_1) {
        status = supplicant_process_message_1(supplicant, frame);
    } else if (supplicant->state ==
                   RIN_WIFI_WPA2_SUPPLICANT_WAIT_MESSAGE_3 &&
               message_1 &&
               supplicant_read_be64(frame + 9u) ==
                   supplicant->message_1_replay_counter &&
               frame[0] == supplicant->eapol_protocol_version &&
               supplicant_equal(frame + WPA2_EAPOL_NONCE_OFFSET,
                                 supplicant->authenticator_nonce, 32u)) {
        status = supplicant_send_message_2(supplicant, frame[0]);
        if (status != RIN_WIFI_WPA2_SUPPLICANT_OK)
            status = supplicant_fail(supplicant, status);
    } else if (supplicant->state ==
                   RIN_WIFI_WPA2_SUPPLICANT_WAIT_MESSAGE_3 &&
               message_3) {
        status = supplicant_process_message_3(supplicant, frame, eapol_size,
                                              0);
    } else if (supplicant->state == RIN_WIFI_WPA2_SUPPLICANT_COMPLETE &&
               message_3) {
        status = supplicant_process_message_3(supplicant, frame, eapol_size,
                                              1);
    } else {
        status = supplicant_fail(
            supplicant, RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE);
    }
    supplicant_zero(frame, sizeof(frame));
    return status;
}

int rin_wifi_wpa2_supplicant_prepare_pairwise_ccmp(
    const RinWifiWpa2InstalledKeysV1* installed_keys,
    RinWifiWpa2CcmpStateV1* ccmp_state_out,
    RinWifiWpa2PtkV1* ccmp_key_out) {
    RinWifiWpa2InstalledKeysV1 keys;
    RinWifiWpa2CcmpStateV1 state;
    RinWifiWpa2PtkV1 key;
    int status = RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE;

    if (!installed_keys || !ccmp_state_out || !ccmp_key_out ||
        supplicant_ranges_overlap(installed_keys, sizeof(*installed_keys),
                                  ccmp_state_out,
                                  sizeof(*ccmp_state_out)) ||
        supplicant_ranges_overlap(installed_keys, sizeof(*installed_keys),
                                  ccmp_key_out, sizeof(*ccmp_key_out)) ||
        supplicant_ranges_overlap(ccmp_state_out, sizeof(*ccmp_state_out),
                                  ccmp_key_out, sizeof(*ccmp_key_out))) {
        return RIN_WIFI_WPA2_SUPPLICANT_INVALID_ARGUMENT;
    }

    supplicant_zero(ccmp_state_out, sizeof(*ccmp_state_out));
    supplicant_zero(ccmp_key_out, sizeof(*ccmp_key_out));
    supplicant_zero(&keys, sizeof(keys));
    supplicant_zero(&state, sizeof(state));
    supplicant_zero(&key, sizeof(key));
    supplicant_copy(&keys, installed_keys, sizeof(keys));
    if (!supplicant_installed_keys_valid(&keys) ||
        rin_wifi_wpa2_ccmp_prepare_key(&key, keys.pairwise_key) != 0 ||
        rin_wifi_wpa2_ccmp_state_init(
            &state, keys.association_generation, keys.association_id,
            keys.key_id, keys.supplicant_address,
            keys.authenticator_address) != 0) {
        goto done;
    }
    supplicant_copy(ccmp_state_out, &state, sizeof(state));
    supplicant_copy(ccmp_key_out, &key, sizeof(key));
    status = RIN_WIFI_WPA2_SUPPLICANT_OK;
done:
    supplicant_zero(&keys, sizeof(keys));
    rin_wifi_wpa2_ccmp_state_forget(&state);
    rin_wifi_wpa2_forget_ptk(&key);
    return status;
}

void rin_wifi_wpa2_supplicant_forget_pairwise_ccmp(
    RinWifiWpa2CcmpStateV1* ccmp_state,
    RinWifiWpa2PtkV1* ccmp_key) {
    rin_wifi_wpa2_ccmp_state_forget(ccmp_state);
    rin_wifi_wpa2_forget_ptk(ccmp_key);
}

int rin_wifi_wpa2_supplicant_forget(
    RinWifiWpa2SupplicantV1* supplicant) {
    int status;
    if (!supplicant_valid(supplicant) || supplicant->callback_active != 0u)
        return RIN_WIFI_WPA2_SUPPLICANT_INVALID_ARGUMENT;
    status = supplicant_remove_keys(supplicant);
    rin_wifi_wpa2_forget_pmk(&supplicant->pmk);
    rin_wifi_wpa2_forget_ptk(&supplicant->ptk);
    supplicant_zero(supplicant->authenticator_nonce, 32u);
    supplicant_zero(supplicant->supplicant_nonce, 32u);
    supplicant->state = RIN_WIFI_WPA2_SUPPLICANT_FAILED;
    return status;
}
