/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_SDK_RIN_TTY_ABI_H
#define RIN_SDK_RIN_TTY_ABI_H

#include <stdint.h>

/* This is deliberately a small, pointer-free subset of the Linux termios
 * wire layout.  Both kernel personalities and RinPort libc use it directly,
 * so an ioctl never copies a target-word-dependent structure. */
#define RIN_TTY_ABI_VERSION UINT32_C(0x00010000)
#define RIN_TTY_NCCS UINT32_C(32)

typedef struct RinTtyTermiosV1 {
    uint32_t c_iflag;
    uint32_t c_oflag;
    uint32_t c_cflag;
    uint32_t c_lflag;
    uint8_t c_line;
    uint8_t c_cc[RIN_TTY_NCCS];
    uint8_t reserved[3];
    uint32_t c_ispeed;
    uint32_t c_ospeed;
} RinTtyTermiosV1;

/* Pointer-free terminal geometry exchanged by the TTY ioctl owner.  The
 * fields intentionally match the public `struct winsize` layout so native
 * and compat callers share the same 8-byte copy boundary. */
typedef struct RinTtyWinsizeV1 {
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
    uint16_t ws_ypixel;
} RinTtyWinsizeV1;

/* Linux-compatible request values exposed by RinPort's sys/ioctl.h. */
#define RIN_TTY_IOCTL_TCGETS      UINT32_C(0x5401)
#define RIN_TTY_IOCTL_TCSETS      UINT32_C(0x5402)
#define RIN_TTY_IOCTL_TCSETSW     UINT32_C(0x5403)
#define RIN_TTY_IOCTL_TCSETSF     UINT32_C(0x5404)
#define RIN_TTY_IOCTL_TCSBRK      UINT32_C(0x5409)
#define RIN_TTY_IOCTL_TCXONC      UINT32_C(0x540A)
#define RIN_TTY_IOCTL_TCFLSH      UINT32_C(0x540B)
#define RIN_TTY_IOCTL_TIOCSCTTY   UINT32_C(0x540E)
#define RIN_TTY_IOCTL_TIOCGPGRP   UINT32_C(0x540F)
#define RIN_TTY_IOCTL_TIOCSPGRP   UINT32_C(0x5410)
#define RIN_TTY_IOCTL_TIOCGSID    UINT32_C(0x5429)
#define RIN_TTY_IOCTL_TIOCNOTTY   UINT32_C(0x5422)
#define RIN_TTY_IOCTL_TCSBRKP     UINT32_C(0x5425)
#define RIN_TTY_IOCTL_TIOCGWINSZ  UINT32_C(0x5413)
#define RIN_TTY_IOCTL_TIOCSWINSZ  UINT32_C(0x5414)
#define RIN_TTY_IOCTL_TIOCGPTN    UINT32_C(0x80045430)
#define RIN_TTY_IOCTL_TIOCSPTLCK  UINT32_C(0x40045431)

#define RIN_TTY_TCIFLUSH UINT32_C(0)
#define RIN_TTY_TCOFLUSH UINT32_C(1)
#define RIN_TTY_TCIOFLUSH UINT32_C(2)
#define RIN_TTY_TCOOFF UINT32_C(0)
#define RIN_TTY_TCOON UINT32_C(1)
#define RIN_TTY_TCIOFF UINT32_C(2)
#define RIN_TTY_TCION UINT32_C(3)

/* Values used by the serial adapter; they intentionally match termios.h. */
#define RIN_TTY_CBAUD UINT32_C(0x0000f)
#define RIN_TTY_CBAUDEX UINT32_C(0x01000)
#define RIN_TTY_CS5 UINT32_C(0x00000)
#define RIN_TTY_CS6 UINT32_C(0x00010)
#define RIN_TTY_CS7 UINT32_C(0x00020)
#define RIN_TTY_CS8 UINT32_C(0x00030)
#define RIN_TTY_CSIZE UINT32_C(0x00030)
#define RIN_TTY_CSTOPB UINT32_C(0x00040)
#define RIN_TTY_PARENB UINT32_C(0x00100)
#define RIN_TTY_PARODD UINT32_C(0x00200)
#define RIN_TTY_IXON UINT32_C(0x00400)
#define RIN_TTY_VSTART UINT32_C(8)
#define RIN_TTY_VSTOP UINT32_C(9)

/* Bounded software line-discipline flag subset shared with RinPort termios. */
#define RIN_TTY_IGNCR UINT32_C(0x00080)
#define RIN_TTY_INLCR UINT32_C(0x00040)
#define RIN_TTY_ICRNL UINT32_C(0x00100)
#define RIN_TTY_ISTRIP UINT32_C(0x00020)
#define RIN_TTY_IXANY UINT32_C(0x00800)
#define RIN_TTY_OPOST UINT32_C(0x00001)
#define RIN_TTY_ONLCR UINT32_C(0x00004)
#define RIN_TTY_OCRNL UINT32_C(0x00008)
#define RIN_TTY_ONOCR UINT32_C(0x00010)
#define RIN_TTY_CREAD UINT32_C(0x00080)
#define RIN_TTY_CLOCAL UINT32_C(0x00800)
#define RIN_TTY_ISIG UINT32_C(0x00001)
#define RIN_TTY_ICANON UINT32_C(0x00002)
#define RIN_TTY_ECHO UINT32_C(0x00008)
#define RIN_TTY_ECHOE UINT32_C(0x00010)
#define RIN_TTY_ECHOK UINT32_C(0x00020)
#define RIN_TTY_ECHONL UINT32_C(0x00040)
#define RIN_TTY_NOFLSH UINT32_C(0x00080)
#define RIN_TTY_TOSTOP UINT32_C(0x00100)
#define RIN_TTY_ECHOCTL UINT32_C(0x00200)

#define RIN_TTY_VINTR UINT32_C(0)
#define RIN_TTY_VQUIT UINT32_C(1)
#define RIN_TTY_VERASE UINT32_C(2)
#define RIN_TTY_VKILL UINT32_C(3)
#define RIN_TTY_VEOF UINT32_C(4)
#define RIN_TTY_VTIME UINT32_C(5)
#define RIN_TTY_VMIN UINT32_C(6)
#define RIN_TTY_VSUSP UINT32_C(10)
#define RIN_TTY_VEOL UINT32_C(11)

/* Kernel-internal operations return negative values of these POSIX errors. */
#define RIN_TTY_EPERM UINT32_C(1)
#define RIN_TTY_EIO UINT32_C(5)
#define RIN_TTY_EBADF UINT32_C(9)
#define RIN_TTY_EINTR UINT32_C(4)
#define RIN_TTY_EAGAIN UINT32_C(11)
#define RIN_TTY_EINVAL UINT32_C(22)
#define RIN_TTY_ENOTTY UINT32_C(25)
#define RIN_TTY_EBUSY UINT32_C(16)
#define RIN_TTY_EOPNOTSUPP UINT32_C(207)

#if defined(__cplusplus)
static_assert(sizeof(RinTtyTermiosV1) == 60u, "termios wire ABI drift");
static_assert(sizeof(RinTtyWinsizeV1) == 8u, "winsize wire ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinTtyTermiosV1) == 60u, "termios wire ABI drift");
_Static_assert(sizeof(RinTtyWinsizeV1) == 8u, "winsize wire ABI drift");
#endif

#endif /* RIN_SDK_RIN_TTY_ABI_H */
