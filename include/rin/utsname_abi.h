/* SPDX-License-Identifier: MIT */
/* Stable userspace wire layout for SYS_UNAME. */

#ifndef RIN_SDK_UTSNAME_ABI_H
#define RIN_SDK_UTSNAME_ABI_H

#define RIN_UTSNAME_ABI_V1_FIELD_SIZE 65u
#define RIN_UTSNAME_ABI_V1_FIELDS                                      \
    char sysname[RIN_UTSNAME_ABI_V1_FIELD_SIZE];                        \
    char nodename[RIN_UTSNAME_ABI_V1_FIELD_SIZE];                       \
    char release[RIN_UTSNAME_ABI_V1_FIELD_SIZE];                        \
    char version[RIN_UTSNAME_ABI_V1_FIELD_SIZE];                        \
    char machine[RIN_UTSNAME_ABI_V1_FIELD_SIZE];                        \
    char domainname[RIN_UTSNAME_ABI_V1_FIELD_SIZE]

typedef struct RinUtsnameAbiV1 {
    RIN_UTSNAME_ABI_V1_FIELDS;
} RinUtsnameAbiV1;

#ifndef MIDL_PASS
static inline void rin_utsname_abi_v1_copy(
    char output[RIN_UTSNAME_ABI_V1_FIELD_SIZE], const char* input)
{
    unsigned int index = 0u;
    if (!input) return;
    while (index + 1u < RIN_UTSNAME_ABI_V1_FIELD_SIZE && input[index]) {
        output[index] = input[index];
        ++index;
    }
    output[index] = '\0';
}

static inline void rin_utsname_abi_v1_fill(RinUtsnameAbiV1* value,
                                            const char* machine)
{
    volatile unsigned char* bytes;
    unsigned int index;
    if (!value) return;
    bytes = (volatile unsigned char*)value;
    for (index = 0u; index < (unsigned int)sizeof(*value); ++index)
        bytes[index] = 0u;
    rin_utsname_abi_v1_copy(value->sysname, "RinOS");
    rin_utsname_abi_v1_copy(value->nodename, "rin");
    rin_utsname_abi_v1_copy(value->release, "2.1.0");
    rin_utsname_abi_v1_copy(value->version, "RinOS Kernel 2.1.0");
    rin_utsname_abi_v1_copy(value->machine, machine);
}
#endif /* !MIDL_PASS */

#endif /* RIN_SDK_UTSNAME_ABI_H */
