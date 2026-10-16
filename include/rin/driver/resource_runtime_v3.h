/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_RESOURCE_RUNTIME_V3_H
#define RIN_DRIVER_RESOURCE_RUNTIME_V3_H

#include <stddef.h>
#include <stdint.h>

#include <rin/driver/resource_contract_v3.h>

/* Header-only SDK glue. It owns no hardware policy and has no compatibility
 * fallback. A missing callback or invalid lease is an error. */
#define RIN_DRIVER_RUNTIME_VERSION UINT32_C(0x00010000)
#define RIN_DRIVER_RUNTIME_CAPABILITY_MASK \
    (RIN_DRIVER_CAP_BAR | RIN_DRIVER_CAP_DMA | RIN_DRIVER_CAP_IRQ | \
     RIN_DRIVER_CAP_TIMER | RIN_DRIVER_CAP_REGISTER)

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
#if UINTPTR_MAX == UINT32_MAX
    uint32_t release_padding;
#endif
    RinDriverRuntimeBarAcquireFnV3 acquire_bar;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t acquire_bar_padding;
#endif
    RinDriverRuntimeDmaAcquireFnV3 acquire_dma;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t acquire_dma_padding;
#endif
    RinDriverRuntimeIrqAcquireFnV3 acquire_irq;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t acquire_irq_padding;
#endif
    RinDriverRuntimeTimerArmFnV3 arm_timer;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t arm_timer_padding;
#endif
    RinDriverRuntimeRegisterFnV3 register_device;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t register_device_padding;
#endif
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
    if (!api || api->struct_size < sizeof(*api) ||
        api->version != RIN_DRIVER_RUNTIME_VERSION || !api->release ||
        api->reserved[0] != 0u || api->reserved[1] != 0u ||
        api->reserved[2] != 0u || api->reserved[3] != 0u)
        return 0;
#if UINTPTR_MAX == UINT32_MAX
    if (api->release_padding != 0u || api->acquire_bar_padding != 0u ||
        api->acquire_dma_padding != 0u || api->acquire_irq_padding != 0u ||
        api->arm_timer_padding != 0u ||
        api->register_device_padding != 0u)
        return 0;
#endif
    return 1;
}

#if defined(__cplusplus)
static_assert(sizeof(RinDriverRuntimeApiV3) == 96u,
              "RinDriverRuntimeApiV3 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinDriverRuntimeApiV3) == 96u,
               "RinDriverRuntimeApiV3 ABI drift");
#endif

static inline RinDriverResultV3 rin_driver_runtime_result(
    int32_t backend_result) {
    if (backend_result <= RIN_DRIVER_RESULT_OK &&
        backend_result >= RIN_DRIVER_RESULT_UNSUPPORTED)
        return (RinDriverResultV3)backend_result;
    return RIN_DRIVER_RESULT_BACKEND;
}

static inline void rin_driver_runtime_output_header(void* output,
                                                     uint32_t size) {
    uint8_t* bytes = (uint8_t*)output;
    if (!output) return;
    for (uint32_t index = 0u; index < size; ++index) bytes[index] = 0u;
    ((RinDriverAbiHeaderV3*)output)->struct_size = size;
    ((RinDriverAbiHeaderV3*)output)->version_major =
        RIN_DRIVER_PUBLIC_ABI_MAJOR;
    ((RinDriverAbiHeaderV3*)output)->version_minor =
        RIN_DRIVER_PUBLIC_ABI_MINOR;
}

static inline int rin_driver_runtime_reserved_zero(const uint64_t* reserved,
                                                    uint32_t count) {
    if (!reserved) return 0;
    for (uint32_t index = 0u; index < count; ++index) {
        if (reserved[index] != 0u) return 0;
    }
    return 1;
}

static inline int rin_driver_runtime_can_acquire(
    const RinDriverRuntimeV3* runtime) {
    return runtime && runtime->live_leases != UINT32_MAX;
}

static inline void rin_driver_runtime_discard(
    RinDriverRuntimeV3* runtime, RinDriverResourceHandleV3 lease) {
    if (runtime && runtime->api && lease != 0u)
        (void)rin_driver_runtime_result(runtime->api->release(
            runtime->api->api_context, runtime->device, lease,
            runtime->generation));
}

