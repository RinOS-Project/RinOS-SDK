/* SPDX-License-Identifier: MIT */
/* Public authenticated application identity used by firewall requests. */
#ifndef RIN_SDK_FIREWALL_APPLICATION_ABI_H
#define RIN_SDK_FIREWALL_APPLICATION_ABI_H

#include <stdint.h>
#include "abi.h"

#define RIN_FIREWALL_APPLICATION_CONTEXT_VERSION_V2 UINT16_C(2)
#define RIN_FIREWALL_APPLICATION_CONTEXT_VERSION \
    RIN_FIREWALL_APPLICATION_CONTEXT_VERSION_V2
#define RIN_FIREWALL_APPLICATION_FLAG_SYSTEM_PROCESS UINT16_C(0x0001)
#define RIN_FIREWALL_APPLICATION_KNOWN_FLAGS \
    RIN_FIREWALL_APPLICATION_FLAG_SYSTEM_PROCESS

#pragma pack(push, 1)
typedef struct RinFirewallApplicationContextV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t process_id;
    uint64_t process_instance_cookie;
    uint64_t package_generation;
    uint8_t application_id[RIN_FIREWALL_APPLICATION_ID_SIZE];
    uint8_t package_digest[32];
    uint64_t capability_mask;
    uint64_t namespace_id;
    uint64_t reserved1[2];
} RinFirewallApplicationContextV2;
/* Keep the historical source name as an alias. This does not preserve v1
 * wire compatibility; validators require the embedded v2 version. */
typedef RinFirewallApplicationContextV2 RinFirewallApplicationContextV1;
#pragma pack(pop)

#if defined(__cplusplus)
static_assert(sizeof(RinFirewallApplicationContextV2) == 128u,
              "Firewall application context ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinFirewallApplicationContextV2) == 128u,
               "Firewall application context ABI drift");
#endif

#endif
