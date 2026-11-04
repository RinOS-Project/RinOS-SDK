/* SPDX-License-Identifier: MIT */
#ifndef _RIN_LINUX_RTNETLINK_H
#define _RIN_LINUX_RTNETLINK_H

#include <stdint.h>
#include <linux/netlink.h>
#include <linux/if_addr.h>

#define RTM_NEWLINK 16
#define RTM_DELLINK 17
#define RTM_GETLINK 18
#define RTM_SETLINK 19
#define RTM_NEWADDR 20
#define RTM_DELADDR 21
#define RTM_GETADDR 22
#define RTM_NEWROUTE 24
#define RTM_DELROUTE 25
#define RTM_GETROUTE 26
#define RTM_NEWNEIGH 28
#define RTM_DELNEIGH 29
#define RTM_GETNEIGH 30
#define RTM_NEWRULE 32
#define RTM_DELRULE 33
#define RTM_GETRULE 34
#define RTM_NEWLINKPROP 108
#define RTM_DELLINKPROP 109
#define RTM_GETLINKPROP 110

#define RTMGRP_LINK (1u << 0)
#define RTMGRP_NOTIFY (1u << 2)
#define RTMGRP_IPV4_IFADDR (1u << 4)
#define RTMGRP_IPV4_ROUTE (1u << 6)
#define RTMGRP_IPV4_RULE (1u << 7)
#define RTMGRP_IPV6_IFADDR (1u << 8)
#define RTMGRP_IPV6_ROUTE (1u << 10)
#define RTMGRP_IPV6_RULE (1u << 11)

/* RinOSはGETLINK/GETADDR/GETROUTE/GETRULE dump、route/rule add/delete、
 * linkのadmin up/down、root namespaceのIPv4/IPv6 address変更とlink/
 * address/route/rule multicast snapshotに対応する。IPv6 static addressは
 * NDP duplicate-address detectionを通る。link deletion、non-rootのlink/
 * address管理、その他のlink attributeは未対応。 */
#define RIN_RTNL_SUPPORT_GETLINK 1
#define RIN_RTNL_SUPPORT_GETADDR 1
#define RIN_RTNL_SUPPORT_GETROUTE 1
#define RIN_RTNL_SUPPORT_GETRULE 1
#define RIN_RTNL_SUPPORT_ROUTE_ADD_DELETE 1
#define RIN_RTNL_SUPPORT_RULE_ADD_DELETE 1
/* The aggregate is reserved for complete link and address mutation parity. */
#define RIN_RTNL_SUPPORT_LINK_ADDRESS_MUTATION 0
#define RIN_RTNL_SUPPORT_LINK_ADMIN_STATE 1
#define RIN_RTNL_SUPPORT_IPV4_ADDRESS_MUTATION 1
#define RIN_RTNL_SUPPORT_IPV6_ADDRESS_MUTATION 1

#define IFLA_UNSPEC 0
#define IFLA_ADDRESS 1
#define IFLA_BROADCAST 2
#define IFLA_IFNAME 3
#define IFLA_MTU 4
#define IFLA_LINK 5
#define IFLA_QDISC 6
#define IFLA_STATS 7
#define IFLA_OPERSTATE 16
#define IFLA_LINKMODE 17
#define IFLA_IFALIAS 20
#define IFLA_CARRIER 33
#define IFLA_MAX 64

#define IFF_UP 0x1
#define IFF_BROADCAST 0x2
#define IFF_LOOPBACK 0x8
#define IFF_RUNNING 0x40
#define IFF_MULTICAST 0x1000

#define RT_SCOPE_UNIVERSE 0
#define RT_SCOPE_LINK 253
#define RT_SCOPE_HOST 254
#define RT_SCOPE_NOWHERE 255

#define RTN_UNSPEC 0
#define RTN_UNICAST 1
#define RTN_LOCAL 2
#define RTN_BROADCAST 3
#define RTN_ANYCAST 4
#define RTN_MULTICAST 5
#define RTN_BLACKHOLE 6
#define RTN_UNREACHABLE 7
#define RTN_PROHIBIT 8
#define RTN_THROW 9
#define RTN_NAT 10
#define RTN_XRESOLVE 11

