/* SPDX-License-Identifier: MIT */
/* Producer-owned compositor input ring ABI. Version 2 tags events with the
 * active focus generation while preserving the header and event sizes. */
#ifndef RIN_SDK_COMPOSITOR_INPUT_RING_H
#define RIN_SDK_COMPOSITOR_INPUT_RING_H

#include <stddef.h>
#include <stdint.h>
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#include "native_event.h"
#include "../latency_abi.h"

#define RIN_COMPOSITOR_INPUT_RING_VERSION UINT16_C(2)
#define RIN_COMPOSITOR_INPUT_RING_DEFAULT_CAPACITY UINT32_C(64)
#define RIN_COMPOSITOR_INPUT_RING_MAX_CAPACITY UINT32_C(256)
#define RIN_COMPOSITOR_INPUT_RING_FLAG_SERVER_PRODUCER UINT16_C(0x0001)
#define RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_POINTER_MOTION UINT16_C(0x0001)
#define RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_CRITICAL UINT16_C(0x0002)
#define RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_FOCUS_GENERATION UINT16_C(0x0004)

#pragma pack(push, 8)
typedef struct RinCompositorInputRingHeaderV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t capacity;
    uint32_t event_size;
    uint64_t generation;
    uint64_t producer_sequence;
    uint64_t consumer_sequence;
    uint64_t wake_sequence;
    uint64_t dropped_pointer_motion;
    uint64_t dropped_critical;
    uint64_t focus_generation;
    uint64_t dropped_focus_events;
} RinCompositorInputRingHeaderV1;

#pragma pack(pop)
#pragma pack(push, 4)
typedef struct RinCompositorInputRingEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    /* The former v1 reserved word is the logical seat id. Values 0..3 are
     * currently defined. */
    uint32_t seat_id;
    uint64_t sequence;
    RinGuiNativeEventV1 event;
    RinLatencyTraceV1 trace;
    /* Reuses the former final two reserved words. */
    union {
        uint64_t focus_generation;
        uint32_t reserved[2];
    };
} RinCompositorInputRingEventV1;

typedef struct RinCompositorInputRingSetupV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t capacity;
    uint32_t event_size;
    uint64_t generation;
    char shm_name[64];
    char wake_name[64];
    uint32_t reserved[2];
} RinCompositorInputRingSetupV1;
#pragma pack(pop)

static inline uint32_t rin_compositor_input_ring_bytes(uint32_t capacity) {
    if (capacity == 0u || capacity > RIN_COMPOSITOR_INPUT_RING_MAX_CAPACITY)
        return 0u;
    return (uint32_t)(sizeof(RinCompositorInputRingHeaderV1) +
                      (uint64_t)capacity * sizeof(RinCompositorInputRingEventV1));
}

static inline RinCompositorInputRingEventV1*
rin_compositor_input_ring_slot(RinCompositorInputRingHeaderV1* header,
                               uint64_t sequence) {
    uint8_t* base = (uint8_t*)header + sizeof(*header);
    return (RinCompositorInputRingEventV1*)
        (base + (sequence % header->capacity) * header->event_size);
}

static inline uint64_t rin_compositor_input_ring_load_acquire(
    const uint64_t* value) {
#if defined(_MSC_VER)
    return (uint64_t)_InterlockedCompareExchange64(
        (volatile __int64*)value, 0, 0);
#else
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
#endif
}

static inline uint64_t rin_compositor_input_ring_load_relaxed(
    const uint64_t* value) {
#if defined(_MSC_VER)
    return (uint64_t)_InterlockedCompareExchange64(
        (volatile __int64*)value, 0, 0);
#else
    return __atomic_load_n(value, __ATOMIC_RELAXED);
#endif
}

static inline void rin_compositor_input_ring_store_release(
    uint64_t* value, uint64_t desired) {
#if defined(_MSC_VER)
    (void)_InterlockedExchange64((volatile __int64*)value, (__int64)desired);
#else
    __atomic_store_n(value, desired, __ATOMIC_RELEASE);
#endif
}

static inline void rin_compositor_input_ring_add_relaxed(
    uint64_t* value, uint64_t increment) {
#if defined(_MSC_VER)
    (void)_InterlockedExchangeAdd64((volatile __int64*)value,
                                    (__int64)increment);
#else
    (void)__atomic_add_fetch(value, increment, __ATOMIC_RELAXED);
#endif
}

