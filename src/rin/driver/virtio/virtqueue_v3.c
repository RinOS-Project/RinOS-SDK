/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <rin/driver/virtio/virtqueue_v3.h>

#define VIRTQ_DESCRIPTOR_FREE 0u
#define VIRTQ_DESCRIPTOR_HEAD 1u
#define VIRTQ_DESCRIPTOR_TAIL 2u

static int power_of_two(uint16_t value) {
    return value != 0u && (value & (uint16_t)(value - 1u)) == 0u;
}

static uint64_t align_up(uint64_t value, uint64_t alignment) {
    return (value + alignment - 1u) & ~(alignment - 1u);
}

/* The descriptor free list is driver-owned metadata.  Validate it before
 * every operation that follows it so a torn or device-corrupted queue cannot
 * turn into an out-of-bounds walk or a partial publication. */
static int virtqueue_shape_valid(const RinVirtqueueV3* queue) {
    uint8_t seen[RIN_VIRTQUEUE_V3_MAX_SIZE];
    uint16_t current;
    uint16_t free_entries = 0u;

    if (!queue || !power_of_two(queue->size) ||
        queue->size > RIN_VIRTQUEUE_V3_MAX_SIZE || !queue->descriptors ||
        !queue->available_flags || !queue->available_index ||
        !queue->available_ring || !queue->used_flags ||
        !queue->used_index || !queue->used_ring ||
        queue->free_count > queue->size ||
        (uint16_t)(__atomic_load_n(queue->used_index, __ATOMIC_ACQUIRE) -
                   queue->last_used) > queue->size) {
        return 0;
    }
    for (uint16_t index = 0u; index < RIN_VIRTQUEUE_V3_MAX_SIZE; ++index)
        seen[index] = 0u;
    for (uint16_t index = 0u; index < queue->size; ++index) {
        if (queue->descriptor_state[index] > VIRTQ_DESCRIPTOR_TAIL)
            return 0;
        if (queue->descriptor_state[index] == VIRTQ_DESCRIPTOR_FREE)
            ++free_entries;
    }
    if (free_entries != queue->free_count ||
        (queue->free_count == 0u
             ? queue->free_head != UINT16_MAX
             : queue->free_head >= queue->size)) {
        return 0;
    }
    current = queue->free_head;
    for (uint16_t index = 0u; index < queue->free_count; ++index) {
        if (current >= queue->size || seen[current] != 0u ||
            queue->descriptor_state[current] != VIRTQ_DESCRIPTOR_FREE)
            return 0;
        seen[current] = 1u;
        current = queue->descriptors[current].next;
    }
    return current == UINT16_MAX;
}

uint64_t rin_virtqueue_v3_memory_size(uint16_t queue_size) {
    if (!power_of_two(queue_size) ||
        queue_size > RIN_VIRTQUEUE_V3_MAX_SIZE) return 0u;
    uint64_t descriptors = (uint64_t)queue_size * sizeof(RinVirtqDescriptorV3);
    uint64_t available = 6u + (uint64_t)queue_size * sizeof(uint16_t);
    uint64_t used_offset = align_up(descriptors + available, 4u);
    return used_offset + 6u + (uint64_t)queue_size * sizeof(RinVirtqUsedElementV3);
}

int rin_virtqueue_v3_init(RinVirtqueueV3* queue, void* memory,
                          uint64_t memory_length, uint16_t queue_size) {
    uint64_t needed = rin_virtqueue_v3_memory_size(queue_size);
    if (!queue || !memory ||
        ((uintptr_t)memory & (sizeof(uint64_t) - 1u)) != 0u ||
        needed == 0u || memory_length < needed) return -1;

    uint8_t* bytes = (uint8_t*)memory;
    for (uint64_t i = 0; i < needed; i++) bytes[i] = 0u;
    uint64_t descriptor_bytes = (uint64_t)queue_size * sizeof(RinVirtqDescriptorV3);
    uint64_t available_bytes = 6u + (uint64_t)queue_size * sizeof(uint16_t);
    uint64_t used_offset = align_up(descriptor_bytes + available_bytes, 4u);
    queue->descriptors = (RinVirtqDescriptorV3*)bytes;
    queue->available_flags = (volatile uint16_t*)(bytes + descriptor_bytes);
    queue->available_index = queue->available_flags + 1;
    queue->available_ring = queue->available_flags + 2;
    queue->used_flags = (volatile uint16_t*)(bytes + used_offset);
    queue->used_index = queue->used_flags + 1;
    queue->used_ring = (volatile RinVirtqUsedElementV3*)(queue->used_flags + 2);
    queue->size = queue_size;
    queue->free_head = 0u;
    queue->free_count = queue_size;
    queue->last_used = 0u;
    for (uint16_t i = 0; i < RIN_VIRTQUEUE_V3_MAX_SIZE; i++) {
        queue->descriptor_state[i] = VIRTQ_DESCRIPTOR_FREE;
    }
    for (uint16_t i = 0; i < queue_size; i++) {
        queue->descriptors[i].next = (uint16_t)(i + 1u);
    }
    queue->descriptors[queue_size - 1u].next = UINT16_MAX;
    return 0;
}

int rin_virtqueue_v3_submit(RinVirtqueueV3* queue, uint64_t address,
                            uint32_t length, uint16_t flags,
                            uint16_t* descriptor_id) {
    RinVirtqueueBufferV3 buffer = {address, length, flags, 0u};
    return rin_virtqueue_v3_submit_chain(queue, &buffer, 1u,
                                         descriptor_id);
}

