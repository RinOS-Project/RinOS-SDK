/* SPDX-License-Identifier: MIT */
/*
 * RinOS compositor socket protocol.
 *
 * This header is deliberately C-compatible.  The same wire declarations are
 * consumed by the C native runtime and by the C++ compositor/desktop
 * services; keeping them here prevents the two sides from growing private
 * layouts.
 */
#ifndef RIN_SDK_COMPOSITOR_PROTOCOL_H
#define RIN_SDK_COMPOSITOR_PROTOCOL_H

#include <stdint.h>
#include "../ipc/shm_abi.h"
#include "compositor_abi.h"
#include "native_event.h"
#include "compositor_input_ring.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_COMPOSITOR_PROTOCOL_VERSION 3u
#define RIN_COMPOSITOR_ABI_VERSION 1u
#define RIN_COMPOSITOR_MIN_PROTOCOL_VERSION 2u
#define RIN_COMPOSITOR_PROTOCOL_VERSION_V2 2u
#define RIN_COMPOSITOR_PROTOCOL_VERSION_V3 3u
#define RIN_COMPOSITOR_MAGIC 0x434F4D50u /* 'COMP' */
#define RIN_COMPOSITOR_SOCKET_PATH "/run/rin/compositor.sock"
#define RIN_COMPOSITOR_MAX_BUFFERS 2u
#define RIN_COMPOSITOR_MAX_OUTPUTS 8u
#define RIN_COMPOSITOR_LATENCY_LOCK_CATEGORIES 7u

#define RIN_COMPOSITOR_FEATURE_SHM_BUFFERS   UINT64_C(0x00000001)
#define RIN_COMPOSITOR_FEATURE_INPUT_ROUTING UINT64_C(0x00000002)
#define RIN_COMPOSITOR_FEATURE_TEXT_INPUT    UINT64_C(0x00000004)
#define RIN_COMPOSITOR_FEATURE_MULTI_OUTPUT  UINT64_C(0x00000008)
#define RIN_COMPOSITOR_FEATURE_WINDOW_POLICY UINT64_C(0x00000010)
#define RIN_COMPOSITOR_FEATURE_RECONNECT     UINT64_C(0x00000020)
#define RIN_COMPOSITOR_FEATURE_SEMANTIC_CURSOR UINT64_C(0x00000040)
#define RIN_COMPOSITOR_FEATURE_ICON_METADATA UINT64_C(0x00000080)
#define RIN_COMPOSITOR_FEATURE_DESKTOP_WINDOWS UINT64_C(0x00000100)
#define RIN_COMPOSITOR_FEATURE_RESERVED_WORK_AREA UINT64_C(0x00000200)
#define RIN_COMPOSITOR_FEATURE_SHELL_ACTIONS UINT64_C(0x00000400)
#define RIN_COMPOSITOR_FEATURE_INPUT_SHARED_QUEUE UINT64_C(0x00000800)
#define RIN_COMPOSITOR_FEATURE_INPUT_WAKE_HANDLE UINT64_C(0x00001000)
#define RIN_COMPOSITOR_FEATURE_FRAME_CALLBACK UINT64_C(0x00002000)
#define RIN_COMPOSITOR_FEATURE_PRESENT_FEEDBACK UINT64_C(0x00004000)

#define RIN_COMPOSITOR_SURFACE_FLAG_CURSOR  UINT32_C(0x00000001)
#define RIN_COMPOSITOR_SURFACE_FLAG_OVERLAY UINT32_C(0x00000002)
#define RIN_COMPOSITOR_SURFACE_FLAG_OPAQUE  UINT32_C(0x00000004)
#define RIN_COMPOSITOR_SURFACE_ROLE_SHIFT 8u
#define RIN_COMPOSITOR_SURFACE_ROLE_MASK UINT32_C(0x00000f00)

enum RinCompositorSurfaceRole {
    RIN_COMPOSITOR_ROLE_NORMAL = 0,
    RIN_COMPOSITOR_ROLE_DESKTOP = 1,
    RIN_COMPOSITOR_ROLE_PANEL = 2,
    RIN_COMPOSITOR_ROLE_POPUP = 3,
    RIN_COMPOSITOR_ROLE_MENU = 4,
    RIN_COMPOSITOR_ROLE_DIALOG = 5,
    RIN_COMPOSITOR_ROLE_TOOLTIP = 6,
    RIN_COMPOSITOR_ROLE_NOTIFICATION = 7,
    RIN_COMPOSITOR_ROLE_CURSOR = 8,
};

