// SPDX-License-Identifier: MIT
#ifndef RIN_SDK_DNS_ABI_H
#define RIN_SDK_DNS_ABI_H

#include <stdint.h>
#include "../syscall_abi.h"

#define RIN_DNS_NAME_MAX 255
#define RIN_DNS_GETADDRINFO_MAX_RESULTS 8u
/* A dual-family resolver response may legally carry the bounded answer set
 * for both families: up to eight AAAA records and up to eight A records. */
#define RIN_DNS_GETADDRINFO_DUAL_MAX_RESULTS \
    (RIN_DNS_GETADDRINFO_MAX_RESULTS * 2u)
#define RIN_DNS_AI_PASSIVE 0x0001u
#define RIN_DNS_AI_CANONNAME 0x0002u
#define RIN_DNS_AI_NUMERICHOST 0x0004u
#define RIN_DNS_AI_NUMERICSERV 0x0008u
#define RIN_DNS_AI_SUPPORTED_FLAGS \
    (RIN_DNS_AI_PASSIVE | RIN_DNS_AI_CANONNAME | \
     RIN_DNS_AI_NUMERICHOST | RIN_DNS_AI_NUMERICSERV)

#define RIN_DNS_STATUS_DONE       0u
#define RIN_DNS_STATUS_PENDING    1u
#define RIN_DNS_STATUS_FAILED     2u
#define RIN_DNS_STATUS_TIMED_OUT  3u
#define RIN_DNS_STATUS_CANCELLED  4u
#define RIN_DNS_STATUS_CONFIG_MISSING 5u
#define RIN_DNS_STATUS_SEND_BLOCKED  6u
#define RIN_DNS_STATUS_NOT_FOUND     7u

typedef struct RinDnsResolveResult {
    uint8_t addr[4];
    uint8_t reserved0[4];
    uint32_t status;
} RinDnsResolveResult;

/* Legacy nonblocking getaddrinfo result.  V1 publishes one address per
 * transaction and remains available to callers that have not adopted V2. */
typedef struct RinDnsGetAddrInfoResult {
    int32_t ai_flags;
    int32_t ai_family;
    int32_t ai_socktype;
    int32_t ai_protocol;
    uint32_t ai_addrlen;
    uint32_t canon_len;
    uint16_t port;
    uint16_t reserved0;
    uint8_t addr[16];
    char canonical_name[RIN_DNS_NAME_MAX + 1u];
    uint32_t status;
    uint32_t reserved1;
} RinDnsGetAddrInfoResult;

/* The app runtime expands the compact service V2 payload into independent
 * records so libcxx can apply the same validation to every endpoint before
 * publishing any result. */
typedef struct RinDnsGetAddrInfoBatchResult {
    RinDnsGetAddrInfoResult entries[RIN_DNS_GETADDRINFO_MAX_RESULTS];
    uint32_t count;
    uint32_t status;
    uint32_t reserved0;
    uint32_t reserved1;
} RinDnsGetAddrInfoBatchResult;

/* The asynchronous V3 resolver result preserves the wire order: all IPv6
 * endpoints first, followed by all IPv4 endpoints.  Unlike V2, `entries`
 * may therefore contain mixed address families. */
typedef struct RinDnsGetAddrInfoDualResult {
    RinDnsGetAddrInfoResult entries[RIN_DNS_GETADDRINFO_DUAL_MAX_RESULTS];
    uint32_t count;
    uint32_t status;
    uint32_t reserved0;
    uint32_t reserved1;
} RinDnsGetAddrInfoDualResult;

#endif
