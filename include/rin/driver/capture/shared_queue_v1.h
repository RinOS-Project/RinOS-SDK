/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_CAPTURE_SHARED_QUEUE_V1_H
#define RIN_DRIVER_CAPTURE_SHARED_QUEUE_V1_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_CAPTURE_SHARED_QUEUE_VERSION UINT32_C(1)
#define RIN_CAPTURE_SHARED_QUEUE_MAX_SLOTS UINT32_C(16)

typedef enum RinCaptureSharedQueueResult {
    RIN_CAPTURE_SHARED_QUEUE_OK = 0,
    RIN_CAPTURE_SHARED_QUEUE_INVALID_ARGUMENT = -1,
    RIN_CAPTURE_SHARED_QUEUE_NOT_INITIALIZED = -2,
    RIN_CAPTURE_SHARED_QUEUE_FULL = -3,
    RIN_CAPTURE_SHARED_QUEUE_EMPTY = -4,
    RIN_CAPTURE_SHARED_QUEUE_STALE = -5,
    RIN_CAPTURE_SHARED_QUEUE_BUSY = -6,
    RIN_CAPTURE_SHARED_QUEUE_BOUNDS = -7
} RinCaptureSharedQueueResult;

typedef enum RinCaptureSharedQueueSlotState {
    RIN_CAPTURE_SHARED_QUEUE_FREE = 0,
    RIN_CAPTURE_SHARED_QUEUE_PRODUCER = 1,
    RIN_CAPTURE_SHARED_QUEUE_READY = 2,
    RIN_CAPTURE_SHARED_QUEUE_CONSUMER = 3
} RinCaptureSharedQueueSlotState;

typedef struct RinCaptureSharedBufferV1 {
    uint8_t* data;
    uint32_t capacity;
    uint32_t size;
    uint64_t sequence;
    uint64_t monotonic_time_ns;
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t flags;
    uint32_t state;
} RinCaptureSharedBufferV1;

typedef struct RinCaptureSharedFrameV1 {
    uint8_t* data;
    uint32_t size;
    uint64_t sequence;
    uint64_t monotonic_time_ns;
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t flags;
} RinCaptureSharedFrameV1;

/* The queue and slot storage remain caller-owned. Queue operations only
 * transfer bounded producer/consumer ownership and do not grant device access.
 */
typedef struct RinCaptureSharedQueueV1 {
    volatile uint32_t lock;
    uint32_t initialized;
    uint32_t slot_count;
    uint64_t generation;
    RinCaptureSharedBufferV1* slots;
} RinCaptureSharedQueueV1;

int rin_capture_shared_queue_validate(
    const RinCaptureSharedQueueV1* queue, uint32_t minimum_capacity);
int rin_capture_shared_queue_init(RinCaptureSharedQueueV1* queue,
                                  RinCaptureSharedBufferV1* slots,
                                  uint32_t slot_count, uint64_t generation);
int rin_capture_shared_queue_producer_acquire(
    RinCaptureSharedQueueV1* queue, uint64_t* token_out,
    uint8_t** data_out, uint32_t* capacity_out);
int rin_capture_shared_queue_producer_publish(
    RinCaptureSharedQueueV1* queue, uint64_t token, uint32_t size,
    uint64_t sequence, uint64_t monotonic_time_ns, uint32_t format,
    uint32_t width, uint32_t height, uint32_t flags);
int rin_capture_shared_queue_producer_abort(
    RinCaptureSharedQueueV1* queue, uint64_t token);
int rin_capture_shared_queue_consumer_acquire(
    RinCaptureSharedQueueV1* queue, uint64_t* token_out,
    RinCaptureSharedFrameV1* frame_out);
int rin_capture_shared_queue_consumer_release(
    RinCaptureSharedQueueV1* queue, uint64_t token);
int rin_capture_shared_queue_shutdown(RinCaptureSharedQueueV1* queue);

#if defined(__cplusplus)
static_assert(sizeof(RinCaptureSharedQueueResult) == 4u,
              "capture queue result ABI drift");
static_assert(sizeof(RinCaptureSharedBufferV1) ==
                  (sizeof(void*) == 8u ? 56u : 48u),
              "capture queue buffer ABI drift");
static_assert(sizeof(RinCaptureSharedFrameV1) ==
                  (sizeof(void*) == 8u ? 48u : 40u),
              "capture queue frame ABI drift");
static_assert(sizeof(RinCaptureSharedQueueV1) ==
                  (sizeof(void*) == 8u ? 32u : 24u),
              "capture queue ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinCaptureSharedQueueResult) == 4u,
               "capture queue result ABI drift");
_Static_assert(sizeof(RinCaptureSharedBufferV1) ==
                   (sizeof(void*) == 8u ? 56u : 48u),
               "capture queue buffer ABI drift");
_Static_assert(sizeof(RinCaptureSharedFrameV1) ==
                   (sizeof(void*) == 8u ? 48u : 40u),
               "capture queue frame ABI drift");
_Static_assert(sizeof(RinCaptureSharedQueueV1) ==
                   (sizeof(void*) == 8u ? 32u : 24u),
               "capture queue ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_CAPTURE_SHARED_QUEUE_V1_H */
