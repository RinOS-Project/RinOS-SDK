/* SPDX-License-Identifier: MIT */
/* Versioned, fixed-width Firewall rule and packet ABI. */

#ifndef RIN_SDK_FIREWALL_ABI_H
#define RIN_SDK_FIREWALL_ABI_H

#include <stddef.h>
#include <stdint.h>

#define RIN_FIREWALL_ABI_MAGIC UINT32_C(0x31574652) /* "RFW1" */
#define RIN_FIREWALL_ABI_VERSION UINT16_C(1)
#define RIN_FIREWALL_MAX_RULES UINT32_C(128)
#define RIN_FIREWALL_RULE_NAME_MAX UINT32_C(32)
#define RIN_FIREWALL_RULE_DESCRIPTION_MAX UINT32_C(96)
#define RIN_FIREWALL_APPLICATION_ID_SIZE UINT32_C(32)
#define RIN_FIREWALL_DIRECTION_COUNT UINT32_C(3)
#define RIN_FIREWALL_SYSTEM_PRIORITY_MAX UINT32_C(999)
#define RIN_FIREWALL_USER_PRIORITY_MIN UINT32_C(1000)

typedef enum RinFirewallDirectionV1 {
    RIN_FIREWALL_DIRECTION_INPUT = 1,
    RIN_FIREWALL_DIRECTION_OUTPUT = 2,
    RIN_FIREWALL_DIRECTION_FORWARD = 3
} RinFirewallDirectionV1;

typedef enum RinFirewallActionV1 {
    RIN_FIREWALL_ACTION_ALLOW = 1,
    RIN_FIREWALL_ACTION_DROP = 2,
    RIN_FIREWALL_ACTION_REJECT = 3
} RinFirewallActionV1;

typedef enum RinFirewallFamilyV1 {
    RIN_FIREWALL_FAMILY_ANY = 0,
    RIN_FIREWALL_FAMILY_IPV4 = 4,
    RIN_FIREWALL_FAMILY_IPV6 = 6
} RinFirewallFamilyV1;

typedef enum RinFirewallProtocolV1 {
    RIN_FIREWALL_PROTOCOL_ANY = 0,
    RIN_FIREWALL_PROTOCOL_ICMP = 1,
    RIN_FIREWALL_PROTOCOL_TCP = 6,
    RIN_FIREWALL_PROTOCOL_UDP = 17,
    RIN_FIREWALL_PROTOCOL_ICMPV6 = 58
} RinFirewallProtocolV1;

typedef enum RinFirewallRuleClassV1 {
    RIN_FIREWALL_RULE_CLASS_SYSTEM = 1,
    RIN_FIREWALL_RULE_CLASS_USER = 2,
    RIN_FIREWALL_RULE_CLASS_APPLICATION = 3,
    RIN_FIREWALL_RULE_CLASS_CONTAINER = 4
} RinFirewallRuleClassV1;

/* Container rule ownership uses a class-specific word from the fixed v1
 * reserved tail. Owner IDs are stable across adapter restarts. */
#define RIN_FIREWALL_CONTAINER_OWNER_UNKNOWN UINT32_C(0)
#define RIN_FIREWALL_CONTAINER_OWNER_R8S UINT32_C(1)

typedef enum RinFirewallNetworkProfileV1 {
    RIN_FIREWALL_PROFILE_ANY = 0,
    RIN_FIREWALL_PROFILE_PUBLIC = 1,
    RIN_FIREWALL_PROFILE_PRIVATE = 2,
    RIN_FIREWALL_PROFILE_TRUSTED = 3
} RinFirewallNetworkProfileV1;

#define RIN_FIREWALL_RULE_FLAG_ENABLED UINT16_C(0x0001)
#define RIN_FIREWALL_RULE_FLAG_MATCH_INTERFACE UINT16_C(0x0002)
#define RIN_FIREWALL_RULE_FLAG_MATCH_NAMESPACE UINT16_C(0x0004)
#define RIN_FIREWALL_RULE_FLAG_MATCH_APPLICATION UINT16_C(0x0008)
#define RIN_FIREWALL_RULE_FLAG_MATCH_PROFILE UINT16_C(0x0010)
#define RIN_FIREWALL_RULE_FLAG_SYSTEM_CRITICAL UINT16_C(0x0020)
#define RIN_FIREWALL_RULE_FLAG_MATCH_PROCESS UINT16_C(0x0040)
/* The application package is no longer installed.  The service owns this
 * lifecycle bit; an orphaned rule is always disabled and retained so a later
 * reinstall can be reviewed instead of silently losing user policy. */
