/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_MULTICAST_SOURCE_FILTER_V1_H
#define RIN_SDK_MULTICAST_SOURCE_FILTER_V1_H

#include <stdint.h>

#define RIN_MCAST_SOURCE_FILTER_ABI_VERSION UINT32_C(1)
#define RIN_MCAST_SOURCE_FILTER_MAX_SOURCES UINT32_C(64)
#define RIN_MCAST_SOURCE_FILTER_V1 0x7f50
#define RIN_MCAST_FILTER_EXCLUDE UINT32_C(0)
#define RIN_MCAST_FILTER_INCLUDE UINT32_C(1)

#ifndef MCAST_JOIN_GROUP
#define MCAST_JOIN_GROUP 42
#endif
#ifndef MCAST_BLOCK_SOURCE
#define MCAST_BLOCK_SOURCE 43
#endif
#ifndef MCAST_UNBLOCK_SOURCE
#define MCAST_UNBLOCK_SOURCE 44
#endif
#ifndef MCAST_LEAVE_GROUP
#define MCAST_LEAVE_GROUP 45
#endif
#ifndef MCAST_JOIN_SOURCE_GROUP
#define MCAST_JOIN_SOURCE_GROUP 46
#endif
#ifndef MCAST_LEAVE_SOURCE_GROUP
#define MCAST_LEAVE_SOURCE_GROUP 47
#endif

/* Pass this fixed-width record to setsockopt/getsockopt at IPPROTO_IP or
 * IPPROTO_IPV6 with RIN_MCAST_SOURCE_FILTER_V1. Address bytes are in network
 * order. IPv4 uses the first four bytes and requires the remaining bytes to
 * be zero. Unused source entries and reserved words must be zero. The POSIX
 * group_req and group_source_req layouts are declared by libc's netinet/in.h. */
typedef struct RinMulticastSourceFilterAbiV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t address_family;
    uint32_t interface_index;
    uint32_t filter_mode;
    uint32_t source_count;
    uint32_t reserved[2];
    uint8_t group[16];
    uint8_t sources[RIN_MCAST_SOURCE_FILTER_MAX_SOURCES][16];
} RinMulticastSourceFilterAbiV1;

#if defined(__cplusplus)
static_assert(sizeof(RinMulticastSourceFilterAbiV1) == 1072u,
              "multicast source-filter ABI size drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinMulticastSourceFilterAbiV1) == 1072u,
               "multicast source-filter ABI size drift");
#endif

#endif
