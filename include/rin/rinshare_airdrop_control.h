/* SPDX-License-Identifier: MIT */
/* Versioned AirDrop receive-consent ABI; separate from authenticated RinShare. */
#ifndef RIN_SDK_RINSHARE_AIRDROP_CONTROL_H
#define RIN_SDK_RINSHARE_AIRDROP_CONTROL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_SHARE_CONTROL_VERSION_AIRDROP 7u
#define RIN_SHARE_CONTROL_AIRDROP_GET_PENDING 13u
#define RIN_SHARE_CONTROL_AIRDROP_DECIDE 14u
#define RIN_SHARE_CONTROL_AIRDROP_GET_DIRECTORY_STATE 15u
#define RIN_SHARE_AIRDROP_CONTROL_VERSION 1u
#define RIN_SHARE_AIRDROP_CONTROL_MAX_FILES 32u
#define RIN_SHARE_AIRDROP_CONTROL_MAX_NAME 255u
#define RIN_SHARE_AIRDROP_CONTROL_MAX_TEXT 95u
#define RIN_SHARE_AIRDROP_CONTROL_MAX_TOTAL_BYTES (UINT64_C(1) << 30)

enum RinShareAirDropControlDecisionKindV1 {
    RIN_SHARE_AIRDROP_CONTROL_DECISION_NONE = 0u,
    RIN_SHARE_AIRDROP_CONTROL_DECISION_ACCEPT = 1u,
    RIN_SHARE_AIRDROP_CONTROL_DECISION_DENY = 2u
};

enum RinShareAirDropControlSenderAuthenticationV1 {
    RIN_SHARE_AIRDROP_CONTROL_SENDER_UNVERIFIED = 1u
};

#pragma pack(push, 1)
typedef struct RinShareAirDropControlFileV1 {
    char name[RIN_SHARE_AIRDROP_CONTROL_MAX_NAME + 1u];
} RinShareAirDropControlFileV1;

typedef struct RinShareAirDropControlPendingV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t available;
    uint32_t sender_authentication;
    uint64_t request_id;
    uint64_t generation;
    uint64_t expires_at_ms;
    uint64_t maximum_total_bytes;
    uint32_t interface_id;
    uint32_t file_count;
    uint64_t device_generation;
    char transfer_id[37];
    char sender_computer_name[RIN_SHARE_AIRDROP_CONTROL_MAX_TEXT + 1u];
    char sender_model_name[RIN_SHARE_AIRDROP_CONTROL_MAX_TEXT + 1u];
    RinShareAirDropControlFileV1 files[RIN_SHARE_AIRDROP_CONTROL_MAX_FILES];
    uint64_t reserved[2];
} RinShareAirDropControlPendingV1;

typedef struct RinShareAirDropControlResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    RinShareAirDropControlPendingV1 pending;
    uint64_t reserved[2];
} RinShareAirDropControlResponseV1;

typedef struct RinShareAirDropControlDecisionV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t request_id;
    uint64_t generation;
    uint32_t decision;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinShareAirDropControlDecisionV1;

typedef struct RinShareAirDropControlAckV1 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t reserved0;
    uint64_t generation;
    uint64_t reserved[2];
} RinShareAirDropControlAckV1;

typedef struct RinShareAirDropControlDirectoryStateV1 {
    uint32_t struct_size;
    uint32_t version;
    int32_t result;
    uint32_t available;
    uint64_t generation;
    uint64_t reserved[2];
} RinShareAirDropControlDirectoryStateV1;
#pragma pack(pop)

static inline int rin_share_airdrop_control_text_valid(
    const char* text, size_t capacity, int allow_empty)
{
    size_t index;
    if (text == NULL || capacity == 0u) return 0;
    for (index = 0u; index < capacity; ++index) {
        const unsigned char value = (unsigned char)text[index];
        if (value == '\0') {
            size_t tail;
            if (index == 0u && allow_empty == 0) return 0;
            for (tail = index + 1u; tail < capacity; ++tail)
                if (text[tail] != '\0') return 0;
            return 1;
        }
        if (value < 0x80u) {
            if (value < 0x20u || value == 0x7fu) return 0;
            continue;
        }
        {
            uint32_t codepoint;
            size_t width;
            unsigned char first = value;
            if (first >= 0xc2u && first <= 0xdfu) {
                codepoint = first & 0x1fu;
                width = 2u;
            } else if (first >= 0xe0u && first <= 0xefu) {
                codepoint = first & 0x0fu;
                width = 3u;
            } else if (first >= 0xf0u && first <= 0xf4u) {
                codepoint = first & 0x07u;
                width = 4u;
            } else {
                return 0;
            }
            if (width > capacity - index) return 0;
            for (size_t continuation = 1u; continuation < width;
                 ++continuation) {
                const unsigned char byte =
                    (unsigned char)text[index + continuation];
                if (byte == '\0' || (byte & 0xc0u) != 0x80u) return 0;
                codepoint = (codepoint << 6u) | (byte & 0x3fu);
            }
            if ((width == 2u && codepoint < 0x80u) ||
                (width == 3u && codepoint < 0x800u) ||
                (width == 4u && codepoint < 0x10000u) ||
                codepoint > 0x10ffffu ||
                (codepoint >= 0xd800u && codepoint <= 0xdfffu) ||
                (codepoint >= 0x7fu && codepoint <= 0x9fu) ||
                (first == 0xe0u &&
                 (unsigned char)text[index + 1u] < 0xa0u) ||
                (first == 0xedu &&
                 (unsigned char)text[index + 1u] >= 0xa0u) ||
                (first == 0xf0u &&
                 (unsigned char)text[index + 1u] < 0x90u) ||
                (first == 0xf4u &&
                 (unsigned char)text[index + 1u] >= 0x90u))
                return 0;
            index += width - 1u;
        }
    }
    return 0;
}

