/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_NET_KERBEROS_OPERATION_ABI_H
#define RIN_SDK_NET_KERBEROS_OPERATION_ABI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This is the service-side operation envelope for the product Kerberos
 * provider.  It is deliberately not a GSS token format: the provider owns
 * RFC 4120/4121 encoding and cryptography, while the authenticated keyring
 * owner keeps ccache/keytab bytes inside the service process.  The runtime
 * sees only bounded context/output tokens.
 */
#define RIN_KERBEROS_OPERATION_ABI_VERSION UINT16_C(1)
#define RIN_KERBEROS_OPERATION_CREDENTIAL_INITIATOR UINT32_C(1)
#define RIN_KERBEROS_OPERATION_CREDENTIAL_ACCEPTOR UINT32_C(2)
#define RIN_KERBEROS_OPERATION_KNOWN_CREDENTIALS \
    (RIN_KERBEROS_OPERATION_CREDENTIAL_INITIATOR | \
     RIN_KERBEROS_OPERATION_CREDENTIAL_ACCEPTOR)

#define RIN_KERBEROS_OPERATION_INIT_SEC_CONTEXT UINT16_C(1)
#define RIN_KERBEROS_OPERATION_ACCEPT_SEC_CONTEXT UINT16_C(2)
#define RIN_KERBEROS_OPERATION_DELETE_SEC_CONTEXT UINT16_C(3)
#define RIN_KERBEROS_OPERATION_WRAP UINT16_C(4)
#define RIN_KERBEROS_OPERATION_UNWRAP UINT16_C(5)
#define RIN_KERBEROS_OPERATION_GET_MIC UINT16_C(6)
#define RIN_KERBEROS_OPERATION_VERIFY_MIC UINT16_C(7)
#define RIN_KERBEROS_OPERATION_KNOWN_OPERATIONS \
    (UINT16_C(1u << (RIN_KERBEROS_OPERATION_INIT_SEC_CONTEXT - 1u)) | \
     UINT16_C(1u << (RIN_KERBEROS_OPERATION_ACCEPT_SEC_CONTEXT - 1u)) | \
     UINT16_C(1u << (RIN_KERBEROS_OPERATION_DELETE_SEC_CONTEXT - 1u)) | \
     UINT16_C(1u << (RIN_KERBEROS_OPERATION_WRAP - 1u)) | \
     UINT16_C(1u << (RIN_KERBEROS_OPERATION_UNWRAP - 1u)) | \
     UINT16_C(1u << (RIN_KERBEROS_OPERATION_GET_MIC - 1u)) | \
     UINT16_C(1u << (RIN_KERBEROS_OPERATION_VERIFY_MIC - 1u)))

#define RIN_KERBEROS_OPERATION_MAX_CONTEXT_TOKEN_SIZE UINT32_C(256)
#define RIN_KERBEROS_OPERATION_MAX_INPUT_SIZE UINT32_C(65536)
#define RIN_KERBEROS_OPERATION_MAX_OUTPUT_SIZE UINT32_C(65536)

typedef struct RinKerberosOperationRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t credential_kind;
    uint32_t flags;
    uint32_t context_token_size;
    uint32_t input_size;
    uint32_t output_capacity;
} RinKerberosOperationRequestV1;

typedef struct RinKerberosOperationResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint64_t generation;
    uint32_t context_token_size;
    uint32_t output_size;
    uint32_t reserved;
} RinKerberosOperationResponseV1;

#ifdef __cplusplus
}

static_assert(sizeof(RinKerberosOperationRequestV1) == 28u,
              "RinKerberosOperationRequestV1 ABI drift");
static_assert(sizeof(RinKerberosOperationResponseV1) == 32u,
              "RinKerberosOperationResponseV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinKerberosOperationRequestV1) == 28u,
               "RinKerberosOperationRequestV1 ABI drift");
_Static_assert(sizeof(RinKerberosOperationResponseV1) == 32u,
               "RinKerberosOperationResponseV1 ABI drift");
#endif

#endif /* RIN_SDK_NET_KERBEROS_OPERATION_ABI_H */
