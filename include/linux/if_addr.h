/* SPDX-License-Identifier: MIT */
#ifndef _UAPI__LINUX_IF_ADDR_H
#define _UAPI__LINUX_IF_ADDR_H

#include <stdint.h>
#include <linux/netlink.h>

struct ifaddrmsg {
    uint8_t ifa_family;
    uint8_t ifa_prefixlen;
    uint8_t ifa_flags;
    uint8_t ifa_scope;
    uint32_t ifa_index;
};

enum {
    IFA_UNSPEC,
    IFA_ADDRESS,
    IFA_LOCAL,
    IFA_LABEL,
    IFA_BROADCAST,
    IFA_ANYCAST,
    IFA_CACHEINFO,
    IFA_MULTICAST,
    IFA_FLAGS,
    IFA_RT_PRIORITY,
    IFA_TARGET_NETNSID,
    IFA_PROTO,
    IFA_MC_USERS,
    __IFA_MAX,
};
#define IFA_MAX (__IFA_MAX - 1)

#define IFA_F_SECONDARY       0x01
#define IFA_F_TEMPORARY       IFA_F_SECONDARY
#define IFA_F_NODAD           0x02
#define IFA_F_OPTIMISTIC      0x04
#define IFA_F_DADFAILED       0x08
#define IFA_F_HOMEADDRESS     0x10
#define IFA_F_DEPRECATED      0x20
#define IFA_F_TENTATIVE       0x40
#define IFA_F_PERMANENT       0x80
#define IFA_F_MANAGETEMPADDR  0x100
#define IFA_F_NOPREFIXROUTE   0x200
#define IFA_F_MCAUTOJOIN      0x400
#define IFA_F_STABLE_PRIVACY  0x800
#define IFA_F_GLOBAL          0x1000

struct ifa_cacheinfo {
    uint32_t ifa_prefered;
    uint32_t ifa_valid;
    uint32_t cstamp;
    uint32_t tstamp;
};

#ifndef __KERNEL__
struct rtattr;
#define IFA_RTA(r) \
    ((struct rtattr*)(((char*)(r)) + \
                      NLMSG_ALIGN(sizeof(struct ifaddrmsg))))
#define IFA_PAYLOAD(n) NLMSG_PAYLOAD(n, sizeof(struct ifaddrmsg))
#endif

#define IFAPROT_UNSPEC     0
#define IFAPROT_KERNEL_LO  1
#define IFAPROT_KERNEL_RA  2
#define IFAPROT_KERNEL_LL  3

#endif
