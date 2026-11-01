/* SPDX-License-Identifier: MIT */
#ifndef RIN_NANOSLEEP_ABI_H
#define RIN_NANOSLEEP_ABI_H

#include <stdint.h>

/* SYS_NANOSLEEP のslot 2で選択する固定幅request形式。 */
#define RIN_NANOSLEEP_ABI_SELECTOR_V1 UINT32_C(0x4e534c50)
#define RIN_NANOSLEEP_ABI_VERSION_V1 UINT32_C(1)

typedef struct RinNanosleepRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t reserved0;
    int64_t duration_sec;
    int64_t duration_nsec;
    int64_t remaining_sec;
    int64_t remaining_nsec;
    uint32_t reserved[2];
} RinNanosleepRequestV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNanosleepRequestV1) == 56,
              "RinNanosleepRequestV1 ABI drift");
#else
_Static_assert(sizeof(RinNanosleepRequestV1) == 56,
               "RinNanosleepRequestV1 ABI drift");
#endif

/* slot 1にRinNanosleepRequestV1へのpointer、slot 2に上記selectorを渡す。
 * durationは単調時計を基準とする相対時間。成功時のremaining欄は0、
 * EINTR時は残時間を非負の秒・ナノ秒で返す。起床精度はscheduler timerの
 * 分解能に従う。slot 2に別の値を渡すと、従来のarchitecture別timespec入力を
 * 維持し、slot 2を無視する。 */

#endif /* RIN_NANOSLEEP_ABI_H */
