/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <rin/driver/usb/fido/ctaphid.h>

#include <stddef.h>

static void hid_zero(void* destination, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)destination;
    if (!destination) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static uint32_t read_be32(const uint8_t* bytes)
{
    return ((uint32_t)bytes[0] << 24u) | ((uint32_t)bytes[1] << 16u) |
           ((uint32_t)bytes[2] << 8u) | bytes[3];
}

static void write_be32(uint8_t* bytes, uint32_t value)
{
    bytes[0] = (uint8_t)(value >> 24u);
    bytes[1] = (uint8_t)(value >> 16u);
    bytes[2] = (uint8_t)(value >> 8u);
    bytes[3] = (uint8_t)value;
}

static void copy_chunk(uint8_t* destination, const uint8_t* source,
                       uint32_t size)
{
    uint32_t index;
    for (index = 0u; index < size; ++index) destination[index] = source[index];
}

void rin_ctaphid_assembler_init(RinCtapHidAssembler* assembler)
{
    if (assembler) hid_zero(assembler, sizeof(*assembler));
}

void rin_ctaphid_assembler_cancel(RinCtapHidAssembler* assembler)
{
    rin_ctaphid_assembler_init(assembler);
}

static RinResultCode finish_if_complete(RinCtapHidAssembler* assembler,
                                        RinCtapHidMessage* message,
                                        uint32_t* feed_state)
{
    if (assembler->received_size != assembler->expected_size)
        return RIN_RESULT_OK;
    message->channel = assembler->channel;
    message->command = assembler->command;
    message->size = assembler->expected_size;
    copy_chunk(message->data, assembler->data, message->size);
    *feed_state = RIN_CTAPHID_FEED_COMPLETE;
    rin_ctaphid_assembler_cancel(assembler);
    return RIN_RESULT_OK;
}

RinResultCode rin_ctaphid_feed(
    RinCtapHidAssembler* assembler,
    const uint8_t report[RIN_CTAPHID_REPORT_SIZE],
    RinCtapHidMessage* message, uint32_t* feed_state)
{
    uint32_t channel;
    uint32_t remaining;
    uint32_t chunk;
    uint8_t marker;
    if (message) hid_zero(message, sizeof(*message));
    if (feed_state) *feed_state = RIN_CTAPHID_FEED_INCOMPLETE;
    if (!assembler || !report || !message || !feed_state)
        return RIN_RESULT_INVALID_ARGUMENT;
    channel = read_be32(report);
    marker = report[4];
    if ((marker & 0x80u) != 0u) {
        uint32_t size = ((uint32_t)report[5] << 8u) | report[6];
        uint8_t command = marker & 0x7fu;
        if (assembler->active) return RIN_RESULT_BUSY;
        if (channel == 0u || command == 0u || size > RIN_CTAPHID_MESSAGE_MAX)
            return RIN_RESULT_CORRUPT_DATA;
        assembler->channel = channel;
        assembler->command = command;
        assembler->expected_size = size;
        assembler->received_size = 0u;
        assembler->next_sequence = 0u;
        assembler->active = 1u;
        chunk = size < RIN_CTAPHID_INIT_DATA_SIZE
                    ? size
                    : RIN_CTAPHID_INIT_DATA_SIZE;
        copy_chunk(assembler->data, report + 7u, chunk);
        assembler->received_size = chunk;
        return finish_if_complete(assembler, message, feed_state);
    }
    if (!assembler->active || channel != assembler->channel ||
        marker != assembler->next_sequence || marker > 0x7fu) {
        rin_ctaphid_assembler_cancel(assembler);
        return RIN_RESULT_CORRUPT_DATA;
    }
    remaining = assembler->expected_size - assembler->received_size;
    chunk = remaining < RIN_CTAPHID_CONT_DATA_SIZE
                ? remaining
                : RIN_CTAPHID_CONT_DATA_SIZE;
    copy_chunk(assembler->data + assembler->received_size, report + 5u, chunk);
    assembler->received_size += chunk;
    ++assembler->next_sequence;
    return finish_if_complete(assembler, message, feed_state);
}

