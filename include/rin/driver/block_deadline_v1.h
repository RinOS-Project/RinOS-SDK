/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_BLOCK_DEADLINE_V1_H
#define RIN_DRIVER_BLOCK_DEADLINE_V1_H

#include <stdint.h>

/* The OS-Core block owner initializes and updates this cancellation token.
 * Drivers treat it as read-only; only the OS-Core deadline helper checks the
 * flag so cancellation reads retain the kernel's atomic ordering. */
typedef struct RinBlockCancelToken {
    volatile uint32_t cancelled;
} RinBlockCancelToken;

/* A bounded block request deadline shared with the selected backend. The
 * timestamp is in the OS-Core monotonic-millisecond clock domain. The pointed
 * to cancellation token remains OS-Core-owned for the duration of the call. */
typedef struct RinBlockIoDeadline {
    uint64_t deadline_ms;
    const RinBlockCancelToken* cancel_token;
} RinBlockIoDeadline;

#endif /* RIN_DRIVER_BLOCK_DEADLINE_V1_H */
