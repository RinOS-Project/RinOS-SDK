/* SPDX-License-Identifier: MIT */
/* Process-owned GPU memory allocation syscall contract. */
#ifndef RIN_SDK_GPU_H
#define RIN_SDK_GPU_H

#include "abi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_GPU_ALLOCATION_DESC_VERSION_V1 UINT32_C(1)
#define RIN_GPU_ALLOCATION_INFO_VERSION_V1 UINT32_C(1)
#define RIN_GPU_ALLOCATION_STATE_ACTIVE_V1 UINT32_C(1)
#define RIN_GPU_ALLOCATION_MAX_LEASES_V1 UINT32_C(64)

#define RIN_GPU_HEAP_LOCAL UINT32_C(1)
#define RIN_GPU_HEAP_SYSTEM UINT32_C(2)

#define RIN_GPU_ALLOCATION_GPU_READ UINT32_C(0x00000001)
#define RIN_GPU_ALLOCATION_GPU_WRITE UINT32_C(0x00000002)
#define RIN_GPU_ALLOCATION_CPU_VISIBLE UINT32_C(0x00000004)
#define RIN_GPU_ALLOCATION_ZEROED UINT32_C(0x00000008)
#define RIN_GPU_SYNC_CPU_TO_DEVICE UINT32_C(1)
#define RIN_GPU_SYNC_DEVICE_TO_CPU UINT32_C(2)
#define RIN_GPU_MAP_CPU_READ UINT32_C(0x00000001)
#define RIN_GPU_MAP_CPU_WRITE UINT32_C(0x00000002)
#define RIN_GPU_MEMORY_MAX_TRANSFER_BYTES UINT64_C(1048576)
#define RIN_GPU_MEMORY_MAPPING_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_SUBMIT_DESC_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_SUBMIT_DESC_VERSION_V2 UINT32_C(2)
#define RIN_GPU_PROCESS_SUBMIT_RECEIPT_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_QUEUE_TIMELINE_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_RESOURCE_BINDING_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_MAX_COMMANDS_V1 UINT32_C(1024)
#define RIN_GPU_PROCESS_MAX_RESOURCES_V1 UINT32_C(8)
#define RIN_GPU_PROCESS_MAX_QUEUES_V1 UINT32_C(8)
#define RIN_GPU_PROCESS_MAX_COMPUTE_BINDINGS_V1 UINT32_C(64)
#define RIN_GPU_PROCESS_COMPUTE_BINDING_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_SHADER_MAX_BYTES_V1 UINT64_C(1048640)
#define RIN_GPU_PROCESS_QUERY_DESC_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_QUERY_RESULT_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_PRODUCT_STATUS_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_PRODUCT_REPORT_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_QUERY_VALUE_COUNT_V1 UINT32_C(8)
#define RIN_GPU_PROCESS_QUERY_TIMESTAMP_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_QUERY_OCCLUSION_V1 UINT32_C(2)
#define RIN_GPU_PROCESS_QUERY_PIPELINE_STATISTICS_V1 UINT32_C(3)
#define RIN_GPU_PROCESS_PRODUCT_STATUS_READY_V1 UINT32_C(0x00000001)
#define RIN_GPU_PROCESS_PRODUCT_STATUS_SUSPENDED_V1 UINT32_C(0x00000002)
#define RIN_GPU_PROCESS_PRODUCT_STATUS_RECOVERY_REQUIRED_V1 \
    UINT32_C(0x00000004)
#define RIN_GPU_PROCESS_PRODUCT_STATUS_LOST_V1 UINT32_C(0x00000008)
#define RIN_GPU_PROCESS_PRODUCT_STATUS_KNOWN_V1 UINT32_C(0x0000000f)
#define RIN_GPU_PROCESS_PRODUCT_REPORT_RESET_V1 UINT32_C(0x00000001)
#define RIN_GPU_PROCESS_PRODUCT_REPORT_WATCHDOG_V1 UINT32_C(0x00000002)
#define RIN_GPU_PROCESS_PRODUCT_REPORT_DEVICE_FAULT_V1 UINT32_C(0x00000004)
#define RIN_GPU_PROCESS_PRODUCT_REPORT_MEMORY_REBOUND_V1 \
    UINT32_C(0x00000008)
