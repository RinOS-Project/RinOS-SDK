// SPDX-License-Identifier: MIT
#ifndef RIN_SDK_WEB_WORKERD_PROTOCOL_H
#define RIN_SDK_WEB_WORKERD_PROTOCOL_H

#include <stdint.h>
#include "../ipc/shm_abi.h"
#include "request_body_capability.h"

#define RIN_WORKERD_SERVICE_ID "workerd"
#define RIN_WORKERD_MAGIC 0x57525632u /* "WRV2" */
#define RIN_WORKERD_VERSION 3u
#define RIN_WORKERD_INLINE_THRESHOLD 16384u

enum {
    RIN_WORKERD_CMD_FETCH_V1 = 1,
    RIN_WORKERD_CMD_DECODE_IMAGE_V1 = 2,
    RIN_WORKERD_CMD_CANCEL_V1 = 3
};

enum {
    RIN_WORKERD_PAYLOAD_INLINE = 0,
    RIN_WORKERD_PAYLOAD_SHM = 1,
    /* Reserved for the authenticated opaque capability path.  v3 callers
     * must not select it until RequestServer has a token resolver. */
    RIN_WORKERD_PAYLOAD_CAPABILITY = 2
};

typedef RinRequestBodyCapabilityV1 RinWorkerdBodyCapability;

enum {
    RIN_WORKERD_FETCH_FLAG_FORCE_CONNECTION_CLOSE = 1u << 0,
    RIN_WORKERD_FETCH_FLAG_FORCE_IDENTITY_ENCODING = 1u << 1
};

enum {
    RIN_WORKERD_FETCH_CLASS_DEFAULT = 0,
    RIN_WORKERD_FETCH_CLASS_SCRIPT = 1,
    RIN_WORKERD_FETCH_CLASS_STYLESHEET = 2,
    RIN_WORKERD_FETCH_CLASS_IMAGE = 3
};

typedef struct RinWorkerdMsgHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t command;
    int32_t status;
    uint32_t request_id;
    uint32_t payload_len;
    uint32_t reserved0;
    uint32_t reserved1;
} RinWorkerdMsgHeader;

typedef struct RinWorkerdShmRegion {
    uint32_t size;
    char name[RIN_SHM_NAME_MAX];
} RinWorkerdShmRegion;

typedef struct RinWorkerdFetchRequest {
    uint32_t timeout_ms;
    uint32_t connect_header_timeout_ms;
    uint32_t body_idle_timeout_ms;
    uint32_t follow_redirects;
    uint32_t options_flags;
    uint32_t request_class;
    uint32_t method_len;
    uint32_t url_len;
    uint32_t header_count;
    uint32_t body_len;
    uint32_t body_storage_kind;
    RinWorkerdShmRegion body_region;
} RinWorkerdFetchRequest;

typedef struct RinWorkerdFetchHeaderField {
    uint32_t key_len;
    uint32_t value_len;
} RinWorkerdFetchHeaderField;

typedef struct RinWorkerdFetchResponse {
    int32_t status_code;
    uint32_t transfer_complete;
    uint32_t success;
    int32_t io_error_kind;
    uint32_t status_text_len;
    uint32_t final_url_len;
    uint32_t error_len;
    uint32_t header_count;
    uint32_t body_len;
    uint32_t body_storage_kind;
    RinWorkerdShmRegion body_region;
} RinWorkerdFetchResponse;

typedef struct RinWorkerdDecodeImageRequest {
    uint32_t url_len;
    uint32_t body_len;
    uint32_t body_storage_kind;
    RinWorkerdShmRegion body_region;
} RinWorkerdDecodeImageRequest;

typedef struct RinWorkerdDecodeImageResponse {
    uint32_t width;
    uint32_t height;
    uint32_t pixel_count;
    uint32_t image_type;
    uint32_t error_len;
    uint32_t internal_reason_len;
    uint32_t internal_chunk_len;
    uint32_t loaded;
    uint32_t pixel_storage_kind;
    RinWorkerdShmRegion pixel_region;
} RinWorkerdDecodeImageResponse;

typedef struct RinWorkerdCancelRequest {
    uint32_t request_id;
} RinWorkerdCancelRequest;

#endif
