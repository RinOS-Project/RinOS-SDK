/* SPDX-License-Identifier: MIT */
#ifndef RIN_API_RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_ABI_H
#define RIN_API_RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_ABI_H

#include <stdint.h>

/* Canonical legacy SDK contract; retained only for the legacy namespace. */

#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_MAGIC \
    UINT32_C(0x31515249) /* IRQ1 */
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_VERSION UINT16_C(2)
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CAPACITY UINT32_C(8)

#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_CLEAN UINT32_C(0)
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_INITIALIZING UINT32_C(1)
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_ACTIVE UINT32_C(2)
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_CLOSING UINT32_C(3)
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_POISONED UINT32_C(4)

#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_ENABLE UINT8_C(1)
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_DISABLE UINT8_C(2)
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_FREE UINT8_C(3)
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_REQUEST UINT8_C(4)

#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_OK 0
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_INVALID -1
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_BUSY -2
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CLOSED -3
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CORRUPT -4
#define RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_EXHAUSTED -5

typedef struct RinLegacyDriverV1IrqControlCommand {
    uint8_t irq;
    uint8_t operation;
    uint16_t reserved;
    uint32_t generation;
    uint32_t handler;
    uint32_t data;
} RinLegacyDriverV1IrqControlCommand;

typedef struct RinLegacyDriverV1IrqControlResponse {
    volatile uint32_t generation;
    volatile int32_t result;
} RinLegacyDriverV1IrqControlResponse;

typedef struct RinLegacyDriverV1IrqControlPage {
    uint32_t magic;
    uint16_t version;
    uint16_t struct_size;
    volatile uint32_t lock;
    volatile uint32_t phase;
    volatile uint32_t count;
    volatile uint32_t overflow;
    volatile uint32_t request_generation;
    volatile uint32_t completed_generation;
    volatile int32_t last_result;
    uint32_t reserved;
    volatile RinLegacyDriverV1IrqControlCommand
        commands[RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CAPACITY];
    volatile RinLegacyDriverV1IrqControlResponse
        responses[RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CAPACITY];
    volatile uint32_t waiter_generation;
} RinLegacyDriverV1IrqControlPage;

static inline void rin_legacy_driver_v1_irq_control_relax(void)
{
#if defined(__i386__) || defined(__x86_64__)
    __asm__ volatile("pause" ::: "memory");
#endif
}

static inline int rin_legacy_driver_v1_irq_control_valid_operation(
    uint8_t operation)
{
    return operation == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_ENABLE ||
           operation == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_DISABLE ||
           operation == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_FREE ||
           operation == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_REQUEST;
}

static inline int rin_legacy_driver_v1_irq_control_enqueue(
    volatile RinLegacyDriverV1IrqControlPage* page,
    uint8_t irq, uint8_t operation, uint32_t handler, uint32_t data,
    uint32_t wait_for_result, uint32_t* generation_out)
{
    uint32_t count;
    uint32_t generation;
    uint32_t phase;
    int identity_valid;
    volatile RinLegacyDriverV1IrqControlCommand* command;
    if (generation_out) *generation_out = 0u;
    if (!page || irq >= 16u ||
        !rin_legacy_driver_v1_irq_control_valid_operation(operation) ||
        (operation == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_REQUEST &&
         handler == 0u) ||
        (wait_for_result != 0u &&
         operation != RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_REQUEST)) {
        return RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_INVALID;
    }
    while (__atomic_exchange_n(&page->lock, 1u,
                               __ATOMIC_ACQUIRE) != 0u) {
        rin_legacy_driver_v1_irq_control_relax();
    }
    phase = __atomic_load_n(&page->phase, __ATOMIC_ACQUIRE);
    identity_valid =
        page->magic == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_MAGIC &&
        page->version == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_VERSION &&
        page->struct_size == sizeof(*page) && page->reserved == 0u;
    if (!identity_valid ||
        (operation == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_REQUEST
             ? phase != RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_ACTIVE
             : (phase != RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_INITIALIZING &&
                phase != RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_ACTIVE &&
                phase != RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_CLOSING))) {
        __atomic_store_n(&page->lock, 0u, __ATOMIC_RELEASE);
        return identity_valid &&
                       phase == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_CLEAN
                   ? RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CLOSED
                   : RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CORRUPT;
    }
    count = __atomic_load_n(&page->count, __ATOMIC_RELAXED);
    generation = __atomic_load_n(
        &page->request_generation, __ATOMIC_RELAXED);
    if (__atomic_load_n(&page->overflow, __ATOMIC_RELAXED) != 0u ||
        count >= RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CAPACITY ||
        generation == UINT32_MAX ||
        __atomic_load_n(&page->waiter_generation,
                        __ATOMIC_RELAXED) != 0u) {
        if (count >= RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CAPACITY ||
            generation == UINT32_MAX) {
            __atomic_store_n(&page->overflow, 1u, __ATOMIC_RELEASE);
            __atomic_store_n(&page->lock, 0u, __ATOMIC_RELEASE);
            return RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_EXHAUSTED;
        }
        __atomic_store_n(&page->lock, 0u, __ATOMIC_RELEASE);
        return __atomic_load_n(&page->overflow, __ATOMIC_RELAXED) != 0u
                   ? RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_EXHAUSTED
                   : RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_BUSY;
    }
    command = &page->commands[count];
    command->irq = irq;
    command->operation = operation;
    command->reserved = 0u;
    command->generation = generation + 1u;
    command->handler = handler;
    command->data = data;
    if (wait_for_result != 0u) {
        __atomic_store_n(&page->waiter_generation, generation + 1u,
                         __ATOMIC_RELAXED);
    }
    __atomic_store_n(&page->request_generation, generation + 1u,
                     __ATOMIC_RELAXED);
    __atomic_store_n(&page->count, count + 1u, __ATOMIC_RELEASE);
    __atomic_store_n(&page->lock, 0u, __ATOMIC_RELEASE);
    if (generation_out) *generation_out = generation + 1u;
    return RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_OK;
}