#define RIN_GPU_PROCESS_PRODUCT_REPORT_KNOWN_V1 UINT32_C(0x0000000f)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_REQUEST_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_RESPONSE_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_STATUS_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_IMAGE_LAYOUT_VERSION_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_DISPLAYS_V1 UINT32_C(16)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_MODES_V1 UINT32_C(64)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_PLANES_V1 UINT32_C(16)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_LAYOUT_PLANES_V1 UINT32_C(4)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_NAME_SIZE_V1 UINT32_C(64)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_FORMATS_V1 UINT32_C(16)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_PRESENT_MODES_V1 UINT32_C(8)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_LAYOUT_UNKNOWN_V1 UINT32_C(0)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_LAYOUT_LINEAR_V1 UINT32_C(1)
/* Stable RinOS process-ABI values. These are not Vulkan VkFormat or
 * VkColorSpaceKHR enumerants; OS-Core adapters translate them explicitly. */
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_PIXEL_FORMAT_RGBA8_UNORM_V1 \
    UINT32_C(1)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_COLOR_ENCODING_SRGB_NONLINEAR_V1 \
    UINT32_C(1)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_FIFO_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_PENDING_V1 UINT32_C(1)
#define RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_COMPLETE_V1 UINT32_C(2)
/* RinGpuBackendCommandV1's canonical packed record size. The kernel pins this
 * SDK value to sizeof(RinGpuBackendCommandV1) at its dispatch boundary. */
#define RIN_GPU_PROCESS_COMMAND_RECORD_SIZE_V1 UINT32_C(872)

enum RinGpuSdkOperationV1 {
    RIN_GPU_SDK_MEMORY_ALLOCATE = 1,
    RIN_GPU_SDK_MEMORY_QUERY = 2,
    RIN_GPU_SDK_MEMORY_DESTROY = 3,
    RIN_GPU_SDK_MEMORY_SYNC = 4,
    RIN_GPU_SDK_MEMORY_UPLOAD = 5,
    RIN_GPU_SDK_MEMORY_READBACK = 6,
    RIN_GPU_SDK_MEMORY_MAP = 7,
    RIN_GPU_SDK_MEMORY_UNMAP = 8,
    /* Service-only broker dispatch; the kernel admits this operation only
     * for the exact signed ringpu-capability system service. */
    RIN_GPU_SDK_CAPABILITY_DISPATCH = 9,
    RIN_GPU_SDK_PROCESS_QUEUE_BIND = 10,
    RIN_GPU_SDK_PROCESS_QUEUE_RELEASE = 11,
    RIN_GPU_SDK_PROCESS_SUBMIT = 12,
    RIN_GPU_SDK_PROCESS_QUEUE_TIMELINE_QUERY = 13,
    RIN_GPU_SDK_PROCESS_RESOURCE_BIND = 14,
    RIN_GPU_SDK_PROCESS_RESOURCE_UNBIND = 15,
    RIN_GPU_SDK_PROCESS_SHADER_MODULE_CREATE = 16,
    RIN_GPU_SDK_PROCESS_OBJECT_DESTROY = 17,
    RIN_GPU_SDK_PROCESS_COMPUTE_PIPELINE_CREATE = 18,
    RIN_GPU_SDK_PROCESS_COMPUTE_BIND_GROUP_CREATE = 19,
    RIN_GPU_SDK_PROCESS_QUERY_CREATE = 20,
    RIN_GPU_SDK_PROCESS_QUERY_RESULT = 21,
    RIN_GPU_SDK_PROCESS_PRODUCT_GET_STATUS = 22,
    RIN_GPU_SDK_PROCESS_PRODUCT_POLL = 23,
    RIN_GPU_SDK_PROCESS_DIRECT_DISPLAY_DISPATCH = 24,
    /* V2 derives its exact allocation lease set from authenticated command
     * resource cookies; callers never need a foreign allocation handle. */
    RIN_GPU_SDK_PROCESS_SUBMIT_V2 = 25
};

typedef uint64_t RinGpuAllocationV1;
/* An authenticated process-local queue token, not a RingGPU implementation
 * handle or a physical queue register value. */
typedef uint64_t RinGpuProcessQueueV1;

typedef struct RinGpuProcessResourceV1 {
    uint64_t allocation;
    uint32_t required_gpu_access;
    uint32_t reserved;
} RinGpuProcessResourceV1;

/* Associates an opaque process-local API resource cookie with an exact range
 * of a process-owned allocation. The kernel keeps a memory lease until
 * unbind or process exit. Offsets are allocation-relative; this record never
 * carries a GPU virtual address. */
typedef struct RinGpuProcessResourceBindingV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t resource_cookie;
    RinGpuAllocationV1 allocation;
    uint64_t allocation_offset;
    uint64_t size_bytes;
    uint32_t required_gpu_access;
    uint32_t reserved;
} RinGpuProcessResourceBindingV1;

