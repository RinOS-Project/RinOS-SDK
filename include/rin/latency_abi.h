/* SPDX-License-Identifier: MIT */
/* Fixed end-to-end latency trace and bounded histogram ABI. */
#ifndef RIN_SDK_LATENCY_ABI_H
#define RIN_SDK_LATENCY_ABI_H

#include <stdint.h>

#define RIN_LATENCY_TRACE_VERSION UINT16_C(1)
#define RIN_LATENCY_HISTOGRAM_VERSION UINT16_C(1)
#define RIN_LATENCY_HISTOGRAM_BUCKETS UINT32_C(16)

enum RinLatencyTraceStageV1 {
    RIN_LATENCY_STAGE_DEVICE = 0,
    RIN_LATENCY_STAGE_COMPOSITOR_POLL = 1,
    RIN_LATENCY_STAGE_DISPATCH = 2,
    RIN_LATENCY_STAGE_ROUTE = 3,
    RIN_LATENCY_STAGE_CLIENT_DEQUEUE = 4,
    RIN_LATENCY_STAGE_CLIENT_CALLBACK = 5,
    RIN_LATENCY_STAGE_DAMAGE_COMMIT = 6,
    RIN_LATENCY_STAGE_PRESENT = 7
};

/* Timestamps are monotonic nanoseconds.  Zero means that the stage was not
 * observed (for example an event still in the input ring).  This extension is
 * carried beside the legacy RinGuiNativeEventV1 and never changes that ABI. */
typedef struct RinLatencyTraceV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t sequence;
    uint32_t event_type;
    uint32_t coalesced_count;
    uint64_t t0_device_ns;
    uint64_t t1_compositor_poll_ns;
    uint64_t t2_dispatch_ns;
    uint64_t t3_route_ns;
    uint64_t t4_client_dequeue_ns;
    uint64_t t5_client_callback_ns;
    uint64_t t6_damage_commit_ns;
    uint64_t t7_present_ns;
    uint32_t source;
    uint32_t reserved;
} RinLatencyTraceV1;

/* A fixed histogram is safe for release builds: no allocation, no locks, and
 * no formatting or logging on the input hot path.  Bucket n covers
 * [2^n, 2^(n+1)) nanoseconds; bucket 15 is the saturated tail. */
typedef struct RinLatencyHistogramV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t samples;
    uint64_t sum_ns;
    uint64_t max_ns;
    uint64_t buckets[RIN_LATENCY_HISTOGRAM_BUCKETS];
} RinLatencyHistogramV1;

static inline void rin_latency_trace_init(RinLatencyTraceV1* trace,
                                          uint64_t sequence,
                                          uint32_t event_type,
                                          uint32_t source,
                                          uint64_t device_ns) {
    if (!trace) return;
    trace->struct_size = (uint32_t)sizeof(*trace);
    trace->version = RIN_LATENCY_TRACE_VERSION;
    trace->flags = 0u;
    trace->sequence = sequence;
    trace->event_type = event_type;
    trace->coalesced_count = 1u;
    trace->t0_device_ns = device_ns;
    trace->t1_compositor_poll_ns = 0u;
    trace->t2_dispatch_ns = 0u;
    trace->t3_route_ns = 0u;
    trace->t4_client_dequeue_ns = 0u;
    trace->t5_client_callback_ns = 0u;
    trace->t6_damage_commit_ns = 0u;
    trace->t7_present_ns = 0u;
    trace->source = source;
    trace->reserved = 0u;
}

static inline void rin_latency_histogram_init(RinLatencyHistogramV1* histogram) {
    uint32_t index;
    if (!histogram) return;
    histogram->struct_size = (uint32_t)sizeof(*histogram);
    histogram->version = RIN_LATENCY_HISTOGRAM_VERSION;
    histogram->flags = 0u;
    histogram->samples = 0u;
    histogram->sum_ns = 0u;
    histogram->max_ns = 0u;
    for (index = 0u; index < RIN_LATENCY_HISTOGRAM_BUCKETS; ++index)
        histogram->buckets[index] = 0u;
}

static inline uint32_t rin_latency_bucket(uint64_t duration_ns) {
    uint32_t bucket = 0u;
    while (duration_ns > 1u && bucket + 1u < RIN_LATENCY_HISTOGRAM_BUCKETS) {
        duration_ns >>= 1u;
        ++bucket;
    }
    return bucket;
}

static inline void rin_latency_histogram_record(
    RinLatencyHistogramV1* histogram, uint64_t duration_ns) {
    uint32_t bucket;
    if (!histogram) return;
    bucket = rin_latency_bucket(duration_ns);
    ++histogram->samples;
    histogram->sum_ns += duration_ns;
    if (duration_ns > histogram->max_ns) histogram->max_ns = duration_ns;
    ++histogram->buckets[bucket];
}

/* Return the upper bucket boundary for a quantile.  The histogram is
 * intentionally bounded; callers can report the bucket as an honest range
 * instead of pretending to have unbounded precision. */
static inline uint64_t rin_latency_histogram_percentile(
    const RinLatencyHistogramV1* histogram, uint32_t numerator,
    uint32_t denominator) {
    uint64_t target;
    uint64_t cumulative = 0u;
    uint32_t index;
    if (!histogram || histogram->samples == 0u || denominator == 0u ||
        numerator > denominator)
        return 0u;
    target = (histogram->samples - 1u) / denominator * numerator;
    target += ((histogram->samples - 1u) % denominator * numerator) /
              denominator;
    for (index = 0u; index < RIN_LATENCY_HISTOGRAM_BUCKETS; ++index) {
        cumulative += histogram->buckets[index];
        if (cumulative > target)
            return index == 0u ? 1u : UINT64_C(1) << index;
    }
    return UINT64_C(1) << (RIN_LATENCY_HISTOGRAM_BUCKETS - 1u);
}

#ifdef __cplusplus
static_assert(sizeof(RinLatencyTraceV1) == 96u, "latency trace ABI drift");
static_assert(sizeof(RinLatencyHistogramV1) == 160u,
              "latency histogram ABI drift");
#else
_Static_assert(sizeof(RinLatencyTraceV1) == 96u, "latency trace ABI drift");
_Static_assert(sizeof(RinLatencyHistogramV1) == 160u,
               "latency histogram ABI drift");
#endif

#endif /* RIN_SDK_LATENCY_ABI_H */

