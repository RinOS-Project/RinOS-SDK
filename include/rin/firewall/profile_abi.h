/* SPDX-License-Identifier: MIT */
/* Versioned Firewall network-profile records. */
#ifndef RIN_SDK_FIREWALL_PROFILE_ABI_H
#define RIN_SDK_FIREWALL_PROFILE_ABI_H

#include <stdint.h>

#define RIN_FIREWALL_PROFILE_MAX_INTERFACES UINT32_C(16)
#define RIN_FIREWALL_PROFILE_IDENTITY_BYTES UINT32_C(32)

typedef struct RinFirewallProfileEntryV1 {
    uint32_t interface_id;
    uint8_t profile;
    uint8_t reserved0[3];
    uint64_t generation;
    uint8_t identity[RIN_FIREWALL_PROFILE_IDENTITY_BYTES];
} RinFirewallProfileEntryV1;

typedef struct RinFirewallProfileTableV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t generation;
    uint32_t entry_count;
    uint32_t reserved1;
    RinFirewallProfileEntryV1 entries[RIN_FIREWALL_PROFILE_MAX_INTERFACES];
} RinFirewallProfileTableV1;

#if defined(__cplusplus)
static_assert(sizeof(RinFirewallProfileEntryV1) == 48u,
              "Firewall profile entry ABI drift");
static_assert(sizeof(RinFirewallProfileTableV1) == 792u,
              "Firewall profile table ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinFirewallProfileEntryV1) == 48u,
               "Firewall profile entry ABI drift");
_Static_assert(sizeof(RinFirewallProfileTableV1) == 792u,
               "Firewall profile table ABI drift");
#endif

#endif /* RIN_SDK_FIREWALL_PROFILE_ABI_H */
