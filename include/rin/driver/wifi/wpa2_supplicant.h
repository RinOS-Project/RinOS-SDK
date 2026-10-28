/* SPDX-License-Identifier: MIT */
/* Bounded WPA2-PSK/CCMP EAPOL four-way supplicant transaction. */

#ifndef RIN_DRIVER_WIFI_WPA2_SUPPLICANT_H
#define RIN_DRIVER_WIFI_WPA2_SUPPLICANT_H

#include <stdint.h>

#include <rin/driver/wifi/wpa2.h>

#define RIN_WIFI_WPA2_SUPPLICANT_VERSION 1u
#define RIN_WIFI_WPA2_SUPPLICANT_RSN_IE_SIZE 22u
#define RIN_WIFI_WPA2_KEY_INSTALL 1u
#define RIN_WIFI_WPA2_KEY_REMOVE 2u

typedef enum RinWifiWpa2SupplicantStatus {
    RIN_WIFI_WPA2_SUPPLICANT_OK = 0,
    RIN_WIFI_WPA2_SUPPLICANT_INVALID_ARGUMENT = -1,
    RIN_WIFI_WPA2_SUPPLICANT_INVALID_STATE = -2,
    RIN_WIFI_WPA2_SUPPLICANT_MALFORMED = -3,
    RIN_WIFI_WPA2_SUPPLICANT_SECURITY_FAILURE = -4,
    RIN_WIFI_WPA2_SUPPLICANT_TRANSPORT_FAILURE = -5,
    RIN_WIFI_WPA2_SUPPLICANT_UNSUPPORTED = -6
} RinWifiWpa2SupplicantStatus;

typedef enum RinWifiWpa2SupplicantState {
    RIN_WIFI_WPA2_SUPPLICANT_UNINITIALIZED = 0,
    RIN_WIFI_WPA2_SUPPLICANT_WAIT_MESSAGE_1 = 1,
    RIN_WIFI_WPA2_SUPPLICANT_WAIT_MESSAGE_3 = 2,
    RIN_WIFI_WPA2_SUPPLICANT_COMPLETE = 3,
    RIN_WIFI_WPA2_SUPPLICANT_FAILED = 4
} RinWifiWpa2SupplicantState;

typedef struct RinWifiWpa2InstalledKeysV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t action;
    uint32_t key_id;
    uint64_t association_generation;
    uint32_t association_id;
    uint32_t reserved0;
    uint64_t replay_counter;
    uint8_t authenticator_address[6];
    uint8_t supplicant_address[6];
    uint8_t receive_sequence_counter[8];
    uint8_t pairwise_key[16];
    uint8_t group_key[16];
    uint8_t reserved1[4];
} RinWifiWpa2InstalledKeysV1;

typedef int (*RinWifiWpa2RandomFn)(void* context, uint8_t* output,
                                   uint32_t output_size);
typedef int (*RinWifiWpa2SendEapolFn)(void* context,
                                     const uint8_t* eapol,
                                     uint32_t eapol_size);
typedef int (*RinWifiWpa2SetKeysFn)(
    void* context, const RinWifiWpa2InstalledKeysV1* keys);

typedef struct RinWifiWpa2SupplicantTransportV1 {
    uint32_t struct_size;
    uint32_t version;
    RinWifiWpa2RandomFn random_bytes;
    RinWifiWpa2SendEapolFn send_eapol;
    RinWifiWpa2SetKeysFn set_keys;
    void* context;
} RinWifiWpa2SupplicantTransportV1;

/* The owner serializes calls. Callbacks must not re-enter or modify their
 * inputs. Key install/remove is association-scoped and removal is idempotent;
 * the controller requests removal after every failed or ambiguous install. */

typedef struct RinWifiWpa2SupplicantV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t callback_active;
    uint64_t association_generation;
    uint32_t association_id;
    uint32_t keys_installed;
    uint64_t message_1_replay_counter;
    uint64_t message_3_replay_counter;
    uint8_t authenticator_address[6];
    uint8_t supplicant_address[6];
    uint8_t authenticator_nonce[32];
    uint8_t supplicant_nonce[32];
    uint8_t rsn_information_element[RIN_WIFI_WPA2_SUPPLICANT_RSN_IE_SIZE];
    uint8_t eapol_protocol_version;
    uint8_t reserved0;
    RinWifiWpa2PmkV1 pmk;
    RinWifiWpa2PtkV1 ptk;
    RinWifiWpa2SupplicantTransportV1 transport;
} RinWifiWpa2SupplicantV1;

int rin_wifi_wpa2_supplicant_init(
    RinWifiWpa2SupplicantV1* supplicant,
    const RinWifiWpa2SupplicantTransportV1* transport,
    uint64_t association_generation, uint32_t association_id,
    const uint8_t authenticator_address[6],
    const uint8_t supplicant_address[6], const uint8_t* passphrase,
    uint32_t passphrase_size, const uint8_t* ssid, uint32_t ssid_size,
    const uint8_t rsn_information_element[
        RIN_WIFI_WPA2_SUPPLICANT_RSN_IE_SIZE]);

int rin_wifi_wpa2_supplicant_process(
    RinWifiWpa2SupplicantV1* supplicant, const uint8_t* eapol,
    uint32_t eapol_size);

/* Converts one successfully delivered INSTALL request into the pairwise CCMP
 * state a station driver owns.  It checks the association identity, discards
 * the EAPOL-only KCK/KEK, and publishes the state and CCMP-only key together.
 * The caller must invoke this from its successful `set_keys` INSTALL handling
 * before accepting protected data, and forget both outputs for the matching
 * REMOVE request.  Inputs and outputs must not overlap. */
int rin_wifi_wpa2_supplicant_prepare_pairwise_ccmp(
    const RinWifiWpa2InstalledKeysV1* installed_keys,
    RinWifiWpa2CcmpStateV1* ccmp_state_out,
    RinWifiWpa2PtkV1* ccmp_key_out);

/* Erases the complete pairwise data-plane capability built by
 * `rin_wifi_wpa2_supplicant_prepare_pairwise_ccmp()`. */
void rin_wifi_wpa2_supplicant_forget_pairwise_ccmp(
    RinWifiWpa2CcmpStateV1* ccmp_state,
    RinWifiWpa2PtkV1* ccmp_key);

int rin_wifi_wpa2_supplicant_forget(
    RinWifiWpa2SupplicantV1* supplicant);

_Static_assert(sizeof(RinWifiWpa2InstalledKeysV1) == 96u,
               "WPA2 installed-key ABI drift");

#endif /* RIN_DRIVER_WIFI_WPA2_SUPPLICANT_H */
