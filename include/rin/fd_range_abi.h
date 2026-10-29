/* SPDX-License-Identifier: MIT */
/* Descriptor range syscall flags shared by RinOS kernel and clients. */

#ifndef RIN_FD_RANGE_ABI_H
#define RIN_FD_RANGE_ABI_H

#include <stdint.h>

#define RIN_CLOSE_RANGE_FLAG_CLOEXEC UINT32_C(0x00000004)
/* Linux-compatible value: separate the caller's table from sharers, then
 * apply the range operation to the private fork.  RinOS separates at process
 * granularity (threads of one process keep sharing one table). */
#define RIN_CLOSE_RANGE_FLAG_UNSHARE UINT32_C(0x00000002)

#endif /* RIN_FD_RANGE_ABI_H */
