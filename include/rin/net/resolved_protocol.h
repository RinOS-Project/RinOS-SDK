/* SPDX-License-Identifier: MIT */

#ifndef RIN_SDK_NET_RESOLVED_PROTOCOL_H
#define RIN_SDK_NET_RESOLVED_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#include <rin/net/socket_abi.h>
#include "resolved_nameinfo_policy.h"

#define RIN_RESOLVED_MAGIC 0x52445631u /* "RDV1" */
#define RIN_RESOLVED_VERSION 1u
#define RIN_RESOLVED_SERVICE_ID "resolved"
#define RIN_RESOLVED_APP_SCOPE 0u
#define RIN_RESOLVED_USER_SCOPE 2u
#define RIN_RESOLVED_STATUS_PERMISSION_DENIED (-13)
#define RIN_RESOLVED_EAI_BADFLAGS (-1)
#define RIN_RESOLVED_EAI_NONAME (-2)
#define RIN_RESOLVED_EAI_AGAIN (-3)
#define RIN_RESOLVED_EAI_FAIL (-4)
#define RIN_RESOLVED_EAI_FAMILY (-6)
#define RIN_RESOLVED_EAI_SOCKTYPE (-7)
#define RIN_RESOLVED_EAI_SERVICE (-8)
#define RIN_RESOLVED_EAI_MEMORY (-10)
#define RIN_RESOLVED_EAI_SYSTEM (-11)
#define RIN_RESOLVED_AF_UNSPEC 0
#define RIN_RESOLVED_AF_INET 2
#define RIN_RESOLVED_AF_INET6 10
#define RIN_RESOLVED_SOCK_STREAM 1
#define RIN_RESOLVED_SOCK_DGRAM 2
#define RIN_RESOLVED_IPPROTO_TCP 6
#define RIN_RESOLVED_IPPROTO_UDP 17
#define RIN_RESOLVED_IPV4_ADDRESS_BYTES 4u
#define RIN_RESOLVED_IPV6_ADDRESS_BYTES 16u
#define RIN_RESOLVED_SOCKADDR_IN_BYTES 16u
#define RIN_RESOLVED_SOCKADDR_IN6_BYTES 28u
#define RIN_RESOLVED_HOST_NAME_MAX 255u
#define RIN_RESOLVED_SERVICE_NAME_MAX 5u
#define RIN_RESOLVED_GAI_PASSIVE 1u
#define RIN_RESOLVED_GAI_CANONNAME 2u
#define RIN_RESOLVED_GAI_NUMERICHOST 4u
#define RIN_RESOLVED_GAI_NUMERICSERV 8u
#define RIN_RESOLVED_GAI_MAX_RESULTS 8u
#define RIN_RESOLVED_GAI_SUPPORTED_FLAGS \
    (RIN_RESOLVED_GAI_PASSIVE | RIN_RESOLVED_GAI_CANONNAME | \
     RIN_RESOLVED_GAI_NUMERICHOST | RIN_RESOLVED_GAI_NUMERICSERV)
#define RIN_RESOLVED_IPV6_REVERSE_NAME_BYTES 73u
#define RIN_RESOLVED_NAMEINFO_NUMERICHOST 1u
#define RIN_RESOLVED_NAMEINFO_NUMERICSERV 2u
#define RIN_RESOLVED_NAMEINFO_NOFQDN 4u
#define RIN_RESOLVED_NAMEINFO_NAMEREQD 8u
#define RIN_RESOLVED_NAMEINFO_DGRAM 16u
#define RIN_RESOLVED_NAMEINFO_SUPPORTED_FLAGS \
    (RIN_RESOLVED_NAMEINFO_NUMERICHOST | \
     RIN_RESOLVED_NAMEINFO_NUMERICSERV | \
     RIN_RESOLVED_NAMEINFO_NOFQDN | \
     RIN_RESOLVED_NAMEINFO_NAMEREQD | RIN_RESOLVED_NAMEINFO_DGRAM)

#define RIN_RESOLVED_CONFIG_V2_VERSION 2u
#define RIN_RESOLVED_CONFIG_V3_VERSION 3u
#define RIN_RESOLVED_SEARCH_DOMAIN_MAX 4u

enum {
    RIN_RESOLVED_CMD_RESOLVE_A_V1 = 1,
    RIN_RESOLVED_CMD_GETADDRINFO_V1 = 2,
    RIN_RESOLVED_CMD_GETNAMEINFO_V1 = 3,
    RIN_RESOLVED_CMD_SET_SERVERS_V1 = 4,
    RIN_RESOLVED_CMD_FLUSH_CACHE_V1 = 5,
    RIN_RESOLVED_CMD_SET_CONFIG_V2 = 6,
    RIN_RESOLVED_CMD_SET_CONFIG_V3 = 7,
    RIN_RESOLVED_CMD_GETADDRINFO_V2 = 8,
    /* One AF_UNSPEC request, with A and AAAA lookups owned concurrently by
     * resolved.  Unlike V2, the response contains both address families. */
    RIN_RESOLVED_CMD_GETADDRINFO_DUAL_V3 = 9
};