#define RTA_UNSPEC 0
#define RTA_DST 1
#define RTA_SRC 2
#define RTA_IIF 3
#define RTA_OIF 4
#define RTA_GATEWAY 5
#define RTA_PRIORITY 6
#define RTA_PREFSRC 7
#define RTA_METRICS 8
#define RTA_MULTIPATH 9
#define RTA_TABLE 15
#define RTA_MARK 16
#define RTA_UID 17
#define RTA_MAX 32

#define RT_TABLE_UNSPEC 0
#define RT_TABLE_COMPAT 252
#define RT_TABLE_DEFAULT 253
#define RT_TABLE_MAIN 254
#define RT_TABLE_LOCAL 255

#define RTM_F_NOTIFY 0x100
#define RTM_F_CLONED 0x200
#define RTM_F_EQUALIZE 0x400
#define RTM_F_PREFIX 0x800

struct ifinfomsg {
    uint8_t ifi_family;
    uint8_t __ifi_pad;
    uint16_t ifi_type;
    int32_t ifi_index;
    uint32_t ifi_flags;
    uint32_t ifi_change;
};

struct rtmsg {
    uint8_t rtm_family;
    uint8_t rtm_dst_len;
    uint8_t rtm_src_len;
    uint8_t rtm_tos;
    uint8_t rtm_table;
    uint8_t rtm_protocol;
    uint8_t rtm_scope;
    uint8_t rtm_type;
    uint32_t rtm_flags;
};

struct fib_rule_hdr {
    uint8_t family;
    uint8_t dst_len;
    uint8_t src_len;
    uint8_t tos;
    uint8_t table;
    uint8_t res1;
    uint8_t res2;
    uint8_t action;
    uint32_t flags;
};

#define FRA_UNSPEC 0
#define FRA_DST 1
#define FRA_SRC 2
#define FRA_IIFNAME 3
#define FRA_GOTO 4
#define FRA_UNUSED2 5
#define FRA_PRIORITY 6
#define FRA_UNUSED3 7
#define FRA_UNUSED4 8
#define FRA_UNUSED5 9
#define FRA_FWMARK 10
#define FRA_FLOW 11
#define FRA_TUN_ID 12
#define FRA_SUPPRESS_IFGROUP 13
#define FRA_SUPPRESS_PREFIXLEN 14
#define FRA_TABLE 15
#define FRA_FWMASK 16
#define FRA_OIFNAME 17
#define FRA_PAD 18
#define FRA_L3MDEV 19
#define FRA_UID_RANGE 20
#define FRA_PROTOCOL 21
#define FRA_IP_PROTO 22
#define FRA_SPORT_RANGE 23
#define FRA_DPORT_RANGE 24
#define FRA_MAX FRA_DPORT_RANGE

#define FR_ACT_UNSPEC 0
#define FR_ACT_TO_TBL 1
#define FR_ACT_GOTO 2
#define FR_ACT_NOP 3
#define FR_ACT_RES3 4
#define FR_ACT_RES4 5
#define FR_ACT_BLACKHOLE 6
#define FR_ACT_UNREACHABLE 7
#define FR_ACT_PROHIBIT 8
#define FR_ACT_MAX FR_ACT_PROHIBIT

struct rtattr {
    uint16_t rta_len;
    uint16_t rta_type;
};

#define RTA_ALIGNTO 4u
#define RTA_ALIGN(length) (((length) + RTA_ALIGNTO - 1u) & ~(RTA_ALIGNTO - 1u))
#define RTA_LENGTH(length) ((uint32_t)sizeof(struct rtattr) + (uint32_t)(length))
#define RTA_SPACE(length) ((uint32_t)RTA_ALIGN(RTA_LENGTH(length)))
#define RTA_DATA(attribute) ((void*)((uint8_t*)(attribute) + sizeof(struct rtattr)))
#define RTA_PAYLOAD(attribute) ((uint32_t)(attribute)->rta_len - (uint32_t)sizeof(struct rtattr))

struct ifinfomsg;
struct ifaddrmsg;
struct rtmsg;

#endif