static inline int rin_compositor_input_ring_header_valid(
    const RinCompositorInputRingHeaderV1* header, uint32_t mapped_bytes) {
    uint64_t required;
    if (!header || header->struct_size != sizeof(*header) ||
        header->version != RIN_COMPOSITOR_INPUT_RING_VERSION ||
        (header->flags & ~RIN_COMPOSITOR_INPUT_RING_FLAG_SERVER_PRODUCER) != 0u ||
        header->capacity == 0u ||
        header->capacity > RIN_COMPOSITOR_INPUT_RING_MAX_CAPACITY ||
        header->event_size != sizeof(RinCompositorInputRingEventV1)) return 0;
    required = sizeof(*header) +
               (uint64_t)header->capacity * header->event_size;
    if (required > UINT32_MAX || mapped_bytes < (uint32_t)required)
        return 0;
    if (rin_compositor_input_ring_load_acquire(&header->consumer_sequence) >
        rin_compositor_input_ring_load_acquire(&header->producer_sequence))
        return 0;
    return rin_compositor_input_ring_load_acquire(
               &header->producer_sequence) -
               rin_compositor_input_ring_load_acquire(
                   &header->consumer_sequence) <=
           header->capacity;
}

static inline int rin_compositor_input_ring_event_is_motion(
    const RinCompositorInputRingEventV1* item) {
    return item && item->event.type == RIN_GUI_NATIVE_EVENT_POINTER_MOVE;
}

static inline int rin_compositor_input_ring_event_valid(
    const RinCompositorInputRingEventV1* item) {
    const uint16_t known_flags =
        RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_POINTER_MOTION |
        RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_CRITICAL |
        RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_FOCUS_GENERATION;
    if (!item || item->struct_size != sizeof(*item) ||
        item->version != RIN_COMPOSITOR_INPUT_RING_VERSION ||
        (item->flags & ~known_flags) != 0u ||
        item->surface_id == 0u || item->seat_id >= 4u ||
        item->event.struct_size != sizeof(item->event) ||
        item->event.version != RIN_GUI_NATIVE_EVENT_VERSION ||
        item->trace.struct_size != sizeof(item->trace) ||
        item->trace.version != RIN_LATENCY_TRACE_VERSION ||
        item->trace.sequence == 0u ||
        (((item->flags & RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_FOCUS_GENERATION) != 0u) !=
         (item->focus_generation != 0u)))
        return 0;
    if (item->event.type == RIN_GUI_NATIVE_EVENT_POINTER_MOVE)
        return (item->flags & RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_POINTER_MOTION) != 0u;
    return (item->flags & RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_CRITICAL) != 0u;
}

static inline int rin_compositor_input_ring_event_current(
    const RinCompositorInputRingHeaderV1* header,
    const RinCompositorInputRingEventV1* event) {
    if (!header || !event) return 0;
    if ((event->flags &
         RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_FOCUS_GENERATION) == 0u)
        return header->focus_generation == 0u;
    return event->focus_generation ==
           rin_compositor_input_ring_load_acquire(&header->focus_generation);
}

/* Returns 0 for a new record, 1 for a coalesced record, and a negative value
 * when a critical record cannot be admitted. */
static inline int rin_compositor_input_ring_publish(
    RinCompositorInputRingHeaderV1* header,
    const RinCompositorInputRingEventV1* event, uint32_t mapped_bytes) {
    uint64_t producer;
    uint64_t consumer;
    RinCompositorInputRingEventV1* slot;
    if (!event || !rin_compositor_input_ring_event_valid(event) ||
        !rin_compositor_input_ring_header_valid(header, mapped_bytes))
        return -1;
    if (!rin_compositor_input_ring_event_current(header, event)) {
        rin_compositor_input_ring_add_relaxed(&header->dropped_focus_events,
                                              1u);
        return 1;
    }
    producer =
        rin_compositor_input_ring_load_relaxed(&header->producer_sequence);
    consumer =
        rin_compositor_input_ring_load_acquire(&header->consumer_sequence);
    if (producer - consumer >= header->capacity) {
        if (rin_compositor_input_ring_event_is_motion(event) && producer != consumer) {
            slot = rin_compositor_input_ring_slot(header, producer - 1u);
            if (rin_compositor_input_ring_event_is_motion(slot) &&
                slot->surface_id == event->surface_id &&
                slot->focus_generation == event->focus_generation) {
                RinCompositorInputRingEventV1 replacement = *event;
                replacement.trace.coalesced_count =
                    slot->trace.coalesced_count + event->trace.coalesced_count;
                *slot = replacement;
                rin_compositor_input_ring_add_relaxed(
                    &header->dropped_pointer_motion, 1u);
                return 1;
            }
        }
        rin_compositor_input_ring_add_relaxed(&header->dropped_critical, 1u);
        return -2;
    }
    slot = rin_compositor_input_ring_slot(header, producer);
    *slot = *event;
    rin_compositor_input_ring_store_release(&header->producer_sequence,
                                            producer + 1u);
    if (producer == consumer)
        rin_compositor_input_ring_add_relaxed(&header->wake_sequence, 1u);
    return 0;
}