typedef struct RinResolvedMsgHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t command;
    int32_t status;
    uint32_t payload_len;
    uint32_t reserved0;
} RinResolvedMsgHeader;

typedef struct RinResolvedResolveARequest {
    uint32_t hostname_len;
} RinResolvedResolveARequest;

typedef struct RinResolvedResolveAResponse {
    uint8_t addr[4];
    uint8_t reserved0[4];
} RinResolvedResolveAResponse;

typedef struct RinResolvedGetAddrInfoRequest {
    int32_t ai_flags;
    int32_t ai_family;
    int32_t ai_socktype;
    int32_t ai_protocol;
    uint32_t node_len;
    uint32_t service_len;
} RinResolvedGetAddrInfoRequest;

typedef struct RinResolvedGetAddrInfoResponse {
    int32_t ai_flags;
    int32_t ai_family;
    int32_t ai_socktype;
    int32_t ai_protocol;
    uint32_t ai_addrlen;
    uint32_t canon_len;
    uint16_t port;
    uint16_t reserved0;
    uint8_t addr[16];
} RinResolvedGetAddrInfoResponse;

/* V2 keeps request semantics but returns every admitted address from one DNS
 * answer section.  The variable payload is `address_count` contiguous
 * family-sized addresses followed by the optional canonical name. */
typedef struct RinResolvedGetAddrInfoBatchResponse {
    int32_t ai_flags;
    int32_t ai_family;
    int32_t ai_socktype;
    int32_t ai_protocol;
    uint32_t ai_addrlen;
    uint32_t address_count;
    uint32_t canon_len;
    uint16_t port;
    uint16_t reserved0;
} RinResolvedGetAddrInfoBatchResponse;

/* V3 is deliberately limited to AF_UNSPEC.  The payload is exactly
 * `ipv6_count * 16` bytes, then `ipv4_count * 4` bytes, then canon_len text.
 * Address order is IPv6 first, IPv4 second; each family keeps DNS answer
 * order.  A successful response must contain at least one address. */
typedef struct RinResolvedGetAddrInfoDualResponse {
    int32_t ai_flags;
    int32_t ai_socktype;
    int32_t ai_protocol;
    uint32_t ipv6_count;
    uint32_t ipv4_count;
    uint32_t canon_len;
    uint16_t port;
    uint16_t reserved0;
} RinResolvedGetAddrInfoDualResponse;

#if defined(__cplusplus)
static_assert(sizeof(RinResolvedGetAddrInfoDualResponse) == 28u,
              "RinResolvedGetAddrInfoDualResponse ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinResolvedGetAddrInfoDualResponse) == 28u,
               "RinResolvedGetAddrInfoDualResponse ABI drift");
#endif

typedef struct RinResolvedGetNameInfoRequest {
    uint32_t flags;
    int32_t family;
    uint32_t addr_len;
    uint16_t port;
    uint16_t reserved0;
    uint8_t addr[16];
} RinResolvedGetNameInfoRequest;

typedef struct RinResolvedGetNameInfoResponse {
    uint32_t host_len;
    uint32_t serv_len;
} RinResolvedGetNameInfoResponse;

typedef struct RinResolvedSetServersRequest {
    uint32_t count;
    uint8_t addrs[4][4];
} RinResolvedSetServersRequest;

typedef struct RinResolvedSetConfigV2 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t count;
    uint32_t local_domain_len;
    uint8_t addrs[4][4];
    char local_domain[RIN_RESOLVED_LOCAL_DOMAIN_MAX + 3u];
} RinResolvedSetConfigV2;

/* V3 preserves the V2 server/local-domain contract and adds a bounded,
 * canonical DHCP domain-search snapshot.  It deliberately uses fixed-size
 * storage so the service can validate and publish it atomically. */
typedef struct RinResolvedSetConfigV3 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t count;
    uint32_t local_domain_len;
    uint8_t addrs[4][4];
    char local_domain[RIN_RESOLVED_LOCAL_DOMAIN_MAX + 3u];
    uint32_t search_domain_count;
    uint32_t search_domain_lens[RIN_RESOLVED_SEARCH_DOMAIN_MAX];
    char search_domains[RIN_RESOLVED_SEARCH_DOMAIN_MAX]
                       [RIN_RESOLVED_LOCAL_DOMAIN_MAX + 3u];
} RinResolvedSetConfigV3;