/* Stable semantic cursor names.  The compositor owns the image and theme
 * lookup; clients only select a bounded value. */
enum RinCursorType {
    RIN_CURSOR_DEFAULT = 0,
    RIN_CURSOR_HELP = 1,
    RIN_CURSOR_PROGRESS = 2,
    RIN_CURSOR_BUSY = 3,
    RIN_CURSOR_PRECISION = 4,
    RIN_CURSOR_TEXT = 5,
    RIN_CURSOR_LINK = 6,
    RIN_CURSOR_UNAVAILABLE = 7,
    RIN_CURSOR_NOT_ALLOWED = 8,
    RIN_CURSOR_RESIZE_NS = 9,
    RIN_CURSOR_RESIZE_EW = 10,
    RIN_CURSOR_RESIZE_NWSE = 11,
    RIN_CURSOR_RESIZE_NESW = 12,
    RIN_CURSOR_MOVE = 13,
    RIN_CURSOR_ALTERNATE = 14,
    RIN_CURSOR_PEN = 15,
    RIN_CURSOR_GRAB = 16,
    RIN_CURSOR_GRABBING = 17,
    RIN_CURSOR_SLEEP = 18,
    RIN_CURSOR_EXCITED = 19,
    RIN_CURSOR_TYPE_COUNT = 20,
};

#define RIN_COMPOSITOR_WINDOW_STATE_MINIMIZED UINT32_C(0x00000001)
#define RIN_COMPOSITOR_WINDOW_STATE_MAXIMIZED UINT32_C(0x00000002)
#define RIN_COMPOSITOR_WINDOW_STATE_FULLSCREEN UINT32_C(0x00000004)
#define RIN_COMPOSITOR_WINDOW_STATE_ALWAYS_TOP UINT32_C(0x00000008)
#define RIN_COMPOSITOR_WINDOW_STATE_KNOWN_MASK UINT32_C(0x0000000f)

enum RinCompositorMsgType {
    RIN_COMPOSITOR_HELLO = 1,
    RIN_COMPOSITOR_CREATE_SURFACE = 2,
    RIN_COMPOSITOR_DESTROY_SURFACE = 3,
    RIN_COMPOSITOR_ATTACH_BUFFER = 4, /* v1 compatibility */
    RIN_COMPOSITOR_DAMAGE = 5,
    RIN_COMPOSITOR_SET_POSITION = 6,
    RIN_COMPOSITOR_SET_ZORDER = 7,
    RIN_COMPOSITOR_SET_VISIBLE = 8,
    RIN_COMPOSITOR_CURSOR_UPDATE = 9,
    RIN_COMPOSITOR_FOCUS_REQUEST = 10,
    RIN_COMPOSITOR_FRAME_ACK = 11,
    RIN_COMPOSITOR_OUTPUT_INFO = 12,
    RIN_COMPOSITOR_COMMIT = 13, /* v1 compatibility */
    RIN_COMPOSITOR_POLL_INPUT = 14,
    RIN_COMPOSITOR_SET_CLIP = 15,
    RIN_COMPOSITOR_ATTACH_BUFFERS_V2 = 16,
    RIN_COMPOSITOR_COMMIT_V2 = 17,
    RIN_COMPOSITOR_SET_SIZE = 18,
    RIN_COMPOSITOR_SET_TEXT_INPUT_STATE = 19,
    RIN_COMPOSITOR_GET_TEXT_COMPOSITION = 20,
    RIN_COMPOSITOR_ENUMERATE_OUTPUTS = 21,
    RIN_COMPOSITOR_SET_WINDOW_STATE = 22,
    RIN_COMPOSITOR_SET_POINTER_CAPTURE = 23,
    RIN_COMPOSITOR_SET_KEYBOARD_GRAB = 24,
    RIN_COMPOSITOR_SET_MODAL = 25,
    RIN_COMPOSITOR_SET_TEXT_COMPOSITION = 26,
    RIN_COMPOSITOR_GET_FRAME_INFO = 27,
    RIN_COMPOSITOR_SET_TITLE = 28,
    RIN_COMPOSITOR_POLL_INPUT_V2 = 29,
    RIN_COMPOSITOR_SET_CURSOR = 30,
    RIN_COMPOSITOR_SET_ICON = 31,
    RIN_COMPOSITOR_ENUMERATE_WINDOWS = 32,
    RIN_COMPOSITOR_WINDOW_ACTION = 33,
    RIN_COMPOSITOR_SET_RESERVED_AREA = 34,
    RIN_COMPOSITOR_GET_WORK_AREA = 35,
    RIN_COMPOSITOR_SETUP_INPUT_RING = 36,
    RIN_COMPOSITOR_GET_LATENCY_STATS = 37,
    RIN_COMPOSITOR_SET_FRAME_CALLBACK = 38,
    RIN_COMPOSITOR_GET_SCHEDULER_STATE = 39,
    RIN_COMPOSITOR_INJECT_INPUT = 40,
};

