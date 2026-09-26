/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_RUNTIME_V3_H
#define RIN_DRIVER_RUNTIME_V3_H

#include <stddef.h>
#include <stdint.h>

#include <rin/driver/contract_v3.h>

/* Header-only SDK glue. It owns no hardware policy and has no compatibility
 * fallback. A missing callback or invalid lease is an error. */
#define RIN_DRIVER_RUNTIME_VERSION UINT32_C(0x00010000)

typedef enum RinDriverRuntimeStateV3 {
    RIN_DRIVER_RUNTIME_UNINITIALIZED = 0,
    RIN_DRIVER_RUNTIME_READY = 1,
    RIN_DRIVER_RUNTIME_QUIESCING = 2,
    RIN_DRIVER_RUNTIME_STOPPED = 3
} RinDriverRuntimeStateV3;

typedef RinDriverResultV3 (*RinDriverRuntimeReleaseFnV3)(
    uint64_t api_context, RinDriverHandleV3 device,
    RinDriverResourceHandleV3 lease, RinDriverGenerationV3 generation);
typedef RinDriverResultV3 (*RinDriverRuntimeBarAcquireFnV3)(
    uint64_t api_context, RinDriverHandleV3 device,
    RinDriverGenerationV3 generation, uint16_t bar_index, uint16_t flags,
    RinDriverBarLeaseV3* output);
typedef RinDriverResultV3 (*RinDriverRuntimeDmaAcquireFnV3)(
    uint64_t api_context, RinDriverHandleV3 device,
    RinDriverGenerationV3 generation, uint64_t length, uint32_t alignment,
    RinDriverDmaFlagsV3 flags, RinDriverDmaLeaseV3* output);
typedef RinDriverResultV3 (*RinDriverRuntimeIrqAcquireFnV3)(
    uint64_t api_context, RinDriverHandleV3 device,
    RinDriverGenerationV3 generation, uint32_t mode, uint32_t vector,
    RinDriverIrqFlagsV3 flags, RinDriverIrqLeaseV3* output);
typedef RinDriverResultV3 (*RinDriverRuntimeTimerArmFnV3)(
    uint64_t api_context, RinDriverHandleV3 device,
    RinDriverGenerationV3 generation, uint64_t deadline_ns,
    uint64_t period_ns, RinDriverTimerFlagsV3 flags,
    RinDriverTimerLeaseV3* output);
typedef RinDriverResultV3 (*RinDriverRuntimeRegisterFnV3)(
    uint64_t api_context, RinDriverHandleV3 device,
    RinDriverGenerationV3 generation, const RinDriverRegistrationV3* request,
    RinDriverResourceHandleV3* output);

typedef struct RinDriverRuntimeApiV3 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t api_context;
    RinDriverRuntimeReleaseFnV3 release;
    RinDriverRuntimeBarAcquireFnV3 acquire_bar;
    RinDriverRuntimeDmaAcquireFnV3 acquire_dma;
    RinDriverRuntimeIrqAcquireFnV3 acquire_irq;
    RinDriverRuntimeTimerArmFnV3 arm_timer;
    RinDriverRuntimeRegisterFnV3 register_device;
    uint64_t reserved[4];
} RinDriverRuntimeApiV3;

typedef struct RinDriverRuntimeV3 {
    const RinDriverRuntimeApiV3* api;
    RinDriverHandleV3 device;
    RinDriverGenerationV3 generation;
    RinDriverCapabilitySetV3 capabilities;
    uint32_t state;
    uint32_t live_leases;
    uint64_t reserved[2];
} RinDriverRuntimeV3;

static inline int rin_driver_runtime_api_valid(
    const RinDriverRuntimeApiV3* api) {
    return api && api->struct_size >= offsetof(RinDriverRuntimeApiV3, reserved) &&
           api->version == RIN_DRIVER_RUNTIME_VERSION && api->release;
}

static inline RinDriverResultV3 rin_driver_runtime_result(
    int32_t backend_result) {
    if (backend_result >= RIN_DRIVER_RESULT_OK &&
        backend_result <= RIN_DRIVER_RESULT_UNSUPPORTED)
        return (RinDriverResultV3)backend_result;
    return RIN_DRIVER_RESULT_BACKEND;
}

