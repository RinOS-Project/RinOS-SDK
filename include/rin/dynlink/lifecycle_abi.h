/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_SDK_RIN_DYNLINK_LIFECYCLE_ABI_H
#define RIN_SDK_RIN_DYNLINK_LIFECYCLE_ABI_H

#include <stdint.h>

#define RIN_DLINVENTORY_LIFECYCLE_ABI_VERSION 1u

typedef enum RinDlLifecycleOperationV1 {
    RIN_DL_LIFECYCLE_INIT_NEXT = 1u,
    RIN_DL_LIFECYCLE_FINI_NEXT = 2u,
    RIN_DL_LIFECYCLE_PROCESS_FINI_NEXT = 3u,
    RIN_DL_LIFECYCLE_FINI_FINISH = 4u
} RinDlLifecycleOperationV1;

/* Input/output record for the SYS_DLINVENTORY lifecycle suboperation.  A
 * successful call returns one when callback_address is populated and zero
 * when the requested callback walk is complete. */
typedef struct RinDlLifecycleRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint64_t handle;
    uint64_t token;
    uint64_t callback_address;
} RinDlLifecycleRequestV1;

#if defined(__cplusplus)
static_assert(sizeof(RinDlLifecycleRequestV1) == 32u,
              "RinDlLifecycleRequestV1 ABI size");
#else
_Static_assert(sizeof(RinDlLifecycleRequestV1) == 32u,
               "RinDlLifecycleRequestV1 ABI size");
#endif

#endif /* RIN_SDK_RIN_DYNLINK_LIFECYCLE_ABI_H */
