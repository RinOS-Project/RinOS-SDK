/* SPDX-License-Identifier: MIT */
#ifndef RIN_THREAD_NAME_ABI_H
#define RIN_THREAD_NAME_ABI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_THREAD_NAME_CALL_VERSION 1u
#define RIN_THREAD_NAME_CAPACITY 16u

enum RinThreadNameOperation {
    RIN_THREAD_NAME_OPERATION_SET = 1,
    RIN_THREAD_NAME_OPERATION_GET = 2
};

typedef struct RinThreadNameCallV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint64_t thread_id; /* zero selects the calling thread */
    char name[RIN_THREAD_NAME_CAPACITY];
    uint32_t reserved[2];
} RinThreadNameCallV1;

#if defined(__cplusplus)
static_assert(sizeof(RinThreadNameCallV1) == 40u,
              "RinThreadNameCallV1 ABI size mismatch");
#else
_Static_assert(sizeof(RinThreadNameCallV1) == 40u,
               "RinThreadNameCallV1 ABI size mismatch");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_THREAD_NAME_ABI_H */
