/* SPDX-License-Identifier: MIT */
/* Public network-interface inspection and configuration ABI. */
#ifndef RIN_SDK_NET_INTERFACE_ABI_H
#define RIN_SDK_NET_INTERFACE_ABI_H

#include <stdint.h>
#include "ipv4.h"

#ifndef RIN_MAC_ADDR_DEFINED
#define RIN_MAC_ADDR_DEFINED
typedef struct {
    uint8_t bytes[6];
} RinMACAddr;
#endif

#ifndef RIN_NET_INTERFACE_EX_DEFINED
#define RIN_NET_INTERFACE_EX_DEFINED
typedef struct {
    char name[32];
    RinMACAddr mac;
    RinIPv4Addr ip;
    RinIPv4Addr netmask;
    RinIPv4Addr gateway;
    int is_up;
    /* 0/1 when the platform owns this information; -1 when unknown. */
    int is_dhcp;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_packets;
    uint64_t tx_packets;
} RinNetInterfaceEx;
#endif

int net_interface_list(RinNetInterfaceEx* ifaces, int max);
int net_interface_get(const char* name, RinNetInterfaceEx* iface);
int net_interface_set_ip(const char* name, RinIPv4Addr ip, RinIPv4Addr netmask);
int net_interface_up(const char* name);
int net_interface_down(const char* name);

#endif
