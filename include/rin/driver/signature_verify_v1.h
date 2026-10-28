/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_SIGNATURE_VERIFY_V1_H
#define RIN_DRIVER_SIGNATURE_VERIFY_V1_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_DRIVER_SIGNATURE_VERIFY_V1_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_SIGNATURE_KEY_ID_SIZE 32u

/* Result values are part of the driver/runtime ABI. */
typedef enum RinSignatureVerifyResult {
    RIN_SIGNATURE_VERIFY_OK = 0,
    RIN_SIGNATURE_VERIFY_INVALID_ARGUMENT = -1,
    RIN_SIGNATURE_VERIFY_INVALID_LAYOUT = -2,
    RIN_SIGNATURE_VERIFY_UNSUPPORTED = -3,
    RIN_SIGNATURE_VERIFY_UNTRUSTED_KEY = -4,
    RIN_SIGNATURE_VERIFY_INVALID_KEY = -5,
    RIN_SIGNATURE_VERIFY_INVALID_SIGNATURE = -6
} RinSignatureVerifyResult;

/* Verify a detached RSA/SHA-256 signature against the supplied public key.
 * The key identifier is checked against the signature envelope. This
 * operation verifies cryptographic validity only; selecting a trusted key is
 * the caller's responsibility and cannot be established by these arguments.
 */
RinSignatureVerifyResult rin_signature_verify_digest_rsa_sha256(
    const uint8_t digest[32],
    const void* signature_region,
    uint32_t signature_region_size,
    const uint8_t* public_key_der,
    uint32_t public_key_der_size,
    const uint8_t expected_key_id[RIN_DRIVER_SIGNATURE_KEY_ID_SIZE]);

RinSignatureVerifyResult rin_signature_verify_rsa_sha256(
    const void* image,
    uint32_t image_size,
    uint64_t signature_offset,
    uint32_t signature_region_size,
    const uint8_t* public_key_der,
    uint32_t public_key_der_size,
    const uint8_t expected_key_id[RIN_DRIVER_SIGNATURE_KEY_ID_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_SIGNATURE_VERIFY_V1_H */