typedef struct RinCompositorHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t type;
    uint32_t payload_size;
    uint32_t request_id;
    uint32_t reserved;
} RinCompositorHeader;

/* HELLO is the only request required before feature use.  The response
 * echoes the selected protocol and the server feature bitmap. */
typedef struct RinCompositorHelloV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t min_protocol_version;
    uint32_t max_protocol_version;
    uint64_t features;
    uint32_t reserved[2];
} RinCompositorHelloV1;

typedef struct RinCompositorCreateSurface {
    uint32_t width;
    uint32_t height;
    uint32_t format; /* 0=BGRA32, 1=RGBA32 */
    int32_t x;
    int32_t y;
    uint32_t flags; /* bit0: cursor, bit1: overlay */
} RinCompositorCreateSurface;

typedef struct RinCompositorDamage {
    uint32_t surface_id;
    int32_t x;
    int32_t y;
    uint32_t w;
    uint32_t h;
} RinCompositorDamage;

typedef struct RinCompositorPosition {
    uint32_t surface_id;
    int32_t x;
    int32_t y;
} RinCompositorPosition;

typedef struct RinCompositorZOrder {
    uint32_t surface_id;
    int32_t z;
} RinCompositorZOrder;

typedef struct RinCompositorVisibility {
    uint32_t surface_id;
    uint32_t visible;
} RinCompositorVisibility;

typedef struct RinCompositorOutputInfo {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t format;
    uint64_t framebuffer_paddr;
} RinCompositorOutputInfo;

typedef struct RinCompositorAttachBuffer {
    uint32_t surface_id;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t format;
    uint64_t bytes;
    char shm_name[RIN_SHM_NAME_MAX];
} RinCompositorAttachBuffer;

typedef struct RinCompositorCommit {
    uint32_t surface_id;
    uint32_t reserved;
    uint64_t frame_sequence;
} RinCompositorCommit;

typedef struct RinCompositorAttachBuffersV2 {
    uint32_t surface_id;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t format;
    uint32_t reserved;
    uint64_t bytes[RIN_COMPOSITOR_MAX_BUFFERS];
    char shm_name[RIN_COMPOSITOR_MAX_BUFFERS][RIN_SHM_NAME_MAX];
} RinCompositorAttachBuffersV2;

typedef struct RinCompositorCommitV2 {
    uint32_t surface_id;
    uint32_t buffer_slot;
    uint32_t flags;
    uint32_t reserved;
    uint64_t frame_sequence;
} RinCompositorCommitV2;

typedef struct RinCompositorSetSize {
    uint32_t surface_id;
    uint32_t width;
    uint32_t height;
    uint32_t reserved;
} RinCompositorSetSize;

typedef struct RinCompositorTextInputState {
    uint32_t surface_id;
    uint32_t reserved;
    RinTextInputStateV1 state;
} RinCompositorTextInputState;

typedef struct RinCompositorClipRect {
    int32_t x;
    int32_t y;
    uint32_t w;
    uint32_t h;
} RinCompositorClipRect;

#define RIN_COMPOSITOR_MAX_CLIP_RECTS 8u

typedef struct RinCompositorClip {
    uint32_t surface_id;
    uint32_t rect_count;
    RinCompositorClipRect rects[RIN_COMPOSITOR_MAX_CLIP_RECTS];
} RinCompositorClip;

