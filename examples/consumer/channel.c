/* SPDX-License-Identifier: MIT */
#include <rin/base.h>
#include <rin/ipc.h>

int main(void)
{
    static const char payload[] = "ping";
    char received[5] = {0};
    RinChannel sender = RIN_HANDLE_INVALID;
    RinChannel receiver = RIN_HANDLE_INVALID;
    RinIpcMessageV1 message = {0};
    RinResult result;

    result = rin_channel_create_v1(&sender, &receiver);
    if (result != RIN_SUCCESS) return 1;

    message.struct_size = sizeof(message);
    message.version = RIN_SDK_STRUCT_VERSION_1;
    message.bytes.address = (uint64_t)(uintptr_t)payload;
    message.bytes.size = sizeof(payload);
    result = rin_channel_send_v1(sender, &message);
    if (result != RIN_SUCCESS) goto cleanup;

    message = (RinIpcMessageV1){0};
    message.struct_size = sizeof(message);
    message.version = RIN_SDK_STRUCT_VERSION_1;
    message.bytes.address = (uint64_t)(uintptr_t)received;
    message.bytes.size = sizeof(received);
    result = rin_channel_receive_v1(receiver, &message);
    if (result == RIN_SUCCESS && message.bytes.size != sizeof(payload))
        result = RIN_ERROR_INTEGRITY;

cleanup:
    if (receiver != RIN_HANDLE_INVALID &&
        rin_object_close_v1((RinObject)receiver) != RIN_SUCCESS &&
        result == RIN_SUCCESS)
        result = RIN_ERROR_IO;
    if (sender != RIN_HANDLE_INVALID &&
        rin_object_close_v1((RinObject)sender) != RIN_SUCCESS &&
        result == RIN_SUCCESS)
        result = RIN_ERROR_IO;
    return result == RIN_SUCCESS ? 0 : 1;
}
