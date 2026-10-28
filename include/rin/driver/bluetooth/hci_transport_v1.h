// SPDX-License-Identifier: MIT

#ifndef RIN_DRIVER_BLUETOOTH_HCI_TRANSPORT_V1_H
#define RIN_DRIVER_BLUETOOTH_HCI_TRANSPORT_V1_H

#include <stdint.h>

typedef int (*RinBluetoothHciSendCommandFn)(
    void* context, uint16_t opcode, const uint8_t* parameters,
    uint8_t parameter_size);

typedef uint64_t (*RinBluetoothHciMonotonicMsFn)(void* context);

typedef struct RinBluetoothHciTransportV1 {
    uint32_t version;
    uint32_t struct_size;
    RinBluetoothHciSendCommandFn send_command;
    RinBluetoothHciMonotonicMsFn monotonic_ms;
    void* context;
} RinBluetoothHciTransportV1;

#endif /* RIN_DRIVER_BLUETOOTH_HCI_TRANSPORT_V1_H */
