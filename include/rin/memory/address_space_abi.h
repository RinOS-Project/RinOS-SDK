// SPDX-License-Identifier: MIT
/*
 * RinOS native user address-space contract.
 *
 * Values in this header are shared by the kernel and RinNative runtimes.
 * They describe Rin's own ABI and are not Linux/POSIX compatibility values.
 */

#ifndef RIN_ADDRESS_SPACE_ABI_H
#define RIN_ADDRESS_SPACE_ABI_H

/* Exclusive upper bound for a native x86_64 user stack mapping. */
#define RIN_USER_STACK_TOP_EXCLUSIVE 0x00007FFFFFF00000ULL

#endif /* RIN_ADDRESS_SPACE_ABI_H */