/* A process-owned compute bind-group entry. The range is relative to the
 * process resource cookie, not a GPUVA or physical address. */
typedef struct RinGpuProcessComputeBindingV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t resource_cookie;
    uint64_t offset;
    uint64_t size_bytes;
    uint32_t access;
    uint32_t reserved;
} RinGpuProcessComputeBindingV1;

/* Query descriptions and results use RingPU's bounded timestamp, occlusion,
 * and pipeline-statistics query types. The token remains process-owned. */
typedef struct RinGpuProcessQueryDescV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t query_type;
    uint32_t flags;
    uint32_t reserved0;
    uint32_t reserved1;
} RinGpuProcessQueryDescV1;

typedef struct RinGpuProcessQueryResultV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t query_type;
    uint32_t available;
    uint32_t reserved0;
    uint32_t reserved1;
    uint64_t values[RIN_GPU_PROCESS_QUERY_VALUE_COUNT_V1];
} RinGpuProcessQueryResultV1;

/* `commands` is a byte slice of canonical public RinGpuBackendCommandV1
 * records; command_record_size must equal
 * RIN_GPU_PROCESS_COMMAND_RECORD_SIZE_V1. The descriptor and both slices are
 * copied by the syscall before submit; their addresses are never retained as
 * command cookies. */
typedef struct RinGpuProcessSubmitDescV1 {
    uint32_t struct_size;
    uint32_t version;
    RinSliceV1 commands;
    uint32_t command_count;
    uint32_t command_record_size;
    RinSliceV1 resources;
    uint32_t resource_count;
    uint32_t reserved;
} RinGpuProcessSubmitDescV1;

/* V2 omits allocation handles. The kernel copies the commands, resolves every
 * resource cookie through the authenticated process-resource namespace, and
 * derives exact allocation/range/access leases before submit. This supports
 * capability-bound resources without exposing the allocation owner's handle.
 */
typedef struct RinGpuProcessSubmitDescV2 {
    uint32_t struct_size;
    uint32_t version;
    RinSliceV1 commands;
    uint32_t command_count;
    uint32_t command_record_size;
    uint64_t reserved[2];
} RinGpuProcessSubmitDescV2;

typedef struct RinGpuProcessSubmitReceiptV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t queue_id;
    uint32_t reserved0;
    uint64_t sequence;
    uint64_t completion_value;
    uint64_t device_epoch;
    uint64_t iommu_map_generation;
    uint64_t reserved[2];
} RinGpuProcessSubmitReceiptV1;

/* Nonblocking snapshot of the real, monotonically completed backend timeline
 * for one authenticated process queue. The caller supplies the device and
 * IOMMU generations from its submit receipt; a reset/rebind is reported as
 * stale instead of making an old timeline appear complete. */
typedef struct RinGpuProcessQueueTimelineV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t queue_id;
    uint32_t reserved0;
    uint64_t completed_value;
    uint64_t device_epoch;
    uint64_t iommu_map_generation;
} RinGpuProcessQueueTimelineV1;

/* API-neutral snapshot of the live RinGPU product runtime. Counts and queue
 * completion values are copied from the generation-bound kernel runtime; the
 * record contains no Vulkan types or physical/display addresses. */
typedef struct RinGpuProcessProductStatusV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t queue_count;
    uint64_t iommu_domain_cookie;
    uint64_t iommu_map_generation;
    uint64_t device_epoch;
    uint32_t pending_submission_count;
    uint32_t active_resource_lease_count;
    uint32_t active_allocation_count;
    uint32_t cleanup_pending_count;
    uint32_t reset_count;
    uint32_t max_resources_per_submission;
    uint64_t completed_values[RIN_GPU_PROCESS_MAX_QUEUES_V1];
} RinGpuProcessProductStatusV1;

typedef struct RinGpuProcessProductReportV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t completed_count;
    uint32_t failed_count;
    uint32_t pending_submission_count;
    uint32_t active_resource_lease_count;
    uint32_t reset_count;
    uint64_t device_epoch;
    uint64_t iommu_domain_cookie;
    uint64_t iommu_map_generation;
    uint64_t observed_monotonic_ns;
    uint64_t completed_values[RIN_GPU_PROCESS_MAX_QUEUES_V1];
} RinGpuProcessProductReportV1;

/* API-neutral OS-Core direct-display wire protocol. These records describe
 * output geometry and allocation-relative image storage; they contain no
 * Vulkan types, process pointers, GPU virtual addresses, IOVAs, or scanout
 * addresses. Every request is bound by the kernel to its authenticated
 * process instance. The present allocation must be owned by that process and
 * remains leased until its returned token is completed or cancelled. */
