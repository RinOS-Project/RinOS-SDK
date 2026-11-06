/* SPDX-License-Identifier: MIT */
/* Versioned poll/select request with a signed 64-bit relative nanosecond
 * timeout. Select mode carries kernel-copied fd_set pointers and size flags. */
#ifndef RIN_SDK_POLL_PPOLL_TIMEOUT_ABI_H
#define RIN_SDK_POLL_PPOLL_TIMEOUT_ABI_H

#include <stddef.h>
#include <stdint.h>

#define RIN_PPOLL_TIMEOUT64_USER_ABI_VERSION 1u
#define RIN_PPOLL_TIMEOUT64_USER_MAX_FDS 1024u
#define RIN_PPOLL_TIMEOUT64_USER_SIGSET_SIZE 8u
#define RIN_PPOLL_TIMEOUT64_USER_FDSET_MAX_BYTES 128u
#define RIN_PPOLL_TIMEOUT64_USER_FLAG_SELECT UINT16_C(0x0001)
#define RIN_PPOLL_TIMEOUT64_USER_SELECT_SIZE_SHIFT 1u
#define RIN_PPOLL_TIMEOUT64_USER_SELECT_SIZE_MASK UINT16_C(0x01fe)
#define RIN_PPOLL_TIMEOUT64_USER_SELECT_FLAGS(byte_count) \
    ((uint16_t)(RIN_PPOLL_TIMEOUT64_USER_FLAG_SELECT | \
                ((uint16_t)(byte_count) << \
                 RIN_PPOLL_TIMEOUT64_USER_SELECT_SIZE_SHIFT)))
/* In select mode: fds_address is readfds, reserved[0] is writefds, and
 * reserved[1] is exceptfds. sigmask_address keeps its pselect meaning. */
#define RIN_PPOLL_TIMEOUT64_USER_SELECT_WRITE_ADDRESS(request) \
    ((request)->reserved[0])
#define RIN_PPOLL_TIMEOUT64_USER_SELECT_EXCEPT_ADDRESS(request) \
    ((request)->reserved[1])

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
    uint32_t fdset_bytes;
    uint16_t select_flags;
    if (request == NULL ||
        request->struct_size != sizeof(*request) ||
        request->version != RIN_PPOLL_TIMEOUT64_USER_ABI_VERSION ||
        request->nfds > RIN_PPOLL_TIMEOUT64_USER_MAX_FDS ||
        request->sigset_size != RIN_PPOLL_TIMEOUT64_USER_SIGSET_SIZE ||
        request->timeout_ns < -1)
        return 0;
    if (request->flags == 0u)
        return (request->nfds == 0u || request->fds_address != 0u) &&
               request->reserved[0] == 0u && request->reserved[1] == 0u;
    select_flags = (uint16_t)(RIN_PPOLL_TIMEOUT64_USER_FLAG_SELECT |
                              RIN_PPOLL_TIMEOUT64_USER_SELECT_SIZE_MASK);
    if ((request->flags & RIN_PPOLL_TIMEOUT64_USER_FLAG_SELECT) == 0u ||
        (request->flags & (uint16_t)~select_flags) != 0u)
        return 0;
    fdset_bytes = (uint32_t)(request->flags &
                             RIN_PPOLL_TIMEOUT64_USER_SELECT_SIZE_MASK) >>
                  RIN_PPOLL_TIMEOUT64_USER_SELECT_SIZE_SHIFT;
    return fdset_bytes != 0u &&
           fdset_bytes <= RIN_PPOLL_TIMEOUT64_USER_FDSET_MAX_BYTES &&
           request->nfds <= fdset_bytes * 8u;
}

#endif /* RIN_SDK_POLL_PPOLL_TIMEOUT_ABI_H */