#if defined(__cplusplus)
static_assert(sizeof(RinResolvedSetConfigV2) == 288u,
              "RinResolvedSetConfigV2 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinResolvedSetConfigV2) == 288u,
               "RinResolvedSetConfigV2 ABI drift");
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinResolvedSetConfigV3) == 1332u,
              "RinResolvedSetConfigV3 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinResolvedSetConfigV3) == 1332u,
               "RinResolvedSetConfigV3 ABI drift");
#endif

static inline int rin_resolved_request_header_valid(
    const RinResolvedMsgHeader* header,
    uint32_t maximum_payload)
{
    return header != NULL && header->magic == RIN_RESOLVED_MAGIC &&
           header->version == RIN_RESOLVED_VERSION && header->status == 0 &&
           header->reserved0 == 0u && header->payload_len <= maximum_payload;
}

static inline int rin_resolved_response_header_valid(
    const RinResolvedMsgHeader* header,
    uint32_t expected_command,
    uint32_t payload_capacity)
{
    return header != NULL && header->magic == RIN_RESOLVED_MAGIC &&
           header->version == RIN_RESOLVED_VERSION &&
           header->command == expected_command && header->reserved0 == 0u &&
           header->payload_len <= payload_capacity;
}

static inline uint32_t rin_resolved_address_bytes_for_family(int32_t family)
{
    if (family == RIN_RESOLVED_AF_INET) {
        return RIN_RESOLVED_IPV4_ADDRESS_BYTES;
    }
    if (family == RIN_RESOLVED_AF_INET6) {
        return RIN_RESOLVED_IPV6_ADDRESS_BYTES;
    }
    return 0u;
}

static inline uint32_t rin_resolved_sockaddr_bytes_for_family(int32_t family)
{
    if (family == RIN_RESOLVED_AF_INET) {
        return RIN_RESOLVED_SOCKADDR_IN_BYTES;
    }
    if (family == RIN_RESOLVED_AF_INET6) {
        return RIN_RESOLVED_SOCKADDR_IN6_BYTES;
    }
    return 0u;
}

static inline int rin_resolved_getaddrinfo_status_valid(int32_t status)
{
    return status == 0 || status == RIN_RESOLVED_EAI_BADFLAGS ||
           status == RIN_RESOLVED_EAI_NONAME ||
           status == RIN_RESOLVED_EAI_AGAIN ||
           status == RIN_RESOLVED_EAI_FAIL ||
           status == RIN_RESOLVED_EAI_FAMILY ||
           status == RIN_RESOLVED_EAI_SOCKTYPE ||
           status == RIN_RESOLVED_EAI_SERVICE ||
           status == RIN_RESOLVED_EAI_MEMORY ||
           status == RIN_RESOLVED_EAI_SYSTEM;
}

static inline int rin_resolved_getaddrinfo_socket_pair(
    int32_t socket_type,
    int32_t protocol,
    int32_t* effective_socket_type,
    int32_t* effective_protocol)
{
    int32_t output_socket_type;
    int32_t output_protocol;
    if (socket_type == 0) {
        if (protocol == 0 || protocol == RIN_RESOLVED_IPPROTO_TCP) {
            output_socket_type = RIN_RESOLVED_SOCK_STREAM;
            output_protocol = RIN_RESOLVED_IPPROTO_TCP;
        } else if (protocol == RIN_RESOLVED_IPPROTO_UDP) {
            output_socket_type = RIN_RESOLVED_SOCK_DGRAM;
            output_protocol = RIN_RESOLVED_IPPROTO_UDP;
        } else {
            return 0;
        }
    } else if (socket_type == RIN_RESOLVED_SOCK_STREAM &&
               (protocol == 0 || protocol == RIN_RESOLVED_IPPROTO_TCP)) {
        output_socket_type = RIN_RESOLVED_SOCK_STREAM;
        output_protocol = RIN_RESOLVED_IPPROTO_TCP;
    } else if (socket_type == RIN_RESOLVED_SOCK_DGRAM &&
               (protocol == 0 || protocol == RIN_RESOLVED_IPPROTO_UDP)) {
        output_socket_type = RIN_RESOLVED_SOCK_DGRAM;
        output_protocol = RIN_RESOLVED_IPPROTO_UDP;
    } else {
        return 0;
    }
    if (effective_socket_type) *effective_socket_type = output_socket_type;
    if (effective_protocol) *effective_protocol = output_protocol;
    return 1;
}

static inline int rin_resolved_text_bytes_valid(const uint8_t* text,
                                                 uint32_t length,
                                                 uint32_t maximum)
{
    uint32_t index;
    if (length == 0u) return 1;
    if (!text || length > maximum) return 0;
    for (index = 0u; index < length; ++index) {
        if (text[index] < 0x21u || text[index] > 0x7eu) return 0;
    }
    return 1;
}