static inline int rin_compositor_input_ring_pop(
    RinCompositorInputRingHeaderV1* header,
    RinCompositorInputRingEventV1* event, uint32_t mapped_bytes) {
    uint64_t producer;
    uint64_t consumer;
    RinCompositorInputRingEventV1* slot;
    if (!event || !rin_compositor_input_ring_header_valid(header, mapped_bytes))
        return -1;
    producer =
        rin_compositor_input_ring_load_acquire(&header->producer_sequence);
    consumer =
        rin_compositor_input_ring_load_relaxed(&header->consumer_sequence);
    if (producer - consumer > header->capacity) return -1;
    while (consumer < producer) {
        slot = rin_compositor_input_ring_slot(header, consumer);
        *event = *slot;
        ++consumer;
        rin_compositor_input_ring_store_release(&header->consumer_sequence,
                                                consumer);
        if (!rin_compositor_input_ring_event_valid(event)) return -1;
        if (rin_compositor_input_ring_event_current(header, event)) return 1;
        rin_compositor_input_ring_add_relaxed(&header->dropped_focus_events,
                                              1u);
    }
    return 0;
}

/* Dequeue at most `capacity` records with one producer/consumer index
 * publication.  A corrupt record is consumed and reported as an error; the
 * caller must not reinterpret the remaining records as valid. */
static inline int rin_compositor_input_ring_pop_batch(
    RinCompositorInputRingHeaderV1* header,
    RinCompositorInputRingEventV1* events, uint32_t capacity,
    uint32_t mapped_bytes, uint32_t* count_out) {
    uint64_t producer;
    uint64_t consumer;
    uint64_t available;
    uint64_t consumed = 0u;
    uint32_t count = 0u;
    if (count_out != NULL) *count_out = 0u;
    if (!header || !events || capacity == 0u || count_out == NULL ||
        !rin_compositor_input_ring_header_valid(header, mapped_bytes))
        return -1;
    producer =
        rin_compositor_input_ring_load_acquire(&header->producer_sequence);
    consumer =
        rin_compositor_input_ring_load_relaxed(&header->consumer_sequence);
    if (producer - consumer > header->capacity) return -1;
    available = producer - consumer;
    while (consumed < available && count < capacity) {
        RinCompositorInputRingEventV1 item =
            *rin_compositor_input_ring_slot(header, consumer + consumed);
        ++consumed;
        if (!rin_compositor_input_ring_event_valid(&item)) {
            rin_compositor_input_ring_store_release(
                &header->consumer_sequence, consumer + consumed);
            *count_out = count;
            return -1;
        }
        if (!rin_compositor_input_ring_event_current(header, &item)) {
            rin_compositor_input_ring_add_relaxed(
                &header->dropped_focus_events, 1u);
            continue;
        }
        events[count++] = item;
    }
    rin_compositor_input_ring_store_release(&header->consumer_sequence,
                                            consumer + consumed);
    *count_out = count;
    return count == 0u ? 0 : 1;
}

#ifdef __cplusplus
static_assert(sizeof(RinCompositorInputRingHeaderV1) == 80u,
              "input ring header ABI drift");
static_assert(sizeof(RinCompositorInputRingEventV1) == 164u,
              "input ring event ABI drift");
static_assert(sizeof(RinCompositorInputRingSetupV1) == 160u,
              "input ring setup ABI drift");
#else
_Static_assert(sizeof(RinCompositorInputRingHeaderV1) == 80u,
               "input ring header ABI drift");
_Static_assert(sizeof(RinCompositorInputRingEventV1) == 164u,
               "input ring event ABI drift");
_Static_assert(sizeof(RinCompositorInputRingSetupV1) == 160u,
               "input ring setup ABI drift");
#endif

#endif /* RIN_SDK_COMPOSITOR_INPUT_RING_H */