static inline RinDriverResultV3 rin_driver_runtime_init(
    RinDriverRuntimeV3* runtime) {
    uint8_t* bytes;
    if (!runtime) return RIN_DRIVER_RESULT_INVALID;
    bytes = (uint8_t*)runtime;
    for (uint32_t index = 0u; index < sizeof(*runtime); ++index)
        bytes[index] = 0u;
    return RIN_DRIVER_RESULT_OK;
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
        !rin_driver_runtime_reserved_zero(context->reserved, 4u) ||
        (context->granted_capabilities &
         ~RIN_DRIVER_CAPABILITY_MASK) != 0u ||
        (runtime->state != RIN_DRIVER_RUNTIME_UNINITIALIZED &&
         runtime->state != RIN_DRIVER_RUNTIME_STOPPED) ||
        runtime->live_leases != 0u ||
        !rin_driver_runtime_reserved_zero(runtime->reserved, 2u))
        return RIN_DRIVER_RESULT_INVALID;
    if ((required_capabilities & ~RIN_DRIVER_RUNTIME_CAPABILITY_MASK) != 0u)
        return RIN_DRIVER_RESULT_UNSUPPORTED;
    if ((context->granted_capabilities & required_capabilities) !=
        required_capabilities)
        return RIN_DRIVER_RESULT_DENIED;
    if (((context->granted_capabilities & RIN_DRIVER_CAP_BAR) != 0u &&
         !api->acquire_bar) ||
        ((context->granted_capabilities & RIN_DRIVER_CAP_DMA) != 0u &&
         !api->acquire_dma) ||
        ((context->granted_capabilities & RIN_DRIVER_CAP_IRQ) != 0u &&
         !api->acquire_irq) ||
        ((context->granted_capabilities & RIN_DRIVER_CAP_TIMER) != 0u &&
         !api->arm_timer) ||
        ((context->granted_capabilities & RIN_DRIVER_CAP_REGISTER) != 0u &&
         !api->register_device))
        return RIN_DRIVER_RESULT_UNSUPPORTED;
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
        capability == 0u)
        return RIN_DRIVER_RESULT_INVALID;
    if ((capability & ~RIN_DRIVER_RUNTIME_CAPABILITY_MASK) != 0u)
        return RIN_DRIVER_RESULT_UNSUPPORTED;
    if ((runtime->capabilities & capability) != capability)
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
    result = rin_driver_runtime_result(runtime->api->release(
        runtime->api->api_context, runtime->device, lease, generation));
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
    RinDriverResultV3 required = rin_driver_runtime_require(
        runtime, RIN_DRIVER_CAP_BAR);
    if (output) rin_driver_runtime_output_header(output, sizeof(*output));
    if (required != RIN_DRIVER_RESULT_OK) return required;
    if (!runtime->api->acquire_bar || !output ||
        (flags & (uint16_t)~RIN_DRIVER_MMIO_FLAG_MASK) != 0u)
        return RIN_DRIVER_RESULT_INVALID;
    if (flags == 0u) return RIN_DRIVER_RESULT_INVALID;
    if (!rin_driver_runtime_can_acquire(runtime))
        return RIN_DRIVER_RESULT_NO_SPACE;
    result = rin_driver_runtime_result(runtime->api->acquire_bar(
        runtime->api->api_context, runtime->device, runtime->generation,
        bar_index, flags, output));
    if (result != RIN_DRIVER_RESULT_OK) {
        rin_driver_runtime_discard(runtime, output->lease);
        rin_driver_runtime_output_header(output, sizeof(*output));
        return result;
    }
    if (output->struct_size < sizeof(*output) || output->lease == 0u ||
        output->version_major != RIN_DRIVER_PUBLIC_ABI_MAJOR ||
        output->version_minor > RIN_DRIVER_PUBLIC_ABI_MINOR ||
        output->bar_index != bar_index || output->generation != runtime->generation ||
        output->length == 0u || output->offset > UINT64_MAX - output->length ||
        output->flags != flags || (output->capability != 0u &&
        output->capability != RIN_DRIVER_CAP_BAR) ||
        !rin_driver_runtime_reserved_zero(output->reserved, 2u)) {
        rin_driver_runtime_discard(runtime, output->lease);
        rin_driver_runtime_output_header(output, sizeof(*output));
        return RIN_DRIVER_RESULT_BACKEND;
    }
    ++runtime->live_leases;
    return result;
}