static inline int rin_resolved_service_port_bytes(
    const uint8_t* service,
    uint32_t length,
    uint16_t* port_out)
{
    uint32_t index;
    uint32_t port = 0u;
    if (!port_out || length > RIN_RESOLVED_SERVICE_NAME_MAX ||
        (length > 0u && !service)) {
        return 0;
    }
    for (index = 0u; index < length; ++index) {
        uint32_t digit;
        if (service[index] < (uint8_t)'0' ||
            service[index] > (uint8_t)'9') {
            return 0;
        }
        digit = (uint32_t)(service[index] - (uint8_t)'0');
        if (port > (65535u - digit) / 10u) return 0;
        port = port * 10u + digit;
    }
    *port_out = (uint16_t)port;
    return 1;
}

static inline int rin_resolved_getaddrinfo_request_status(
    const RinResolvedGetAddrInfoRequest* request,
    const uint8_t* variable_payload,
    uint32_t payload_size)
{
    uint64_t exact_size;
    int32_t effective_socket_type;
    int32_t effective_protocol;
    uint16_t service_port;
    if (!request || payload_size < sizeof(*request)) {
        return RIN_RESOLVED_EAI_FAIL;
    }
    exact_size = (uint64_t)sizeof(*request) + (uint64_t)request->node_len +
                 (uint64_t)request->service_len;
    if (exact_size != (uint64_t)payload_size ||
        (exact_size > sizeof(*request) && !variable_payload)) {
        return RIN_RESOLVED_EAI_FAIL;
    }
    if (((uint32_t)request->ai_flags & ~RIN_RESOLVED_GAI_SUPPORTED_FLAGS) !=
        0u) {
        return RIN_RESOLVED_EAI_BADFLAGS;
    }
    if (request->ai_family != RIN_RESOLVED_AF_UNSPEC &&
        request->ai_family != RIN_RESOLVED_AF_INET &&
        request->ai_family != RIN_RESOLVED_AF_INET6) {
        return RIN_RESOLVED_EAI_FAMILY;
    }
    if (!rin_resolved_getaddrinfo_socket_pair(
            request->ai_socktype, request->ai_protocol,
            &effective_socket_type, &effective_protocol)) {
        return RIN_RESOLVED_EAI_SOCKTYPE;
    }
    (void)effective_socket_type;
    (void)effective_protocol;
    if (request->node_len > RIN_RESOLVED_HOST_NAME_MAX ||
        !rin_resolved_text_bytes_valid(variable_payload, request->node_len,
                                       RIN_RESOLVED_HOST_NAME_MAX)) {
        return RIN_RESOLVED_EAI_NONAME;
    }
    if (request->service_len > RIN_RESOLVED_SERVICE_NAME_MAX ||
        !rin_resolved_service_port_bytes(
            variable_payload ? variable_payload + request->node_len : NULL,
            request->service_len, &service_port)) {
        return RIN_RESOLVED_EAI_SERVICE;
    }
    (void)service_port;
    if (request->node_len == 0u && request->service_len == 0u) {
        return RIN_RESOLVED_EAI_NONAME;
    }
    if (((uint32_t)request->ai_flags & RIN_RESOLVED_GAI_CANONNAME) != 0u &&
        request->node_len == 0u) {
        return RIN_RESOLVED_EAI_BADFLAGS;
    }
    return 0;
}

static inline int rin_resolved_getaddrinfo_response_valid(
    const RinResolvedGetAddrInfoResponse* response,
    const uint8_t* canonical_name,
    uint32_t payload_size,
    const RinResolvedGetAddrInfoRequest* request,
    uint16_t expected_port)
{
    uint32_t address_bytes;
    uint32_t sockaddr_bytes;
    uint32_t index;
    uint64_t exact_size;
    int32_t effective_socket_type;
    int32_t effective_protocol;
    int canonical_requested;
    if (!response || !request || payload_size < sizeof(*response) ||
        ((uint32_t)request->ai_flags &
         ~RIN_RESOLVED_GAI_SUPPORTED_FLAGS) != 0u ||
        (request->ai_family != RIN_RESOLVED_AF_UNSPEC &&
         request->ai_family != RIN_RESOLVED_AF_INET &&
         request->ai_family != RIN_RESOLVED_AF_INET6) ||
        !rin_resolved_getaddrinfo_socket_pair(
            request->ai_socktype, request->ai_protocol,
            &effective_socket_type, &effective_protocol)) {
        return 0;
    }
    address_bytes = rin_resolved_address_bytes_for_family(response->ai_family);
    sockaddr_bytes = rin_resolved_sockaddr_bytes_for_family(response->ai_family);
    exact_size = (uint64_t)sizeof(*response) + (uint64_t)response->canon_len;
    canonical_requested =
        (((uint32_t)request->ai_flags & RIN_RESOLVED_GAI_CANONNAME) != 0u);
    if (address_bytes == 0u || response->ai_addrlen != sockaddr_bytes ||
        exact_size != (uint64_t)payload_size || response->reserved0 != 0u ||
        response->ai_flags != request->ai_flags ||
        (request->ai_family != RIN_RESOLVED_AF_UNSPEC &&
         response->ai_family != request->ai_family) ||
        response->ai_socktype != effective_socket_type ||
        response->ai_protocol != effective_protocol ||
        response->port != expected_port ||
        (canonical_requested && response->canon_len == 0u) ||
        (!canonical_requested && response->canon_len != 0u) ||
        !rin_resolved_text_bytes_valid(canonical_name, response->canon_len,
                                       RIN_RESOLVED_HOST_NAME_MAX)) {
        return 0;
    }
    for (index = address_bytes; index < sizeof(response->addr); ++index) {
        if (response->addr[index] != 0u) return 0;
    }
    return 1;
}

