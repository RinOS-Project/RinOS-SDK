/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_SDK_RIN_TTY_TERMIO_ABI_H
#define RIN_SDK_RIN_TTY_TERMIO_ABI_H

#include <stdint.h>

/* 旧端末属性の固定wire layout（16-bit flag 4語、line byte、制御文字8個）。 */
#define RIN_TTY_TERMIO_NCC UINT32_C(8)

typedef struct RinTtyTermioV1 {
    uint16_t c_iflag;
    uint16_t c_oflag;
    uint16_t c_cflag;
    uint16_t c_lflag;
    uint8_t c_line;
    uint8_t c_cc[RIN_TTY_TERMIO_NCC];
} RinTtyTermioV1;

#define RIN_TTY_IOCTL_TCGETA UINT32_C(0x5405)
#define RIN_TTY_IOCTL_TCSETA UINT32_C(0x5406)
#define RIN_TTY_IOCTL_TCSETAW UINT32_C(0x5407)
#define RIN_TTY_IOCTL_TCSETAF UINT32_C(0x5408)

#if defined(__cplusplus)
static_assert(sizeof(RinTtyTermioV1) == 18u, "termio wire ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinTtyTermioV1) == 18u, "termio wire ABI drift");
#endif

#endif /* RIN_SDK_RIN_TTY_TERMIO_ABI_H */