typedef struct RinCompositorOutputDescriptorV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t id;
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t format;
    uint32_t scale_numerator;
    uint32_t scale_denominator;
    uint32_t reserved[2];
} RinCompositorOutputDescriptorV1;

typedef struct RinCompositorOutputListV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t count;
    uint32_t reserved;
    RinCompositorOutputDescriptorV1 outputs[RIN_COMPOSITOR_MAX_OUTPUTS];
} RinCompositorOutputListV1;

typedef struct RinCompositorWindowStateV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    uint32_t state;
    uint32_t workspace;
    uint32_t reserved[2];
} RinCompositorWindowStateV1;

typedef struct RinCompositorSetTitleV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    char title[128];
    uint32_t reserved[2];
} RinCompositorSetTitleV1;

/* The runtime reports the owning executable path automatically.  A caller
 * may replace it with an explicit icon source path; the compositor stores
 * only this bounded metadata and never executes or parses the target. */
typedef struct RinCompositorSetIconV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    char executable_path[256];
    uint32_t reserved[2];
} RinCompositorSetIconV1;

typedef struct RinCompositorInputPolicyV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    uint32_t enabled;
    uint32_t reserved[2];
} RinCompositorInputPolicyV1;

typedef struct RinCompositorTextCompositionUpdateV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    RinTextCompositionV1 composition;
} RinCompositorTextCompositionUpdateV1;

typedef struct RinCompositorFrameInfoV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t frame_sequence;
    uint64_t presentation_timestamp_ms;
    uint64_t missed_frames;
    uint32_t damage_rect_count;
    uint32_t reserved[3];
} RinCompositorFrameInfoV1;

/* v3 frame pacing is an explicit opt-in.  A client may arm one or more
 * in-flight frame tokens and acknowledge the presentation through
 * RIN_COMPOSITOR_FRAME_ACK; commit requests beyond max_inflight receive the
 * retryable status instead of building unbounded damage pressure. */
typedef struct RinCompositorFrameCallbackV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id; /* zero means all surfaces owned by this client */
    uint32_t enabled;
    uint32_t max_inflight;
    uint32_t reserved0;
    uint64_t last_acked_frame;
    uint32_t reserved[2];
} RinCompositorFrameCallbackV1;

typedef struct RinCompositorFrameAckV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    uint32_t reserved0;
    uint64_t frame_sequence;
    uint32_t reserved[2];
} RinCompositorFrameAckV1;

#define RIN_COMPOSITOR_SCHEDULER_FLAG_PRIORITY_UNAVAILABLE UINT32_C(0x0001)
#define RIN_COMPOSITOR_SCHEDULER_FLAG_INVERSION_SUSPECTED UINT32_C(0x0002)
typedef struct RinCompositorSchedulerStateV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t compositor_owner_tid;
    uint64_t render_owner_tid;
    uint32_t compositor_waiters;
    uint32_t render_waiters;
    int32_t compositor_owner_policy;
    int32_t compositor_owner_priority;
    int32_t render_owner_policy;
    int32_t render_owner_priority;
    uint64_t inversion_suspicions;
    uint64_t max_wait_ns;
    uint32_t reserved[2];
} RinCompositorSchedulerStateV1;

#define RIN_COMPOSITOR_SYNTHETIC_SOURCE_ACCESSIBILITY UINT32_C(1)
typedef struct RinCompositorSyntheticInputV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    uint32_t seat_id;
    uint32_t source;
    RinGuiNativeEventV1 event;
    uint32_t reserved[2];
} RinCompositorSyntheticInputV1;

/* Read-only observability endpoint. Histograms are fixed-size so querying
 * them never allocates and does not change the input ABI. */
typedef struct RinCompositorLatencyStatsV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t reserved0[2];
    RinLatencyHistogramV1 composite;
    RinLatencyHistogramV1 present;
    RinLatencyHistogramV1 input_to_client;
    uint64_t damage_overflow_count;
    uint64_t full_frame_fallback_count;
    uint64_t input_motion_coalesced_count;
    uint64_t rate_limit_hits;
    RinLatencyHistogramV1 lock_wait;
    RinLatencyHistogramV1 lock_hold;
} RinCompositorLatencyStatsV1;

