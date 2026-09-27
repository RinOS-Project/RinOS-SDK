// SPDX-License-Identifier: MIT
#ifndef RIN_SDK_NETIF_ABI_H
#define RIN_SDK_NETIF_ABI_H

#include <stdint.h>
#include "../syscall_abi.h"
#include "netdev_link.h"

#define RIN_NETINFO_IFNAME_MAX 16

#define RIN_NETINFO_FLAG_DEVICE_READY 0x00000001u
#define RIN_NETINFO_FLAG_CONFIGURED   0x00000002u
#define RIN_NETINFO_FLAG_IPV4_CONFIGURED RIN_NETINFO_FLAG_CONFIGURED
#define RIN_NETINFO_FLAG_IPV6_CONFIGURED 0x00000004u
#define RIN_NETINFO_KNOWN_FLAGS \
    (RIN_NETINFO_FLAG_DEVICE_READY | RIN_NETINFO_FLAG_IPV4_CONFIGURED | \
     RIN_NETINFO_FLAG_IPV6_CONFIGURED | RIN_NETINFO_FLAG_LINK_UP | \
     RIN_NETINFO_FLAG_LINK_KNOWN | RIN_NETINFO_FLAG_SIGNAL_VALID)

#define RIN_NETINFO_FLAG_LINK_UP       0x00000008u
#define RIN_NETINFO_FLAG_LINK_KNOWN    0x00000010u
#define RIN_NETINFO_FLAG_SIGNAL_VALID  0x00000020u

#define RIN_NET_WIFI_INFO_VERSION UINT16_C(1)
#define RIN_NET_WIFI_INFO_FLAG_CONNECTED       UINT16_C(0x0001)
#define RIN_NET_WIFI_INFO_FLAG_SSID_VALID      UINT16_C(0x0002)
#define RIN_NET_WIFI_INFO_FLAG_BSSID_VALID     UINT16_C(0x0004)
#define RIN_NET_WIFI_INFO_FLAG_RSSI_VALID      UINT16_C(0x0008)
#define RIN_NET_WIFI_INFO_FLAG_SIGNAL_VALID    UINT16_C(0x0010)
#define RIN_NET_WIFI_INFO_KNOWN_FLAGS \
    (RIN_NET_WIFI_INFO_FLAG_CONNECTED | RIN_NET_WIFI_INFO_FLAG_SSID_VALID | \
     RIN_NET_WIFI_INFO_FLAG_BSSID_VALID | RIN_NET_WIFI_INFO_FLAG_RSSI_VALID | \
     RIN_NET_WIFI_INFO_FLAG_SIGNAL_VALID)

#define RIN_NETCFG_SOURCE_DHCP          1u
#define RIN_NETCFG_SOURCE_CLEAR         2u
#define RIN_NETCFG_SOURCE_DHCPV6        3u

#define RIN_IPV6_CONFIG_VERSION          1u
#define RIN_IPV6_CONFIG_FLAG_HAS_ADDRESS 0x00000001u
#define RIN_IPV6_CONFIG_KNOWN_FLAGS      RIN_IPV6_CONFIG_FLAG_HAS_ADDRESS
#define RIN_IPV6_CONFIG_MAX_DNS_SERVERS  3u

typedef struct RinNetPrimaryInfo {
    char ifname[RIN_NETINFO_IFNAME_MAX];
    uint8_t mac[6];
    uint16_t device_generation;
    uint8_t ip[4];
    uint8_t netmask[4];
    uint8_t gateway[4];
    uint8_t dns[4];
    uint32_t flags;
    uint32_t link_type;
    uint32_t signal_percent;
} RinNetPrimaryInfo;

/* Live counters owned by the target network platform owner.  This is kept
 * separate from RinNetPrimaryInfo so the identity/configuration ABI remains
 * stable while statistics can be sampled with its own generation binding. */
#define RIN_NET_STATISTICS_VERSION 1u
typedef struct RinNetInterfaceStatisticsV1 {
    uint32_t version;
    uint32_t struct_size;
    uint64_t device_generation;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
    uint64_t rx_errors;
    uint64_t tx_errors;
} RinNetInterfaceStatisticsV1;

/* Generation-bound IP-layer counters.  These counters are deliberately
 * separate from RinNetInterfaceStatisticsV1: L2 driver accounting cannot be
 * substituted for IP protocol statistics without changing their meaning. */
