/* SPDX-License-Identifier: MIT */
#ifndef _RIN_LINUX_PACKET_DIAG_H
#define _RIN_LINUX_PACKET_DIAG_H

#include <stdint.h>

struct packet_diag_req {
    uint8_t sdiag_family;
    uint8_t sdiag_protocol;
    uint16_t pad;
    uint32_t pdiag_ino;
    uint32_t pdiag_show;
    uint32_t pdiag_cookie[2];
};

#define PACKET_SHOW_INFO UINT32_C(0x00000001)
#define PACKET_SHOW_MCLIST UINT32_C(0x00000002)
#define PACKET_SHOW_RING_CFG UINT32_C(0x00000004)
#define PACKET_SHOW_FANOUT UINT32_C(0x00000008)
#define PACKET_SHOW_MEMINFO UINT32_C(0x00000010)
#define PACKET_SHOW_FILTER UINT32_C(0x00000020)

struct packet_diag_msg {
    uint8_t pdiag_family;
    uint8_t pdiag_type;
    uint16_t pdiag_num;
    uint32_t pdiag_ino;
    uint32_t pdiag_cookie[2];
};

enum {
    PACKET_DIAG_NONE,
    PACKET_DIAG_INFO,
    PACKET_DIAG_MCLIST,
    PACKET_DIAG_RX_RING,
    PACKET_DIAG_TX_RING,
    PACKET_DIAG_FANOUT,
    PACKET_DIAG_UID,
    PACKET_DIAG_MEMINFO,
    PACKET_DIAG_FILTER
};

struct packet_diag_info {
    uint32_t pdi_index;
    uint32_t pdi_version;
    uint32_t pdi_reserve;
    uint32_t pdi_copy_thresh;
    uint32_t pdi_tstamp;
    uint32_t pdi_flags;
};

struct packet_diag_ring {
    uint32_t pdr_block_size;
    uint32_t pdr_block_nr;
    uint32_t pdr_frame_size;
    uint32_t pdr_frame_nr;
    uint32_t pdr_retire_tmo;
    uint32_t pdr_sizeof_priv;
    uint32_t pdr_features;
};

#endif
