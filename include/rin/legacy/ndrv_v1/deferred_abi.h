/* SPDX-License-Identifier: MIT */
#ifndef RIN_API_RIN_LEGACY_DRIVER_DEFERRED_ABI_H
#define RIN_API_RIN_LEGACY_DRIVER_DEFERRED_ABI_H

#include <stdint.h>

/* Canonical legacy SDK contract; retained only for the legacy namespace. */

#define RIN_LEGACY_DRIVER_DEFERRED_MAGIC UINT32_C(0x31524644) /* DFR1 */
#define RIN_LEGACY_DRIVER_DEFERRED_VERSION UINT16_C(1)

#define RIN_LEGACY_DRIVER_DEFERRED_PENDING UINT32_C(0x00000001)
#define RIN_LEGACY_DRIVER_DEFERRED_RUNNING UINT32_C(0x00000002)
#define RIN_LEGACY_DRIVER_DEFERRED_CLOSING UINT32_C(0x00000004)
#define RIN_LEGACY_DRIVER_DEFERRED_POISONED UINT32_C(0x00000008)
#define RIN_LEGACY_DRIVER_DEFERRED_KNOWN_FLAGS \
    (RIN_LEGACY_DRIVER_DEFERRED_PENDING | \
     RIN_LEGACY_DRIVER_DEFERRED_RUNNING | \
     RIN_LEGACY_DRIVER_DEFERRED_CLOSING | \
     RIN_LEGACY_DRIVER_DEFERRED_POISONED)

#define RIN_LEGACY_DRIVER_DEFERRED_OK 0
#define RIN_LEGACY_DRIVER_DEFERRED_EMPTY 1
#define RIN_LEGACY_DRIVER_DEFERRED_INVALID -1
#define RIN_LEGACY_DRIVER_DEFERRED_BUSY -2
#define RIN_LEGACY_DRIVER_DEFERRED_CLOSED -3
#define RIN_LEGACY_DRIVER_DEFERRED_CORRUPT -4
#define RIN_LEGACY_DRIVER_DEFERRED_EXHAUSTED -5

#if defined(_MSC_VER)
#define RIN_LEGACY_DRIVER_DEFERRED_ALIGN8 __declspec(align(8))
#elif defined(__GNUC__) || defined(__clang__)
#define RIN_LEGACY_DRIVER_DEFERRED_ALIGN8 __attribute__((aligned(8)))
#else
#define RIN_LEGACY_DRIVER_DEFERRED_ALIGN8
#endif

typedef struct RIN_LEGACY_DRIVER_DEFERRED_ALIGN8
    RinLegacyDriverDeferredQueueV1 {
    uint32_t magic;
    uint16_t version;
    uint16_t struct_size;
    volatile uint32_t lock;
    volatile uint32_t flags;
    uint64_t request_generation;
    uint64_t callback;
    uint64_t reserved[2];
} RinLegacyDriverDeferredQueueV1;

typedef int (*RinLegacyDriverDeferredCallback)(void);

static inline int rin_legacy_driver_deferred_queue_identity_valid(
    const volatile RinLegacyDriverDeferredQueueV1* queue)
{
    return queue && queue->magic == RIN_LEGACY_DRIVER_DEFERRED_MAGIC &&
           queue->version == RIN_LEGACY_DRIVER_DEFERRED_VERSION &&
           queue->struct_size == sizeof(*queue) &&
           queue->reserved[0] == 0u && queue->reserved[1] == 0u;
}

static inline void rin_legacy_driver_deferred_queue_lock(
    volatile RinLegacyDriverDeferredQueueV1* queue)
{
    while (__atomic_exchange_n(&queue->lock, 1u, __ATOMIC_ACQUIRE) != 0u) {
    }
}

static inline void rin_legacy_driver_deferred_queue_unlock(
    volatile RinLegacyDriverDeferredQueueV1* queue)
{
    __atomic_store_n(&queue->lock, 0u, __ATOMIC_RELEASE);
}

static inline int rin_legacy_driver_deferred_queue_state_valid(
    const volatile RinLegacyDriverDeferredQueueV1* queue, uint32_t flags)
{
    uint32_t work_flags = flags &
        (RIN_LEGACY_DRIVER_DEFERRED_PENDING |
         RIN_LEGACY_DRIVER_DEFERRED_RUNNING);
    return rin_legacy_driver_deferred_queue_identity_valid(queue) &&
           (flags & ~RIN_LEGACY_DRIVER_DEFERRED_KNOWN_FLAGS) == 0u &&
           (work_flags != 0u
                ? queue->callback != 0u && queue->request_generation != 0u
                : queue->callback == 0u);
}

/* IRQ-safe producer side. The kernel consumer holds the same short lock with
 * local interrupts disabled, but invokes the callback only after unlocking. */
