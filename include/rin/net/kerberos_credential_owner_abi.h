/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_NET_KERBEROS_CREDENTIAL_OWNER_ABI_H
#define RIN_SDK_NET_KERBEROS_CREDENTIAL_OWNER_ABI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The Kerberos provider does not receive a username, password, ticket-cache
 * path, or keytab path for default credentials.  The product session owner
 * returns provider-owned opaque handles instead.  The owner is responsible
 * for binding the initiator handle to the live RinOS session identity and for
 * sourcing it from the authenticated credential store; it must not synthesize
 * an identity or use a fixed token.
 *
 * The handle is provider-owned.  Neither the PAL nor managed code dereferences
 * it, and no ccache/keytab bytes cross this ABI.  A provider may use the
 * handle to drive its RFC 4120/RFC 4121 implementation and must release it
 * through the owner before the provider handle becomes unreachable.
 */
#define RIN_KERBEROS_CREDENTIAL_OWNER_ABI_VERSION UINT16_C(1)
#define RIN_KERBEROS_CREDENTIAL_OWNER_CAP_SESSION_INITIATOR \
    UINT32_C(0x00000001)
#define RIN_KERBEROS_CREDENTIAL_OWNER_CAP_ACCEPTOR_KEYTAB \
    UINT32_C(0x00000002)
#define RIN_KERBEROS_CREDENTIAL_OWNER_KNOWN_CAPABILITIES \
    (RIN_KERBEROS_CREDENTIAL_OWNER_CAP_SESSION_INITIATOR | \
     RIN_KERBEROS_CREDENTIAL_OWNER_CAP_ACCEPTOR_KEYTAB)

#define RIN_KERBEROS_CREDENTIAL_OWNER_OK UINT32_C(0)
#define RIN_KERBEROS_CREDENTIAL_OWNER_UNAVAILABLE UINT32_C(1)
#define RIN_KERBEROS_CREDENTIAL_OWNER_INVALID_SESSION UINT32_C(2)
#define RIN_KERBEROS_CREDENTIAL_OWNER_EXPIRED UINT32_C(3)

typedef struct RinKerberosCredentialOwnerV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t capability_mask;
    void* context;

    /* Returns an opaque credential handle for the current authenticated
     * RinOS session.  desired_name is provider-owned and may be NULL for
     * GSS_C_NO_NAME; NULL must never become a synthesized username. */
    uint32_t (*acquire_session_initiator)(
        void* context, uint32_t* minor_status, void* desired_name,
        void** output);

    /* Returns an opaque acceptor credential backed by the product keytab
     * owner.  A missing keytab is an unavailable credential, not success. */
    uint32_t (*acquire_acceptor_keytab)(
        void* context, uint32_t* minor_status, void** output);

    /* Releases an owner handle through the same owner that issued it. */
    uint32_t (*release_credential)(
        void* context, uint32_t* minor_status, void** input);
} RinKerberosCredentialOwnerV1;

/* Product security owns this symbol.  A missing weak symbol means that the
 * Kerberos package cannot be advertised, even when a generic auth provider
 * is present. */
const RinKerberosCredentialOwnerV1*
rin_kerberos_credential_owner_get_v1(void);

#ifdef __cplusplus
}

static_assert(offsetof(RinKerberosCredentialOwnerV1, context) == 16u,
              "RinKerberosCredentialOwnerV1 context ABI drift");
static_assert(sizeof(RinKerberosCredentialOwnerV1) ==
                  offsetof(RinKerberosCredentialOwnerV1,
                           release_credential) +
                      sizeof(((RinKerberosCredentialOwnerV1*)0)->
                                 release_credential),
              "RinKerberosCredentialOwnerV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(offsetof(RinKerberosCredentialOwnerV1, context) == 16u,
               "RinKerberosCredentialOwnerV1 context ABI drift");
_Static_assert(sizeof(RinKerberosCredentialOwnerV1) ==
                   offsetof(RinKerberosCredentialOwnerV1,
                            release_credential) +
                       sizeof(((RinKerberosCredentialOwnerV1*)0)->
                                  release_credential),
               "RinKerberosCredentialOwnerV1 ABI drift");
#endif

#endif /* RIN_SDK_NET_KERBEROS_CREDENTIAL_OWNER_ABI_H */
