/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_NET_AUTH_PROVIDER_ABI_H
#define RIN_SDK_NET_AUTH_PROVIDER_ABI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_AUTH_PROVIDER_ABI_VERSION UINT16_C(1)
#define RIN_AUTH_PROVIDER_PACKAGE_NEGOTIATE UINT32_C(1u << 0)
#define RIN_AUTH_PROVIDER_PACKAGE_NTLM UINT32_C(1u << 1)
#define RIN_AUTH_PROVIDER_PACKAGE_KERBEROS UINT32_C(1u << 2)
#define RIN_AUTH_PROVIDER_KNOWN_PACKAGES \
    (RIN_AUTH_PROVIDER_PACKAGE_NEGOTIATE | \
     RIN_AUTH_PROVIDER_PACKAGE_NTLM | \
     RIN_AUTH_PROVIDER_PACKAGE_KERBEROS)
#define RIN_AUTH_PROVIDER_MAX_TOKEN_SIZE UINT32_C(65536)

#define RIN_AUTH_PROVIDER_OK UINT32_C(0)
#define RIN_AUTH_PROVIDER_UNAVAILABLE UINT32_C(1)
#define RIN_AUTH_PROVIDER_ABI_MISMATCH UINT32_C(2)
/* Callback status values are provider-local and must be translated by the
 * native PAL before they reach the GSS ABI. */
#define RIN_AUTH_PROVIDER_CONTINUE_NEEDED UINT32_C(3)

typedef struct RinAuthProviderBufferV1 {
    uint64_t length;
    uint8_t* data;
} RinAuthProviderBufferV1;

/* Buffer-producing callbacks must return provider-owned storage through this
 * pair. The PAL releases malformed or oversized buffers before exposing them
 * to managed code; max_token_size is therefore an enforced output bound. */

/* The handles are provider-owned opaque objects. The PAL only forwards them
 * back to the same provider and never dereferences or stores their contents. */
typedef struct RinAuthProviderV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t package_mask;
    uint32_t max_token_size;
    void* context;

    void (*release_buffer)(void* context, void* buffer, uint64_t length);
    uint32_t (*display_minor_status)(void* context, uint32_t* minor_status,
                                     uint32_t status_value,
                                     RinAuthProviderBufferV1* output);
    uint32_t (*display_major_status)(void* context, uint32_t* minor_status,
                                     uint32_t status_value,
                                     RinAuthProviderBufferV1* output);
    uint32_t (*import_user_name)(void* context, uint32_t* minor_status,
                                 char* input, uint32_t input_length,
                                 void** output);
    uint32_t (*import_principal_name)(void* context, uint32_t* minor_status,
                                      char* input, uint32_t input_length,
                                      void** output);
    uint32_t (*release_name)(void* context, uint32_t* minor_status,
                             void** input);
    uint32_t (*acquire_acceptor_cred)(void* context, uint32_t* minor_status,
                                      void** output);
    uint32_t (*initiate_cred_spnego)(void* context, uint32_t* minor_status,
                                     void* desired_name, void** output);
    uint32_t (*release_cred)(void* context, uint32_t* minor_status,
                             void** input);
    uint32_t (*init_sec_context)(
        void* context, uint32_t* minor_status, void* claimant_cred,
        void** security_context, uint32_t package_type, void* target_name,
        uint32_t requested_flags, uint8_t* input, uint32_t input_length,
        RinAuthProviderBufferV1* output, uint32_t* return_flags,
        int32_t* ntlm_used);
    uint32_t (*init_sec_context_ex)(
        void* context, uint32_t* minor_status, void* claimant_cred,
        void** security_context, uint32_t package_type, void* channel_binding,
        int32_t channel_binding_size, void* target_name,
        uint32_t requested_flags, uint8_t* input, uint32_t input_length,
        RinAuthProviderBufferV1* output, uint32_t* return_flags,
        int32_t* ntlm_used);
    uint32_t (*accept_sec_context)(
        void* context, uint32_t* minor_status, void* acceptor_cred,
        void** security_context, void* channel_binding,
        int32_t channel_binding_size, uint8_t* input, uint32_t input_length,
        RinAuthProviderBufferV1* output, uint32_t* return_flags,
        int32_t* ntlm_used);
    uint32_t (*delete_sec_context)(void* context, uint32_t* minor_status,
                                   void** security_context);
    uint32_t (*wrap)(void* context, uint32_t* minor_status,
                     void* security_context, int32_t* encrypt,
                     uint8_t* input, int32_t input_length,
                     RinAuthProviderBufferV1* output);
    uint32_t (*unwrap)(void* context, uint32_t* minor_status,
                       void* security_context, int32_t* encrypt,
                       uint8_t* input, int32_t input_length,
                       RinAuthProviderBufferV1* output);
    uint32_t (*get_mic)(void* context, uint32_t* minor_status,
                        void* security_context, uint8_t* input,
                        int32_t input_length, RinAuthProviderBufferV1* output);
    uint32_t (*verify_mic)(void* context, uint32_t* minor_status,
                           void* security_context, uint8_t* input,
                           int32_t input_length, uint8_t* token,
                           int32_t token_length);
    uint32_t (*initiate_cred_with_password)(
        void* context, uint32_t* minor_status, int32_t package_type,
        void* desired_name, char* password, uint32_t password_length,
        void** output);
    uint32_t (*is_ntlm_installed)(void* context);
    uint32_t (*get_user)(void* context, uint32_t* minor_status,
                         void* security_context,
                         RinAuthProviderBufferV1* output);
} RinAuthProviderV1;