static inline int rin_resolved_getaddrinfo_batch_response_valid(
    const RinResolvedGetAddrInfoBatchResponse* response,
    const uint8_t* addresses,
    const uint8_t* canonical_name,
    uint32_t payload_size,
    const RinResolvedGetAddrInfoRequest* request,
    uint16_t expected_port)
{
    uint32_t address_bytes;
    uint32_t sockaddr_bytes;
    uint64_t address_total;
    uint64_t exact_size;
    int32_t effective_socket_type;
    int32_t effective_protocol;
    int canonical_requested;
    if (!response || !request || !addresses ||
        ((uint32_t)request->ai_flags &
         ~RIN_RESOLVED_GAI_SUPPORTED_FLAGS) != 0u ||
        (request->ai_family != RIN_RESOLVED_AF_UNSPEC &&
         request->ai_family != RIN_RESOLVED_AF_INET &&
         request->ai_family != RIN_RESOLVED_AF_INET6) ||
        !rin_resolved_getaddrinfo_socket_pair(
            request->ai_socktype, request->ai_protocol,
            &effective_socket_type, &effective_protocol)) {
        return 0;
    }
    address_bytes = rin_resolved_address_bytes_for_family(response->ai_family);
    sockaddr_bytes = rin_resolved_sockaddr_bytes_for_family(response->ai_family);
    if (address_bytes == 0u ||
        response->address_count == 0u ||
        response->address_count > RIN_RESOLVED_GAI_MAX_RESULTS ||
        response->reserved0 != 0u ||
        response->ai_addrlen != sockaddr_bytes ||
        response->ai_flags != request->ai_flags ||
        (request->ai_family != RIN_RESOLVED_AF_UNSPEC &&
         response->ai_family != request->ai_family) ||
        response->ai_socktype != effective_socket_type ||
        response->ai_protocol != effective_protocol ||
        response->port != expected_port) {
        return 0;
    }
    address_total = (uint64_t)response->address_count * address_bytes;
    exact_size = (uint64_t)sizeof(*response) + address_total +
                 (uint64_t)response->canon_len;
    canonical_requested =
        (((uint32_t)request->ai_flags & RIN_RESOLVED_GAI_CANONNAME) != 0u);
    if (exact_size != (uint64_t)payload_size ||
        (canonical_requested && response->canon_len == 0u) ||
        (!canonical_requested && response->canon_len != 0u) ||
        !rin_resolved_text_bytes_valid(canonical_name, response->canon_len,
                                       RIN_RESOLVED_HOST_NAME_MAX)) {
        return 0;
    }
    return 1;
}

