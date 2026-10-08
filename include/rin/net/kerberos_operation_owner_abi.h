/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_NET_KERBEROS_OPERATION_OWNER_ABI_H
#define RIN_SDK_NET_KERBEROS_OPERATION_OWNER_ABI_H

#include <stddef.h>
#include <stdint.h>

#include "kerberos_operation_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Credential acquisition and provider operations are separate contracts. A
 * provider receives the opaque credential token from the credential owner and
 * returns it to this table; it never dereferences the token or receives raw
 * ccache/keytab bytes. The implementation is the authenticated session owner
 * or its runtime IPC client.
 */
#define RIN_KERBEROS_OPERATION_OWNER_ABI_VERSION UINT16_C(1)

typedef struct RinKerberosOperationOwnerV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    void* context;

    uint32_t (*operation)(
        void* context, uint32_t* minor_status, void* credential,
        const RinKerberosOperationRequestV1* request,
        const uint8_t* context_token, uint32_t context_token_size,
        const uint8_t* input, uint32_t input_size, uint8_t* output,
        uint32_t output_capacity, uint32_t* output_size,
        uint8_t* next_context_token, uint32_t next_context_capacity,
        uint32_t* next_context_token_size, uint64_t* generation,
        uint32_t* provider_result, uint32_t* return_flags);
} RinKerberosOperationOwnerV1;

const RinKerberosOperationOwnerV1*
rin_kerberos_operation_owner_get_v1(void);

#ifdef __cplusplus
}

static_assert(sizeof(RinKerberosOperationOwnerV1) ==
                  offsetof(RinKerberosOperationOwnerV1, operation) +
                      sizeof(((RinKerberosOperationOwnerV1*)0)->operation),
              "RinKerberosOperationOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinKerberosOperationOwnerV1) ==
                   offsetof(RinKerberosOperationOwnerV1, operation) +
                       sizeof(((RinKerberosOperationOwnerV1*)0)->operation),
               "RinKerberosOperationOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_NET_KERBEROS_OPERATION_OWNER_ABI_H */
