/* SPDX-License-Identifier: MIT */
/* Producer-owned compositor input ring ABI. */
#ifndef RIN_SDK_COMPOSITOR_INPUT_RING_H
#define RIN_SDK_COMPOSITOR_INPUT_RING_H

#include <stdint.h>
#include "gui_native_event.h"
#include "latency_abi.h"

#define RIN_COMPOSITOR_INPUT_RING_VERSION UINT16_C(1)
#define RIN_COMPOSITOR_INPUT_RING_DEFAULT_CAPACITY UINT32_C(64)
#define RIN_COMPOSITOR_INPUT_RING_MAX_CAPACITY UINT32_C(256)
#define RIN_COMPOSITOR_INPUT_RING_FLAG_SERVER_PRODUCER UINT16_C(0x0001)
#define RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_POINTER_MOTION UINT16_C(0x0001)
#define RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_CRITICAL UINT16_C(0x0002)

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
    uint64_t reserved[2];
} RinCompositorInputRingHeaderV1;

#pragma pack(pop)
#pragma pack(push, 4)
typedef struct RinCompositorInputRingEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    /* v1 reserved word promoted to a logical seat id without changing the
     * fixed event size.  Values 0..3 are currently defined. */
    uint32_t seat_id;
    uint64_t sequence;
    RinGuiNativeEventV1 event;
    RinLatencyTraceV1 trace;
    uint32_t reserved[2];
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
    if (__atomic_load_n(&header->consumer_sequence, __ATOMIC_ACQUIRE) >
        __atomic_load_n(&header->producer_sequence, __ATOMIC_ACQUIRE))
        return 0;
    return __atomic_load_n(&header->producer_sequence, __ATOMIC_ACQUIRE) -
               __atomic_load_n(&header->consumer_sequence, __ATOMIC_ACQUIRE) <=
           header->capacity;
}

static inline int rin_compositor_input_ring_event_is_motion(
    const RinCompositorInputRingEventV1* item) {
    return item && item->event.type == RIN_GUI_NATIVE_EVENT_POINTER_MOVE;
}

static inline int rin_compositor_input_ring_event_valid(
    const RinCompositorInputRingEventV1* item) {
    if (!item || item->struct_size != sizeof(*item) ||
        item->version != RIN_COMPOSITOR_INPUT_RING_VERSION ||
        (item->flags & ~(RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_POINTER_MOTION |
                         RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_CRITICAL)) != 0u ||
        item->surface_id == 0u || item->seat_id >= 4u ||
        item->event.struct_size != sizeof(item->event) ||
        item->event.version != RIN_GUI_NATIVE_EVENT_VERSION ||
        item->trace.struct_size != sizeof(item->trace) ||
        item->trace.version != RIN_LATENCY_TRACE_VERSION ||
        item->trace.sequence == 0u || item->reserved[0] != 0u ||
        item->reserved[1] != 0u)
        return 0;
    if (item->event.type == RIN_GUI_NATIVE_EVENT_POINTER_MOVE)
        return (item->flags & RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_POINTER_MOTION) != 0u;
    return (item->flags & RIN_COMPOSITOR_INPUT_RING_EVENT_FLAG_CRITICAL) != 0u;
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
    producer = __atomic_load_n(&header->producer_sequence, __ATOMIC_RELAXED);
    consumer = __atomic_load_n(&header->consumer_sequence, __ATOMIC_ACQUIRE);
    if (producer - consumer >= header->capacity) {
        if (rin_compositor_input_ring_event_is_motion(event) && producer != consumer) {
            slot = rin_compositor_input_ring_slot(header, producer - 1u);
            if (rin_compositor_input_ring_event_is_motion(slot) &&
                slot->surface_id == event->surface_id) {
                RinCompositorInputRingEventV1 replacement = *event;
                replacement.trace.coalesced_count =
                    slot->trace.coalesced_count + event->trace.coalesced_count;
                *slot = replacement;
                __atomic_add_fetch(&header->dropped_pointer_motion, 1u,
                                   __ATOMIC_RELAXED);
                return 1;
            }
        }
        __atomic_add_fetch(&header->dropped_critical, 1u, __ATOMIC_RELAXED);
        return -2;
    }
    slot = rin_compositor_input_ring_slot(header, producer);
    *slot = *event;
    __atomic_store_n(&header->producer_sequence, producer + 1u,
                     __ATOMIC_RELEASE);
    if (producer == consumer)
        __atomic_add_fetch(&header->wake_sequence, 1u, __ATOMIC_RELAXED);
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
    producer = __atomic_load_n(&header->producer_sequence, __ATOMIC_ACQUIRE);
    consumer = __atomic_load_n(&header->consumer_sequence, __ATOMIC_RELAXED);
    if (consumer == producer) return 0;
    if (producer - consumer > header->capacity) return -1;
    slot = rin_compositor_input_ring_slot(header, consumer);
    *event = *slot;
    __atomic_store_n(&header->consumer_sequence, consumer + 1u,
                     __ATOMIC_RELEASE);
    return rin_compositor_input_ring_event_valid(event) ? 1 : -1;
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

#endif /* RIN_SHARED_COMPOSITOR_INPUT_RING_H */