typedef struct RinCompositorPollInputV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    uint32_t reserved[2];
} RinCompositorPollInputV2;

typedef struct RinCompositorInputEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    RinGuiNativeEventV1 event;
    uint32_t reserved[2];
} RinCompositorInputEventV1;

typedef struct RinCompositorSetCursorV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    uint32_t cursor_type;
    uint32_t reserved[2];
} RinCompositorSetCursorV1;

/* Window metadata is exposed only to the authenticated desktop shell.  The
 * bounded cursor lets a shell consume more than one page without leaking the
 * owning PID or session credentials to userspace. */
#define RIN_COMPOSITOR_WINDOW_LIST_MAX 16u
#pragma pack(push, 4)
typedef struct RinCompositorWindowDescriptorV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    uint32_t state;
    uint32_t workspace;
    uint32_t output_id;
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t window_flags;
    uint64_t app_identity_id;
    char app_identity[96];
    char title[96];
    char icon_path[160];
} RinCompositorWindowDescriptorV1;

typedef struct RinCompositorWindowListV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t count;
    uint32_t next_cursor;
    uint32_t reserved[2];
    RinCompositorWindowDescriptorV1 windows[RIN_COMPOSITOR_WINDOW_LIST_MAX];
} RinCompositorWindowListV1;

enum RinCompositorWindowAction {
    RIN_COMPOSITOR_WINDOW_ACTION_ACTIVATE = 1,
    RIN_COMPOSITOR_WINDOW_ACTION_MINIMIZE = 2,
    RIN_COMPOSITOR_WINDOW_ACTION_RESTORE = 3,
    RIN_COMPOSITOR_WINDOW_ACTION_CLOSE = 4,
};

typedef struct RinCompositorWindowActionV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t surface_id;
    uint32_t action;
    uint32_t reserved[2];
} RinCompositorWindowActionV1;

typedef struct RinCompositorReservedAreaV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t output_id;
    uint32_t left;
    uint32_t top;
    uint32_t right;
    uint32_t bottom;
    uint32_t reserved[2];
} RinCompositorReservedAreaV1;

typedef struct RinCompositorWorkAreaV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t output_id;
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t reserved[2];
} RinCompositorWorkAreaV1;
#pragma pack(pop)

static inline int rin_compositor_hello_valid(const RinCompositorHelloV1* hello)
{
    return hello != 0 && hello->struct_size == sizeof(*hello) &&
           hello->version == 1u && hello->flags == 0u &&
           hello->min_protocol_version != 0u &&
           hello->min_protocol_version <= hello->max_protocol_version &&
           hello->reserved[0] == 0u && hello->reserved[1] == 0u;
}

static inline int rin_compositor_surface_role_valid(uint32_t role)
{
    return role <= RIN_COMPOSITOR_ROLE_CURSOR;
}

#ifdef __cplusplus
static_assert(sizeof(RinCompositorHeader) == 24u, "compositor header size");
static_assert(sizeof(RinCompositorHelloV1) == 32u, "compositor hello size");
static_assert(sizeof(RinCompositorAttachBuffersV2) == 168u,
              "compositor v2 attach size");
static_assert(sizeof(RinCompositorCommitV2) == 24u,
              "compositor v2 commit size");
static_assert(sizeof(RinCompositorTextInputState) == 40u,
              "compositor text-input size");
static_assert(sizeof(RinCompositorClipRect) == 16u,
              "compositor clip rect size");
static_assert(sizeof(RinCompositorOutputDescriptorV1) == 52u,
              "compositor output descriptor size");
static_assert(sizeof(RinCompositorOutputListV1) == 432u,
              "compositor output list size");
static_assert(sizeof(RinCompositorWindowStateV1) == 28u,
              "compositor window state size");
static_assert(sizeof(RinCompositorSetTitleV1) == 148u,
              "compositor title size");
static_assert(sizeof(RinCompositorSetIconV1) == 276u,
              "compositor icon metadata size");
static_assert(sizeof(RinCompositorInputPolicyV1) == 24u,
              "compositor input policy size");
static_assert(sizeof(RinCompositorFrameInfoV1) == 48u,
              "compositor frame info size");
static_assert(sizeof(RinCompositorFrameCallbackV1) == 40u,
              "compositor frame callback size");
