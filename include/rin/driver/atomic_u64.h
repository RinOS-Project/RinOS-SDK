/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_ATOMIC_U64_H
#define RIN_DRIVER_ATOMIC_U64_H

#include <stdint.h>

/* i686 gives uint64_t only 4-byte type alignment. Use this ABI storage type
 * when a shared record requires naturally aligned 64-bit atomic fields. */
typedef uint64_t RinAtomicU64 __attribute__((aligned(8)));

#endif /* RIN_DRIVER_ATOMIC_U64_H */