RinResultCode rin_ctaphid_fragment(uint32_t channel, uint8_t command,
                                   const uint8_t* payload,
                                   uint32_t payload_size,
                                   RinCtapHidEmitReport emit,
                                   void* emit_context)
{
    uint8_t report[RIN_CTAPHID_REPORT_SIZE];
    uint32_t offset = 0u;
    uint32_t chunk;
    uint8_t sequence = 0u;
    RinResultCode result;
    if (channel == 0u || command == 0u || command > 0x7fu ||
        (!payload && payload_size != 0u) ||
        payload_size > RIN_CTAPHID_MESSAGE_MAX || !emit)
        return RIN_RESULT_INVALID_ARGUMENT;
    hid_zero(report, sizeof(report));
    write_be32(report, channel);
    report[4] = command | 0x80u;
    report[5] = (uint8_t)(payload_size >> 8u);
    report[6] = (uint8_t)payload_size;
    chunk = payload_size < RIN_CTAPHID_INIT_DATA_SIZE
                ? payload_size
                : RIN_CTAPHID_INIT_DATA_SIZE;
    copy_chunk(report + 7u, payload, chunk);
    result = emit(emit_context, report);
    if (result != RIN_RESULT_OK) return result;
    offset = chunk;
    while (offset < payload_size) {
        if (sequence > 0x7fu) return RIN_RESULT_LIMIT_EXCEEDED;
        hid_zero(report, sizeof(report));
        write_be32(report, channel);
        report[4] = sequence++;
        chunk = payload_size - offset;
        if (chunk > RIN_CTAPHID_CONT_DATA_SIZE)
            chunk = RIN_CTAPHID_CONT_DATA_SIZE;
        copy_chunk(report + 5u, payload + offset, chunk);
        result = emit(emit_context, report);
        if (result != RIN_RESULT_OK) return result;
        offset += chunk;
    }
    return RIN_RESULT_OK;
}

void rin_ctaphid_client_init(RinCtapHidClient* client)
{
    if (!client) return;
    hid_zero(client, sizeof(*client));
    client->state = RIN_CTAPHID_CLIENT_DISCONNECTED;
}

void rin_ctaphid_client_disconnect(RinCtapHidClient* client)
{
    rin_ctaphid_client_init(client);
}

static RinResultCode set_deadline(RinCtapHidClient* client, uint64_t now_ms,
                                  uint32_t timeout_ms)
{
    if (!client || timeout_ms == 0u ||
        timeout_ms > RIN_CTAPHID_TRANSACTION_TIMEOUT_MAX_MS)
        return RIN_RESULT_INVALID_ARGUMENT;
    client->deadline_ms =
        now_ms > UINT64_MAX - timeout_ms ? UINT64_MAX : now_ms + timeout_ms;
    return RIN_RESULT_OK;
}

RinResultCode rin_ctaphid_client_begin_init(
    RinCtapHidClient* client, const uint8_t nonce[8], uint64_t now_ms,
    uint32_t timeout_ms, RinCtapHidEmitReport emit, void* emit_context)
{
    uint32_t request_channel;
    RinResultCode result;
    if (!client || !nonce || !emit) return RIN_RESULT_INVALID_ARGUMENT;
    if (client->state == RIN_CTAPHID_CLIENT_ALLOCATING)
        return RIN_RESULT_BUSY;
    result = set_deadline(client, now_ms, timeout_ms);
    if (result != RIN_RESULT_OK) return result;
    request_channel = client->channel != 0u ? client->channel
                                            : RIN_CTAPHID_BROADCAST_CID;
    result = rin_ctaphid_fragment(request_channel, RIN_CTAPHID_INIT, nonce,
                                  8u, emit, emit_context);
    if (result != RIN_RESULT_OK) {
        rin_ctaphid_client_disconnect(client);
        return result;
    }
    rin_ctaphid_assembler_cancel(&client->assembler);
    copy_chunk(client->nonce, nonce, 8u);
    client->expected_command = RIN_CTAPHID_INIT;
    client->keepalive_status = 0u;
    client->state = RIN_CTAPHID_CLIENT_ALLOCATING;
    return RIN_RESULT_OK;
}