#define RIN_NET_IP_GLOBAL_STATISTICS_VERSION 1u
#define RIN_NET_IP_GLOBAL_STATISTICS_AF_IPV4 4u
#define RIN_NET_IP_GLOBAL_STATISTICS_AF_IPV6 6u
#define RIN_NET_IP_GLOBAL_STATISTICS_FLAG_OUTPUT_PACKET_REQUESTS 0x00000001u
#define RIN_NET_IP_GLOBAL_STATISTICS_FLAG_RECEIVED_PACKETS 0x00000002u
#define RIN_NET_IP_GLOBAL_STATISTICS_FLAG_RECEIVED_PACKETS_DELIVERED 0x00000004u
#define RIN_NET_IP_GLOBAL_STATISTICS_FLAG_RECEIVED_PACKETS_FORWARDED 0x00000008u
#define RIN_NET_IP_GLOBAL_STATISTICS_KNOWN_FLAGS \
    (RIN_NET_IP_GLOBAL_STATISTICS_FLAG_OUTPUT_PACKET_REQUESTS | \
     RIN_NET_IP_GLOBAL_STATISTICS_FLAG_RECEIVED_PACKETS | \
     RIN_NET_IP_GLOBAL_STATISTICS_FLAG_RECEIVED_PACKETS_DELIVERED | \
     RIN_NET_IP_GLOBAL_STATISTICS_FLAG_RECEIVED_PACKETS_FORWARDED)

typedef struct RinNetIpGlobalStatisticsV1 {
    uint32_t version;
    uint32_t struct_size;
    uint64_t device_generation;
    uint32_t address_family;
    uint32_t supported_flags;
    uint64_t output_packet_requests;
    uint64_t received_packets;
    uint64_t received_packets_delivered;
    uint64_t received_packets_forwarded;
} RinNetIpGlobalStatisticsV1;

/* Generation-bound UDP-layer counters.  These are intentionally separate
 * from both interface counters and IP-layer counters: a datagram can reach
 * UDP without matching a local endpoint, and listener ownership lives in the
 * transport layer rather than the device driver. */
#define RIN_NET_UDP_GLOBAL_STATISTICS_VERSION 1u
#define RIN_NET_UDP_GLOBAL_STATISTICS_AF_IPV4 4u
#define RIN_NET_UDP_GLOBAL_STATISTICS_AF_IPV6 6u
#define RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_DATAGRAMS_SENT 0x00000001u
#define RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_DATAGRAMS_RECEIVED 0x00000002u
#define RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_INCOMING_DISCARDED 0x00000004u
#define RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_INCOMING_ERRORS 0x00000008u
#define RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_LISTENERS 0x00000010u
#define RIN_NET_UDP_GLOBAL_STATISTICS_KNOWN_FLAGS \
    (RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_DATAGRAMS_SENT | \
     RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_DATAGRAMS_RECEIVED | \
     RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_INCOMING_DISCARDED | \
     RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_INCOMING_ERRORS | \
     RIN_NET_UDP_GLOBAL_STATISTICS_FLAG_LISTENERS)

typedef struct RinNetUdpGlobalStatisticsV1 {
    uint32_t version;
    uint32_t struct_size;
    uint64_t device_generation;
    uint32_t address_family;
    uint32_t supported_flags;
    uint64_t datagrams_sent;
    uint64_t datagrams_received;
    uint64_t incoming_datagrams_discarded;
    uint64_t incoming_datagrams_with_errors;
    uint64_t udp_listeners;
} RinNetUdpGlobalStatisticsV1;

/* Generation-bound Wi-Fi telemetry returned by the privileged network owner.
 * The SSID is copied only for the currently associated BSS; a disconnected
 * or stale provider returns no identity and keeps signal_percent unknown. */
typedef struct RinNetWifiInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t device_generation;
    uint64_t association_generation;
    uint8_t ssid_length;
    uint8_t ssid[32];
    uint8_t bssid[6];
    uint8_t reserved0;
    int32_t signal_dbm;
    uint32_t signal_percent;
    uint64_t reserved;
} RinNetWifiInfoV1;

typedef struct RinIPv4Config {
    uint8_t ip[4];
    uint8_t netmask[4];
    uint8_t gateway[4];
    uint8_t dns[4];
    uint32_t dns_count;
    uint32_t source;
} RinIPv4Config;

/* This is the ownership boundary between the signed DHCP client service and
 * the kernel's NDP/DAD address owner.  Lifetimes are relative, monotonic
 * milliseconds: userspace owns renewal policy while the kernel guarantees
 * expiry withdrawal even if the service dies. */
typedef struct RinIPv6Config {
    uint32_t version;
    uint32_t struct_size;
    uint32_t source;
    uint32_t flags;
    uint16_t device_generation;
    uint16_t reserved0;
    uint32_t reserved1;
    uint64_t lease_generation;
    uint64_t preferred_lifetime_ms;
    uint64_t valid_lifetime_ms;
    uint8_t address[16];
    uint8_t dns[RIN_IPV6_CONFIG_MAX_DNS_SERVERS][16];
    uint32_t dns_count;
    uint32_t reserved2;
} RinIPv6Config;