static inline RinDriverResultV3 rin_driver_runtime_start(
    RinDriverRuntimeV3* runtime, const RinDriverRuntimeApiV3* api,
    const RinDriverContextV3Public* context,
    RinDriverCapabilitySetV3 required_capabilities) {
    if (!runtime || !rin_driver_runtime_api_valid(api) || !context ||
        context->struct_size < sizeof(*context) ||
        context->version_major != RIN_DRIVER_PUBLIC_ABI_MAJOR ||
        context->version_minor > RIN_DRIVER_PUBLIC_ABI_MINOR ||
        context->driver == 0u || context->device == 0u ||
        context->generation == 0u ||
        (required_capabilities & ~RIN_DRIVER_CAPABILITY_MASK) != 0u ||
        (context->granted_capabilities & required_capabilities) !=
            required_capabilities)
        return RIN_DRIVER_RESULT_INVALID;
    runtime->api = api;
    runtime->device = context->device;
    runtime->generation = context->generation;
    runtime->capabilities = context->granted_capabilities;
    runtime->state = RIN_DRIVER_RUNTIME_READY;
    runtime->live_leases = 0u;
    runtime->reserved[0] = 0u;
    runtime->reserved[1] = 0u;
    return RIN_DRIVER_RESULT_OK;
}

static inline RinDriverResultV3 rin_driver_runtime_require(
    const RinDriverRuntimeV3* runtime, RinDriverCapabilitySetV3 capability) {
    if (!runtime || runtime->state != RIN_DRIVER_RUNTIME_READY ||
        (capability & ~RIN_DRIVER_CAPABILITY_MASK) != 0u ||
        (runtime->capabilities & capability) != capability)
        return RIN_DRIVER_RESULT_DENIED;
    return RIN_DRIVER_RESULT_OK;
}

static inline RinDriverResultV3 rin_driver_runtime_release(
    RinDriverRuntimeV3* runtime, RinDriverResourceHandleV3 lease,
    RinDriverGenerationV3 generation) {
    RinDriverResultV3 result;
    if (!runtime || !runtime->api || lease == 0u ||
        generation != runtime->generation ||
        runtime->state == RIN_DRIVER_RUNTIME_UNINITIALIZED ||
        runtime->state == RIN_DRIVER_RUNTIME_STOPPED)
        return RIN_DRIVER_RESULT_STALE;
    result = runtime->api->release(runtime->api->api_context, runtime->device,
                                    lease, generation);
    if (result == RIN_DRIVER_RESULT_OK && runtime->live_leases != 0u)
        --runtime->live_leases;
    return result;
}

static inline int rin_driver_runtime_lease_valid(
    const RinDriverRuntimeV3* runtime, RinDriverResourceHandleV3 lease,
    RinDriverGenerationV3 generation) {
    return runtime && lease != 0u && generation == runtime->generation &&
           runtime->live_leases != UINT32_MAX;
}

static inline RinDriverResultV3 rin_driver_runtime_acquire_bar(
    RinDriverRuntimeV3* runtime, uint16_t bar_index, uint16_t flags,
    RinDriverBarLeaseV3* output) {
    RinDriverResultV3 result;
    if (rin_driver_runtime_require(runtime, RIN_DRIVER_CAP_BAR) !=
            RIN_DRIVER_RESULT_OK || !runtime->api->acquire_bar || !output ||
        (flags & (uint16_t)~RIN_DRIVER_MMIO_FLAG_MASK) != 0u)
        return RIN_DRIVER_RESULT_INVALID;
    result = runtime->api->acquire_bar(runtime->api->api_context,
                                       runtime->device, runtime->generation,
                                       bar_index, flags, output);
    if (result != RIN_DRIVER_RESULT_OK) return result;
    if (output->struct_size < sizeof(*output) || output->lease == 0u ||
        output->generation != runtime->generation || output->length == 0u ||
        (output->flags & (uint16_t)~RIN_DRIVER_MMIO_FLAG_MASK) != 0u)
        return RIN_DRIVER_RESULT_BACKEND;
    ++runtime->live_leases;
    return result;
}

static inline RinDriverResultV3 rin_driver_runtime_acquire_dma(
    RinDriverRuntimeV3* runtime, uint64_t length, uint32_t alignment,
    RinDriverDmaFlagsV3 flags, RinDriverDmaLeaseV3* output) {
    RinDriverResultV3 result;
    if (rin_driver_runtime_require(runtime, RIN_DRIVER_CAP_DMA) !=
            RIN_DRIVER_RESULT_OK || !runtime->api->acquire_dma || !output ||
        length == 0u || alignment == 0u ||
        (alignment & (alignment - 1u)) != 0u ||
        (flags & ~RIN_DRIVER_DMA_FLAG_MASK) != 0u)
        return RIN_DRIVER_RESULT_INVALID;
    result = runtime->api->acquire_dma(runtime->api->api_context,
                                       runtime->device, runtime->generation,
                                       length, alignment, flags, output);
    if (result != RIN_DRIVER_RESULT_OK) return result;
    if (output->struct_size < sizeof(*output) || output->lease == 0u ||
        output->generation != runtime->generation ||
        output->length < length || output->segment_count == 0u ||
        output->segment_count > RIN_DRIVER_MAX_SG_SEGMENTS ||
        (output->flags & ~RIN_DRIVER_DMA_FLAG_MASK) != 0u)
        return RIN_DRIVER_RESULT_BACKEND;
    ++runtime->live_leases;
    return result;
}

