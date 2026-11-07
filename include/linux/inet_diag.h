/* SPDX-License-Identifier: MIT */
#ifndef _RIN_LINUX_INET_DIAG_H
#define _RIN_LINUX_INET_DIAG_H

#include <stdint.h>
#include <linux/sock_diag.h>

#define INET_DIAG_NOCOOKIE UINT32_C(0xffffffff)

struct inet_diag_sockid {
    uint16_t idiag_sport;
    uint16_t idiag_dport;
    uint32_t idiag_src[4];
    uint32_t idiag_dst[4];
    uint32_t idiag_if;
    uint32_t idiag_cookie[2];
};

struct inet_diag_req_v2 {
    uint8_t sdiag_family;
    uint8_t sdiag_protocol;
    uint8_t idiag_ext;
    uint8_t pad;
    uint32_t idiag_states;
    struct inet_diag_sockid id;
};

struct inet_diag_msg {
    uint8_t idiag_family;
    uint8_t idiag_state;
    uint8_t idiag_timer;
    uint8_t idiag_retrans;
    struct inet_diag_sockid id;
    uint32_t idiag_expires;
    uint32_t idiag_rqueue;
    uint32_t idiag_wqueue;
    uint32_t idiag_uid;
    uint32_t idiag_inode;
};

struct inet_diag_meminfo {
    uint32_t idiag_rmem;
    uint32_t idiag_wmem;
    uint32_t idiag_fmem;
    uint32_t idiag_tmem;
};

enum {
    INET_DIAG_NONE,
    INET_DIAG_MEMINFO,
    INET_DIAG_INFO,
    INET_DIAG_VEGASINFO,
    INET_DIAG_CONG,
    INET_DIAG_TOS,
    INET_DIAG_TCLASS,
    INET_DIAG_SKMEMINFO,
    INET_DIAG_SHUTDOWN,
    INET_DIAG_DCTCPINFO,
    INET_DIAG_PROTOCOL,
    INET_DIAG_SKV6ONLY,
    INET_DIAG_LOCALS,
    INET_DIAG_PEERS,
    INET_DIAG_PAD,
    INET_DIAG_MARK,
    INET_DIAG_BBRINFO,
    INET_DIAG_CLASS_ID,
    INET_DIAG_MD5SIG,
    INET_DIAG_ULP_INFO,
    INET_DIAG_SK_BPF_STORAGES,
    INET_DIAG_CGROUP_ID,
    INET_DIAG_SOCKOPT,
    __INET_DIAG_MAX
};

#define INET_DIAG_MAX (__INET_DIAG_MAX - 1)

#endif
