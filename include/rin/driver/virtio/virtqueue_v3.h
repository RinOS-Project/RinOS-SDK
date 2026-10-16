/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVERS_VIRTQUEUE_V3_H
#define RIN_DRIVERS_VIRTQUEUE_V3_H

#include <stdint.h>

#define RIN_VIRTQ_DESC_F_NEXT  UINT16_C(1)
#define RIN_VIRTQ_DESC_F_WRITE UINT16_C(2)
#define RIN_VIRTQUEUE_V3_MAX_SIZE 256u
#define RIN_VIRTQUEUE_V3_MAX_BATCH 32u

typedef struct RinVirtqDescriptorV3 {
    uint64_t address;
    uint32_t length;
    uint16_t flags;
    uint16_t next;
} RinVirtqDescriptorV3;

typedef struct RinVirtqUsedElementV3 {
    uint32_t id;
    uint32_t length;
} RinVirtqUsedElementV3;

typedef struct RinVirtqueueBufferV3 {
    uint64_t address;
    uint32_t length;
    uint16_t flags;
    uint16_t reserved;
} RinVirtqueueBufferV3;

typedef struct RinVirtqueueCompletionV3 {
    uint16_t descriptor_id;
    uint16_t reserved;
    uint32_t length;
} RinVirtqueueCompletionV3;

typedef struct RinVirtqueueV3 {
    RinVirtqDescriptorV3* descriptors;
    volatile uint16_t* available_flags;
    volatile uint16_t* available_index;
    volatile uint16_t* available_ring;
    volatile uint16_t* used_flags;
    volatile uint16_t* used_index;
    volatile RinVirtqUsedElementV3* used_ring;
    uint16_t size;
    uint16_t free_head;
    uint16_t free_count;
    uint16_t last_used;
    uint8_t descriptor_state[RIN_VIRTQUEUE_V3_MAX_SIZE];
} RinVirtqueueV3;

int rin_virtqueue_v3_init(RinVirtqueueV3* queue, void* memory,
                          uint64_t memory_length, uint16_t queue_size);
int rin_virtqueue_v3_submit(RinVirtqueueV3* queue, uint64_t address,
                            uint32_t length, uint16_t flags,
                            uint16_t* descriptor_id);
int rin_virtqueue_v3_submit_chain(
    RinVirtqueueV3* queue, const RinVirtqueueBufferV3* buffers,
    uint16_t buffer_count, uint16_t* descriptor_id);
int rin_virtqueue_v3_pop_used(RinVirtqueueV3* queue, uint16_t* descriptor_id,
                              uint32_t* length);
/* Pop at most capacity completions.  On a malformed later completion, the
 * already popped count is returned through completion_count and -1 is
 * returned so the owner can quarantine the queue without treating the batch
 * as successful. */
int rin_virtqueue_v3_pop_used_batch(
    RinVirtqueueV3* queue, RinVirtqueueCompletionV3* completions,
    uint32_t capacity, uint32_t* completion_count);
uint64_t rin_virtqueue_v3_memory_size(uint16_t queue_size);

#endif
