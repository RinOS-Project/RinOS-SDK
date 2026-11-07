/* SPDX-License-Identifier: MIT */
#ifndef _RIN_LINUX_NETLINK_H
#define _RIN_LINUX_NETLINK_H

#include <stdint.h>

#define NETLINK_ROUTE 0
#define NETLINK_SOCK_DIAG 4

#define SOL_NETLINK 270
#define NETLINK_LIST_MEMBERSHIPS 9
#define NETLINK_ADD_MEMBERSHIP 1
#define NETLINK_DROP_MEMBERSHIP 2
#define NETLINK_PKTINFO 3
#define NETLINK_BROADCAST_ERROR 4
#define NETLINK_NO_ENOBUFS 5
#define NETLINK_LISTEN_ALL_NSID 8
#define NETLINK_CAP_ACK 10
#define NETLINK_EXT_ACK 11

#define NLMSG_NOOP 1
#define NLMSG_ERROR 2
#define NLMSG_DONE 3
#define NLMSG_OVERRUN 4
#define NLMSG_MIN_TYPE 0x10

#define NLM_F_REQUEST 0x01
#define NLM_F_MULTI 0x02
#define NLM_F_ACK 0x04
#define NLM_F_ECHO 0x08
#define NLM_F_DUMP_INTR 0x10
#define NLM_F_DUMP_FILTERED 0x20
#define NLM_F_ROOT 0x100
#define NLM_F_MATCH 0x200
#define NLM_F_ATOMIC 0x400
#define NLM_F_DUMP (NLM_F_ROOT | NLM_F_MATCH)
#define NLM_F_REPLACE 0x100
#define NLM_F_EXCL 0x200
#define NLM_F_CREATE 0x400
#define NLM_F_APPEND 0x800

#define NLMSG_ALIGNTO 4u
#define NLMSG_ALIGN(length) (((length) + NLMSG_ALIGNTO - 1u) & ~(NLMSG_ALIGNTO - 1u))
#define NLMSG_HDRLEN ((uint32_t)NLMSG_ALIGN(sizeof(struct nlmsghdr)))
#define NLMSG_LENGTH(length) ((uint32_t)(NLMSG_HDRLEN + (uint32_t)(length)))
#define NLMSG_SPACE(length) ((uint32_t)NLMSG_ALIGN(NLMSG_LENGTH(length)))
#define NLMSG_DATA(header) ((void*)((uint8_t*)(header) + NLMSG_HDRLEN))
#define NLMSG_PAYLOAD(header, length) \
    ((header)->nlmsg_len - NLMSG_SPACE((length)))
#define NLMSG_OK(header, remaining) \
    ((remaining) >= (int)sizeof(struct nlmsghdr) && \
     (header)->nlmsg_len >= sizeof(struct nlmsghdr) && \
     (header)->nlmsg_len <= (uint32_t)(remaining))
#define NLMSG_NEXT(header, remaining) \
    ((remaining) -= (int)NLMSG_ALIGN((header)->nlmsg_len), \
     (struct nlmsghdr*)(((uint8_t*)(header)) + NLMSG_ALIGN((header)->nlmsg_len)))

struct sockaddr_nl {
    uint16_t nl_family;
    uint16_t nl_pad;
    uint32_t nl_pid;
    uint32_t nl_groups;
};

struct nlmsghdr {
    uint32_t nlmsg_len;
    uint16_t nlmsg_type;
    uint16_t nlmsg_flags;
    uint32_t nlmsg_seq;
    uint32_t nlmsg_pid;
};

struct nlmsgerr {
    int32_t error;
    struct nlmsghdr msg;
};

struct nlattr {
    uint16_t nla_len;
    uint16_t nla_type;
};

#define NLA_ALIGNTO 4u
#define NLA_ALIGN(length) (((length) + NLA_ALIGNTO - 1u) & ~(NLA_ALIGNTO - 1u))
#define NLA_HDRLEN ((uint32_t)NLA_ALIGN(sizeof(struct nlattr)))
#define NLA_DATA(attribute) ((void*)((uint8_t*)(attribute) + NLA_HDRLEN))
#define NLA_LENGTH(length) ((uint32_t)(NLA_HDRLEN + (uint32_t)(length)))
#define NLA_SPACE(length) ((uint32_t)NLA_ALIGN(NLA_LENGTH(length)))

#define NLA_F_NESTED (1u << 15)
#define NLA_F_NET_BYTEORDER (1u << 14)
#define NLA_TYPE_MASK (~(NLA_F_NESTED | NLA_F_NET_BYTEORDER))

#endif