#define RIN_FIREWALL_RULE_FLAG_ORPHANED UINT16_C(0x0080)
#define RIN_FIREWALL_RULE_KNOWN_FLAGS \
    (RIN_FIREWALL_RULE_FLAG_ENABLED | RIN_FIREWALL_RULE_FLAG_MATCH_INTERFACE | \
     RIN_FIREWALL_RULE_FLAG_MATCH_NAMESPACE | \
     RIN_FIREWALL_RULE_FLAG_MATCH_APPLICATION | \
     RIN_FIREWALL_RULE_FLAG_MATCH_PROFILE | \
     RIN_FIREWALL_RULE_FLAG_SYSTEM_CRITICAL | \
     RIN_FIREWALL_RULE_FLAG_MATCH_PROCESS | \
     RIN_FIREWALL_RULE_FLAG_ORPHANED)

#define RIN_FIREWALL_PACKET_FLAG_FRAGMENT UINT16_C(0x0001)
#define RIN_FIREWALL_PACKET_FLAG_PROCESS_IDENTITY UINT16_C(0x0002)
#define RIN_FIREWALL_PACKET_FLAG_USER_IDENTITY UINT16_C(0x0004)
#define RIN_FIREWALL_PACKET_KNOWN_FLAGS \
    (RIN_FIREWALL_PACKET_FLAG_FRAGMENT | \
     RIN_FIREWALL_PACKET_FLAG_PROCESS_IDENTITY | \
     RIN_FIREWALL_PACKET_FLAG_USER_IDENTITY)

#define RIN_FIREWALL_STATE_UNTRACKED UINT32_C(0)
#define RIN_FIREWALL_STATE_NEW UINT32_C(1)
#define RIN_FIREWALL_STATE_ESTABLISHED UINT32_C(2)
#define RIN_FIREWALL_STATE_RELATED UINT32_C(4)
#define RIN_FIREWALL_STATE_INVALID UINT32_C(8)
#define RIN_FIREWALL_STATE_KNOWN_MASK \
    (RIN_FIREWALL_STATE_NEW | RIN_FIREWALL_STATE_ESTABLISHED | \
     RIN_FIREWALL_STATE_RELATED | RIN_FIREWALL_STATE_INVALID)

typedef enum RinFirewallStatusV1 {
    RIN_FIREWALL_OK = 0,
    RIN_FIREWALL_INVALID_ARGUMENT = -1,
    RIN_FIREWALL_ABI_MISMATCH = -2,
    RIN_FIREWALL_MALFORMED = -3,
    RIN_FIREWALL_CAPACITY = -4,
    RIN_FIREWALL_DUPLICATE = -5,
    RIN_FIREWALL_NOT_FOUND = -6,
    RIN_FIREWALL_UNSUPPORTED = -7,
    RIN_FIREWALL_GENERATION_EXHAUSTED = -8,
    RIN_FIREWALL_ACCESS_DENIED = -9,
    RIN_FIREWALL_BUSY = -10,
    /* The caller is authorized to request the operation, but the
     * administrator-facing safety warning has not been acknowledged. */
    RIN_FIREWALL_CONFIRMATION_REQUIRED = -11,
    /* A verified process exceeded the bounded NEW-connection budget. */
    RIN_FIREWALL_RATE_LIMIT = -12
} RinFirewallStatusV1;

#pragma pack(push, 1)

typedef struct RinFirewallRuleV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t id;
    uint32_t priority;
    /* Zero is machine-wide; non-zero rules match only that authenticated UID. */
    uint32_t owner_uid;
    uint16_t rule_class;
    uint8_t direction;
    uint8_t action;
    uint8_t family;
    uint8_t protocol;
    uint8_t source_prefix_len;
    uint8_t destination_prefix_len;
    uint16_t source_port_first;
    uint16_t source_port_last;
    uint16_t destination_port_first;
    uint16_t destination_port_last;
    uint8_t icmp_type;
    uint8_t icmp_code;
    uint16_t reserved0;
    uint32_t interface_id;
    uint64_t namespace_id;
    uint32_t network_profile;
    uint32_t connection_state_mask;
    uint8_t source_address[16];
    uint8_t destination_address[16];
    uint8_t application_id[RIN_FIREWALL_APPLICATION_ID_SIZE];
    char name[RIN_FIREWALL_RULE_NAME_MAX];
    char description[RIN_FIREWALL_RULE_DESCRIPTION_MAX];
    uint64_t created_at_ms;
    uint64_t updated_at_ms;
    uint64_t packet_count;
    uint64_t byte_count;
    /* With MATCH_PROCESS, reserved[0..1] carry process_id and
     * reserved[2..3] carry process_instance_cookie, little-endian words.
     * Without MATCH_PROCESS, container rules use reserved[0] for a stable
     * container owner ID and require reserved[1..3] to be zero. Other rule
     * classes require all four words to be zero. */
    uint32_t reserved[4];
} RinFirewallRuleV1;