/* A read-only NDP snapshot gives DHCPv6 precisely the link-local and RA
 * managed/other evidence it needs; it does not expose mutable NDP state. */
typedef struct RinNetIPv6Info {
    uint32_t version;
    uint32_t struct_size;
    uint32_t ndp_generation;
    uint16_t device_generation;
    uint8_t flags;
    uint8_t current_hop_limit;
    uint16_t link_mtu;
    uint16_t reserved0;
    uint8_t link_local[16];
} RinNetIPv6Info;

#define RIN_NET_IPV6_INFO_VERSION             1u
#define RIN_NET_IPV6_INFO_FLAG_LINK_LOCAL     0x01u
#define RIN_NET_IPV6_INFO_FLAG_MANAGED         0x02u
#define RIN_NET_IPV6_INFO_FLAG_OTHER           0x04u
#define RIN_NET_IPV6_INFO_KNOWN_FLAGS \
    (RIN_NET_IPV6_INFO_FLAG_LINK_LOCAL | RIN_NET_IPV6_INFO_FLAG_MANAGED | \
     RIN_NET_IPV6_INFO_FLAG_OTHER)

#if defined(__cplusplus)
static_assert(sizeof(RinNetPrimaryInfo) == 52u,
              "RinNetPrimaryInfo ABI drift");
static_assert(sizeof(RinNetInterfaceStatisticsV1) == 64u,
              "RinNetInterfaceStatisticsV1 ABI drift");
static_assert(sizeof(RinNetIpGlobalStatisticsV1) == 56u,
              "RinNetIpGlobalStatisticsV1 ABI drift");
static_assert(sizeof(RinNetUdpGlobalStatisticsV1) == 64u,
              "RinNetUdpGlobalStatisticsV1 ABI drift");
static_assert(sizeof(RinNetWifiInfoV1) == 80u,
              "RinNetWifiInfoV1 ABI drift");
static_assert(__builtin_offsetof(RinNetPrimaryInfo, device_generation) == 22u,
              "RinNetPrimaryInfo.device_generation ABI drift");
static_assert(__builtin_offsetof(RinNetPrimaryInfo, flags) == 40u,
              "RinNetPrimaryInfo.flags ABI drift");
static_assert(__builtin_offsetof(RinNetPrimaryInfo, link_type) == 44u,
              "RinNetPrimaryInfo.link_type ABI drift");
static_assert(sizeof(RinIPv6Config) == 120u,
              "RinIPv6Config ABI drift");
static_assert(sizeof(RinNetIPv6Info) == 36u,
              "RinNetIPv6Info ABI drift");
#else
_Static_assert(sizeof(RinNetPrimaryInfo) == 52u,
               "RinNetPrimaryInfo ABI drift");
_Static_assert(sizeof(RinNetInterfaceStatisticsV1) == 64u,
               "RinNetInterfaceStatisticsV1 ABI drift");
_Static_assert(sizeof(RinNetIpGlobalStatisticsV1) == 56u,
               "RinNetIpGlobalStatisticsV1 ABI drift");
_Static_assert(sizeof(RinNetUdpGlobalStatisticsV1) == 64u,
               "RinNetUdpGlobalStatisticsV1 ABI drift");
_Static_assert(sizeof(RinNetWifiInfoV1) == 80u,
               "RinNetWifiInfoV1 ABI drift");
_Static_assert(__builtin_offsetof(RinNetPrimaryInfo, device_generation) == 22u,
               "RinNetPrimaryInfo.device_generation ABI drift");
_Static_assert(__builtin_offsetof(RinNetPrimaryInfo, flags) == 40u,
               "RinNetPrimaryInfo.flags ABI drift");
_Static_assert(__builtin_offsetof(RinNetPrimaryInfo, link_type) == 44u,
               "RinNetPrimaryInfo.link_type ABI drift");
_Static_assert(sizeof(RinIPv6Config) == 120u,
               "RinIPv6Config ABI drift");
_Static_assert(sizeof(RinNetIPv6Info) == 36u,
               "RinNetIPv6Info ABI drift");
#endif

int rin_net_get_interface_statistics(RinNetInterfaceStatisticsV1* out);
int rin_net_get_ip_global_statistics(
    uint32_t address_family, RinNetIpGlobalStatisticsV1* out);
int rin_net_get_udp_global_statistics(
    uint32_t address_family, RinNetUdpGlobalStatisticsV1* out);

#endif
