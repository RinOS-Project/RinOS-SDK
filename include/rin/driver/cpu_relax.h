/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_CPU_RELAX_H
#define RIN_DRIVER_CPU_RELAX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Portable processor wait hints for driver-owned bounded polling loops.
 * These helpers do not access kernel state or require a runtime object. */
static inline void rin_driver_cpu_relax(void)
{
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile("pause");
#elif defined(__aarch64__)
    __asm__ volatile("yield");
#else
    __atomic_signal_fence(__ATOMIC_RELAXED);
#endif
}

static inline void rin_driver_cpu_relax_with_barrier(void)
{
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile("pause" ::: "memory");
#elif defined(__aarch64__)
    __asm__ volatile("yield" ::: "memory");
#else
    __atomic_signal_fence(__ATOMIC_SEQ_CST);
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_CPU_RELAX_H */
