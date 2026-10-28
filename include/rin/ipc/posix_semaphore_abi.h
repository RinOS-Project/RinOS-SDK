/* SPDX-License-Identifier: MIT */
/* Fixed shared-page layout for the RinOS POSIX named-semaphore owner. */

#ifndef RIN_SDK_RIN_POSIX_SEMAPHORE_ABI_H
#define RIN_SDK_RIN_POSIX_SEMAPHORE_ABI_H

#include <stddef.h>
#include <stdint.h>

#define RIN_POSIX_SEMAPHORE_ABI_VERSION UINT32_C(0x00010000)
#define RIN_POSIX_SEMAPHORE_MAGIC       UINT32_C(0x31534d52) /* RMS1 */

#define RIN_POSIX_SEMAPHORE_PAGE_SIZE UINT32_C(4096)
#define RIN_POSIX_SEMAPHORE_NAME_MAX  UINT32_C(52)
#define RIN_POSIX_SEMAPHORE_MAX_VALUE INT32_MAX
#define RIN_POSIX_SEMAPHORE_WAITERS_SHARED_FLAG UINT32_C(0x80000000)
#define RIN_POSIX_SEMAPHORE_WAITERS_COUNT_MASK UINT32_C(0x7fffffff)

#define RIN_POSIX_SEMAPHORE_OPEN_CREAT UINT32_C(0x00000001)
#define RIN_POSIX_SEMAPHORE_OPEN_EXCL  UINT32_C(0x00000002)
#define RIN_POSIX_SEMAPHORE_OPEN_MODE_SHIFT 8u
#define RIN_POSIX_SEMAPHORE_OPEN_MODE_MASK \
    (UINT32_C(0777) << RIN_POSIX_SEMAPHORE_OPEN_MODE_SHIFT)
#define RIN_POSIX_SEMAPHORE_OPEN_MODE_PRESENT UINT32_C(0x00020000)
#define RIN_POSIX_SEMAPHORE_OPEN_FLAG_MASK \
    (RIN_POSIX_SEMAPHORE_OPEN_CREAT | RIN_POSIX_SEMAPHORE_OPEN_EXCL | \
     RIN_POSIX_SEMAPHORE_OPEN_MODE_MASK | \
     RIN_POSIX_SEMAPHORE_OPEN_MODE_PRESENT)

/* POSIX_SEM_OPENの3引数ABIを維持するためmodeをflagsに格納する。
 * MODE_PRESENTにより明示mode 000とmodeを送らない旧callerを区別する。
 * 旧O_CREAT callerのmodeは0666としてprocess umaskを適用する。 */

/* The public sem_t begins at value.  The preamble stays private to the
 * kernel and lets close reject arbitrary shared-memory mappings. */
typedef struct RinPosixSemaphoreSharedV1 {
    uint32_t magic;
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t reserved0;
    volatile int32_t value;
    volatile int32_t waiters;
    uint64_t reserved[5];
} RinPosixSemaphoreSharedV1;

#define RIN_POSIX_SEMAPHORE_VALUE_OFFSET UINT32_C(16)

#if defined(__cplusplus)
static_assert(sizeof(RinPosixSemaphoreSharedV1) == 64u,
              "POSIX semaphore shared ABI drift");
static_assert(offsetof(RinPosixSemaphoreSharedV1, value) ==
                  RIN_POSIX_SEMAPHORE_VALUE_OFFSET,
              "POSIX semaphore value offset drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinPosixSemaphoreSharedV1) == 64u,
               "POSIX semaphore shared ABI drift");
_Static_assert(offsetof(RinPosixSemaphoreSharedV1, value) ==
                   RIN_POSIX_SEMAPHORE_VALUE_OFFSET,
               "POSIX semaphore value offset drift");
#endif

#endif /* RIN_SDK_RIN_POSIX_SEMAPHORE_ABI_H */
