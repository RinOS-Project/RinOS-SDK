/* SPDX-License-Identifier: MIT */
/* Public four-octet IPv4 address ABI. */
#ifndef RIN_SDK_NET_IPV4_H
#define RIN_SDK_NET_IPV4_H

#include <stdint.h>

#ifndef RIN_IPV4_ADDR_DEFINED
#define RIN_IPV4_ADDR_DEFINED
typedef struct RinIPv4Addr {
    uint8_t bytes[4];
} RinIPv4Addr;
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinIPv4Addr) == 4u,
              "RinIPv4Addr transport ABI must remain four bytes");
#else
_Static_assert(sizeof(RinIPv4Addr) == 4u,
               "RinIPv4Addr transport ABI must remain four bytes");
#endif

#endif
