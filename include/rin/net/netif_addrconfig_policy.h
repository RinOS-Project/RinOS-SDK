/* SPDX-License-Identifier: MIT */
/* Strict decoder for the primary-interface address-configuration snapshot. */

#ifndef RIN_SDK_NETIF_ADDRCONFIG_POLICY_H
#define RIN_SDK_NETIF_ADDRCONFIG_POLICY_H

#include <stddef.h>
#include <stdint.h>

#include "netif_abi.h"

static inline int rin_netif_addrconfig_bytes_are_zero(const void* value,
                                                       size_t size)
{
    const uint8_t* bytes = (const uint8_t*)value;
    size_t index;
    if (!bytes) return 0;
    for (index = 0u; index < size; ++index) {
        if (bytes[index] != 0u) return 0;
    }
    return 1;
}

static inline int rin_netif_addrconfig_name_valid(
    const char name[RIN_NETINFO_IFNAME_MAX])
{
    size_t index;
    if (!name || name[0] == '\0') return 0;
    for (index = 1u; index < RIN_NETINFO_IFNAME_MAX; ++index) {
        if (name[index] != '\0') continue;
        while (++index < RIN_NETINFO_IFNAME_MAX) {
            if (name[index] != '\0') return 0;
        }
        return 1;
    }
    return 0;
}

static inline int rin_netif_addrconfig_decode(
    const RinNetPrimaryInfo* info,
    int* ipv4_configured,
    int* ipv6_configured)
{
    int has_ipv4;
    if (ipv4_configured) *ipv4_configured = 0;
    if (ipv6_configured) *ipv6_configured = 0;
    if (!info || !ipv4_configured || !ipv6_configured) return -1;
    if ((info->flags & ~RIN_NETINFO_KNOWN_FLAGS) != 0u) return -1;
    if (info->flags == 0u)
        return rin_netif_addrconfig_bytes_are_zero(info, sizeof(*info))
            ? 0 : -1;
    if (info->link_type > RIN_NET_LINK_TYPE_MAX ||
        (info->signal_percent > 100u &&
         info->signal_percent != RIN_NET_SIGNAL_UNKNOWN) ||
        ((info->flags & RIN_NETINFO_FLAG_LINK_UP) != 0u &&
         (info->flags & RIN_NETINFO_FLAG_LINK_KNOWN) == 0u) ||
        ((info->flags & RIN_NETINFO_FLAG_LINK_KNOWN) == 0u &&
         info->link_type != RIN_NET_LINK_TYPE_UNKNOWN) ||
        ((info->flags & RIN_NETINFO_FLAG_SIGNAL_VALID) != 0u &&
         (info->link_type != RIN_NET_LINK_TYPE_WIFI ||
          info->signal_percent > 100u)) ||
        ((info->flags & RIN_NETINFO_FLAG_SIGNAL_VALID) == 0u &&
         info->signal_percent != 0u))
        return -1;
    if ((info->flags & RIN_NETINFO_FLAG_DEVICE_READY) == 0u ||
        info->device_generation == 0u ||
        !rin_netif_addrconfig_name_valid(info->ifname))
        return -1;
    has_ipv4 = !rin_netif_addrconfig_bytes_are_zero(info->ip,
                                                     sizeof(info->ip));
    if (has_ipv4 !=
        ((info->flags & RIN_NETINFO_FLAG_IPV4_CONFIGURED) != 0u))
        return -1;
    *ipv4_configured = has_ipv4;
    *ipv6_configured =
        (info->flags & RIN_NETINFO_FLAG_IPV6_CONFIGURED) != 0u;
    return 0;
}

#endif /* RIN_SDK_NETIF_ADDRCONFIG_POLICY_H */