int rin_virtqueue_v3_submit_chain(
    RinVirtqueueV3* queue, const RinVirtqueueBufferV3* buffers,
    uint16_t buffer_count, uint16_t* descriptor_id) {
    uint16_t scan;
    uint16_t previous = UINT16_MAX;
    uint16_t head = UINT16_MAX;
    if (!virtqueue_shape_valid(queue) || !buffers || !descriptor_id ||
        buffer_count == 0u ||
        buffer_count > queue->size || buffer_count > queue->free_count) {
        return -1;
    }
    for (uint16_t index = 0u; index < buffer_count; ++index) {
        if (buffers[index].address == 0u || buffers[index].length == 0u ||
            (buffers[index].flags & ~RIN_VIRTQ_DESC_F_WRITE) != 0u ||
            buffers[index].reserved != 0u) return -1;
    }
    scan = queue->free_head;
    for (uint16_t index = 0u; index < buffer_count; ++index) {
        if (scan >= queue->size ||
            queue->descriptor_state[scan] != VIRTQ_DESCRIPTOR_FREE) {
            return -1;
        }
        scan = queue->descriptors[scan].next;
    }
    for (uint16_t index = 0u; index < buffer_count; ++index) {
        uint16_t id = queue->free_head;
        queue->free_head = queue->descriptors[id].next;
        queue->free_count--;
        queue->descriptor_state[id] = index == 0u
            ? VIRTQ_DESCRIPTOR_HEAD : VIRTQ_DESCRIPTOR_TAIL;
        queue->descriptors[id].address = buffers[index].address;
        queue->descriptors[id].length = buffers[index].length;
        queue->descriptors[id].flags = buffers[index].flags;
        queue->descriptors[id].next = UINT16_MAX;
        if (previous != UINT16_MAX) {
            queue->descriptors[previous].flags |= RIN_VIRTQ_DESC_F_NEXT;
            queue->descriptors[previous].next = id;
        } else {
            head = id;
        }
        previous = id;
    }
    uint16_t available = __atomic_load_n(queue->available_index,
                                         __ATOMIC_RELAXED);
    queue->available_ring[available & (uint16_t)(queue->size - 1u)] = head;
    __atomic_store_n(queue->available_index, (uint16_t)(available + 1u),
                     __ATOMIC_RELEASE);
    *descriptor_id = head;
    return 0;
}

int rin_virtqueue_v3_pop_used(RinVirtqueueV3* queue, uint16_t* descriptor_id,
                              uint32_t* length) {
    if (!virtqueue_shape_valid(queue) || !descriptor_id || !length) return -1;
    uint16_t used = __atomic_load_n(queue->used_index, __ATOMIC_ACQUIRE);
    uint16_t pending = (uint16_t)(used - queue->last_used);
    if (pending == 0u) return 0;
    if (pending > queue->size) return -1;
    RinVirtqUsedElementV3 element =
        queue->used_ring[queue->last_used & (uint16_t)(queue->size - 1u)];
    if (element.id >= queue->size ||
        queue->descriptor_state[element.id] != VIRTQ_DESCRIPTOR_HEAD) {
        return -1;
    }
    uint16_t id = (uint16_t)element.id;
    uint16_t current = id;
    uint16_t chain_count = 0u;
    uint64_t chain_capacity = 0u;
    for (;;) {
        uint16_t flags;
        if (current >= queue->size ||
            queue->descriptor_state[current] !=
                (chain_count == 0u ? VIRTQ_DESCRIPTOR_HEAD
                                   : VIRTQ_DESCRIPTOR_TAIL) ||
            queue->descriptors[current].address == 0u ||
            queue->descriptors[current].length == 0u ||
            ++chain_count > queue->size) {
            return -1;
        }
        flags = queue->descriptors[current].flags;
        chain_capacity += queue->descriptors[current].length;
        if ((flags & RIN_VIRTQ_DESC_F_NEXT) == 0u) break;
        current = queue->descriptors[current].next;
    }
    if ((uint32_t)queue->free_count + chain_count > queue->size ||
        (uint64_t)element.length > chain_capacity) return -1;
    current = id;
    for (uint16_t index = 0u; index < chain_count; ++index) {
        uint16_t next = queue->descriptors[current].next;
        queue->descriptors[current].address = 0u;
        queue->descriptors[current].length = 0u;
        queue->descriptors[current].flags = 0u;
        queue->descriptors[current].next = queue->free_head;
        queue->descriptor_state[current] = VIRTQ_DESCRIPTOR_FREE;
        queue->free_head = current;
        queue->free_count++;
        current = next;
    }
    queue->last_used++;
    *descriptor_id = id;
    *length = element.length;
    return 1;
}

int rin_virtqueue_v3_pop_used_batch(
    RinVirtqueueV3* queue, RinVirtqueueCompletionV3* completions,
    uint32_t capacity, uint32_t* completion_count) {
    uint32_t count = 0u;

    if (!completion_count) return -1;
    *completion_count = 0u;
    if (!completions || capacity == 0u ||
        capacity > RIN_VIRTQUEUE_V3_MAX_BATCH) return -1;
    while (count < capacity) {
        uint16_t descriptor_id = UINT16_MAX;
        uint32_t length = 0u;
        int popped = rin_virtqueue_v3_pop_used(
            queue, &descriptor_id, &length);
        if (popped < 0) {
            *completion_count = count;
            return -1;
        }
        if (popped == 0) break;
        completions[count].descriptor_id = descriptor_id;
        completions[count].reserved = 0u;
        completions[count].length = length;
        ++count;
    }
    *completion_count = count;
    return 0;
}