static inline int rin_legacy_driver_v1_irq_control_submit(
    volatile RinLegacyDriverV1IrqControlPage* page,
    uint8_t irq, uint8_t operation)
{
    return rin_legacy_driver_v1_irq_control_enqueue(
        page, irq, operation, 0u, 0u, 0u, (uint32_t*)0);
}

/* `drv_request_irq()` is process-context only after `drv_init`: the caller
 * waits for the timer-owned registry transaction while its generation blocks
 * later commands from overwriting the matching result slot. */
static inline int rin_legacy_driver_v1_irq_control_request(
    volatile RinLegacyDriverV1IrqControlPage* page,
    uint8_t irq, uint32_t handler, uint32_t data)
{
    uint32_t generation;
    int result = rin_legacy_driver_v1_irq_control_enqueue(
        page, irq, RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_REQUEST,
        handler, data, 1u, &generation);
    if (result != RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_OK) return result;
    for (;;) {
        uint32_t completed = __atomic_load_n(
            &page->completed_generation, __ATOMIC_ACQUIRE);
        uint32_t phase = __atomic_load_n(&page->phase, __ATOMIC_ACQUIRE);
        volatile RinLegacyDriverV1IrqControlResponse* response;
        if (completed < generation) {
            if (phase == RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_POISONED) {
                return RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CORRUPT;
            }
            rin_legacy_driver_v1_irq_control_relax();
            continue;
        }
        while (__atomic_exchange_n(&page->lock, 1u,
                                   __ATOMIC_ACQUIRE) != 0u) {
            rin_legacy_driver_v1_irq_control_relax();
        }
        response = &page->responses[
            generation % RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CAPACITY];
        if (page->magic != RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_MAGIC ||
            page->version != RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_VERSION ||
            page->struct_size != sizeof(*page) || page->reserved != 0u ||
            page->waiter_generation != generation ||
            response->generation != generation) {
            __atomic_store_n(
                &page->phase,
                RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_PHASE_POISONED,
                __ATOMIC_RELEASE);
            __atomic_store_n(&page->lock, 0u, __ATOMIC_RELEASE);
            return RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_CORRUPT;
        }
        result = response->result;
        __atomic_store_n(&page->waiter_generation, 0u,
                         __ATOMIC_RELEASE);
        __atomic_store_n(&page->lock, 0u, __ATOMIC_RELEASE);
        return result;
    }
}

#if defined(__cplusplus)
static_assert(sizeof(RinLegacyDriverV1IrqControlCommand) == 16u,
              "legacy v1 IRQ control command ABI drift");
static_assert(sizeof(RinLegacyDriverV1IrqControlResponse) == 8u,
              "legacy v1 IRQ control response ABI drift");
static_assert(sizeof(RinLegacyDriverV1IrqControlPage) == 236u,
              "legacy v1 IRQ control page ABI drift");
static_assert(__builtin_offsetof(RinLegacyDriverV1IrqControlPage, lock) == 8u &&
                  __builtin_offsetof(
                      RinLegacyDriverV1IrqControlPage, phase) == 12u &&
                  __builtin_offsetof(
                      RinLegacyDriverV1IrqControlPage, count) == 16u &&
                  __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                     request_generation) == 24u &&
                  __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                     completed_generation) == 28u &&
                  __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                     last_result) == 32u &&
                  __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                     commands) == 40u &&
                  __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                     responses) == 168u &&
                  __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                     waiter_generation) == 232u,
              "legacy v1 IRQ control page field offset drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinLegacyDriverV1IrqControlCommand) == 16u,
               "legacy v1 IRQ control command ABI drift");
_Static_assert(sizeof(RinLegacyDriverV1IrqControlResponse) == 8u,
               "legacy v1 IRQ control response ABI drift");
_Static_assert(sizeof(RinLegacyDriverV1IrqControlPage) == 236u,
               "legacy v1 IRQ control page ABI drift");
_Static_assert(__builtin_offsetof(RinLegacyDriverV1IrqControlPage, lock) == 8u &&
                   __builtin_offsetof(
                       RinLegacyDriverV1IrqControlPage, phase) == 12u &&
                   __builtin_offsetof(
                       RinLegacyDriverV1IrqControlPage, count) == 16u &&
                   __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                      request_generation) == 24u &&
                   __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                      completed_generation) == 28u &&
                   __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                      last_result) == 32u &&
                   __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                      commands) == 40u &&
                   __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                      responses) == 168u &&
                   __builtin_offsetof(RinLegacyDriverV1IrqControlPage,
                                      waiter_generation) == 232u,
               "legacy v1 IRQ control page field offset drift");
#endif

#endif /* RIN_API_RIN_LEGACY_DRIVER_V1_IRQ_CONTROL_ABI_H */
