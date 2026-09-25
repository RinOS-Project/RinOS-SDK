/* SPDX-License-Identifier: MIT */
/* Versioned Firewall connection-observation records. */
#ifndef RIN_SDK_FIREWALL_CONNTRACK_ABI_H
#define RIN_SDK_FIREWALL_CONNTRACK_ABI_H

#include <stdint.h>

#define RIN_FIREWALL_CONNTRACK_MAX_ENTRIES UINT32_C(64)
#define RIN_FIREWALL_CONNTRACK_ABI_VERSION_V2 UINT16_C(2)

#pragma pack(push, 1)
typedef struct RinFirewallConntrackEntryV1 {
    uint32_t active;
    uint32_t namespace_id;
    uint32_t interface_id;
    uint8_t family;
    uint8_t protocol;
    uint8_t reserved0[2];
    uint8_t source_address[16];
    uint8_t destination_address[16];
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t state;
    uint32_t directions_seen;
    uint8_t tcp_fin_directions;
    uint8_t reserved1[3];
    uint64_t last_seen_ms;
} RinFirewallConntrackEntryV1;

typedef struct RinFirewallConntrackQueryV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t namespace_id;
    uint32_t interface_id;
    uint8_t family;
    uint8_t protocol;
    uint8_t reserved1[2];
    uint32_t cursor;
    uint32_t reserved2;
} RinFirewallConntrackQueryV1;

/* v2 carries the kernel's complete 64-bit namespace identity.  v1 remains
 * available for clients that can only represent 32-bit namespace IDs. */
typedef struct RinFirewallConntrackEntryV2 {
    uint32_t active;
    uint64_t namespace_id;
    uint32_t interface_id;
    uint8_t family;
    uint8_t protocol;
    uint8_t reserved0[2];
    uint8_t source_address[16];
    uint8_t destination_address[16];
    uint16_t source_port;
    uint16_t destination_port;
    uint32_t state;
    uint32_t directions_seen;
    uint8_t tcp_fin_directions;
    uint8_t reserved1[3];
    uint64_t last_seen_ms;
} RinFirewallConntrackEntryV2;

typedef struct RinFirewallConntrackQueryV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t namespace_id;
    uint32_t interface_id;
    uint8_t family;
    uint8_t protocol;
    uint8_t reserved1[2];
    uint32_t cursor;
    uint32_t reserved2;
} RinFirewallConntrackQueryV2;
#pragma pack(pop)

#if defined(__cplusplus)
static_assert(sizeof(RinFirewallConntrackEntryV1) == 72u,
              "Firewall conntrack entry ABI drift");
static_assert(sizeof(RinFirewallConntrackQueryV1) == 28u,
              "Firewall conntrack query ABI drift");
static_assert(sizeof(RinFirewallConntrackEntryV2) == 76u,
              "Firewall conntrack v2 entry ABI drift");
static_assert(sizeof(RinFirewallConntrackQueryV2) == 32u,
              "Firewall conntrack v2 query ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinFirewallConntrackEntryV1) == 72u,
               "Firewall conntrack entry ABI drift");
_Static_assert(sizeof(RinFirewallConntrackQueryV1) == 28u,
               "Firewall conntrack query ABI drift");
_Static_assert(sizeof(RinFirewallConntrackEntryV2) == 76u,
               "Firewall conntrack v2 entry ABI drift");
_Static_assert(sizeof(RinFirewallConntrackQueryV2) == 32u,
               "Firewall conntrack v2 query ABI drift");
#endif

#endif /* RIN_SDK_FIREWALL_CONNTRACK_ABI_H */
