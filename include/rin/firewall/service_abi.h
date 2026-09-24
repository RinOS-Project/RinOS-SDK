/* SPDX-License-Identifier: MIT */
/* Stable Firewall service wire records shared by clients and firewalld. */
#ifndef RIN_SDK_FIREWALL_SERVICE_ABI_H
#define RIN_SDK_FIREWALL_SERVICE_ABI_H

#include <stdint.h>

#include "abi.h"
#include "application_abi.h"
#include "conntrack_abi.h"
#include "permission_abi.h"
#include "profile_abi.h"

#define RIN_FIREWALL_SERVICE_ABI_VERSION UINT32_C(1)
#define RIN_FIREWALL_SERVICE_CAP_READ UINT32_C(0x00000001)
#define RIN_FIREWALL_SERVICE_CAP_ADMIN UINT32_C(0x00000002)
#define RIN_FIREWALL_SERVICE_CAP_AUDIT UINT32_C(0x00000004)
#define RIN_FIREWALL_SERVICE_CAP_SYSTEM_RULES UINT32_C(0x00000008)
#define RIN_FIREWALL_SERVICE_CAP_PERMISSION UINT32_C(0x00000010)
#define RIN_FIREWALL_SERVICE_CAP_SYSTEM_RULES_WARNING_ACK UINT32_C(0x00000020)
#define RIN_FIREWALL_SERVICE_KNOWN_CAPS \
    (RIN_FIREWALL_SERVICE_CAP_READ | RIN_FIREWALL_SERVICE_CAP_ADMIN | \
     RIN_FIREWALL_SERVICE_CAP_AUDIT | RIN_FIREWALL_SERVICE_CAP_SYSTEM_RULES | \
     RIN_FIREWALL_SERVICE_CAP_PERMISSION | \
     RIN_FIREWALL_SERVICE_CAP_SYSTEM_RULES_WARNING_ACK)

#define RIN_FIREWALL_SERVICE_EVENT_BATCH_CAPACITY UINT32_C(16)
#define RIN_FIREWALL_SERVICE_LOG_BATCH_CAPACITY UINT32_C(16)

#pragma pack(push, 1)
typedef struct RinFirewallServicePeerV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t uid;
    uint32_t capabilities;
    uint64_t session_id;
    uint64_t session_cookie;
    uint64_t process_generation;
    uint8_t reserved1[16];
} RinFirewallServicePeerV1;

typedef struct RinFirewallServiceStatusV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t enabled;
    uint32_t rule_count;
    uint64_t generation;
    uint64_t evaluated_packets;
    uint64_t dropped_packets;
    uint64_t rejected_packets;
    uint64_t log_sequence;
    uint64_t profile_generation;
    uint32_t allow_log;
    uint8_t reserved1[12];
} RinFirewallServiceStatusV1;

typedef struct RinFirewallServiceLogEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t sequence;
    uint64_t timestamp_ms;
    RinFirewallPacketV1 packet;
    RinFirewallDecisionV1 decision;
} RinFirewallServiceLogEventV1;

typedef enum RinFirewallServiceEventTypeV1 {
    RIN_FIREWALL_SERVICE_EVENT_RULESET_CHANGED = 1,
    RIN_FIREWALL_SERVICE_EVENT_PROFILE_CHANGED = 2,
    RIN_FIREWALL_SERVICE_EVENT_APPLICATION_INSTALLED = 3,
    RIN_FIREWALL_SERVICE_EVENT_APPLICATION_UNINSTALLED = 4
} RinFirewallServiceEventTypeV1;

typedef struct RinFirewallServiceEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t type;
    uint64_t sequence;
    uint64_t generation;
    uint32_t interface_id;
    uint32_t profile;
    uint32_t rule_count;
    uint32_t reserved0;
    uint8_t application_id[RIN_FIREWALL_APPLICATION_ID_SIZE];
} RinFirewallServiceEventV1;

typedef struct RinFirewallServicePermissionResultV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t request_id;
    uint32_t coalesced;
    uint32_t queue_count;
    uint32_t expired_count;
    uint8_t reserved1[4];
} RinFirewallServicePermissionResultV1;

typedef struct RinFirewallServiceWarningAckV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t token;
    uint64_t expires_at_ms;
    uint8_t reserved1[8];
} RinFirewallServiceWarningAckV1;
#pragma pack(pop)

#if defined(__cplusplus)
static_assert(sizeof(RinFirewallServicePeerV1) == 56u,
              "Firewall service peer ABI drift");
static_assert(sizeof(RinFirewallServiceStatusV1) == 80u,
              "Firewall service status ABI drift");
static_assert(sizeof(RinFirewallServiceLogEventV1) == 200u,
              "Firewall service log ABI drift");
static_assert(sizeof(RinFirewallServiceEventV1) == 72u,
              "Firewall service event ABI drift");
static_assert(sizeof(RinFirewallServicePermissionResultV1) == 32u,
              "Firewall permission result ABI drift");
static_assert(sizeof(RinFirewallServiceWarningAckV1) == 32u,
              "Firewall warning acknowledgement ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinFirewallServicePeerV1) == 56u,
               "Firewall service peer ABI drift");
_Static_assert(sizeof(RinFirewallServiceStatusV1) == 80u,
               "Firewall service status ABI drift");
_Static_assert(sizeof(RinFirewallServiceLogEventV1) == 200u,
               "Firewall service log ABI drift");
_Static_assert(sizeof(RinFirewallServiceEventV1) == 72u,
               "Firewall service event ABI drift");
_Static_assert(sizeof(RinFirewallServicePermissionResultV1) == 32u,
               "Firewall permission result ABI drift");
_Static_assert(sizeof(RinFirewallServiceWarningAckV1) == 32u,
               "Firewall warning acknowledgement ABI drift");
#endif

#endif /* RIN_SDK_FIREWALL_SERVICE_ABI_H */
