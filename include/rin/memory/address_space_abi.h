// SPDX-License-Identifier: MIT
/*
 * RinOS native user address-space contract.
 *
 * Values in this header are shared by the kernel and RinNative runtimes.
 * They describe Rin's own ABI and are not Linux/POSIX compatibility values.
 */

#ifndef RIN_ADDRESS_SPACE_ABI_H
#define RIN_ADDRESS_SPACE_ABI_H

/* Shared lower bound for native user mappings.  The first page range below
 * this value is reserved so NULL and low-page arithmetic cannot become a
 * dereferenceable mapping. */
#define RIN_USER_ADDR_BASE 0x0000000000010000ULL

/* Exclusive upper bound for the canonical x86_64 user address space. */
#define RIN_USER_ADDR_TOP_EXCLUSIVE 0x0000800000000000ULL

/* Exclusive upper bound for a native x86_64 user stack mapping.  The stack
 * arena deliberately ends below the overall user address-space limit. */
#define RIN_USER_STACK_TOP_EXCLUSIVE 0x00007FFFFFF00000ULL

#endif /* RIN_ADDRESS_SPACE_ABI_H */
