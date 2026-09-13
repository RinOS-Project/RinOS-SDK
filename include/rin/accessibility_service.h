/* SPDX-License-Identifier: MIT */
/* Bounded local desktop-accessibility service wire contract. */

#ifndef RIN_SDK_ACCESSIBILITY_SERVICE_H
#define RIN_SDK_ACCESSIBILITY_SERVICE_H

#include <stdint.h>

#define RIN_ACCESSIBILITY_SERVICE_ABI_VERSION UINT16_C(2)
#define RIN_ACCESSIBILITY_SERVICE_SOCKET_PATH "/run/rin/accessibilityd.sock"

#define RIN_ACCESSIBILITY_SERVICE_OP_PUBLISH UINT16_C(1)
#define RIN_ACCESSIBILITY_SERVICE_OP_QUERY   UINT16_C(2)
#define RIN_ACCESSIBILITY_SERVICE_OP_REVOKE  UINT16_C(3)
#define RIN_ACCESSIBILITY_SERVICE_OP_ACTION  UINT16_C(4)

#define RIN_ACCESSIBILITY_SERVICE_MAX_NODES UINT32_C(128)
#define RIN_ACCESSIBILITY_SERVICE_MAX_NAME_BYTES UINT16_C(192)
#define RIN_ACCESSIBILITY_SERVICE_MAX_DESCRIPTION_BYTES UINT16_C(192)
#define RIN_ACCESSIBILITY_SERVICE_MAX_VALUE_BYTES UINT16_C(192)

#define RIN_ACCESSIBILITY_SERVICE_NODE_STATE_MASK UINT32_C(0x000007ff)
#define RIN_ACCESSIBILITY_SERVICE_NODE_ACTION_MASK UINT32_C(0x000007ff)
#define RIN_ACCESSIBILITY_SERVICE_NODE_STATE_EDITABLE UINT32_C(0x00000010)
#define RIN_ACCESSIBILITY_SERVICE_ACTION_SET_VALUE UINT32_C(0x00000004)
#define RIN_ACCESSIBILITY_SERVICE_MAX_ROLE UINT16_C(18)

typedef struct RinAccessibilityServiceMessageHeaderV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t opcode;
    uint64_t request_id;
    uint32_t payload_size;
    uint32_t flags;
    int32_t status;
    uint32_t reserved;
} RinAccessibilityServiceMessageHeaderV1;

typedef struct RinAccessibilityWireTextV1 {
    uint16_t size;
    uint16_t reserved;
    uint8_t bytes[RIN_ACCESSIBILITY_SERVICE_MAX_VALUE_BYTES];
} RinAccessibilityWireTextV1;

typedef struct RinAccessibilityWireNodeV1 {
    uint64_t id;
    uint64_t parent_id;
    uint16_t role;
    uint16_t reserved0;
    uint32_t state;
    uint32_t actions;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    uint32_t cursor_start;
    uint32_t cursor_end;
    uint32_t selection_start;
    uint32_t selection_end;
    uint32_t editable_start;
    uint32_t editable_end;
    RinAccessibilityWireTextV1 name;
    RinAccessibilityWireTextV1 description;
    RinAccessibilityWireTextV1 value;
    uint64_t reserved[2];
} RinAccessibilityWireNodeV1;

typedef struct RinAccessibilityWireSnapshotV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t window;
    uint64_t generation;
    uint32_t node_count;
    uint32_t reserved0;
    RinAccessibilityWireNodeV1 nodes[RIN_ACCESSIBILITY_SERVICE_MAX_NODES];
} RinAccessibilityWireSnapshotV1;

typedef struct RinAccessibilityServiceQueryV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t target_process_id;
    uint64_t target_process_instance_cookie;
    uint64_t target_window;
    uint64_t reserved[2];
} RinAccessibilityServiceQueryV1;

/* An action is bound to the immutable tree generation that exposed the node.
 * `action` contains exactly one advertised AccessibilityAction bit.  Version
 * two adds a fixed-size UTF-8 replacement value for SET_VALUE.  The value is
 * required to be all-zero for every other action, so providers never need to
 * interpret an unexpected payload. */
typedef struct RinAccessibilityServiceActionV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t target_process_id;
    uint64_t target_process_instance_cookie;
    uint64_t target_window;
    uint64_t snapshot_generation;
    uint64_t target_node_id;
    uint32_t action;
    uint32_t reserved0;
    RinAccessibilityWireTextV1 value;
    uint32_t reserved1;
} RinAccessibilityServiceActionV2;

#if defined(__cplusplus)
static_assert(sizeof(RinAccessibilityServiceMessageHeaderV1) == 32u,
              "RinAccessibilityServiceMessageHeaderV1 ABI drift");
static_assert(sizeof(RinAccessibilityWireTextV1) == 196u,
              "RinAccessibilityWireTextV1 ABI drift");
static_assert(sizeof(RinAccessibilityServiceQueryV1) == 48u,
              "RinAccessibilityServiceQueryV1 ABI drift");
static_assert(sizeof(RinAccessibilityServiceActionV2) == 256u,
              "RinAccessibilityServiceActionV2 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinAccessibilityServiceMessageHeaderV1) == 32u,
               "RinAccessibilityServiceMessageHeaderV1 ABI drift");
_Static_assert(sizeof(RinAccessibilityWireTextV1) == 196u,
               "RinAccessibilityWireTextV1 ABI drift");
_Static_assert(sizeof(RinAccessibilityServiceQueryV1) == 48u,
               "RinAccessibilityServiceQueryV1 ABI drift");
_Static_assert(sizeof(RinAccessibilityServiceActionV2) == 256u,
               "RinAccessibilityServiceActionV2 ABI drift");
#endif

#endif /* RIN_SDK_ACCESSIBILITY_SERVICE_H */


