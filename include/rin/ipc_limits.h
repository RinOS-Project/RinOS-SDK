/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_IPC_LIMITS_H
#define RIN_SDK_IPC_LIMITS_H

#include <stdint.h>

/* Maximum number of readiness sources accepted by rin_wait_many_v1 and
 * rin_wait_set_set_items_v1. */
#define RIN_WAIT_MAX_ITEMS UINT32_C(64)

#endif /* RIN_SDK_IPC_LIMITS_H */
