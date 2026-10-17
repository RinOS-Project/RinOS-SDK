/* SPDX-License-Identifier: GPL-2.0-or-later */
/* USB HID framing for CTAP 2 authenticators. */

#ifndef RIN_KERNEL_RIN_CTAPHID_H
#define RIN_KERNEL_RIN_CTAPHID_H

#include <stdint.h>

#include "rin/contract_abi.h"
#define RIN_CTAPHID_REPORT_SIZE UINT32_C(64)
#define RIN_CTAPHID_INIT_DATA_SIZE UINT32_C(57)
#define RIN_CTAPHID_CONT_DATA_SIZE UINT32_C(59)
#define RIN_CTAPHID_MESSAGE_MAX UINT32_C(4096)
#define RIN_CTAPHID_BROADCAST_CID UINT32_C(0xFFFFFFFF)

#define RIN_CTAPHID_PING UINT8_C(0x01)
#define RIN_CTAPHID_MSG UINT8_C(0x03)
#define RIN_CTAPHID_LOCK UINT8_C(0x04)
#define RIN_CTAPHID_INIT UINT8_C(0x06)
#define RIN_CTAPHID_WINK UINT8_C(0x08)
#define RIN_CTAPHID_CBOR UINT8_C(0x10)
#define RIN_CTAPHID_CANCEL UINT8_C(0x11)
#define RIN_CTAPHID_KEEPALIVE UINT8_C(0x3B)
#define RIN_CTAPHID_ERROR UINT8_C(0x3F)

#define RIN_CTAPHID_FEED_INCOMPLETE UINT32_C(0)
#define RIN_CTAPHID_FEED_COMPLETE UINT32_C(1)

#define RIN_CTAPHID_CAPABILITY_WINK UINT32_C(0x01)
#define RIN_CTAPHID_CAPABILITY_CBOR UINT32_C(0x04)
#define RIN_CTAPHID_CAPABILITY_NMSG UINT32_C(0x08)

#define RIN_CTAPHID_KEEPALIVE_PROCESSING UINT32_C(1)
#define RIN_CTAPHID_KEEPALIVE_UP_NEEDED UINT32_C(2)

#define RIN_CTAPHID_CLIENT_DISCONNECTED UINT32_C(0)
#define RIN_CTAPHID_CLIENT_ALLOCATING UINT32_C(1)
#define RIN_CTAPHID_CLIENT_IDLE UINT32_C(2)
#define RIN_CTAPHID_CLIENT_WAITING UINT32_C(3)
#define RIN_CTAPHID_CLIENT_CANCELING UINT32_C(4)

#define RIN_CTAPHID_CLIENT_EVENT_NONE UINT32_C(0)
#define RIN_CTAPHID_CLIENT_EVENT_READY UINT32_C(1)
#define RIN_CTAPHID_CLIENT_EVENT_KEEPALIVE UINT32_C(2)
#define RIN_CTAPHID_CLIENT_EVENT_RESPONSE UINT32_C(3)
#define RIN_CTAPHID_CLIENT_EVENT_CANCELED UINT32_C(4)

#define RIN_CTAPHID_TRANSACTION_TIMEOUT_MAX_MS UINT32_C(120000)

typedef struct RinCtapHidMessage {
    uint32_t channel;
    uint8_t command;
    uint8_t reserved[3];
    uint32_t size;
    uint8_t data[RIN_CTAPHID_MESSAGE_MAX];
} RinCtapHidMessage;

typedef struct RinCtapHidAssembler {
    uint32_t channel;
    uint32_t expected_size;
    uint32_t received_size;
    uint8_t command;
    uint8_t next_sequence;
    uint8_t active;
    uint8_t reserved;
    uint8_t data[RIN_CTAPHID_MESSAGE_MAX];
} RinCtapHidAssembler;

typedef struct RinCtapHidClient {
    RinCtapHidAssembler assembler;
    uint64_t deadline_ms;
    uint32_t channel;
    uint32_t state;
    uint8_t nonce[8];
    uint8_t protocol_version;
    uint8_t device_major;
    uint8_t device_minor;
    uint8_t device_build;
    uint8_t capabilities;
    uint8_t keepalive_status;
    uint8_t expected_command;
    uint8_t reserved;
} RinCtapHidClient;

typedef RinResultCode (*RinCtapHidEmitReport)(
    void* context, const uint8_t report[RIN_CTAPHID_REPORT_SIZE]);

void rin_ctaphid_assembler_init(RinCtapHidAssembler* assembler);
void rin_ctaphid_assembler_cancel(RinCtapHidAssembler* assembler);
RinResultCode rin_ctaphid_feed(
    RinCtapHidAssembler* assembler,
    const uint8_t report[RIN_CTAPHID_REPORT_SIZE],
    RinCtapHidMessage* message, uint32_t* feed_state);
RinResultCode rin_ctaphid_fragment(uint32_t channel, uint8_t command,
                                   const uint8_t* payload,
                                   uint32_t payload_size,
                                   RinCtapHidEmitReport emit,
                                   void* emit_context);

void rin_ctaphid_client_init(RinCtapHidClient* client);
void rin_ctaphid_client_disconnect(RinCtapHidClient* client);
RinResultCode rin_ctaphid_client_begin_init(
    RinCtapHidClient* client, const uint8_t nonce[8], uint64_t now_ms,
    uint32_t timeout_ms, RinCtapHidEmitReport emit, void* emit_context);
RinResultCode rin_ctaphid_client_begin_cbor(
    RinCtapHidClient* client, const uint8_t* payload, uint32_t payload_size,
    uint64_t now_ms, uint32_t timeout_ms, RinCtapHidEmitReport emit,
    void* emit_context);
RinResultCode rin_ctaphid_client_cancel(RinCtapHidClient* client,
                                        RinCtapHidEmitReport emit,
                                        void* emit_context);
RinResultCode rin_ctaphid_client_feed(
    RinCtapHidClient* client,
    const uint8_t report[RIN_CTAPHID_REPORT_SIZE], uint64_t now_ms,
    RinCtapHidMessage* message, uint32_t* event);
RinResultCode rin_ctaphid_client_poll(RinCtapHidClient* client,
                                      uint64_t now_ms);

#endif /* RIN_KERNEL_RIN_CTAPHID_H */
