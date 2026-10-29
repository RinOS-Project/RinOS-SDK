/* SPDX-License-Identifier: MIT */
/* RinOS kernel interfaces for Linux-compatible splice-family calls. */
#ifndef RIN_SPLICE_ABI_H
#define RIN_SPLICE_ABI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_SPLICE_CALL_ABI_VERSION_V1 UINT16_C(1)

#define RIN_SPLICE_OP_SPLICE UINT16_C(1)
#define RIN_SPLICE_OP_TEE UINT16_C(2)
#define RIN_SPLICE_OP_VMSPLICE UINT16_C(3)
#define RIN_SPLICE_OP_COPY_FILE_RANGE UINT16_C(4)

#define RIN_SPLICE_F_MOVE UINT32_C(0x01)
#define RIN_SPLICE_F_NONBLOCK UINT32_C(0x02)
#define RIN_SPLICE_F_MORE UINT32_C(0x04)
#define RIN_SPLICE_F_GIFT UINT32_C(0x08)
#define RIN_SPLICE_F_ALL (RIN_SPLICE_F_MOVE | RIN_SPLICE_F_NONBLOCK | \
                          RIN_SPLICE_F_MORE | RIN_SPLICE_F_GIFT)

/* MOVE and MORE are accepted optimization hints and may be ignored by a
 * backend. NONBLOCK makes pipe/socket waits return EAGAIN. GIFT applies only
 * to vmsplice from user memory into a pipe; the caller must not modify gifted
 * pages until the pipe releases them. */

/* All pointer fields are zero-extended user virtual addresses. The request
 * itself and every pointed-to structure use this fixed layout on IA-32,
 * x86_64 native, and x86_64 compat32. vmsplice iovecs retain the caller's
 * native pointer-width layout and are decoded by the architecture adapter. */
typedef struct RinSpliceCallV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    int32_t fd_in;
    int32_t fd_out;
    uint32_t flags;
    uint32_t iov_count;
    uint64_t offset_in_pointer;
    uint64_t offset_out_pointer;
    uint64_t iov_pointer;
    uint64_t length;
    uint64_t reserved[2];
} RinSpliceCallV1;

#ifdef __cplusplus
}
#endif

#endif /* RIN_SPLICE_ABI_H */