static inline int rin_share_airdrop_control_file_valid(
    const RinShareAirDropControlFileV1* file)
{
    size_t index;
    if (file == NULL ||
        !rin_share_airdrop_control_text_valid(
            file->name, sizeof(file->name), 0))
        return 0;
    if (file->name[0] == '.' &&
        (file->name[1] == '\0' ||
         (file->name[1] == '.' && file->name[2] == '\0')))
        return 0;
    for (index = 0u; file->name[index] != '\0'; ++index)
        if (file->name[index] == '/' || file->name[index] == '\\' ||
            file->name[index] == ':')
            return 0;
    return 1;
}

static inline int rin_share_airdrop_control_pending_valid(
    const RinShareAirDropControlPendingV1* pending)
{
    uint32_t index;
    if (pending == NULL || pending->struct_size != sizeof(*pending) ||
        pending->version != RIN_SHARE_AIRDROP_CONTROL_VERSION ||
        pending->available > 1u || pending->reserved[0] != 0u ||
        pending->reserved[1] != 0u)
        return 0;
    if (pending->available == 0u) {
        const uint8_t* bytes = (const uint8_t*)pending;
        size_t offset;
        for (offset = 8u; offset < sizeof(*pending); ++offset)
            if (bytes[offset] != 0u) return 0;
        return 1;
    }
    if (pending->sender_authentication !=
            RIN_SHARE_AIRDROP_CONTROL_SENDER_UNVERIFIED ||
        pending->request_id == 0u || pending->request_id == UINT64_MAX ||
        pending->generation == 0u || pending->generation == UINT64_MAX ||
        pending->expires_at_ms == 0u ||
        pending->expires_at_ms == UINT64_MAX ||
        pending->maximum_total_bytes == 0u ||
        pending->maximum_total_bytes >
            RIN_SHARE_AIRDROP_CONTROL_MAX_TOTAL_BYTES ||
        pending->interface_id == 0u || pending->device_generation == 0u ||
        pending->device_generation == UINT64_MAX ||
        pending->file_count == 0u ||
        pending->file_count > RIN_SHARE_AIRDROP_CONTROL_MAX_FILES ||
        !rin_share_airdrop_control_text_valid(
            pending->transfer_id, sizeof(pending->transfer_id), 0) ||
        !rin_share_airdrop_control_text_valid(
            pending->sender_computer_name,
            sizeof(pending->sender_computer_name), 1) ||
        !rin_share_airdrop_control_text_valid(
            pending->sender_model_name,
            sizeof(pending->sender_model_name), 1))
        return 0;
    for (index = 0u; index < pending->file_count; ++index)
        if (!rin_share_airdrop_control_file_valid(&pending->files[index]))
            return 0;
    for (; index < RIN_SHARE_AIRDROP_CONTROL_MAX_FILES; ++index) {
        const uint8_t* bytes =
            (const uint8_t*)&pending->files[index];
        size_t offset;
        for (offset = 0u; offset < sizeof(pending->files[index]); ++offset)
            if (bytes[offset] != 0u) return 0;
    }
    return 1;
}

static inline int rin_share_airdrop_control_response_valid(
    const RinShareAirDropControlResponseV1* response)
{
    return response != NULL && response->struct_size == sizeof(*response) &&
        response->version == RIN_SHARE_AIRDROP_CONTROL_VERSION &&
        response->result == 0 && response->reserved0 == 0u &&
        rin_share_airdrop_control_pending_valid(&response->pending) &&
        ((response->pending.available == 0u &&
          response->generation != UINT64_MAX) ||
         (response->pending.available == 1u &&
          response->generation == response->pending.generation)) &&
        response->reserved[0] == 0u && response->reserved[1] == 0u;
}

static inline int rin_share_airdrop_control_decision_valid(
    const RinShareAirDropControlDecisionV1* decision)
{
    return decision != NULL && decision->struct_size == sizeof(*decision) &&
        decision->version == RIN_SHARE_AIRDROP_CONTROL_VERSION &&
        decision->request_id != 0u && decision->request_id != UINT64_MAX &&
        decision->generation != 0u && decision->generation != UINT64_MAX &&
        (decision->decision == RIN_SHARE_AIRDROP_CONTROL_DECISION_ACCEPT ||
         decision->decision == RIN_SHARE_AIRDROP_CONTROL_DECISION_DENY) &&
        decision->reserved0 == 0u && decision->reserved[0] == 0u &&
        decision->reserved[1] == 0u;
}

static inline int rin_share_airdrop_control_ack_valid(
    const RinShareAirDropControlAckV1* response)
{
    return response != NULL && response->struct_size == sizeof(*response) &&
        response->version == RIN_SHARE_AIRDROP_CONTROL_VERSION &&
        response->result == 0 && response->reserved0 == 0u &&
        response->generation != 0u && response->generation != UINT64_MAX &&
        response->reserved[0] == 0u && response->reserved[1] == 0u;
}

static inline int rin_share_airdrop_control_directory_state_valid(
    const RinShareAirDropControlDirectoryStateV1* response)
{
    return response != NULL && response->struct_size == sizeof(*response) &&
        response->version == RIN_SHARE_AIRDROP_CONTROL_VERSION &&
        response->result == 0 && response->available <= 1u &&
        ((response->available == 0u && response->generation != UINT64_MAX) ||
         (response->available == 1u && response->generation != 0u &&
          response->generation != UINT64_MAX)) &&
        response->reserved[0] == 0u && response->reserved[1] == 0u;
}
#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_RINSHARE_AIRDROP_CONTROL_H */