typedef struct RinFirewallRuleSetV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t generation;
    uint32_t enabled;
    uint32_t rule_count;
    uint8_t default_action[RIN_FIREWALL_DIRECTION_COUNT];
    uint8_t reserved0;
    uint64_t reserved[4];
    RinFirewallRuleV1 rules[RIN_FIREWALL_MAX_RULES];
} RinFirewallRuleSetV1;

typedef struct RinFirewallPacketV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t namespace_id;
    uint32_t interface_id;
    uint32_t network_profile;
    uint8_t direction;
    uint8_t family;
    uint8_t protocol;
    uint8_t connection_state;
    uint8_t source_address[16];
    uint8_t destination_address[16];
    uint16_t source_port;
    uint16_t destination_port;
    uint8_t icmp_type;
    uint8_t icmp_code;
    uint16_t payload_length;
    uint16_t fragment_offset;
    uint8_t extension_header_count;
    uint8_t reserved0;
    uint8_t application_id[RIN_FIREWALL_APPLICATION_ID_SIZE];
    /* With PROCESS_IDENTITY, reserved[0..1] are the process ID and instance
     * cookie. With USER_IDENTITY, reserved[2] carries the authenticated
     * non-zero owner UID; otherwise it remains zero. */
    uint64_t reserved[3];
} RinFirewallPacketV1;

typedef struct RinFirewallDecisionV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t action;
    int32_t status;
    uint32_t matched;
    uint64_t rule_id;
    uint32_t priority;
    uint32_t reserved0;
    uint64_t packet_count;
    uint64_t byte_count;
} RinFirewallDecisionV1;

#pragma pack(pop)

typedef RinFirewallRuleV1 RinFirewallRule;

static inline uint32_t rin_firewall_container_rule_owner_id(
    const RinFirewallRuleV1* rule)
{
    if (rule == NULL || rule->rule_class != RIN_FIREWALL_RULE_CLASS_CONTAINER ||
        (rule->flags & RIN_FIREWALL_RULE_FLAG_MATCH_PROCESS) != 0u)
        return RIN_FIREWALL_CONTAINER_OWNER_UNKNOWN;
    return rule->reserved[0];
}

static inline int rin_firewall_container_rule_set_owner_id(
    RinFirewallRuleV1* rule, uint32_t owner_id)
{
    uint32_t index;
    if (rule == NULL || rule->rule_class != RIN_FIREWALL_RULE_CLASS_CONTAINER ||
        (rule->flags & RIN_FIREWALL_RULE_FLAG_MATCH_PROCESS) != 0u ||
        owner_id == RIN_FIREWALL_CONTAINER_OWNER_UNKNOWN)
        return 0;
    for (index = 0u; index < 4u; ++index) rule->reserved[index] = 0u;
    rule->reserved[0] = owner_id;
    return 1;
}

#if defined(__cplusplus)
static_assert(sizeof(RinFirewallRuleV1) == 304u,
              "RinFirewallRuleV1 ABI drift");
static_assert(sizeof(RinFirewallPacketV1) == 128u,
              "RinFirewallPacketV1 ABI drift");
static_assert(sizeof(RinFirewallDecisionV1) == 48u,
              "RinFirewallDecisionV1 ABI drift");
#else
_Static_assert(sizeof(RinFirewallRuleV1) == 304u,
               "RinFirewallRuleV1 ABI drift");
_Static_assert(sizeof(RinFirewallPacketV1) == 128u,
               "RinFirewallPacketV1 ABI drift");
_Static_assert(sizeof(RinFirewallDecisionV1) == 48u,
               "RinFirewallDecisionV1 ABI drift");
#endif

#ifdef __cplusplus
extern "C" {
#endif

int rin_firewall_rule_set_init(RinFirewallRuleSetV1* set);
int rin_firewall_rule_validate(const RinFirewallRuleV1* rule);
int rin_firewall_rule_set_validate(const RinFirewallRuleSetV1* set);
int rin_firewall_packet_validate(const RinFirewallPacketV1* packet);
int rin_firewall_rule_set_add(RinFirewallRuleSetV1* set,
                              const RinFirewallRuleV1* rule);
int rin_firewall_rule_set_update(RinFirewallRuleSetV1* set,
                                 uint64_t rule_id,
                                 const RinFirewallRuleV1* rule);
int rin_firewall_rule_set_remove(RinFirewallRuleSetV1* set,
                                 uint64_t rule_id);
int rin_firewall_evaluate(RinFirewallRuleSetV1* set,
                          const RinFirewallPacketV1* packet,
                          RinFirewallDecisionV1* decision);

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_FIREWALL_ABI_H */