enum RinGpuProcessDirectDisplayOperationV1 {
    RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_DISPLAYS_V1 = 1,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_MODES_V1 = 2,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_PLANES_V1 = 3,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_PLANE_DISPLAYS_V1 = 4,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_PLANE_CAPABILITIES_V1 = 5,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_QUEUE_SUPPORT_V1 = 6,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_QUERY_SURFACE_PROPERTIES_V1 = 7,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_PRESENT_V1 = 8,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_POLL_V1 = 9,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_CANCEL_V1 = 10
};

enum RinGpuProcessDirectDisplayResultV1 {
    RIN_GPU_PROCESS_DIRECT_DISPLAY_OK_V1 = 0,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_INCOMPLETE_V1 = 1,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_NOT_READY_V1 = 2,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_OUT_OF_DATE_V1 = 3,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_DEVICE_LOST_V1 = 4,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_UNSUPPORTED_V1 = 5,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_BACKEND_V1 = 6,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_INVALID_ARGUMENT_V1 = 7,
    RIN_GPU_PROCESS_DIRECT_DISPLAY_LIMIT_V1 = 8
};

typedef struct RinGpuProcessDirectDisplayImagePlaneV1 {
    uint64_t offset_bytes;
    uint64_t row_pitch_bytes;
    uint64_t size_bytes;
} RinGpuProcessDirectDisplayImagePlaneV1;

typedef struct RinGpuProcessDirectDisplayImageLayoutV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t kind;
    uint32_t plane_count;
    uint64_t modifier;
    RinGpuProcessDirectDisplayImagePlaneV1 planes[
        RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_LAYOUT_PLANES_V1];
    uint64_t reserved[2];
} RinGpuProcessDirectDisplayImageLayoutV1;

typedef struct RinGpuProcessDirectDisplayPresentRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t display_cookie;
    uint64_t mode_cookie;
    RinGpuAllocationV1 allocation;
    uint64_t allocation_offset;
    uint64_t allocation_size;
    uint64_t output_generation;
    uint64_t device_generation;
    uint64_t frame_id;
    uint32_t image_index;
    uint32_t width;
    uint32_t height;
    uint32_t pixel_format;
    uint32_t present_schedule;
    uint32_t flags;
    RinGpuProcessDirectDisplayImageLayoutV1 image_layout;
    uint64_t reserved[2];
} RinGpuProcessDirectDisplayPresentRequestV1;

/* Validate the row-pitch lower bound for pixel formats defined by this SDK
 * contract. Unknown formats retain the generic nonzero-pitch requirement and
 * are left for the backend to report as unsupported. */
static inline int rin_gpu_process_direct_display_row_pitch_valid_v1(
    const RinGpuProcessDirectDisplayPresentRequestV1* present) {
    uint64_t minimum_row_pitch;
    uint64_t row_pitch;
    if (!present || present->width == 0u) return 0;
    row_pitch = present->image_layout.planes[0].row_pitch_bytes;
    if (row_pitch == 0u) return 0;
    if (present->pixel_format !=
        RIN_GPU_PROCESS_DIRECT_DISPLAY_PIXEL_FORMAT_RGBA8_UNORM_V1)
        return 1;
    minimum_row_pitch = (uint64_t)present->width * UINT64_C(4);
    return row_pitch >= minimum_row_pitch;
}

typedef struct RinGpuProcessDirectDisplayV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t display_cookie;
    uint64_t output_generation;
    uint64_t device_generation;
    uint64_t current_mode_cookie;
    uint32_t width;
    uint32_t height;
    uint32_t refresh_millihertz;
    uint32_t pixel_format;
    uint32_t physical_width_mm;
    uint32_t physical_height_mm;
    uint32_t flags;
    uint32_t plane_count;
    uint32_t mode_count;
    char display_name[RIN_GPU_PROCESS_DIRECT_DISPLAY_NAME_SIZE_V1];
    /* Keep the following 64-bit fields at the same offsets on i386 and x86_64. */
    uint32_t reserved0;
    uint64_t reserved[2];
} RinGpuProcessDirectDisplayV1;

typedef struct RinGpuProcessDirectDisplayModeV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t display_cookie;
    uint64_t mode_cookie;
    uint64_t output_generation;
    uint32_t width;
    uint32_t height;
    uint32_t refresh_millihertz;
    uint32_t pixel_format;
    uint32_t flags;
    /* Explicit padding keeps the 64-bit reserved field ABI-stable on i386. */
    uint32_t reserved0;
    uint64_t reserved;
} RinGpuProcessDirectDisplayModeV1;

