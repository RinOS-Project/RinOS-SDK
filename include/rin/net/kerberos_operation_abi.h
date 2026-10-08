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
/* Version 2 adds the provider result/return-flags response fields. */
#define RIN_KERBEROS_OPERATION_ABI_VERSION UINT16_C(2)
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

/* Request flags are deliberately limited to operation semantics. Credential
 * material and GSS tokens remain service-owned/opaque. */
#define RIN_KERBEROS_OPERATION_FLAG_ENCRYPT UINT32_C(0x00000001)
#define RIN_KERBEROS_OPERATION_KNOWN_FLAGS \
    RIN_KERBEROS_OPERATION_FLAG_ENCRYPT

#define RIN_KERBEROS_OPERATION_RESULT_COMPLETE UINT32_C(0)
#define RIN_KERBEROS_OPERATION_RESULT_CONTINUE_NEEDED UINT32_C(1)
#define RIN_KERBEROS_OPERATION_KNOWN_RESULTS \
    (RIN_KERBEROS_OPERATION_RESULT_COMPLETE | \
     RIN_KERBEROS_OPERATION_RESULT_CONTINUE_NEEDED)

/* GSS context flags returned by INIT_SEC_CONTEXT/ACCEPT_SEC_CONTEXT. */
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_DELEGATE UINT32_C(0x00000001)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_MUTUAL UINT32_C(0x00000002)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_REPLAY UINT32_C(0x00000004)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_SEQUENCE UINT32_C(0x00000008)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_CONFIDENTIALITY UINT32_C(0x00000010)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_INTEGRITY UINT32_C(0x00000020)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_ANON UINT32_C(0x00000040)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_PROT_READY UINT32_C(0x00000080)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_TRANS UINT32_C(0x00000100)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_DCE_STYLE UINT32_C(0x00001000)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_IDENTIFY UINT32_C(0x00002000)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_EXTENDED_ERROR UINT32_C(0x00004000)
#define RIN_KERBEROS_OPERATION_RETURN_FLAG_DELEG_POLICY UINT32_C(0x00008000)
#define RIN_KERBEROS_OPERATION_KNOWN_RETURN_FLAGS UINT32_C(0x0000f1ff)

/* Provider input envelopes carry GSS-side inputs, not the ccache/keytab. */
#define RIN_KERBEROS_PROVIDER_INPUT_ABI_VERSION UINT16_C(1)
#define RIN_KERBEROS_PROVIDER_INPUT_SEC_CONTEXT UINT16_C(1)
#define RIN_KERBEROS_PROVIDER_INPUT_MESSAGE_PAIR UINT16_C(2)
#define RIN_KERBEROS_PROVIDER_MAX_TARGET_NAME_SIZE UINT32_C(1024)
#define RIN_KERBEROS_PROVIDER_MAX_CHANNEL_BINDING_SIZE UINT32_C(4096)

typedef struct RinKerberosProviderSecContextInputV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t kind;
    uint32_t mechanism;
    uint32_t requested_flags;
    uint32_t target_name_size;
    uint32_t channel_binding_size;
    uint32_t token_size;
    uint32_t reserved;
} RinKerberosProviderSecContextInputV1;

typedef struct RinKerberosProviderMessagePairInputV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t kind;
    uint32_t first_size;
    uint32_t second_size;
    /* RIN_KERBEROS_OPERATION_FLAG_ENCRYPT is meaningful for WRAP only. */
    uint32_t flags;
} RinKerberosProviderMessagePairInputV1;

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
    uint32_t provider_result;
    uint32_t return_flags;
} RinKerberosOperationResponseV1;

#ifdef __cplusplus
}

static_assert(sizeof(RinKerberosOperationRequestV1) == 28u,
              "RinKerberosOperationRequestV1 ABI drift");
static_assert(sizeof(RinKerberosOperationResponseV1) == 32u,
              "RinKerberosOperationResponseV1 ABI drift");
static_assert(sizeof(RinKerberosProviderSecContextInputV1) == 32u,
              "RinKerberosProviderSecContextInputV1 ABI drift");
static_assert(sizeof(RinKerberosProviderMessagePairInputV1) == 20u,
              "RinKerberosProviderMessagePairInputV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinKerberosOperationRequestV1) == 28u,
               "RinKerberosOperationRequestV1 ABI drift");
_Static_assert(sizeof(RinKerberosOperationResponseV1) == 32u,
               "RinKerberosOperationResponseV1 ABI drift");
_Static_assert(sizeof(RinKerberosProviderSecContextInputV1) == 32u,
               "RinKerberosProviderSecContextInputV1 ABI drift");
_Static_assert(sizeof(RinKerberosProviderMessagePairInputV1) == 20u,
               "RinKerberosProviderMessagePairInputV1 ABI drift");
#endif

#endif /* RIN_SDK_NET_KERBEROS_OPERATION_ABI_H */