RinResultCode rin_ctaphid_client_begin_cbor(
    RinCtapHidClient* client, const uint8_t* payload, uint32_t payload_size,
    uint64_t now_ms, uint32_t timeout_ms, RinCtapHidEmitReport emit,
    void* emit_context)
{
    RinResultCode result;
    if (!client || !payload || payload_size == 0u || !emit)
        return RIN_RESULT_INVALID_ARGUMENT;
    if (client->state != RIN_CTAPHID_CLIENT_IDLE)
        return client->state == RIN_CTAPHID_CLIENT_DISCONNECTED
                   ? RIN_RESULT_DEVICE_LOST
                   : RIN_RESULT_BUSY;
    if (client->channel == 0u ||
        (client->capabilities & RIN_CTAPHID_CAPABILITY_CBOR) == 0u)
        return RIN_RESULT_NOT_SUPPORTED;
    result = set_deadline(client, now_ms, timeout_ms);
    if (result != RIN_RESULT_OK) return result;
    result = rin_ctaphid_fragment(client->channel, RIN_CTAPHID_CBOR, payload,
                                  payload_size, emit, emit_context);
    if (result != RIN_RESULT_OK) {
        rin_ctaphid_client_disconnect(client);
        return result;
    }
    rin_ctaphid_assembler_cancel(&client->assembler);
    client->expected_command = RIN_CTAPHID_CBOR;
    client->keepalive_status = 0u;
    client->state = RIN_CTAPHID_CLIENT_WAITING;
    return RIN_RESULT_OK;
}

RinResultCode rin_ctaphid_client_cancel(RinCtapHidClient* client,
                                        RinCtapHidEmitReport emit,
                                        void* emit_context)
{
    RinResultCode result;
    if (!client || !emit) return RIN_RESULT_INVALID_ARGUMENT;
    if (client->state == RIN_CTAPHID_CLIENT_CANCELING)
        return RIN_RESULT_OK;
    if (client->state != RIN_CTAPHID_CLIENT_WAITING ||
        client->expected_command != RIN_CTAPHID_CBOR)
        return RIN_RESULT_NOT_FOUND;
    result = rin_ctaphid_fragment(client->channel, RIN_CTAPHID_CANCEL, NULL,
                                  0u, emit, emit_context);
    if (result != RIN_RESULT_OK) {
        rin_ctaphid_client_disconnect(client);
        return result;
    }
    client->state = RIN_CTAPHID_CLIENT_CANCELING;
    return RIN_RESULT_OK;
}

static RinResultCode transport_error_result(uint8_t error)
{
    if (error == 0x01u) return RIN_RESULT_NOT_SUPPORTED;
    if (error == 0x02u || error == 0x03u)
        return RIN_RESULT_INVALID_ARGUMENT;
    if (error == 0x04u) return RIN_RESULT_CORRUPT_DATA;
    if (error == 0x05u) return RIN_RESULT_TIMED_OUT;
    if (error == 0x06u) return RIN_RESULT_BUSY;
    if (error == 0x0au) return RIN_RESULT_ACCESS_DENIED;
    if (error == 0x0bu) return RIN_RESULT_DEVICE_LOST;
    return RIN_RESULT_IO;
}

RinResultCode rin_ctaphid_client_poll(RinCtapHidClient* client,
                                      uint64_t now_ms)
{
    if (!client) return RIN_RESULT_INVALID_ARGUMENT;
    if (client->state != RIN_CTAPHID_CLIENT_ALLOCATING &&
        client->state != RIN_CTAPHID_CLIENT_WAITING &&
        client->state != RIN_CTAPHID_CLIENT_CANCELING)
        return RIN_RESULT_OK;
    if (now_ms < client->deadline_ms) return RIN_RESULT_OK;
    rin_ctaphid_client_disconnect(client);
    return RIN_RESULT_TIMED_OUT;
}

static RinResultCode accept_init_response(RinCtapHidClient* client,
                                          const RinCtapHidMessage* message)
{
    uint32_t new_channel;
    uint32_t index;
    if (message->command != RIN_CTAPHID_INIT || message->size < 17u)
        return RIN_RESULT_CORRUPT_DATA;
    for (index = 0u; index < 8u; ++index) {
        if (message->data[index] != client->nonce[index])
            return RIN_RESULT_ACCESS_DENIED;
    }
    new_channel = read_be32(message->data + 8u);
    if (new_channel == 0u || new_channel == RIN_CTAPHID_BROADCAST_CID ||
        (client->channel != 0u && new_channel != client->channel))
        return RIN_RESULT_CORRUPT_DATA;
    if (message->data[12u] == 0u)
        return RIN_RESULT_VERSION_MISMATCH;
    if ((message->data[16u] & RIN_CTAPHID_CAPABILITY_CBOR) == 0u)
        return RIN_RESULT_NOT_SUPPORTED;
    client->channel = new_channel;
    client->protocol_version = message->data[12u];
    client->device_major = message->data[13u];
    client->device_minor = message->data[14u];
    client->device_build = message->data[15u];
    client->capabilities = message->data[16u];
    client->deadline_ms = 0u;
    client->expected_command = 0u;
    client->state = RIN_CTAPHID_CLIENT_IDLE;
    return RIN_RESULT_OK;
}