typedef struct RinGpuProcessDirectDisplayPlaneV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t plane_index;
    uint32_t current_stack_index;
    uint64_t current_display_cookie;
    uint64_t output_generation;
    uint64_t device_generation;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinGpuProcessDirectDisplayPlaneV1;

typedef struct RinGpuProcessDirectDisplayPlaneCapabilitiesV1 {
    uint32_t supported_alpha;
    int32_t min_src_x;
    int32_t min_src_y;
    int32_t max_src_x;
    int32_t max_src_y;
    uint32_t min_src_width;
    uint32_t min_src_height;
    uint32_t max_src_width;
    uint32_t max_src_height;
    int32_t min_dst_x;
    int32_t min_dst_y;
    int32_t max_dst_x;
    int32_t max_dst_y;
    uint32_t min_dst_width;
    uint32_t min_dst_height;
    uint32_t max_dst_width;
    uint32_t max_dst_height;
} RinGpuProcessDirectDisplayPlaneCapabilitiesV1;

typedef struct RinGpuProcessDirectDisplaySurfaceFormatV1 {
    int32_t pixel_format;
    int32_t color_encoding;
} RinGpuProcessDirectDisplaySurfaceFormatV1;

typedef struct RinGpuProcessDirectDisplaySurfacePropertiesV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t min_image_count;
    uint32_t max_image_count;
    uint32_t current_extent_width;
    uint32_t current_extent_height;
    uint32_t min_image_extent_width;
    uint32_t min_image_extent_height;
    uint32_t max_image_extent_width;
    uint32_t max_image_extent_height;
    uint32_t max_image_array_layers;
    uint32_t supported_transforms;
    uint32_t current_transform;
    uint32_t supported_composite_alpha;
    uint32_t supported_usage_flags;
    uint32_t format_count;
    uint32_t present_schedule_count;
    uint32_t reserved0[2];
    RinGpuProcessDirectDisplaySurfaceFormatV1 formats[
        RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_FORMATS_V1];
    int32_t present_schedules[
        RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_PRESENT_MODES_V1];
    uint32_t reserved[4];
} RinGpuProcessDirectDisplaySurfacePropertiesV1;

typedef struct RinGpuProcessDirectDisplayPresentStatusV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t reserved0;
    uint64_t output_generation;
    uint64_t device_generation;
    uint64_t reserved[2];
} RinGpuProcessDirectDisplayPresentStatusV1;

/* Unused request fields and all reserved fields must be zero. `capacity` is
 * the maximum record count for bounded query operations and zero otherwise. */
typedef struct RinGpuProcessDirectDisplayRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t capacity;
    uint64_t device_id;
    uint64_t device_generation;
    uint64_t display_cookie;
    uint64_t output_generation;
    uint64_t mode_cookie;
    uint64_t present_token;
    uint32_t plane_index;
    uint32_t queue_family_index;
    uint32_t queue_flags;
    uint32_t queue_count;
    RinGpuProcessDirectDisplayPresentRequestV1 present;
    uint64_t reserved[2];
} RinGpuProcessDirectDisplayRequestV1;

typedef union RinGpuProcessDirectDisplayDataV1 {
    RinGpuProcessDirectDisplayV1 displays[
        RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_DISPLAYS_V1];
    RinGpuProcessDirectDisplayModeV1 modes[
        RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_MODES_V1];
    RinGpuProcessDirectDisplayPlaneV1 planes[
        RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_PLANES_V1];
    uint64_t display_cookies[
        RIN_GPU_PROCESS_DIRECT_DISPLAY_MAX_DISPLAYS_V1];
    RinGpuProcessDirectDisplayPlaneCapabilitiesV1 plane_capabilities;
    RinGpuProcessDirectDisplaySurfacePropertiesV1 surface_properties;
    uint32_t queue_supported;
    uint64_t present_token;
    RinGpuProcessDirectDisplayPresentStatusV1 present_status;
} RinGpuProcessDirectDisplayDataV1;

typedef struct RinGpuProcessDirectDisplayResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t result;
    uint32_t count;
    uint32_t reserved0;
    RinGpuProcessDirectDisplayDataV1 data;
    uint64_t reserved[2];
} RinGpuProcessDirectDisplayResponseV1;

/* Device identity comes from RinDeviceInfoV1. The generation is mandatory so
 * an allocation cannot silently bind to a replacement device with a reused
 * device ID. GPU virtual addresses in the query record remain GPUVA only;
 * they are not physical, IOVA, or display-fetch addresses. */