static inline RinDriverResultV3 rin_driver_runtime_acquire_dma(
    RinDriverRuntimeV3* runtime, uint64_t length, uint32_t alignment,
    RinDriverDmaFlagsV3 flags, RinDriverDmaLeaseV3* output) {
    RinDriverResultV3 result;
    RinDriverResultV3 required = rin_driver_runtime_require(
        runtime, RIN_DRIVER_CAP_DMA);
    if (output) rin_driver_runtime_output_header(output, sizeof(*output));
    if (required != RIN_DRIVER_RESULT_OK) return required;
    if (!runtime->api->acquire_dma || !output ||
        length == 0u || alignment == 0u ||
        (alignment & (alignment - 1u)) != 0u ||
        (flags & ~RIN_DRIVER_DMA_FLAG_MASK) != 0u || flags == 0u ||
        (flags & (RIN_DRIVER_DMA_READ | RIN_DRIVER_DMA_WRITE)) == 0u)
        return RIN_DRIVER_RESULT_INVALID;
    if (!rin_driver_runtime_can_acquire(runtime))
        return RIN_DRIVER_RESULT_NO_SPACE;
    result = rin_driver_runtime_result(runtime->api->acquire_dma(
        runtime->api->api_context, runtime->device, runtime->generation,
        length, alignment, flags, output));
    if (result != RIN_DRIVER_RESULT_OK) {
        rin_driver_runtime_discard(runtime, output->lease);
        rin_driver_runtime_output_header(output, sizeof(*output));
        return result;
    }
    if (output->struct_size < sizeof(*output) || output->lease == 0u ||
        output->version_major != RIN_DRIVER_PUBLIC_ABI_MAJOR ||
        output->version_minor > RIN_DRIVER_PUBLIC_ABI_MINOR ||
        output->generation != runtime->generation ||
        output->memory_lease == 0u || output->length < length ||
        output->alignment < alignment ||
        (output->alignment & (output->alignment - 1u)) != 0u ||
        output->flags != flags || output->reserved0 != 0u ||
        !rin_driver_runtime_reserved_zero(output->reserved, 2u) ||
        output->segment_count == 0u ||
        output->segment_count > RIN_DRIVER_MAX_SG_SEGMENTS ||
        (output->flags & ~RIN_DRIVER_DMA_FLAG_MASK) != 0u) {
        rin_driver_runtime_discard(runtime, output->lease);
        rin_driver_runtime_output_header(output, sizeof(*output));
        return RIN_DRIVER_RESULT_BACKEND;
    }
    ++runtime->live_leases;
    return result;
}

static inline RinDriverResultV3 rin_driver_runtime_acquire_irq(
    RinDriverRuntimeV3* runtime, uint32_t mode, uint32_t vector,
    RinDriverIrqFlagsV3 flags, RinDriverIrqLeaseV3* output) {
    RinDriverResultV3 result;
    RinDriverResultV3 required = rin_driver_runtime_require(
        runtime, RIN_DRIVER_CAP_IRQ);
    if (output) rin_driver_runtime_output_header(output, sizeof(*output));
    if (required != RIN_DRIVER_RESULT_OK) return required;
    if (!runtime->api->acquire_irq || !output ||
        (mode != RIN_DRIVER_IRQ_LEGACY && mode != RIN_DRIVER_IRQ_MSI &&
         mode != RIN_DRIVER_IRQ_MSIX) ||
        (flags & ~RIN_DRIVER_IRQ_FLAG_MASK) != 0u)
        return RIN_DRIVER_RESULT_INVALID;
    if (!rin_driver_runtime_can_acquire(runtime))
        return RIN_DRIVER_RESULT_NO_SPACE;
    result = rin_driver_runtime_result(runtime->api->acquire_irq(
        runtime->api->api_context, runtime->device, runtime->generation,
        mode, vector, flags, output));
    if (result != RIN_DRIVER_RESULT_OK) {
        rin_driver_runtime_discard(runtime, output->lease);
        rin_driver_runtime_output_header(output, sizeof(*output));
        return result;
    }
    if (output->struct_size < sizeof(*output) || output->lease == 0u ||
        output->version_major != RIN_DRIVER_PUBLIC_ABI_MAJOR ||
        output->version_minor > RIN_DRIVER_PUBLIC_ABI_MINOR ||
        output->generation != runtime->generation || output->mode != mode ||
        (vector != 0u && output->vector != vector) ||
        output->vector < 32u || output->vector > 255u ||
        output->flags != flags || output->reserved0 != 0u ||
        !rin_driver_runtime_reserved_zero(output->reserved, 2u)) {
        rin_driver_runtime_discard(runtime, output->lease);
        rin_driver_runtime_output_header(output, sizeof(*output));
        return RIN_DRIVER_RESULT_BACKEND;
    }
    ++runtime->live_leases;
    return result;
}