RinResultCode rin_ctaphid_client_feed(
    RinCtapHidClient* client,
    const uint8_t report[RIN_CTAPHID_REPORT_SIZE], uint64_t now_ms,
    RinCtapHidMessage* message, uint32_t* event)
{
    uint32_t expected_channel;
    uint32_t feed_state;
    uint32_t was_canceling;
    RinResultCode result;
    if (message) hid_zero(message, sizeof(*message));
    if (event) *event = RIN_CTAPHID_CLIENT_EVENT_NONE;
    if (!client || !report || !message || !event)
        return RIN_RESULT_INVALID_ARGUMENT;
    if (client->state == RIN_CTAPHID_CLIENT_DISCONNECTED ||
        client->state == RIN_CTAPHID_CLIENT_IDLE)
        return RIN_RESULT_DEVICE_LOST;
    result = rin_ctaphid_client_poll(client, now_ms);
    if (result != RIN_RESULT_OK) return result;
    expected_channel = client->state == RIN_CTAPHID_CLIENT_ALLOCATING &&
                               client->channel == 0u
                           ? RIN_CTAPHID_BROADCAST_CID
                           : client->channel;
    if (read_be32(report) != expected_channel) return RIN_RESULT_WOULD_BLOCK;
    result = rin_ctaphid_feed(&client->assembler, report, message,
                              &feed_state);
    if (result != RIN_RESULT_OK) {
        rin_ctaphid_client_disconnect(client);
        return result;
    }
    if (feed_state != RIN_CTAPHID_FEED_COMPLETE) return RIN_RESULT_OK;
    if (client->state == RIN_CTAPHID_CLIENT_ALLOCATING) {
        result = accept_init_response(client, message);
        if (result != RIN_RESULT_OK) {
            rin_ctaphid_client_disconnect(client);
            return result;
        }
        *event = RIN_CTAPHID_CLIENT_EVENT_READY;
        return RIN_RESULT_OK;
    }
    if (message->command == RIN_CTAPHID_KEEPALIVE) {
        if (message->size != 1u ||
            (message->data[0] != RIN_CTAPHID_KEEPALIVE_PROCESSING &&
             message->data[0] != RIN_CTAPHID_KEEPALIVE_UP_NEEDED)) {
            rin_ctaphid_client_disconnect(client);
            return RIN_RESULT_CORRUPT_DATA;
        }
        client->keepalive_status = message->data[0];
        *event = RIN_CTAPHID_CLIENT_EVENT_KEEPALIVE;
        return RIN_RESULT_OK;
    }
    if (message->command == RIN_CTAPHID_ERROR) {
        if (message->size != 1u) {
            rin_ctaphid_client_disconnect(client);
            return RIN_RESULT_CORRUPT_DATA;
        }
        result = transport_error_result(message->data[0]);
        if (result == RIN_RESULT_DEVICE_LOST) {
            rin_ctaphid_client_disconnect(client);
        } else {
            client->deadline_ms = 0u;
            client->expected_command = 0u;
            client->state = RIN_CTAPHID_CLIENT_IDLE;
        }
        return result;
    }
    if (message->command != client->expected_command) {
        rin_ctaphid_client_disconnect(client);
        return RIN_RESULT_CORRUPT_DATA;
    }
    was_canceling = client->state == RIN_CTAPHID_CLIENT_CANCELING;
    client->deadline_ms = 0u;
    client->expected_command = 0u;
    client->keepalive_status = 0u;
    client->state = RIN_CTAPHID_CLIENT_IDLE;
    if (was_canceling) {
        hid_zero(message, sizeof(*message));
        *event = RIN_CTAPHID_CLIENT_EVENT_CANCELED;
    } else {
        *event = RIN_CTAPHID_CLIENT_EVENT_RESPONSE;
    }
    return RIN_RESULT_OK;
}
