// SPDX-License-Identifier: MIT
#ifndef RIN_SDK_WEB_REQUESTSERVER_PROTOCOL_H
#define RIN_SDK_WEB_REQUESTSERVER_PROTOCOL_H

#include <stdint.h>

#define RIN_REQUESTSERVER_MAGIC 0x52535131u /* "RSQ1" */
#define RIN_REQUESTSERVER_VERSION 1u

enum {
    RIN_REQUESTSERVER_CMD_PING_V1 = 1,
    RIN_REQUESTSERVER_CMD_GET_STATUS_V1 = 2
};

typedef struct RinRequestServerMsgHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t command;
    int32_t status;
    uint32_t request_id;
    uint32_t payload_len;
    uint32_t reserved0;
    uint32_t reserved1;
} RinRequestServerMsgHeader;

typedef struct RinRequestServerStatusV1 {
    uint32_t flags;
    uint32_t protocol_version;
    char backend_name[64];
    char socket_path[108];
} RinRequestServerStatusV1;

enum {
    RIN_REQUESTSERVER_STATUS_FLAG_READY = 1u << 0,
    RIN_REQUESTSERVER_STATUS_FLAG_LEGACY_FETCH_BACKEND = 1u << 1
};

#endif