static inline int rin_resolved_getaddrinfo_dual_response_valid(
    const RinResolvedGetAddrInfoDualResponse* response,
    const uint8_t* ipv6_addresses, const uint8_t* ipv4_addresses,
    const uint8_t* canonical_name, uint32_t payload_size,
    const RinResolvedGetAddrInfoRequest* request, uint16_t expected_port)
{
    uint64_t ipv6_bytes;
    uint64_t ipv4_bytes;
    uint64_t exact_size;
    int32_t effective_socket_type;
    int32_t effective_protocol;
    int canonical_requested;

    if (!response || !request || request->ai_family != RIN_RESOLVED_AF_UNSPEC ||
        ((uint32_t)request->ai_flags & ~RIN_RESOLVED_GAI_SUPPORTED_FLAGS) !=
            0u ||
        !rin_resolved_getaddrinfo_socket_pair(
            request->ai_socktype, request->ai_protocol,
            &effective_socket_type, &effective_protocol) ||
        response->reserved0 != 0u || response->ai_flags != request->ai_flags ||
        response->ai_socktype != effective_socket_type ||
        response->ai_protocol != effective_protocol ||
        response->port != expected_port ||
        response->ipv6_count > RIN_RESOLVED_GAI_MAX_RESULTS ||
        response->ipv4_count > RIN_RESOLVED_GAI_MAX_RESULTS ||
        (response->ipv6_count == 0u && response->ipv4_count == 0u) ||
        (response->ipv6_count != 0u && !ipv6_addresses) ||
        (response->ipv4_count != 0u && !ipv4_addresses)) {
        return 0;
    }
    ipv6_bytes = (uint64_t)response->ipv6_count *
                 RIN_RESOLVED_IPV6_ADDRESS_BYTES;
    ipv4_bytes = (uint64_t)response->ipv4_count *
                 RIN_RESOLVED_IPV4_ADDRESS_BYTES;
    exact_size = (uint64_t)sizeof(*response) + ipv6_bytes + ipv4_bytes +
                 (uint64_t)response->canon_len;
    canonical_requested =
        (((uint32_t)request->ai_flags & RIN_RESOLVED_GAI_CANONNAME) != 0u);
    if (exact_size != (uint64_t)payload_size ||
        (canonical_requested && response->canon_len == 0u) ||
        (!canonical_requested && response->canon_len != 0u) ||
        !rin_resolved_text_bytes_valid(canonical_name, response->canon_len,
                                       RIN_RESOLVED_HOST_NAME_MAX)) {
        return 0;
    }
    return 1;
}

static inline int rin_resolved_getnameinfo_request_valid(
    const RinResolvedGetNameInfoRequest* request)
{
    uint32_t address_bytes;
    uint32_t index;
    if (!request ||
        (request->flags & ~RIN_RESOLVED_NAMEINFO_SUPPORTED_FLAGS) != 0u ||
        request->reserved0 != 0u) {
        return 0;
    }
    address_bytes = rin_resolved_address_bytes_for_family(request->family);
    if (address_bytes == 0u || request->addr_len != address_bytes) return 0;
    for (index = address_bytes; index < sizeof(request->addr); ++index) {
        if (request->addr[index] != 0u) return 0;
    }
    return 1;
}

static inline int rin_resolved_getnameinfo_response_valid(
    const RinResolvedGetNameInfoResponse* response,
    uint32_t payload_size)
{
    uint64_t exact_size;
    if (!response || response->host_len > RIN_RESOLVED_HOST_NAME_MAX ||
        response->serv_len > RIN_RESOLVED_SERVICE_NAME_MAX) {
        return 0;
    }
    exact_size = (uint64_t)sizeof(*response) + (uint64_t)response->host_len +
                 (uint64_t)response->serv_len;
    return exact_size == (uint64_t)payload_size;
}

static inline int rin_resolved_getnameinfo_payload_valid(
    const RinResolvedGetNameInfoResponse* response,
    const uint8_t* payload, uint32_t payload_size) {
    const uint8_t* host;
    const uint8_t* service;
    uint32_t index;
    if (!rin_resolved_getnameinfo_response_valid(response, payload_size) ||
        !payload || response->host_len == 0u || response->serv_len == 0u)
        return 0;
    host = payload + sizeof(*response);
    service = host + response->host_len;
    for (index = 0u; index < response->host_len; ++index)
        if (host[index] < 0x21u || host[index] > 0x7eu) return 0;
    for (index = 0u; index < response->serv_len; ++index)
        if (service[index] < (uint8_t)'0' ||
            service[index] > (uint8_t)'9')
            return 0;
    return 1;
}

/* Convert 16 network-order address bytes to the exact 32-nibble ip6.arpa
 * owner. The caller buffer is unchanged unless the complete 73-byte string,
 * including its terminator, fits. */
static inline int rin_resolved_ipv6_reverse_name(
    const uint8_t address[RIN_RESOLVED_IPV6_ADDRESS_BYTES],
    char* output,
    size_t output_size)
{
    static const char hexadecimal[] = "0123456789abcdef";
    size_t position = 0u;
    int index;
    if (!address || !output ||
        output_size < RIN_RESOLVED_IPV6_REVERSE_NAME_BYTES) {
        return -1;
    }
    for (index = (int)RIN_RESOLVED_IPV6_ADDRESS_BYTES - 1; index >= 0;
         --index) {
        output[position++] = hexadecimal[address[index] & 0x0fu];
        output[position++] = '.';
        output[position++] = hexadecimal[address[index] >> 4];
        output[position++] = '.';
    }
    output[position++] = 'i';
    output[position++] = 'p';
    output[position++] = '6';
    output[position++] = '.';
    output[position++] = 'a';
    output[position++] = 'r';
    output[position++] = 'p';
    output[position++] = 'a';
    output[position] = '\0';
    return position + 1u == RIN_RESOLVED_IPV6_REVERSE_NAME_BYTES ? 0 : -1;
}

