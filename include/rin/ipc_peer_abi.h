/* SPDX-License-Identifier: MIT */
/* Kernel-authenticated IPC sender identity ABI. */

#ifndef RIN_SDK_IPC_PEER_ABI_H
#define RIN_SDK_IPC_PEER_ABI_H

#include <stdint.h>

/* Identity returned by the kernel with a received IPC message. This record is
 * intentionally independent of the SDK handle aliases so kernel code can
 * consume the authenticated identity without importing userspace APIs. */
typedef struct RinIpcPeerIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    uint64_t reserved;
} RinIpcPeerIdentityV1;

#if defined(__cplusplus)
static_assert(sizeof(RinIpcPeerIdentityV1) == 32u,
              "RinIpcPeerIdentityV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinIpcPeerIdentityV1) == 32u,
               "RinIpcPeerIdentityV1 ABI drift");
#endif

#endif /* RIN_SDK_IPC_PEER_ABI_H */
