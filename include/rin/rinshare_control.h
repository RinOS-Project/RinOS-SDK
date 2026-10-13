/* SPDX-License-Identifier: MIT */
/* Public v1 control ABI for the authenticated RinShare service. */
#ifndef RIN_SDK_RINSHARE_CONTROL_H
#define RIN_SDK_RINSHARE_CONTROL_H

#include <stdint.h>

#include "file_portal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_SHARE_CONTROL_VERSION 1u
#define RIN_SHARE_CONTROL_VERSION_2 2u
#define RIN_SHARE_CONTROL_SERVICE_ID "rinshare-control"
#define RIN_SHARE_CONTROL_SOCKET_PATH "/run/rin/rinshare-control.sock"
#define RIN_SHARE_CONTROL_MAGIC UINT32_C(0x31435352) /* RSC1 */

enum RinShareControlOpcode {
    RIN_SHARE_CONTROL_SET_DIRECTORY_GRANT = 1u,
    RIN_SHARE_CONTROL_SET_APPROVAL = 2u,
    RIN_SHARE_CONTROL_REVOKE_DIRECTORY = 3u,
    RIN_SHARE_CONTROL_GET_PENDING_OFFER = 4u,
    RIN_SHARE_CONTROL_CLEAR_PENDING_OFFER = 5u
    ,RIN_SHARE_CONTROL_SET_POLICY = 6u
    ,RIN_SHARE_CONTROL_GET_POLICY = 7u
    ,RIN_SHARE_CONTROL_LIST_DEVICE = 8u
    ,RIN_SHARE_CONTROL_GET_HISTORY = 9u
};

enum RinShareControlVisibilityV1 {
    RIN_SHARE_CONTROL_VISIBILITY_OFF = 0u,
    RIN_SHARE_CONTROL_VISIBILITY_LIMITED = 1u,
    RIN_SHARE_CONTROL_VISIBILITY_ALL = 2u
};

enum RinShareControlHistoryResultV1 {
    RIN_SHARE_CONTROL_HISTORY_PENDING = 1,
    RIN_SHARE_CONTROL_HISTORY_APPROVED = 2,
    RIN_SHARE_CONTROL_HISTORY_REJECTED = 3
};

#define RIN_SHARE_CONTROL_MAX_DEVICES 16u
#define RIN_SHARE_CONTROL_MAX_HISTORY 32u

typedef struct RinShareControlPolicyV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t visibility;
    uint32_t flags;
    uint64_t expires_at_ms;
    uint64_t generation;
    uint64_t reserved[2];
} RinShareControlPolicyV1;

typedef struct RinShareControlDeviceEntryV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t generation;
    uint64_t last_seen_at_ms;
    uint8_t peer_fingerprint[32];
    char display_name[64];
    uint64_t reserved[2];
} RinShareControlDeviceEntryV1;

typedef struct RinShareControlHistoryEntryV1 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t recorded_at_ms;
    uint64_t transfer_id;
    uint8_t peer_fingerprint[32];
    char display_name[64];
    uint32_t file_count;
    uint32_t reserved1;
    uint64_t total_bytes;
    uint64_t reserved[2];
} RinShareControlHistoryEntryV1;

typedef struct RinShareControlResponseV2 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    RinShareControlPolicyV1 policy;
    uint32_t count;
    uint32_t next_cursor;
    uint64_t reserved1;
    RinShareControlDeviceEntryV1 device;
    RinShareControlHistoryEntryV1 history;
    uint64_t reserved[2];
} RinShareControlResponseV2;

typedef struct RinShareControlMessageV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t magic;
    uint32_t opcode;
    uint64_t grant_id;
    uint32_t object_type;
    uint32_t rights;
    uint32_t flags;
    uint32_t reserved0;
    uint8_t peer_fingerprint[32];
    uint64_t transfer_id;
    uint64_t approval_generation;
    uint64_t approval_expires_at_ms;
    uint64_t reserved[2];
} RinShareControlMessageV1;

typedef struct RinShareControlPendingOfferV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t available;
    uint32_t reserved0;
    uint64_t generation;
    uint64_t transfer_id;
    uint64_t expires_at_ms;
    uint8_t peer_fingerprint[32];
    char display_name[64];
    uint32_t file_count;
    uint32_t reserved1;
    uint64_t total_bytes;
    uint64_t reserved[2];
} RinShareControlPendingOfferV1;

typedef struct RinShareControlResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    RinShareControlPendingOfferV1 pending;
    uint64_t reserved[2];
} RinShareControlResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinShareControlMessageV1) == 112u,
              "RinShareControlMessageV1 ABI drift");
static_assert(sizeof(RinShareControlPendingOfferV1) == 168u,
              "RinShareControlPendingOfferV1 ABI drift");
static_assert(sizeof(RinShareControlResponseV1) == 208u,
              "RinShareControlResponseV1 ABI drift");
static_assert(sizeof(RinShareControlPolicyV1) == 48u,
              "RinShareControlPolicyV1 ABI drift");
static_assert(sizeof(RinShareControlDeviceEntryV1) == 144u,
              "RinShareControlDeviceEntryV1 ABI drift");
static_assert(sizeof(RinShareControlHistoryEntryV1) == 160u,
              "RinShareControlHistoryEntryV1 ABI drift");
static_assert(sizeof(RinShareControlResponseV2) == 408u,
              "RinShareControlResponseV2 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinShareControlMessageV1) == 112u,
               "RinShareControlMessageV1 ABI drift");
_Static_assert(sizeof(RinShareControlPendingOfferV1) == 168u,
               "RinShareControlPendingOfferV1 ABI drift");
_Static_assert(sizeof(RinShareControlResponseV1) == 208u,
               "RinShareControlResponseV1 ABI drift");
_Static_assert(sizeof(RinShareControlPolicyV1) == 48u,
               "RinShareControlPolicyV1 ABI drift");
_Static_assert(sizeof(RinShareControlDeviceEntryV1) == 144u,
               "RinShareControlDeviceEntryV1 ABI drift");
_Static_assert(sizeof(RinShareControlHistoryEntryV1) == 160u,
               "RinShareControlHistoryEntryV1 ABI drift");
_Static_assert(sizeof(RinShareControlResponseV2) == 408u,
               "RinShareControlResponseV2 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_RINSHARE_CONTROL_H */
