/* SPDX-License-Identifier: MIT */
#ifndef _RIN_LINUX_RTNETLINK_H
#define _RIN_LINUX_RTNETLINK_H

#include <stdint.h>
#include <linux/netlink.h>

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
#define RTM_NEWLINKPROP 108
#define RTM_DELLINKPROP 109
#define RTM_GETLINKPROP 110

#define RTMGRP_LINK (1u << 0)
#define RTMGRP_NOTIFY (1u << 2)
#define RTMGRP_IPV4_IFADDR (1u << 4)
#define RTMGRP_IPV4_ROUTE (1u << 6)
#define RTMGRP_IPV6_IFADDR (1u << 8)
#define RTMGRP_IPV6_ROUTE (1u << 10)

/* RinOS serves GETLINK/GETADDR/GETROUTE dumps, route add/delete, link
 * administrative up/down, root-namespace IPv4 address add/replace/delete,
 * and link/address/route multicast snapshots. Link deletion, IPv6 address
 * mutation, and non-root link/address administration remain unsupported. */
#define RIN_RTNL_SUPPORT_GETLINK 1
#define RIN_RTNL_SUPPORT_GETADDR 1
#define RIN_RTNL_SUPPORT_GETROUTE 1
#define RIN_RTNL_SUPPORT_ROUTE_ADD_DELETE 1
/* The aggregate is reserved for complete link and address mutation parity. */
#define RIN_RTNL_SUPPORT_LINK_ADDRESS_MUTATION 0
#define RIN_RTNL_SUPPORT_LINK_ADMIN_STATE 1
#define RIN_RTNL_SUPPORT_IPV4_ADDRESS_MUTATION 1

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

#define IFA_UNSPEC 0
#define IFA_ADDRESS 1
#define IFA_LOCAL 2
#define IFA_LABEL 3
#define IFA_BROADCAST 4
#define IFA_ANYCAST 5
#define IFA_CACHEINFO 6
#define IFA_FLAGS 8
#define IFA_MAX 8

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
#define RT_TABLE_MAIN 254

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

struct ifaddrmsg {
    uint8_t ifa_family;
    uint8_t ifa_prefixlen;
    uint8_t ifa_flags;
    uint8_t ifa_scope;
    uint32_t ifa_index;
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
