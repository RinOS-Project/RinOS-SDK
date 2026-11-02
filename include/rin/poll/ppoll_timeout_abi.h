/* SPDX-License-Identifier: MIT */
/* Versioned poll request with a signed 64-bit relative nanosecond timeout. */
#ifndef RIN_SDK_POLL_PPOLL_TIMEOUT_ABI_H
#define RIN_SDK_POLL_PPOLL_TIMEOUT_ABI_H

#include <stddef.h>
#include <stdint.h>

#define RIN_PPOLL_TIMEOUT64_USER_ABI_VERSION 1u
#define RIN_PPOLL_TIMEOUT64_USER_MAX_FDS 1024u
#define RIN_PPOLL_TIMEOUT64_USER_SIGSET_SIZE 8u

typedef struct RinPpollTimeout64UserV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t nfds;
    uint32_t sigset_size;
    int64_t timeout_ns;
    uint64_t fds_address;
    uint64_t sigmask_address;
    uint64_t reserved[2];
} RinPpollTimeout64UserV1;

#if defined(__cplusplus)
static_assert(sizeof(RinPpollTimeout64UserV1) == 56u,
              "RinPpollTimeout64UserV1 ABI drift");
static_assert(offsetof(RinPpollTimeout64UserV1, timeout_ns) == 16u,
              "RinPpollTimeout64UserV1.timeout_ns ABI drift");
static_assert(offsetof(RinPpollTimeout64UserV1, fds_address) == 24u,
              "RinPpollTimeout64UserV1.fds_address ABI drift");
#else
_Static_assert(sizeof(RinPpollTimeout64UserV1) == 56u,
               "RinPpollTimeout64UserV1 ABI drift");
_Static_assert(offsetof(RinPpollTimeout64UserV1, timeout_ns) == 16u,
               "RinPpollTimeout64UserV1.timeout_ns ABI drift");
_Static_assert(offsetof(RinPpollTimeout64UserV1, fds_address) == 24u,
               "RinPpollTimeout64UserV1.fds_address ABI drift");
#endif

static inline int rin_ppoll_timeout64_user_v1_valid(
    const RinPpollTimeout64UserV1* request)
{
    return request != NULL &&
           request->struct_size == sizeof(*request) &&
           request->version == RIN_PPOLL_TIMEOUT64_USER_ABI_VERSION &&
           request->flags == 0u &&
           request->nfds <= RIN_PPOLL_TIMEOUT64_USER_MAX_FDS &&
           request->sigset_size == RIN_PPOLL_TIMEOUT64_USER_SIGSET_SIZE &&
           request->timeout_ns >= -1 &&
           (request->nfds == 0u || request->fds_address != 0u) &&
           request->reserved[0] == 0u && request->reserved[1] == 0u;
}

#endif /* RIN_SDK_POLL_PPOLL_TIMEOUT_ABI_H */
