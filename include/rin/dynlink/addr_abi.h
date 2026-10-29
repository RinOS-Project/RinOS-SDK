/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_RIN_DYNLINK_ADDR_ABI_H
#define RIN_SDK_RIN_DYNLINK_ADDR_ABI_H

#include <stdint.h>

#define RIN_DLADDR_RLL_V1_ABI_VERSION 1u
#define RIN_DLADDR_RLL_V1_LIBRARY_NAME_CAPACITY 64u
#define RIN_DLADDR_RLL_V1_SYMBOL_NAME_CAPACITY 129u
#define RIN_DLADDR_RLL_V1_FLAG_DATA 0x00000001u
/* The same fixed-width carrier is used by native RIN v3.  A native record
 * is still distinguishable from a legacy RLL record, while preserving the
 * 32-bit syscall copy-out size and the existing libc validation path. */
#define RIN_DLADDR_RLL_V1_FLAG_NATIVE 0x00000002u
#define RIN_DLADDR_NATIVE_V1_ABI_VERSION RIN_DLADDR_RLL_V1_ABI_VERSION
#define RIN_DLADDR_NATIVE_V1_FLAG_DATA RIN_DLADDR_RLL_V1_FLAG_DATA
#define RIN_DLADDR_NATIVE_V1_FLAG_NATIVE RIN_DLADDR_RLL_V1_FLAG_NATIVE

/* Address ownership returned by the legacy RLL32 loader.  The fixed-width
 * fields keep the record identical for an IA-32 caller and an x86_64 kernel;
 * names are copied only after the kernel has validated their bounded NUL
 * terminators. */
typedef struct RinDlAddrRllRecordV1 {
    uint32_t abi_version;
    uint32_t flags;
    uint64_t load_address;
    uint64_t mapped_size;
    uint64_t symbol_address;
    uint32_t library_name_length;
    uint32_t symbol_name_length;
    char library_name[RIN_DLADDR_RLL_V1_LIBRARY_NAME_CAPACITY];
    char symbol_name[RIN_DLADDR_RLL_V1_SYMBOL_NAME_CAPACITY];
} __attribute__((aligned(8))) RinDlAddrRllRecordV1;

#ifdef __cplusplus
static_assert(sizeof(RinDlAddrRllRecordV1) == 240u,
              "RinDlAddrRllRecordV1 ABI size");
#else
_Static_assert(sizeof(RinDlAddrRllRecordV1) == 240u,
               "RinDlAddrRllRecordV1 ABI size");
#endif

#endif /* RIN_SDK_RIN_DYNLINK_ADDR_ABI_H */