static inline int rin_legacy_driver_deferred_request(
    volatile RinLegacyDriverDeferredQueueV1* queue,
    RinLegacyDriverDeferredCallback callback)
{
    uint32_t flags;
    uint64_t callback_value = (uint64_t)(uintptr_t)callback;
    if (!queue || !callback ||
        !rin_legacy_driver_deferred_queue_identity_valid(queue)) {
        return RIN_LEGACY_DRIVER_DEFERRED_INVALID;
    }
    rin_legacy_driver_deferred_queue_lock(queue);
    flags = queue->flags;
    if (!rin_legacy_driver_deferred_queue_state_valid(queue, flags)) {
        queue->flags = flags | RIN_LEGACY_DRIVER_DEFERRED_POISONED;
        rin_legacy_driver_deferred_queue_unlock(queue);
        return RIN_LEGACY_DRIVER_DEFERRED_CORRUPT;
    }
    if ((flags & RIN_LEGACY_DRIVER_DEFERRED_POISONED) != 0u) {
        rin_legacy_driver_deferred_queue_unlock(queue);
        return RIN_LEGACY_DRIVER_DEFERRED_CORRUPT;
    }
    if ((flags & RIN_LEGACY_DRIVER_DEFERRED_CLOSING) != 0u) {
        rin_legacy_driver_deferred_queue_unlock(queue);
        return RIN_LEGACY_DRIVER_DEFERRED_CLOSED;
    }
    if ((flags & (RIN_LEGACY_DRIVER_DEFERRED_PENDING |
                  RIN_LEGACY_DRIVER_DEFERRED_RUNNING)) != 0u &&
        queue->callback != callback_value) {
        rin_legacy_driver_deferred_queue_unlock(queue);
        return RIN_LEGACY_DRIVER_DEFERRED_BUSY;
    }
    if ((flags & RIN_LEGACY_DRIVER_DEFERRED_PENDING) != 0u) {
        rin_legacy_driver_deferred_queue_unlock(queue);
        return RIN_LEGACY_DRIVER_DEFERRED_OK;
    }
    if (queue->request_generation == UINT64_MAX) {
        queue->flags = flags | RIN_LEGACY_DRIVER_DEFERRED_POISONED;
        rin_legacy_driver_deferred_queue_unlock(queue);
        return RIN_LEGACY_DRIVER_DEFERRED_EXHAUSTED;
    }
    queue->callback = callback_value;
    ++queue->request_generation;
    queue->flags = flags | RIN_LEGACY_DRIVER_DEFERRED_PENDING;
    rin_legacy_driver_deferred_queue_unlock(queue);
    return RIN_LEGACY_DRIVER_DEFERRED_OK;
}

static inline int rin_legacy_driver_deferred_cancel(
    volatile RinLegacyDriverDeferredQueueV1* queue,
    RinLegacyDriverDeferredCallback callback)
{
    uint32_t flags;
    uint64_t callback_value = (uint64_t)(uintptr_t)callback;
    if (!queue || !callback ||
        !rin_legacy_driver_deferred_queue_identity_valid(queue)) {
        return RIN_LEGACY_DRIVER_DEFERRED_INVALID;
    }
    rin_legacy_driver_deferred_queue_lock(queue);
    flags = queue->flags;
    if (!rin_legacy_driver_deferred_queue_state_valid(queue, flags) ||
        (flags & RIN_LEGACY_DRIVER_DEFERRED_POISONED) != 0u) {
        queue->flags = flags | RIN_LEGACY_DRIVER_DEFERRED_POISONED;
        rin_legacy_driver_deferred_queue_unlock(queue);
        return RIN_LEGACY_DRIVER_DEFERRED_CORRUPT;
    }
    if ((flags & RIN_LEGACY_DRIVER_DEFERRED_RUNNING) != 0u) {
        rin_legacy_driver_deferred_queue_unlock(queue);
        return RIN_LEGACY_DRIVER_DEFERRED_BUSY;
    }
    if ((flags & RIN_LEGACY_DRIVER_DEFERRED_PENDING) != 0u &&
        queue->callback != callback_value) {
        rin_legacy_driver_deferred_queue_unlock(queue);
        return RIN_LEGACY_DRIVER_DEFERRED_BUSY;
    }
    queue->flags = flags & ~RIN_LEGACY_DRIVER_DEFERRED_PENDING;
    queue->callback = 0u;
    rin_legacy_driver_deferred_queue_unlock(queue);
    return RIN_LEGACY_DRIVER_DEFERRED_OK;
}

#if defined(__cplusplus)
static_assert(sizeof(RinLegacyDriverDeferredQueueV1) == 48u,
              "legacy deferred queue ABI drift");
static_assert(alignof(RinLegacyDriverDeferredQueueV1) == 8u,
              "legacy deferred queue alignment drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinLegacyDriverDeferredQueueV1) == 48u,
               "legacy deferred queue ABI drift");
_Static_assert(_Alignof(RinLegacyDriverDeferredQueueV1) == 8u,
               "legacy deferred queue alignment drift");
#endif

#undef RIN_LEGACY_DRIVER_DEFERRED_ALIGN8

#endif /* RIN_API_RIN_LEGACY_DRIVER_DEFERRED_ABI_H */
