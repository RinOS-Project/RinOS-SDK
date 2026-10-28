/* SPDX-License-Identifier: MIT */
/* WPA2-PSK key derivation and EAPOL-Key MIC primitives. */

#ifndef RIN_DRIVER_WIFI_WPA2_H
#define RIN_DRIVER_WIFI_WPA2_H

#include <stdint.h>

#define RIN_WIFI_WPA2_VERSION 1u
#define RIN_WIFI_WPA2_PASSPHRASE_MIN 8u
#define RIN_WIFI_WPA2_PASSPHRASE_MAX 63u
#define RIN_WIFI_WPA2_SSID_MAX 32u
#define RIN_WIFI_WPA2_NONCE_SIZE 32u
#define RIN_WIFI_WPA2_MIC_SIZE 16u
#define RIN_WIFI_WPA2_MAX_EAPOL_BYTES 1024u
#define RIN_WIFI_WPA2_MAX_KEY_DATA_BYTES 256u
#define RIN_WIFI_WPA2_MAX_WRAPPED_KEY_DATA_BYTES 264u
#define RIN_WIFI_WPA2_CCMP_MIC_SIZE 8u
#define RIN_WIFI_WPA2_CCMP_HEADER_SIZE 8u
#define RIN_WIFI_WPA2_CCMP_TK_SIZE 16u
#define RIN_WIFI_WPA2_CCMP_PN_MAX UINT64_C(0x0000ffffffffffff)
#define RIN_WIFI_WPA2_CCMP_MAX_PAYLOAD_BYTES 2304u

typedef struct RinWifiWpa2PmkV1 {
    uint32_t struct_size;
    uint32_t version;
    uint8_t key[32];
} RinWifiWpa2PmkV1;

typedef struct RinWifiWpa2PtkV1 {
    uint32_t struct_size;
    uint32_t version;
    uint8_t kck[16];
    uint8_t kek[16];
    uint8_t tk[16];
} RinWifiWpa2PtkV1;

int rin_wifi_wpa2_derive_pmk(const uint8_t* passphrase,
                             uint32_t passphrase_size,
                             const uint8_t* ssid, uint32_t ssid_size,
                             RinWifiWpa2PmkV1* pmk_out);

int rin_wifi_wpa2_derive_ptk(const RinWifiWpa2PmkV1* pmk,
                             const uint8_t authenticator_address[6],
                             const uint8_t supplicant_address[6],
                             const uint8_t authenticator_nonce[32],
                             const uint8_t supplicant_nonce[32],
                             RinWifiWpa2PtkV1* ptk_out);

int rin_wifi_wpa2_eapol_mic(const RinWifiWpa2PtkV1* ptk,
                            const uint8_t* eapol, uint32_t eapol_size,
                            uint8_t mic_out[RIN_WIFI_WPA2_MIC_SIZE]);

int rin_wifi_wpa2_eapol_verify_mic(const RinWifiWpa2PtkV1* ptk,
                                   const uint8_t* eapol,
                                   uint32_t eapol_size);

int rin_wifi_wpa2_unwrap_key_data(
    const RinWifiWpa2PtkV1* ptk, const uint8_t* wrapped,
    uint32_t wrapped_size,
    uint8_t plaintext_out[RIN_WIFI_WPA2_MAX_KEY_DATA_BYTES],
    uint32_t* plaintext_size_out);
/* wrapped may alias plaintext_out when the backing object is at least the
 * fixed output size. plaintext_size_out must not overlap plaintext_out. */

/* AES-CCMP protects one 802.11 Data or QoS Data MSDU. `mac_header` is the
 * complete 24, 26, 30, or 32 byte on-air header, including a set Protected
 * Frame bit but excluding the 8-byte CCMP header.  The implementation derives
 * the AAD and traffic priority from that header, accepts only PN 1..2^48-1,
 * and emits or verifies the separate 8-byte CCMP MIC.  Input and output
 * ranges must not overlap; decrypt publishes plaintext only after the MIC
 * verifies. */
