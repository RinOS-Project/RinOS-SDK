/* SPDX-License-Identifier: MIT */
#ifndef _RIN_LINUX_UNIX_DIAG_H
#define _RIN_LINUX_UNIX_DIAG_H

#include <stdint.h>

struct unix_diag_req {
    uint8_t sdiag_family;
    uint8_t sdiag_protocol;
    uint16_t pad;
    uint32_t udiag_states;
    uint32_t udiag_ino;
    uint32_t udiag_show;
    uint32_t udiag_cookie[2];
};

#define UDIAG_SHOW_NAME UINT32_C(0x00000001)
#define UDIAG_SHOW_VFS UINT32_C(0x00000002)
#define UDIAG_SHOW_PEER UINT32_C(0x00000004)
#define UDIAG_SHOW_ICONS UINT32_C(0x00000008)
#define UDIAG_SHOW_RQLEN UINT32_C(0x00000010)
#define UDIAG_SHOW_MEMINFO UINT32_C(0x00000020)
#define UDIAG_SHOW_UID UINT32_C(0x00000040)

struct unix_diag_msg {
    uint8_t udiag_family;
    uint8_t udiag_type;
    uint8_t udiag_state;
    uint8_t pad;
    uint32_t udiag_ino;
    uint32_t udiag_cookie[2];
};

enum {
    UNIX_DIAG_NONE,
    UNIX_DIAG_NAME,
    UNIX_DIAG_VFS,
    UNIX_DIAG_PEER,
    UNIX_DIAG_ICONS,
    UNIX_DIAG_RQLEN,
    UNIX_DIAG_MEMINFO,
    UNIX_DIAG_SHUTDOWN,
    UNIX_DIAG_UID
};

struct unix_diag_vfs {
    uint32_t udiag_vfs_ino;
    uint32_t udiag_vfs_dev;
};

struct unix_diag_rqlen {
    uint32_t udiag_rqueue;
    uint32_t udiag_wqueue;
};

#endif