typedef struct RinGpuAllocationDescV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t heap;
    uint32_t flags;
    uint64_t size_bytes;
    uint64_t alignment;
    uint64_t reserved[4];
} RinGpuAllocationDescV1;

/* A successful query echoes the requested allocation handle, reports a
 * supported heap/access shape and active allocation, and returns aligned
 * address geometry with allocation_size_bytes >= requested_size_bytes,
 * nonzero device/IOMMU generations, and a bounded lease count. The reserved
 * field must be zero; the SDK rejects responses outside this contract. */
typedef struct RinGpuAllocationInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t heap;
    uint32_t flags;
    RinGpuAllocationV1 allocation;
    uint64_t gpu_virtual_address;
    uint64_t heap_offset;
    uint64_t requested_size_bytes;
    uint64_t allocation_size_bytes;
    uint64_t alignment;
    uint64_t iommu_map_generation;
    uint64_t device_epoch;
    uint32_t lease_count;
    uint32_t state;
    uint64_t reserved;
} RinGpuAllocationInfoV1;

typedef struct RinGpuMemoryMappingV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t access;
    uint32_t reserved0;
    RinGpuAllocationV1 allocation;
    uint64_t address;
    uint64_t size_bytes;
    uint64_t mapping;
    uint64_t device_generation;
    uint64_t reserved;
} RinGpuMemoryMappingV1;

#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
RIN_SDK_API RinResult rin_gpu_memory_allocate_v1(
    uint64_t device_id, uint64_t device_generation,
    const RinGpuAllocationDescV1* descriptor,
    RinGpuAllocationV1* allocation_out);
RIN_SDK_API RinResult rin_gpu_memory_query_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, RinGpuAllocationInfoV1* info_out);
RIN_SDK_API RinResult rin_gpu_memory_destroy_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation);
/* Transfers are bounded to 1 MiB per call. Upload/readback use kernel-copied
 * buffers rather than mapping a physical backing into the process. A failed
 * multi-chunk transfer may have completed a prefix; retry the full range when
 * the reported result permits it. */
RIN_SDK_API RinResult rin_gpu_memory_sync_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint32_t action, uint64_t offset,
    uint64_t length);
RIN_SDK_API RinResult rin_gpu_memory_upload_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint64_t offset,
    const void* source, uint64_t length);
RIN_SDK_API RinResult rin_gpu_memory_readback_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint64_t offset,
    void* destination, uint64_t length);
/* Map CPU-visible allocation backing into this process. The mapping is
 * process-instance and device-generation bound; release it with unmap before
 * destroying the allocation. Process exit also revokes outstanding maps.
 * If a successful kernel response contains a malformed mapping record, the
 * wrapper attempts immediate unmap. When cleanup fails, map returns that
 * cleanup error and mapping_out retains only its allocation, device
 * generation, and mapping token (address and size remain zero), so the caller
 * can retry unmap without using an unvalidated CPU address. */
RIN_SDK_API RinResult rin_gpu_memory_map_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint32_t access,
    RinGpuMemoryMappingV1* mapping_out);
RIN_SDK_API RinResult rin_gpu_memory_unmap_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuAllocationV1 allocation, uint64_t mapping);
/* Bind a process-owned token to an admitted backend queue index. The queue
 * index is validated against the exact device generation and later checked
 * against the engine actually selected by command preparation. */
RIN_SDK_API RinResult rin_gpu_process_queue_bind_v1(
    uint64_t device_id, uint64_t device_generation, uint32_t queue_id,
    RinGpuProcessQueueV1* queue_out);
RIN_SDK_API RinResult rin_gpu_process_queue_release_v1(
    RinGpuProcessQueueV1 queue);
/* Submit canonical API-independent command records. Resources are exact
 * process-owned allocation handles with explicit GPU access. Direct buffer
 * and image cookies used by commands must first be bound with
 * rin_gpu_process_resource_bind_v1(), and their backing allocations must be
 * listed here with access covering the command. The kernel copies the records
 * and checks the binding, byte range where present, and allocation declaration
 * before the physical driver receives them. Accepted work remains owned by
 * PID+process-instance until real completion. */
RIN_SDK_API RinResult rin_gpu_process_submit_v1(
    RinGpuProcessQueueV1 queue, const RinGpuProcessSubmitDescV1* descriptor,
    RinGpuProcessSubmitReceiptV1* receipt_out);