static inline int rin_resolved_ipv4_server_address_valid(
    const uint8_t address[4])
{
    return address != NULL && address[0] != 0u && address[0] < 224u &&
           !(address[0] == 255u && address[1] == 255u &&
             address[2] == 255u && address[3] == 255u);
}

static inline int rin_resolved_set_config_v2_valid(
    const RinResolvedSetConfigV2* request) {
    uint32_t index;
    if (!request || request->struct_size != sizeof(*request) ||
        request->version != RIN_RESOLVED_CONFIG_V2_VERSION ||
        request->count > 4u ||
        request->local_domain_len > RIN_RESOLVED_LOCAL_DOMAIN_MAX ||
        (request->local_domain_len != 0u &&
         !rin_resolved_dns_name_valid(request->local_domain,
                                      request->local_domain_len)))
        return 0;
    for (index = 0u; index < request->local_domain_len; ++index) {
        if ((uint8_t)request->local_domain[index] !=
            rin_resolved_ascii_lower(
                (uint8_t)request->local_domain[index]))
            return 0;
    }
    for (index = 0u; index < request->count; ++index) {
        uint32_t previous;
        if (!rin_resolved_ipv4_server_address_valid(request->addrs[index]))
            return 0;
        for (previous = 0u; previous < index; ++previous) {
            if (request->addrs[index][0] == request->addrs[previous][0] &&
                request->addrs[index][1] == request->addrs[previous][1] &&
                request->addrs[index][2] == request->addrs[previous][2] &&
                request->addrs[index][3] == request->addrs[previous][3])
                return 0;
        }
    }
    for (index = request->count; index < 4u; ++index) {
        if (request->addrs[index][0] != 0u ||
            request->addrs[index][1] != 0u ||
            request->addrs[index][2] != 0u ||
            request->addrs[index][3] != 0u)
            return 0;
    }
    for (index = request->local_domain_len;
         index < sizeof(request->local_domain); ++index) {
        if (request->local_domain[index] != '\0') return 0;
    }
    return 1;
}

static inline int rin_resolved_set_config_v2_build(
    const uint8_t* addresses, uint32_t count,
    const char* local_domain, uint32_t local_domain_length,
    RinResolvedSetConfigV2* request_out) {
    uint8_t* output = (uint8_t*)request_out;
    uint32_t index;
    if (!request_out) return -1;
    for (index = 0u; index < sizeof(*request_out); ++index)
        output[index] = 0u;
    if (count > 4u || (count != 0u && !addresses) ||
        local_domain_length > RIN_RESOLVED_LOCAL_DOMAIN_MAX ||
        (local_domain_length != 0u && !local_domain))
        return -1;
    request_out->struct_size = sizeof(*request_out);
    request_out->version = RIN_RESOLVED_CONFIG_V2_VERSION;
    request_out->count = count;
    request_out->local_domain_len = local_domain_length;
    for (index = 0u; index < count * 4u; ++index)
        ((uint8_t*)request_out->addrs)[index] = addresses[index];
    if (local_domain_length != 0u &&
        rin_resolved_local_domain_normalize(
            local_domain, local_domain_length,
            request_out->local_domain) != 0) {
        for (index = 0u; index < sizeof(*request_out); ++index)
            output[index] = 0u;
        return -1;
    }
    if (!rin_resolved_set_config_v2_valid(request_out)) {
        for (index = 0u; index < sizeof(*request_out); ++index)
            output[index] = 0u;
        return -1;
    }
    return 0;
}