int rin_wifi_wpa2_ccmp_encrypt(
    const RinWifiWpa2PtkV1* ptk, const uint8_t* mac_header,
    uint32_t mac_header_size, uint64_t packet_number,
    const uint8_t* plaintext, uint32_t plaintext_size,
    uint8_t* ciphertext_out,
    uint8_t mic_out[RIN_WIFI_WPA2_CCMP_MIC_SIZE]);

int rin_wifi_wpa2_ccmp_decrypt(
    const RinWifiWpa2PtkV1* ptk, const uint8_t* mac_header,
    uint32_t mac_header_size, uint64_t packet_number,
    const uint8_t* ciphertext, uint32_t ciphertext_size,
    const uint8_t mic[RIN_WIFI_WPA2_CCMP_MIC_SIZE],
    uint8_t* plaintext_out);

/* Builds a CCMP-only view from the 16-byte temporal key delivered to a driver
 * by the supplicant.  KCK and KEK remain zero and this result is deliberately
 * invalid for EAPOL MIC or key-wrap APIs; CCMP APIs consume only `tk`. */
int rin_wifi_wpa2_ccmp_prepare_key(
    RinWifiWpa2PtkV1* ccmp_key_out,
    const uint8_t temporal_key[RIN_WIFI_WPA2_CCMP_TK_SIZE]);

/* Association-scoped pairwise CCMP transport state. The owner serializes
 * calls. It binds local/peer transmitter addresses and one key ID, allocates
 * a non-repeating PN per QoS TID, and advances receive replay state only after
 * `rin_wifi_wpa2_ccmp_decrypt()` authenticates the frame. */
typedef struct RinWifiWpa2CcmpStateV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t association_generation;
    uint32_t association_id;
    uint32_t key_id;
    uint8_t local_address[6];
    uint8_t peer_address[6];
    uint8_t reserved0[4];
    uint64_t transmit_next_packet_number[16];
    uint64_t receive_last_packet_number[16];
} RinWifiWpa2CcmpStateV1;

int rin_wifi_wpa2_ccmp_state_init(
    RinWifiWpa2CcmpStateV1* state, uint64_t association_generation,
    uint32_t association_id, uint32_t key_id,
    const uint8_t local_address[6], const uint8_t peer_address[6]);

int rin_wifi_wpa2_ccmp_encrypt_next(
    RinWifiWpa2CcmpStateV1* state, const RinWifiWpa2PtkV1* ptk,
    const uint8_t* mac_header, uint32_t mac_header_size,
    const uint8_t* plaintext, uint32_t plaintext_size,
    uint8_t ccmp_header_out[RIN_WIFI_WPA2_CCMP_HEADER_SIZE],
    uint8_t* ciphertext_out,
    uint8_t mic_out[RIN_WIFI_WPA2_CCMP_MIC_SIZE]);

int rin_wifi_wpa2_ccmp_decrypt_next(
    RinWifiWpa2CcmpStateV1* state, const RinWifiWpa2PtkV1* ptk,
    const uint8_t* mac_header, uint32_t mac_header_size,
    const uint8_t ccmp_header[RIN_WIFI_WPA2_CCMP_HEADER_SIZE],
    const uint8_t* ciphertext, uint32_t ciphertext_size,
    const uint8_t mic[RIN_WIFI_WPA2_CCMP_MIC_SIZE],
    uint8_t* plaintext_out);

void rin_wifi_wpa2_ccmp_state_forget(RinWifiWpa2CcmpStateV1* state);

void rin_wifi_wpa2_forget_pmk(RinWifiWpa2PmkV1* pmk);
void rin_wifi_wpa2_forget_ptk(RinWifiWpa2PtkV1* ptk);

_Static_assert(sizeof(RinWifiWpa2PmkV1) == 40u,
               "WPA2 PMK ABI drift");
_Static_assert(sizeof(RinWifiWpa2PtkV1) == 56u,
               "WPA2 PTK ABI drift");

#endif /* RIN_DRIVER_WIFI_WPA2_H */