/* Product security owns this symbol. A missing weak symbol means that no
 * provider is installed and the System.Net.Security PAL stays unavailable. */
const RinAuthProviderV1* rin_auth_provider_get_v1(void);

#ifdef __cplusplus
}
#endif

#if defined(__cplusplus)
#define RIN_AUTH_PROVIDER_ALIGNOF(type) alignof(type)
#else
#define RIN_AUTH_PROVIDER_ALIGNOF(type) _Alignof(type)
#endif
#define RIN_AUTH_PROVIDER_ALIGN_UP(value, alignment) \
    (((value) + (alignment) - 1u) / (alignment) * (alignment))
#define RIN_AUTH_PROVIDER_BUFFER_V1_EXPECTED_SIZE \
    RIN_AUTH_PROVIDER_ALIGN_UP( \
        sizeof(uint64_t) + sizeof(void*), \
        RIN_AUTH_PROVIDER_ALIGNOF(RinAuthProviderBufferV1))
#define RIN_AUTH_PROVIDER_CALLBACK_POINTER_SIZE \
    sizeof(((RinAuthProviderV1*)0)->release_buffer)
#define RIN_AUTH_PROVIDER_V1_EXPECTED_SIZE \
    RIN_AUTH_PROVIDER_ALIGN_UP( \
        offsetof(RinAuthProviderV1, release_buffer) + \
            20u * RIN_AUTH_PROVIDER_CALLBACK_POINTER_SIZE, \
        RIN_AUTH_PROVIDER_ALIGNOF(RinAuthProviderV1))

#if defined(__cplusplus)
static_assert(offsetof(RinAuthProviderBufferV1, length) == 0u,
              "RinAuthProviderBufferV1.length ABI drift");
static_assert(offsetof(RinAuthProviderBufferV1, data) == sizeof(uint64_t),
              "RinAuthProviderBufferV1.data ABI drift");
static_assert(sizeof(RinAuthProviderBufferV1) ==
                  RIN_AUTH_PROVIDER_BUFFER_V1_EXPECTED_SIZE,
              "RinAuthProviderBufferV1 ABI drift");
static_assert(offsetof(RinAuthProviderV1, context) == 16u,
              "RinAuthProviderV1.context ABI drift");
static_assert(offsetof(RinAuthProviderV1, release_buffer) ==
                  16u + sizeof(void*),
              "RinAuthProviderV1 callback table ABI drift");
static_assert(sizeof(RinAuthProviderV1) == RIN_AUTH_PROVIDER_V1_EXPECTED_SIZE,
              "RinAuthProviderV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(RinAuthProviderBufferV1, length) == 0u,
               "RinAuthProviderBufferV1.length ABI drift");
_Static_assert(offsetof(RinAuthProviderBufferV1, data) == sizeof(uint64_t),
               "RinAuthProviderBufferV1.data ABI drift");
_Static_assert(sizeof(RinAuthProviderBufferV1) ==
                   RIN_AUTH_PROVIDER_BUFFER_V1_EXPECTED_SIZE,
               "RinAuthProviderBufferV1 ABI drift");
_Static_assert(offsetof(RinAuthProviderV1, context) == 16u,
               "RinAuthProviderV1.context ABI drift");
_Static_assert(offsetof(RinAuthProviderV1, release_buffer) ==
                   16u + sizeof(void*),
               "RinAuthProviderV1 callback table ABI drift");
_Static_assert(sizeof(RinAuthProviderV1) == RIN_AUTH_PROVIDER_V1_EXPECTED_SIZE,
               "RinAuthProviderV1 ABI drift");
#endif

#undef RIN_AUTH_PROVIDER_ALIGNOF
#undef RIN_AUTH_PROVIDER_ALIGN_UP
#undef RIN_AUTH_PROVIDER_BUFFER_V1_EXPECTED_SIZE
#undef RIN_AUTH_PROVIDER_CALLBACK_POINTER_SIZE
#undef RIN_AUTH_PROVIDER_V1_EXPECTED_SIZE

#endif /* RIN_SDK_NET_AUTH_PROVIDER_ABI_H */
