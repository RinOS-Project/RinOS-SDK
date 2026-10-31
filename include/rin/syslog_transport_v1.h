/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Public fixed-record syslog transport between libc and servicelogd. */
#ifndef RIN_SDK_SYSLOG_TRANSPORT_V1_H
#define RIN_SDK_SYSLOG_TRANSPORT_V1_H

#include <stddef.h>
#include <stdint.h>

#include <rin/service_log_transport_v1.h>

#define RIN_SYSLOG_TRANSPORT_VERSION 1u
#define RIN_SYSLOG_TRANSPORT_MODE_SUBMIT 4u
#define RIN_SYSLOG_TRANSPORT_SERVICE_ID "syslog"
#define RIN_SYSLOG_TRANSPORT_IDENT_MAX 64u
#define RIN_SYSLOG_TRANSPORT_MESSAGE_MAX 1024u
#define RIN_SYSLOG_TRANSPORT_STATUS_OK 0
#define RIN_SYSLOG_TRANSPORT_STATUS_INVALID 1
#define RIN_SYSLOG_TRANSPORT_STATUS_QUOTA 2
#define RIN_SYSLOG_TRANSPORT_STATUS_PERSIST_FAILED 3

typedef struct RinSyslogTransportRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t request_id;
    uint32_t facility;
    uint32_t severity;
    uint32_t message_length;
    uint32_t flags;
    char ident[RIN_SYSLOG_TRANSPORT_IDENT_MAX];
    char message[RIN_SYSLOG_TRANSPORT_MESSAGE_MAX];
    uint64_t reserved[2];
} RinSyslogTransportRequestV1;

typedef struct RinSyslogTransportReplyV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t request_id;
    int32_t status;
    uint32_t reserved1;
    uint64_t reserved[2];
} RinSyslogTransportReplyV1;

#if defined(__cplusplus)
static_assert(sizeof(RinSyslogTransportRequestV1) == 1136u,
              "RinSyslogTransportRequestV1 ABI drift");
static_assert(offsetof(RinSyslogTransportRequestV1, message) == 96u,
              "RinSyslogTransportRequestV1.message ABI drift");
static_assert(sizeof(RinSyslogTransportReplyV1) == 40u,
              "RinSyslogTransportReplyV1 ABI drift");
#else
_Static_assert(sizeof(RinSyslogTransportRequestV1) == 1136u,
               "RinSyslogTransportRequestV1 ABI drift");
_Static_assert(offsetof(RinSyslogTransportRequestV1, message) == 96u,
               "RinSyslogTransportRequestV1.message ABI drift");
_Static_assert(sizeof(RinSyslogTransportReplyV1) == 40u,
               "RinSyslogTransportReplyV1 ABI drift");
#endif

#endif /* RIN_SDK_SYSLOG_TRANSPORT_V1_H */