static inline int rin_resolved_set_config_v3_valid(
    const RinResolvedSetConfigV3* request) {
    uint32_t index;
    if (!request || request->struct_size != sizeof(*request) ||
        request->version != RIN_RESOLVED_CONFIG_V3_VERSION ||
        request->count > 4u ||
        request->local_domain_len > RIN_RESOLVED_LOCAL_DOMAIN_MAX ||
        request->search_domain_count > RIN_RESOLVED_SEARCH_DOMAIN_MAX ||
        (request->local_domain_len != 0u &&
         !rin_resolved_dns_name_valid(request->local_domain,
                                      request->local_domain_len)))
        return 0;
    for (index = 0u; index < request->local_domain_len; ++index) {
        if ((uint8_t)request->local_domain[index] !=
            rin_resolved_ascii_lower(
                (uint8_t)request->local_domain[index]))
            return 0;
    }
    for (index = 0u; index < request->count; ++index) {
        uint32_t previous;
        if (!rin_resolved_ipv4_server_address_valid(request->addrs[index]))
            return 0;
        for (previous = 0u; previous < index; ++previous) {
            if (request->addrs[index][0] == request->addrs[previous][0] &&
                request->addrs[index][1] == request->addrs[previous][1] &&
                request->addrs[index][2] == request->addrs[previous][2] &&
                request->addrs[index][3] == request->addrs[previous][3])
                return 0;
        }
    }
    for (index = request->count; index < 4u; ++index) {
        if (request->addrs[index][0] != 0u ||
            request->addrs[index][1] != 0u ||
            request->addrs[index][2] != 0u ||
            request->addrs[index][3] != 0u)
            return 0;
    }
    for (index = request->local_domain_len;
         index < sizeof(request->local_domain); ++index) {
        if (request->local_domain[index] != '\0') return 0;
    }
    for (index = 0u; index < request->search_domain_count; ++index) {
        uint32_t previous;
        uint32_t length = request->search_domain_lens[index];
        if (length == 0u || length > RIN_RESOLVED_LOCAL_DOMAIN_MAX ||
            !rin_resolved_dns_name_valid(request->search_domains[index],
                                         length))
            return 0;
        for (previous = 0u; previous < length; ++previous) {
            if ((uint8_t)request->search_domains[index][previous] !=
                rin_resolved_ascii_lower(
                    (uint8_t)request->search_domains[index][previous]))
                return 0;
        }
        for (previous = 0u; previous < index; ++previous) {
            uint32_t other_length = request->search_domain_lens[previous];
            uint32_t byte;
            if (length != other_length) continue;
            for (byte = 0u; byte < length; ++byte) {
                if (request->search_domains[index][byte] !=
                    request->search_domains[previous][byte])
                    break;
            }
            if (byte == length) return 0;
        }
    }
    for (index = 0u; index < RIN_RESOLVED_SEARCH_DOMAIN_MAX; ++index) {
        uint32_t byte;
        uint32_t length = index < request->search_domain_count
            ? request->search_domain_lens[index] : 0u;
        if (index >= request->search_domain_count &&
            request->search_domain_lens[index] != 0u)
            return 0;
        for (byte = length; byte < sizeof(request->search_domains[index]);
             ++byte) {
            if (request->search_domains[index][byte] != '\0') return 0;
        }
    }
    return 1;
}

static inline int rin_resolved_set_config_v3_build(
    const uint8_t* addresses, uint32_t count,
    const char* local_domain, uint32_t local_domain_length,
    const char* const* search_domains,
    const uint32_t* search_domain_lengths, uint32_t search_domain_count,
    RinResolvedSetConfigV3* request_out) {
    uint8_t* output = (uint8_t*)request_out;
    uint32_t index;
    if (!request_out) return -1;
    for (index = 0u; index < sizeof(*request_out); ++index)
        output[index] = 0u;
    if (count > 4u || (count != 0u && !addresses) ||
        local_domain_length > RIN_RESOLVED_LOCAL_DOMAIN_MAX ||
        (local_domain_length != 0u && !local_domain) ||
        search_domain_count > RIN_RESOLVED_SEARCH_DOMAIN_MAX ||
        (search_domain_count != 0u &&
         (!search_domains || !search_domain_lengths)))
        return -1;
    request_out->struct_size = sizeof(*request_out);
    request_out->version = RIN_RESOLVED_CONFIG_V3_VERSION;
    request_out->count = count;
    request_out->local_domain_len = local_domain_length;
    request_out->search_domain_count = search_domain_count;
    for (index = 0u; index < count * 4u; ++index)
        ((uint8_t*)request_out->addrs)[index] = addresses[index];
    if (local_domain_length != 0u &&
        rin_resolved_local_domain_normalize(
            local_domain, local_domain_length,
            request_out->local_domain) != 0) {
        for (index = 0u; index < sizeof(*request_out); ++index)
            output[index] = 0u;
        return -1;
    }
    for (index = 0u; index < search_domain_count; ++index) {
        uint32_t length = search_domain_lengths[index];
        if (length == 0u || length > RIN_RESOLVED_LOCAL_DOMAIN_MAX ||
            !search_domains[index] ||
            rin_resolved_local_domain_normalize(
                search_domains[index], length,
                request_out->search_domains[index]) != 0) {
            uint32_t clear_index;
            for (clear_index = 0u; clear_index < sizeof(*request_out);
                 ++clear_index)
                output[clear_index] = 0u;
            return -1;
        }
        request_out->search_domain_lens[index] = length;
    }
    if (!rin_resolved_set_config_v3_valid(request_out)) {
        for (index = 0u; index < sizeof(*request_out); ++index)
            output[index] = 0u;
        return -1;
    }
    return 0;
}

#endif
