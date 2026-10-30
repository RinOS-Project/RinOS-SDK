// SPDX-License-Identifier: MIT
#ifndef RIN_SDK_IPC_SHM_ABI_H
#define RIN_SDK_IPC_SHM_ABI_H

#include <rin/contract_abi.h>

#define RIN_SHM_NAME_MAX 64

#define RIN_SHM_FLAG_CREAT            0x00000001u
#define RIN_SHM_FLAG_EXCL             0x00000002u
#define RIN_SHM_FLAG_UNLINK_ON_CLOSE  0x00000004u

#define RIN_SHM_PROT_READ   0x00000001u
#define RIN_SHM_PROT_WRITE  0x00000002u
/* Executable aliases are x86_64-only and must never be combined with WRITE.
 * The kernel enforces W^X for every SHM mapping; this bit is not a request
 * for a writable+executable page. */
#define RIN_SHM_PROT_EXEC   0x00000004u

/* The x86 syscall register ABI cannot carry these 64-bit ranges in a single
 * call. RinShmAttachRangeV1 and RinShmProtectV1 in contract_abi.h carry the
 * full-width values and have generated field-offset assertions. */

/* The current page-backed SHM implementation is eagerly allocated.  Keep
 * the bound explicit so NativeAOT/CoreCLR cannot silently request a sparse
 * file-sized mapping that RinOS cannot supply. */
#define RIN_SHM_MAX_SIZE    0x10000000ULL

#endif