RIN_SDK_API RinResult rin_gpu_process_submit_v2(
    RinGpuProcessQueueV1 queue, const RinGpuProcessSubmitDescV2* descriptor,
    RinGpuProcessSubmitReceiptV1* receipt_out);
RIN_SDK_API RinResult rin_gpu_process_queue_query_timeline_v1(
    RinGpuProcessQueueV1 queue, uint64_t expected_device_epoch,
    uint64_t expected_iommu_map_generation,
    RinGpuProcessQueueTimelineV1* timeline_out);
/* The cookie is an opaque API object identity chosen by the process. Binding
 * and unbinding are authenticated to the current process instance. */
RIN_SDK_API RinResult rin_gpu_process_resource_bind_v1(
    uint64_t device_id, uint64_t device_generation,
    const RinGpuProcessResourceBindingV1* binding);
RIN_SDK_API RinResult rin_gpu_process_resource_unbind_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t resource_cookie);
/* Create a process-owned RSH1 module. The kernel copies and validates the
 * bounded byte slice before any backend callback; the token is never a
 * backend cookie. Destroy is BUSY while a pipeline or submission depends on
 * the object. */
RIN_SDK_API RinResult rin_gpu_process_shader_module_create_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t shader_module_token, const void* rin_shader_ir,
    uint64_t shader_ir_size);
RIN_SDK_API RinResult rin_gpu_process_object_destroy_v1(
    uint64_t device_id, uint64_t device_generation, uint64_t object_token);
/* Create a process-owned compute pipeline from a live compute shader module.
 * Opaque process tokens never expose RingGPU/backend object cookies. */
RIN_SDK_API RinResult rin_gpu_process_compute_pipeline_create_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t pipeline_token, uint64_t shader_module_token);
/* Bind-group creation retains the authenticated process resource bindings
 * until the process object is destroyed. */
RIN_SDK_API RinResult rin_gpu_process_compute_bind_group_create_v1(
    uint64_t device_id, uint64_t device_generation,
    uint64_t bind_group_token, uint64_t pipeline_token,
    const RinGpuProcessComputeBindingV1* bindings, uint32_t binding_count);
/* Query creation and result retrieval use an opaque process token. Results
 * return successfully with `available == 0` while submitted work is pending. */
RIN_SDK_API RinResult rin_gpu_process_query_create_v1(
    uint64_t device_id, uint64_t device_generation, uint64_t query_token,
    const RinGpuProcessQueryDescV1* descriptor);
RIN_SDK_API RinResult rin_gpu_process_query_result_v1(
    uint64_t device_id, uint64_t device_generation, uint64_t query_token,
    RinGpuProcessQueryResultV1* result_out);
RIN_SDK_API RinResult rin_gpu_process_product_get_status_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuProcessProductStatusV1* status_out);
RIN_SDK_API RinResult rin_gpu_process_product_poll_v1(
    uint64_t device_id, uint64_t device_generation,
    RinGpuProcessProductReportV1* report_out);
/* Dispatch one authenticated OS-Core direct-display query/present operation.
 * Array queries are bounded by `capacity`; present uses an explicit,
 * allocation-relative image layout and returns a process-local token. */
RIN_SDK_API RinResult rin_gpu_process_direct_display_dispatch_v1(
    const RinGpuProcessDirectDisplayRequestV1* request,
    RinGpuProcessDirectDisplayResponseV1* response_out);
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinGpuAllocationDescV1) == 64u,
              "RinGpuAllocationDescV1 ABI drift");
static_assert(sizeof(RinGpuAllocationInfoV1) == 96u,
              "RinGpuAllocationInfoV1 ABI drift");
static_assert(sizeof(RinGpuMemoryMappingV1) == 64u,
              "RinGpuMemoryMappingV1 ABI drift");
static_assert(sizeof(RinGpuProcessResourceV1) == 16u,
              "RinGpuProcessResourceV1 ABI drift");
static_assert(sizeof(RinGpuProcessSubmitDescV1) == 56u,
              "RinGpuProcessSubmitDescV1 ABI drift");
static_assert(sizeof(RinGpuProcessSubmitDescV2) == 48u,
              "RinGpuProcessSubmitDescV2 ABI drift");
static_assert(sizeof(RinGpuProcessSubmitReceiptV1) == 64u,
              "RinGpuProcessSubmitReceiptV1 ABI drift");
static_assert(sizeof(RinGpuProcessQueueTimelineV1) == 40u,
              "RinGpuProcessQueueTimelineV1 ABI drift");
