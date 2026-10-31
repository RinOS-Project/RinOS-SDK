/* SPDX-License-Identifier: MIT */
#ifndef RIN_ATOMIC_FALLBACK_V1_H
#define RIN_ATOMIC_FALLBACK_V1_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Outlined fallback for generic __atomic_* operations wider than the target's
 * lock-free scalar width. Object addresses may be under-aligned. The value
 * buffers must each designate at least `size` readable or writable bytes;
 * they may overlap one another and the object. The implementation serializes
 * all calls and provides sequentially consistent ordering, which is stronger
 * than every valid order argument. */
void __rin_atomic_load_v1(const volatile void *object, void *result,
                          size_t size, int memory_order);
void __rin_atomic_store_v1(volatile void *object, const void *value,
                           size_t size, int memory_order);
void __rin_atomic_exchange_v1(volatile void *object, const void *desired,
                              void *result, size_t size, int memory_order);
int __rin_atomic_compare_exchange_v1(
    volatile void *object, void *expected, const void *desired, size_t size,
    int weak, int success_order, int failure_order);

#ifdef __cplusplus
}
#endif

#endif
