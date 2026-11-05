/* SPDX-License-Identifier: MIT */
/* Linux-compatible fixed-width cachestat UAPI records. */
#ifndef RIN_SDK_LINUX_CACHESTAT_ABI_H
#define RIN_SDK_LINUX_CACHESTAT_ABI_H

#include <stdint.h>

struct cachestat_range {
    uint64_t off;
    uint64_t len;
};

struct cachestat {
    uint64_t nr_cache;
    uint64_t nr_dirty;
    uint64_t nr_writeback;
    uint64_t nr_evicted;
    uint64_t nr_recently_evicted;
};

#if defined(__cplusplus) && !defined(MIDL_PASS)
static_assert(sizeof(struct cachestat_range) == 16u,
              "cachestat_range ABI size");
static_assert(sizeof(struct cachestat) == 40u, "cachestat ABI size");
#elif !defined(MIDL_PASS)
_Static_assert(sizeof(struct cachestat_range) == 16u,
               "cachestat_range ABI size");
_Static_assert(sizeof(struct cachestat) == 40u, "cachestat ABI size");
#endif

#endif /* RIN_SDK_LINUX_CACHESTAT_ABI_H */