static_assert(sizeof(RinGpuProcessProductStatusV1) == 128u,
              "RinGpuProcessProductStatusV1 ABI drift");
static_assert(sizeof(RinGpuProcessProductReportV1) == 128u,
              "RinGpuProcessProductReportV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayImagePlaneV1) == 24u,
              "RinGpuProcessDirectDisplayImagePlaneV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayImageLayoutV1) == 136u,
              "RinGpuProcessDirectDisplayImageLayoutV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayPresentRequestV1) == 248u,
              "RinGpuProcessDirectDisplayPresentRequestV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayV1) == 160u,
              "RinGpuProcessDirectDisplayV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayModeV1) == 64u,
              "RinGpuProcessDirectDisplayModeV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayPlaneV1) == 64u,
              "RinGpuProcessDirectDisplayPlaneV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayPlaneCapabilitiesV1) == 68u,
              "RinGpuProcessDirectDisplayPlaneCapabilitiesV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplaySurfacePropertiesV1) == 252u,
              "RinGpuProcessDirectDisplaySurfacePropertiesV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayPresentStatusV1) == 48u,
              "RinGpuProcessDirectDisplayPresentStatusV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayRequestV1) == 344u,
              "RinGpuProcessDirectDisplayRequestV1 ABI drift");
static_assert(sizeof(RinGpuProcessDirectDisplayResponseV1) == 4136u,
              "RinGpuProcessDirectDisplayResponseV1 ABI drift");
static_assert(sizeof(RinGpuProcessResourceBindingV1) == 48u,
              "RinGpuProcessResourceBindingV1 ABI drift");
static_assert(sizeof(RinGpuProcessComputeBindingV1) == 40u,
              "RinGpuProcessComputeBindingV1 ABI drift");
static_assert(sizeof(RinGpuProcessQueryDescV1) == 24u,
              "RinGpuProcessQueryDescV1 ABI drift");
static_assert(sizeof(RinGpuProcessQueryResultV1) == 88u,
              "RinGpuProcessQueryResultV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinGpuAllocationDescV1) == 64u,
               "RinGpuAllocationDescV1 ABI drift");
_Static_assert(sizeof(RinGpuAllocationInfoV1) == 96u,
               "RinGpuAllocationInfoV1 ABI drift");
_Static_assert(sizeof(RinGpuMemoryMappingV1) == 64u,
               "RinGpuMemoryMappingV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessResourceV1) == 16u,
               "RinGpuProcessResourceV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessSubmitDescV1) == 56u,
               "RinGpuProcessSubmitDescV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessSubmitDescV2) == 48u,
               "RinGpuProcessSubmitDescV2 ABI drift");
_Static_assert(sizeof(RinGpuProcessSubmitReceiptV1) == 64u,
               "RinGpuProcessSubmitReceiptV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessQueueTimelineV1) == 40u,
               "RinGpuProcessQueueTimelineV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessProductStatusV1) == 128u,
               "RinGpuProcessProductStatusV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessProductReportV1) == 128u,
               "RinGpuProcessProductReportV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayImagePlaneV1) == 24u,
               "RinGpuProcessDirectDisplayImagePlaneV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayImageLayoutV1) == 136u,
               "RinGpuProcessDirectDisplayImageLayoutV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayPresentRequestV1) == 248u,
               "RinGpuProcessDirectDisplayPresentRequestV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayV1) == 160u,
               "RinGpuProcessDirectDisplayV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayModeV1) == 64u,
               "RinGpuProcessDirectDisplayModeV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayPlaneV1) == 64u,
               "RinGpuProcessDirectDisplayPlaneV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayPlaneCapabilitiesV1) == 68u,
               "RinGpuProcessDirectDisplayPlaneCapabilitiesV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplaySurfacePropertiesV1) == 252u,
               "RinGpuProcessDirectDisplaySurfacePropertiesV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayPresentStatusV1) == 48u,
               "RinGpuProcessDirectDisplayPresentStatusV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayRequestV1) == 344u,
               "RinGpuProcessDirectDisplayRequestV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessDirectDisplayResponseV1) == 4136u,
               "RinGpuProcessDirectDisplayResponseV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessResourceBindingV1) == 48u,
               "RinGpuProcessResourceBindingV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessComputeBindingV1) == 40u,
               "RinGpuProcessComputeBindingV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessQueryDescV1) == 24u,
               "RinGpuProcessQueryDescV1 ABI drift");
_Static_assert(sizeof(RinGpuProcessQueryResultV1) == 88u,
               "RinGpuProcessQueryResultV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_GPU_H */