static inline RinDriverResultV3 rin_driver_runtime_acquire_irq(
    RinDriverRuntimeV3* runtime, uint32_t mode, uint32_t vector,
    RinDriverIrqFlagsV3 flags, RinDriverIrqLeaseV3* output) {
    RinDriverResultV3 result;
    if (rin_driver_runtime_require(runtime, RIN_DRIVER_CAP_IRQ) !=
            RIN_DRIVER_RESULT_OK || !runtime->api->acquire_irq || !output ||
        (mode != RIN_DRIVER_IRQ_LEGACY && mode != RIN_DRIVER_IRQ_MSI &&
         mode != RIN_DRIVER_IRQ_MSIX) ||
        (flags & ~RIN_DRIVER_IRQ_FLAG_MASK) != 0u)
        return RIN_DRIVER_RESULT_INVALID;
    result = runtime->api->acquire_irq(runtime->api->api_context,
                                       runtime->device, runtime->generation,
                                       mode, vector, flags, output);
    if (result != RIN_DRIVER_RESULT_OK) return result;
    if (output->struct_size < sizeof(*output) || output->lease == 0u ||
        output->generation != runtime->generation || output->mode != mode ||
        (output->flags & ~RIN_DRIVER_IRQ_FLAG_MASK) != 0u)
        return RIN_DRIVER_RESULT_BACKEND;
    ++runtime->live_leases;
    return result;
}

static inline RinDriverResultV3 rin_driver_runtime_arm_timer(
    RinDriverRuntimeV3* runtime, uint64_t deadline_ns, uint64_t period_ns,
    RinDriverTimerFlagsV3 flags, RinDriverTimerLeaseV3* output) {
    RinDriverResultV3 result;
    if (rin_driver_runtime_require(runtime, RIN_DRIVER_CAP_TIMER) !=
            RIN_DRIVER_RESULT_OK || !runtime->api->arm_timer || !output ||
        deadline_ns == 0u || (flags & ~RIN_DRIVER_TIMER_FLAG_MASK) != 0u ||
        ((flags & RIN_DRIVER_TIMER_PERIODIC) != 0u && period_ns == 0u) ||
        ((flags & RIN_DRIVER_TIMER_ONE_SHOT) != 0u && period_ns != 0u))
        return RIN_DRIVER_RESULT_INVALID;
    result = runtime->api->arm_timer(runtime->api->api_context,
                                     runtime->device, runtime->generation,
                                     deadline_ns, period_ns, flags, output);
    if (result != RIN_DRIVER_RESULT_OK) return result;
    if (output->struct_size < sizeof(*output) || output->lease == 0u ||
        output->generation != runtime->generation ||
        output->deadline_ns != deadline_ns ||
        (output->flags & ~RIN_DRIVER_TIMER_FLAG_MASK) != 0u)
        return RIN_DRIVER_RESULT_BACKEND;
    ++runtime->live_leases;
    return result;
}

static inline RinDriverResultV3 rin_driver_runtime_register(
    RinDriverRuntimeV3* runtime, const RinDriverRegistrationV3* request,
    RinDriverResourceHandleV3* output) {
    RinDriverResultV3 result;
    if (rin_driver_runtime_require(runtime, RIN_DRIVER_CAP_REGISTER) !=
            RIN_DRIVER_RESULT_OK || !runtime->api->register_device ||
        !request || !output || request->struct_size < sizeof(*request) ||
        request->generation != runtime->generation ||
        request->device != runtime->device || request->operations_rva == 0u ||
        request->operations_size == 0u)
        return RIN_DRIVER_RESULT_INVALID;
    result = runtime->api->register_device(runtime->api->api_context,
                                            runtime->device,
                                            runtime->generation, request,
                                            output);
    if (result != RIN_DRIVER_RESULT_OK) return result;
    if (!rin_driver_runtime_lease_valid(runtime, *output,
                                        runtime->generation))
        return RIN_DRIVER_RESULT_BACKEND;
    ++runtime->live_leases;
    return result;
}

static inline RinDriverResultV3 rin_driver_runtime_stop(
    RinDriverRuntimeV3* runtime) {
    if (!runtime || runtime->state != RIN_DRIVER_RUNTIME_READY)
        return RIN_DRIVER_RESULT_STALE;
    if (runtime->live_leases != 0u) return RIN_DRIVER_RESULT_BUSY;
    runtime->state = RIN_DRIVER_RUNTIME_QUIESCING;
    runtime->state = RIN_DRIVER_RUNTIME_STOPPED;
    return RIN_DRIVER_RESULT_OK;
}

#endif /* RIN_DRIVER_RUNTIME_V3_H */
