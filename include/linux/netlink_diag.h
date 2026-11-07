/* SPDX-License-Identifier: MIT */
#ifndef _RIN_LINUX_NETLINK_DIAG_H
#define _RIN_LINUX_NETLINK_DIAG_H

#include <stdint.h>

struct netlink_diag_req {
    uint8_t sdiag_family;
    uint8_t sdiag_protocol;
    uint16_t pad;
    uint32_t ndiag_ino;
    uint32_t ndiag_show;
    uint32_t ndiag_cookie[2];
};

struct netlink_diag_msg {
    uint8_t ndiag_family;
    uint8_t ndiag_type;
    uint8_t ndiag_protocol;
    uint8_t ndiag_state;
    uint32_t ndiag_portid;
    uint32_t ndiag_dst_portid;
    uint32_t ndiag_dst_group;
    uint32_t ndiag_ino;
    uint32_t ndiag_cookie[2];
};

struct netlink_diag_ring {
    uint32_t ndr_block_size;
    uint32_t ndr_block_nr;
    uint32_t ndr_frame_size;
    uint32_t ndr_frame_nr;
};

enum {
    NETLINK_DIAG_NONE,
    NETLINK_DIAG_MEMINFO,
    NETLINK_DIAG_GROUPS,
    NETLINK_DIAG_RX_RING,
    NETLINK_DIAG_TX_RING,
    NETLINK_DIAG_FLAGS
};

#define NDIAG_PROTO_ALL UINT8_C(0xff)
#define NDIAG_SHOW_MEMINFO UINT32_C(0x00000001)
#define NDIAG_SHOW_GROUPS UINT32_C(0x00000002)
#define NDIAG_SHOW_RING_CFG UINT32_C(0x00000004)
#define NDIAG_SHOW_FLAGS UINT32_C(0x00000008)

#endif
