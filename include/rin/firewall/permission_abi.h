/* SPDX-License-Identifier: MIT */
/* Shared ABI for bounded Firewall permission requests. */
#ifndef RIN_SDK_FIREWALL_PERMISSION_ABI_H
#define RIN_SDK_FIREWALL_PERMISSION_ABI_H

#include <stdint.h>

#include "application_abi.h"
#include "abi.h"

#define RIN_FIREWALL_PERMISSION_MAGIC UINT32_C(0x31515052) /* RPQ1 */
#define RIN_FIREWALL_PERMISSION_VERSION UINT16_C(1)
#define RIN_FIREWALL_PERMISSION_MAX_REQUESTS UINT32_C(32)
#define RIN_FIREWALL_PERMISSION_WINDOW_MS UINT64_C(1000)
#define RIN_FIREWALL_PERMISSION_MAX_REQUESTS_PER_WINDOW UINT32_C(8)
#define RIN_FIREWALL_PERMISSION_TIMEOUT_MS UINT64_C(30000)

typedef enum RinFirewallPermissionActionV1 {
    RIN_FIREWALL_PERMISSION_ALLOW_ONCE = 1,
    RIN_FIREWALL_PERMISSION_ALLOW_PERSISTENT = 2,
    RIN_FIREWALL_PERMISSION_BLOCK_ONCE = 3,
    RIN_FIREWALL_PERMISSION_BLOCK_PERSISTENT = 4
} RinFirewallPermissionActionV1;

#pragma pack(push, 1)
typedef struct RinFirewallPermissionRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t request_id;
    uint64_t timestamp_ms;
    RinFirewallApplicationContextV2 application;
    RinFirewallPacketV1 packet;
} RinFirewallPermissionRequestV1;
#pragma pack(pop)

#if defined(__cplusplus)
static_assert(sizeof(RinFirewallPermissionRequestV1) == 280u,
              "Firewall permission request ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinFirewallPermissionRequestV1) == 280u,
               "Firewall permission request ABI drift");
#endif

#endif /* RIN_SDK_FIREWALL_PERMISSION_ABI_H */