static_assert(sizeof(RinCompositorFrameAckV1) == 32u,
              "compositor frame ack size");
static_assert(sizeof(RinCompositorSchedulerStateV1) == 72u,
              "compositor scheduler state size");
static_assert(sizeof(RinCompositorSyntheticInputV1) == 64u,
              "compositor synthetic input size");
static_assert(sizeof(RinCompositorLatencyStatsV1) == 848u,
              "compositor latency stats size");
static_assert(sizeof(RinCompositorPollInputV2) == 20u,
              "compositor v2 input poll size");
static_assert(sizeof(RinCompositorInputEventV1) == 56u,
              "compositor input event size");
static_assert(sizeof(RinCompositorSetCursorV1) == 24u,
              "compositor semantic cursor size");
static_assert(sizeof(RinCompositorWindowDescriptorV1) == 404u,
              "compositor window descriptor size");
static_assert(sizeof(RinCompositorWindowListV1) == 6488u,
              "compositor window list size");
static_assert(sizeof(RinCompositorWindowActionV1) == 24u,
              "compositor window action size");
static_assert(sizeof(RinCompositorReservedAreaV1) == 36u,
              "compositor reserved area size");
static_assert(sizeof(RinCompositorWorkAreaV1) == 36u,
              "compositor work area size");
#else
_Static_assert(sizeof(RinCompositorHeader) == 24u, "compositor header size");
_Static_assert(sizeof(RinCompositorHelloV1) == 32u, "compositor hello size");
_Static_assert(sizeof(RinCompositorAttachBuffersV2) == 168u,
               "compositor v2 attach size");
_Static_assert(sizeof(RinCompositorCommitV2) == 24u,
               "compositor v2 commit size");
_Static_assert(sizeof(RinCompositorTextInputState) == 40u,
               "compositor text-input size");
_Static_assert(sizeof(RinCompositorClipRect) == 16u,
               "compositor clip rect size");
_Static_assert(sizeof(RinCompositorOutputDescriptorV1) == 52u,
               "compositor output descriptor size");
_Static_assert(sizeof(RinCompositorOutputListV1) == 432u,
               "compositor output list size");
_Static_assert(sizeof(RinCompositorWindowStateV1) == 28u,
               "compositor window state size");
_Static_assert(sizeof(RinCompositorSetTitleV1) == 148u,
               "compositor title size");
_Static_assert(sizeof(RinCompositorSetIconV1) == 276u,
               "compositor icon metadata size");
_Static_assert(sizeof(RinCompositorInputPolicyV1) == 24u,
               "compositor input policy size");
_Static_assert(sizeof(RinCompositorFrameInfoV1) == 48u,
               "compositor frame info size");
_Static_assert(sizeof(RinCompositorFrameCallbackV1) == 40u,
               "compositor frame callback size");
_Static_assert(sizeof(RinCompositorFrameAckV1) == 32u,
               "compositor frame ack size");
_Static_assert(sizeof(RinCompositorSchedulerStateV1) == 72u,
               "compositor scheduler state size");
_Static_assert(sizeof(RinCompositorSyntheticInputV1) == 64u,
               "compositor synthetic input size");
_Static_assert(sizeof(RinCompositorLatencyStatsV1) == 848u,
               "compositor latency stats size");
_Static_assert(sizeof(RinCompositorPollInputV2) == 20u,
               "compositor v2 input poll size");
_Static_assert(sizeof(RinCompositorInputEventV1) == 56u,
               "compositor input event size");
_Static_assert(sizeof(RinCompositorSetCursorV1) == 24u,
               "compositor semantic cursor size");
_Static_assert(sizeof(RinCompositorWindowDescriptorV1) == 404u,
               "compositor window descriptor size");
_Static_assert(sizeof(RinCompositorWindowListV1) == 6488u,
               "compositor window list size");
_Static_assert(sizeof(RinCompositorWindowActionV1) == 24u,
               "compositor window action size");
_Static_assert(sizeof(RinCompositorReservedAreaV1) == 36u,
               "compositor reserved area size");
_Static_assert(sizeof(RinCompositorWorkAreaV1) == 36u,
               "compositor work area size");
#endif

#ifdef __cplusplus
}
#endif

#endif