static inline RinDriverResultV3 rin_driver_runtime_arm_timer(
    RinDriverRuntimeV3* runtime, uint64_t deadline_ns, uint64_t period_ns,
    RinDriverTimerFlagsV3 flags, RinDriverTimerLeaseV3* output) {
    RinDriverResultV3 result;
    RinDriverResultV3 required = rin_driver_runtime_require(
        runtime, RIN_DRIVER_CAP_TIMER);
    if (output) rin_driver_runtime_output_header(output, sizeof(*output));
    if (required != RIN_DRIVER_RESULT_OK) return required;
    if (!runtime->api->arm_timer || !output ||
        deadline_ns == 0u || (flags & ~RIN_DRIVER_TIMER_FLAG_MASK) != 0u ||
        (((flags & RIN_DRIVER_TIMER_PERIODIC) != 0u) ==
         ((flags & RIN_DRIVER_TIMER_ONE_SHOT) != 0u)) ||
        ((flags & RIN_DRIVER_TIMER_PERIODIC) != 0u && period_ns == 0u) ||
        ((flags & RIN_DRIVER_TIMER_ONE_SHOT) != 0u && period_ns != 0u))
        return RIN_DRIVER_RESULT_INVALID;
    if (!rin_driver_runtime_can_acquire(runtime))
        return RIN_DRIVER_RESULT_NO_SPACE;
    result = rin_driver_runtime_result(runtime->api->arm_timer(
        runtime->api->api_context, runtime->device, runtime->generation,
        deadline_ns, period_ns, flags, output));
    if (result != RIN_DRIVER_RESULT_OK) {
        rin_driver_runtime_discard(runtime, output->lease);
        rin_driver_runtime_output_header(output, sizeof(*output));
        return result;
    }
    if (output->struct_size < sizeof(*output) || output->lease == 0u ||
        output->version_major != RIN_DRIVER_PUBLIC_ABI_MAJOR ||
        output->version_minor > RIN_DRIVER_PUBLIC_ABI_MINOR ||
        output->generation != runtime->generation ||
        output->deadline_ns != deadline_ns ||
        output->period_ns != period_ns || output->flags != flags ||
        output->reserved0 != 0u ||
        !rin_driver_runtime_reserved_zero(output->reserved, 2u)) {
        rin_driver_runtime_discard(runtime, output->lease);
        rin_driver_runtime_output_header(output, sizeof(*output));
        return RIN_DRIVER_RESULT_BACKEND;
    }
    ++runtime->live_leases;
    return result;
}

static inline RinDriverResultV3 rin_driver_runtime_register(
    RinDriverRuntimeV3* runtime, const RinDriverRegistrationV3* request,
    RinDriverResourceHandleV3* output) {
    RinDriverResultV3 result;
    RinDriverResultV3 required = rin_driver_runtime_require(
        runtime, RIN_DRIVER_CAP_REGISTER);
    if (output) *output = 0u;
    if (required != RIN_DRIVER_RESULT_OK) return required;
    if (!runtime->api->register_device || !request || !output ||
        request->struct_size < sizeof(*request) ||
        request->version_major != RIN_DRIVER_PUBLIC_ABI_MAJOR ||
        request->version_minor > RIN_DRIVER_PUBLIC_ABI_MINOR ||
        request->device_class == 0u || request->flags != 0u ||
        request->generation != runtime->generation ||
        request->device != runtime->device || request->operations_rva == 0u ||
        request->operations_size == 0u ||
        request->operations_rva > UINT64_MAX - request->operations_size ||
        (request->capability & ~runtime->capabilities) != 0u ||
        !rin_driver_runtime_reserved_zero(request->reserved, 2u))
        return RIN_DRIVER_RESULT_INVALID;
    if (!rin_driver_runtime_can_acquire(runtime))
        return RIN_DRIVER_RESULT_NO_SPACE;
    result = rin_driver_runtime_result(runtime->api->register_device(
        runtime->api->api_context, runtime->device, runtime->generation,
        request, output));
    if (result != RIN_DRIVER_RESULT_OK) {
        rin_driver_runtime_discard(runtime, *output);
        *output = 0u;
        return result;
    }
    if (!rin_driver_runtime_lease_valid(runtime, *output,
                                        runtime->generation)) {
        rin_driver_runtime_discard(runtime, *output);
        *output = 0u;
        return RIN_DRIVER_RESULT_BACKEND;
    }
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

#endif /* RIN_DRIVER_RESOURCE_RUNTIME_V3_H */
