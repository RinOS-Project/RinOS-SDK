/* SPDX-License-Identifier: MIT */

#include <rin/driver/bluetooth/hci_runtime_v1.h>

#include <stddef.h>

typedef struct RinBluetoothHciParsedReport {
    uint8_t address_type;
    uint8_t event_type;
    int8_t rssi_dbm;
    uint8_t address[6];
    uint8_t advertising_flags;
    uint8_t advertising_flags_present;
    uint16_t data_size;
    uint8_t name_size;
    const uint8_t* data;
    char name[RIN_BLUETOOTH_HCI_MAX_NAME];
    uint8_t report_format;
    uint8_t data_status;
    uint8_t primary_phy;
    uint8_t secondary_phy;
    uint8_t advertising_sid;
    int8_t tx_power_dbm;
    uint16_t periodic_advertising_interval;
    uint8_t direct_address_type;
    uint8_t direct_address_present;
    uint8_t direct_address[6];
} RinBluetoothHciParsedReport;

static int privacy_config_valid(
    const RinBluetoothHciPrivacyConfigV1* config);
static int advertising_config_valid(
    const RinBluetoothHciAdvertisingConfigV1* config);
static int connection_config_valid(
    const RinBluetoothHciConnectionConfigV1* config);
static int connection_table_valid(
    const RinBluetoothHciControllerV1* controller);
static int static_random_address_valid(const uint8_t address[6]);
static int connection_values_valid(uint16_t interval, uint16_t latency,
                                   uint16_t timeout);

static void bytes_zero(void* output, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)output;
    while (size-- != 0u) *bytes++ = 0u;
}

static void bytes_copy(void* output, const void* input, uint32_t size)
{
    uint8_t* destination = (uint8_t*)output;
    const uint8_t* source = (const uint8_t*)input;
    while (size-- != 0u) *destination++ = *source++;
}

static int bytes_all_zero(const void* value, uint32_t size)
{
    const uint8_t* bytes = (const uint8_t*)value;
    uint8_t any = 0u;
    while (size-- != 0u) any |= *bytes++;
    return any == 0u;
}

static int advance_report_generation(RinBluetoothHciControllerV1* controller)
{
    if (controller->generation == UINT32_MAX)
        return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
    ++controller->generation;
    return RIN_BLUETOOTH_HCI_OK;
}

static uint16_t load_le16(const uint8_t* bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void store_le16(uint8_t* bytes, uint16_t value)
{
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
}

static int transport_valid(const RinBluetoothHciTransportV1* transport)
{
    return transport != NULL &&
           transport->version == RIN_BLUETOOTH_HCI_VERSION &&
           transport->struct_size == sizeof(*transport) &&
           transport->send_command != NULL &&
           transport->monotonic_ms != NULL;
}

static int transport_equal(const RinBluetoothHciTransportV1* left,
                           const RinBluetoothHciTransportV1* right)
{
    return left != NULL && right != NULL &&
           left->version == right->version &&
           left->struct_size == right->struct_size &&
           left->send_command == right->send_command &&
           left->monotonic_ms == right->monotonic_ms &&
           left->context == right->context;
}

/* Transport callbacks are queueing boundaries and may not re-enter or
 * mutate the controller through their context.  The complete bounded
 * controller is hashed rather than copied, keeping this guard allocation
 * free even while discovery tables and fragment scratch are populated. */
static uint32_t controller_hash(
    const RinBluetoothHciControllerV1* controller)
{
    const uint8_t* bytes = (const uint8_t*)controller;
    uint32_t hash = 2166136261u;
    uint32_t index;
    for (index = 0u; index < (uint32_t)sizeof(*controller); ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static int transport_send_checked(RinBluetoothHciControllerV1* controller,
                                  uint16_t opcode,
                                  const uint8_t* parameters,
                                  uint8_t parameter_size)
{
    uint8_t parameter_snapshot[255u] = {0};
    RinBluetoothHciTransportV1 transport_snapshot = controller->transport;
    uint32_t owner_hash = controller_hash(controller);
    uint8_t index;
    for (index = 0u; index < parameter_size; ++index)
        parameter_snapshot[index] = parameters[index];
    int result = transport_snapshot.send_command(
        transport_snapshot.context, opcode, parameter_snapshot,
        parameter_size);
    for (index = 0u; index < parameter_size; ++index) {
        if (parameters[index] != parameter_snapshot[index]) {
            controller->transport = transport_snapshot;
            controller->pending_opcode = 0u;
            controller->scan_active = RIN_BLUETOOTH_HCI_SCAN_INACTIVE;
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
            bytes_zero(parameter_snapshot, sizeof(parameter_snapshot));
            return -2;
        }
    }
    if (controller_hash(controller) != owner_hash ||
        !transport_equal(&controller->transport, &transport_snapshot)) {
        controller->transport = transport_snapshot;
        controller->pending_opcode = 0u;
        controller->scan_active = RIN_BLUETOOTH_HCI_SCAN_INACTIVE;
        controller->state = RIN_BLUETOOTH_HCI_FAILED;
        bytes_zero(parameter_snapshot, sizeof(parameter_snapshot));
        return -2;
    }
    bytes_zero(parameter_snapshot, sizeof(parameter_snapshot));
    return result;
}

static uint64_t monotonic_ms_checked(
    RinBluetoothHciControllerV1* controller, int* mutation_out)
{
    RinBluetoothHciTransportV1 transport_snapshot = controller->transport;
    uint32_t owner_hash = controller_hash(controller);
    uint64_t result = transport_snapshot.monotonic_ms(
        transport_snapshot.context);
    if (mutation_out) *mutation_out = 0;
    if (controller_hash(controller) != owner_hash ||
        !transport_equal(&controller->transport, &transport_snapshot)) {
        controller->transport = transport_snapshot;
        controller->pending_opcode = 0u;
        controller->scan_active = RIN_BLUETOOTH_HCI_SCAN_INACTIVE;
        controller->state = RIN_BLUETOOTH_HCI_FAILED;
        if (mutation_out) *mutation_out = 1;
        return 0u;
    }
    return result;
}

static int controller_valid(const RinBluetoothHciControllerV1* controller)
{
    return controller != NULL &&
           controller->version ==
               RIN_BLUETOOTH_HCI_CONTROLLER_LAYOUT_VERSION &&
           controller->struct_size == sizeof(*controller) &&
           controller->state >= RIN_BLUETOOTH_HCI_UNINITIALIZED &&
           controller->state <= RIN_BLUETOOTH_HCI_LTK_REPLY_PENDING &&
           controller->scan_active <= RIN_BLUETOOTH_HCI_SCAN_EXTENDED &&
           controller->scan_command_family <=
               RIN_BLUETOOTH_HCI_SCAN_EXTENDED &&
           controller->privacy_configured <= 1u &&
           controller->address_resolution_enabled <= 1u &&
           controller->privacy_peer_index <=
               RIN_BLUETOOTH_HCI_MAX_PRIVACY_PEERS &&
           (!controller->address_resolution_enabled ||
            controller->privacy_configured) &&
           ((!controller->privacy_configured &&
             controller->privacy_peer_index == 0u) ||
            (controller->privacy_configured &&
             controller->privacy_peer_index <=
                 controller->privacy.peer_count)) &&
           (!controller->privacy_configured ||
            privacy_config_valid(&controller->privacy)) &&
           controller->advertising_configured <= 1u &&
           controller->advertising_active <= 1u &&
           controller->advertising_reserved[0] == 0u &&
           controller->advertising_reserved[1] == 0u &&
           (!controller->advertising_configured ||
            advertising_config_valid(&controller->advertising)) &&
           (!controller->advertising_active ||
            (controller->advertising_configured &&
             controller->scan_active == RIN_BLUETOOTH_HCI_SCAN_INACTIVE)) &&
           controller->connection_pending <= 1u &&
           controller->connection_cancel_pending <= 1u &&
           controller->disconnect_pending <= 1u &&
           controller->connection_reserved_flags == 0u &&
           controller->connection_reserved == 0u &&
           controller->connection_generation != 0u &&
           controller->disconnect_previous_state >=
               RIN_BLUETOOTH_HCI_UNINITIALIZED &&
           controller->disconnect_previous_state <=
               RIN_BLUETOOTH_HCI_DISCONNECT_PENDING &&
           (!controller->connection_pending ||
            connection_config_valid(&controller->pending_connection)) &&
           (!controller->connection_cancel_pending ||
            controller->state ==
                RIN_BLUETOOTH_HCI_CONNECT_CANCEL_PENDING ||
            controller->state == RIN_BLUETOOTH_HCI_FAILED) &&
           (!controller->disconnect_pending ||
            controller->state == RIN_BLUETOOTH_HCI_DISCONNECT_PENDING ||
            controller->state == RIN_BLUETOOTH_HCI_FAILED) &&
           connection_table_valid(controller) &&
           transport_valid(&controller->transport);
}

static int address_invalid(const uint8_t address[6])
{
    uint8_t any = 0u;
    uint8_t all_ones = 0xffu;
    uint32_t index;
    for (index = 0u; index < 6u; ++index) {
        any |= address[index];
        all_ones &= address[index];
    }
    return any == 0u || all_ones == 0xffu;
}

static int address_is_zero(const uint8_t address[6])
{
    uint8_t any = 0u;
    uint32_t index;
    for (index = 0u; index < 6u; ++index) any |= address[index];
    return any == 0u;
}

static int address_is_all_ones(const uint8_t address[6])
{
    uint8_t all_ones = 0xffu;
    uint32_t index;
    for (index = 0u; index < 6u; ++index) all_ones &= address[index];
    return all_ones == 0xffu;
}

static int connection_config_valid(
    const RinBluetoothHciConnectionConfigV1* config)
{
    uint64_t supervision_window;
    uint64_t required_timeout;
    if (config == NULL ||
        config->version != RIN_BLUETOOTH_HCI_CONNECTION_CONFIG_VERSION ||
        config->struct_size != sizeof(*config) ||
        config->scan_interval < 0x0004u ||
        config->scan_interval > 0x4000u ||
        config->scan_window < 0x0004u ||
        config->scan_window > config->scan_interval ||
        config->peer_address_type > 3u ||
        address_invalid(config->peer_address) ||
        (config->peer_address_type == 3u &&
         !static_random_address_valid(config->peer_address)) ||
        config->own_address_type > 3u ||
        config->connection_interval_min < 0x0006u ||
        config->connection_interval_max > 0x0c80u ||
        config->connection_interval_min >
            config->connection_interval_max ||
        config->connection_latency > 0x01f3u ||
        config->supervision_timeout < 0x000au ||
        config->supervision_timeout > 0x0c80u ||
        config->min_connection_event_length >
            config->max_connection_event_length)
        return 0;
    supervision_window =
        (uint64_t)2u * ((uint64_t)config->connection_latency + 1u) *
        config->connection_interval_max;
    required_timeout = (uint64_t)config->supervision_timeout * 8u;
    return required_timeout > supervision_window;
}

static int connection_table_valid(
    const RinBluetoothHciControllerV1* controller)
{
    uint32_t present = 0u;
    uint32_t pending_disconnects = 0u;
    uint32_t pending_security_commands = 0u;
    uint32_t index;
    uint32_t other;
    if (controller->connection_count > RIN_BLUETOOTH_HCI_MAX_CONNECTIONS)
        return 0;
    for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS; ++index) {
        const RinBluetoothHciConnectionV1* connection =
            &controller->connections[index];
        if (connection->present > 1u || connection->disconnect_pending > 1u ||
            connection->encrypted > 1u ||
            connection->encryption_pending > 1u ||
            connection->encryption_command_pending > 1u ||
            connection->ltk_request_pending > 1u ||
            connection->ltk_reply_command_pending > 1u ||
            !bytes_all_zero(connection->security_reserved,
                            sizeof(connection->security_reserved)) ||
            connection->security_reserved2 != 0u)
            return 0;
        if (!connection->present) {
            if (connection->disconnect_pending || connection->encrypted ||
                connection->encryption_pending ||
                connection->encryption_command_pending ||
                connection->ltk_request_pending ||
                connection->ltk_reply_command_pending ||
                !bytes_all_zero(connection->ltk_random,
                                sizeof(connection->ltk_random)) ||
                connection->ltk_ediv != 0u)
                return 0;
            continue;
        }
        ++present;
        if (connection->version !=
                RIN_BLUETOOTH_HCI_CONNECTION_LAYOUT_VERSION ||
            connection->generation == 0u || connection->handle > 0x0effu ||
            connection->role > 1u || connection->peer_address_type > 3u ||
            address_invalid(connection->peer_address) ||
            (connection->peer_address_type == 3u &&
             !static_random_address_valid(connection->peer_address)) ||
            (connection->encrypted && connection->encryption_pending) ||
            (connection->encryption_command_pending &&
             (!connection->encryption_pending || connection->role != 0u ||
              connection->ltk_request_pending)) ||
            (connection->ltk_request_pending &&
             (connection->role != 1u || connection->encryption_pending ||
              connection->encrypted)) ||
            (connection->ltk_reply_command_pending &&
             !connection->ltk_request_pending) ||
            (!connection->ltk_request_pending &&
             (!bytes_all_zero(connection->ltk_random,
                              sizeof(connection->ltk_random)) ||
              connection->ltk_ediv != 0u)) ||
            connection->generation >= controller->connection_generation ||
            connection->connection_interval < 0x0006u ||
            connection->connection_interval > 0x0c80u ||
            connection->connection_latency > 0x01f3u ||
            !connection_values_valid(connection->connection_interval,
                                     connection->connection_latency,
                                     connection->supervision_timeout))
            return 0;
        if (connection->disconnect_pending) {
            if (!controller->disconnect_pending ||
                controller->disconnect_generation == 0u ||
                connection->handle != controller->disconnect_handle ||
                connection->generation != controller->disconnect_generation)
                return 0;
            ++pending_disconnects;
        }
        if (connection->encryption_command_pending ||
            connection->ltk_reply_command_pending)
            ++pending_security_commands;
        for (other = 0u; other < index; ++other) {
            const RinBluetoothHciConnectionV1* previous =
                &controller->connections[other];
            if (previous->present &&
                (previous->handle == connection->handle ||
                 previous->generation == connection->generation))
                return 0;
        }
    }
    if (present != controller->connection_count || pending_disconnects > 1u ||
        pending_security_commands > 1u)
        return 0;
    if (controller->disconnect_pending) {
        if (controller->disconnect_generation == 0u) {
            if (pending_disconnects != 0u) return 0;
        } else if (pending_disconnects != 1u) {
            return 0;
        }
    } else if (pending_disconnects != 0u) {
        return 0;
    }
    if (controller->state ==
        RIN_BLUETOOTH_HCI_ENCRYPTION_START_PENDING) {
        if (pending_security_commands != 1u ||
            controller->pending_opcode !=
                RIN_BLUETOOTH_HCI_OPCODE_LE_START_ENCRYPTION)
            return 0;
    } else if (controller->state == RIN_BLUETOOTH_HCI_LTK_REPLY_PENDING) {
        if (pending_security_commands != 1u ||
            (controller->pending_opcode !=
                 RIN_BLUETOOTH_HCI_OPCODE_LE_LTK_REQUEST_REPLY &&
             controller->pending_opcode !=
                 RIN_BLUETOOTH_HCI_OPCODE_LE_LTK_REQUEST_NEGATIVE_REPLY))
            return 0;
    } else if (pending_security_commands != 0u &&
               controller->state != RIN_BLUETOOTH_HCI_FAILED) {
        return 0;
    }
    return 1;
}

static int static_random_address_valid(const uint8_t address[6])
{
    uint8_t any_one;
    uint8_t all_ones;
    uint32_t index;
    if (address == NULL || (address[0] & 0xc0u) != 0xc0u) return 0;
    any_one = (uint8_t)(address[0] & 0x3fu);
    all_ones = (uint8_t)(address[0] & 0x3fu);
    for (index = 1u; index < 6u; ++index) {
        any_one |= address[index];
        all_ones &= address[index];
    }
    return any_one != 0u && all_ones != 0x3fu;
}

static int address_equal(const uint8_t left[6], const uint8_t right[6])
{
    uint8_t difference = 0u;
    uint32_t index;
    for (index = 0u; index < 6u; ++index)
        difference |= (uint8_t)(left[index] ^ right[index]);
    return difference == 0u;
}

static int octets_equal(const uint8_t* left, const uint8_t* right,
                        uint32_t size)
{
    uint8_t difference = 0u;
    while (size-- != 0u) difference |= (uint8_t)(*left++ ^ *right++);
    return difference == 0u;
}

static int ranges_overlap(const void* left, uint32_t left_size,
                          const void* right, uint32_t right_size)
{
    uintptr_t left_start;
    uintptr_t right_start;
    if (left == NULL || right == NULL || left_size == 0u || right_size == 0u)
        return 0;
    left_start = (uintptr_t)left;
    right_start = (uintptr_t)right;
    if (left_start <= right_start)
        return right_start - left_start < left_size;
    return left_start - right_start < right_size;
}

static int irk_valid(const uint8_t irk[RIN_BLUETOOTH_HCI_IRK_SIZE])
{
    uint8_t any = 0u;
    uint8_t all_ones = 0xffu;
    uint32_t index;
    for (index = 0u; index < RIN_BLUETOOTH_HCI_IRK_SIZE; ++index) {
        any |= irk[index];
        all_ones &= irk[index];
    }
    return any != 0u && all_ones != 0xffu;
}

static int privacy_config_valid(
    const RinBluetoothHciPrivacyConfigV1* config)
{
    uint32_t index;
    uint32_t other;
    if (config == NULL ||
        config->version != RIN_BLUETOOTH_HCI_VERSION ||
        config->struct_size != sizeof(*config) ||
        config->reserved != 0u || config->peer_count == 0u ||
        config->peer_count > RIN_BLUETOOTH_HCI_MAX_PRIVACY_PEERS ||
        config->rpa_timeout_seconds == 0u ||
        config->rpa_timeout_seconds > 0x0e10u ||
        !irk_valid(config->local_irk_hci))
        return 0;
    for (index = 0u; index < config->peer_count; ++index) {
        const RinBluetoothHciPrivacyPeerV1* peer = &config->peers[index];
        if (peer->identity_address_type > 1u || peer->privacy_mode > 1u ||
            address_invalid(peer->identity_address) ||
            (peer->identity_address_type == 1u &&
             !static_random_address_valid(peer->identity_address)) ||
            !irk_valid(peer->peer_irk_hci))
            return 0;
        for (other = 0u; other < index; ++other) {
            const RinBluetoothHciPrivacyPeerV1* previous =
                &config->peers[other];
            if ((peer->identity_address_type ==
                     previous->identity_address_type &&
                 address_equal(peer->identity_address,
                               previous->identity_address)) ||
                octets_equal(peer->peer_irk_hci, previous->peer_irk_hci,
                             RIN_BLUETOOTH_HCI_IRK_SIZE))
                return 0;
        }
    }
    return 1;
}

static int advertising_config_valid(
    const RinBluetoothHciAdvertisingConfigV1* config)
{
    uint32_t index;
    if (config == NULL ||
        config->version != RIN_BLUETOOTH_HCI_ADVERTISING_CONFIG_VERSION ||
        config->struct_size != sizeof(*config) ||
        config->interval_min < 0x0020u ||
        config->interval_max > 0x4000u ||
        config->interval_min > config->interval_max ||
        (config->advertising_type != 0u &&
         config->advertising_type != 2u &&
         config->advertising_type != 3u) ||
        config->own_address_type > 3u || config->channel_map == 0u ||
        (config->channel_map & ~0x07u) != 0u ||
        config->filter_policy > 3u ||
        config->advertising_data_size >
            RIN_BLUETOOTH_HCI_MAX_LEGACY_ADV_DATA ||
        config->scan_response_data_size >
            RIN_BLUETOOTH_HCI_MAX_LEGACY_ADV_DATA ||
        config->reserved[0] != 0u || config->reserved[1] != 0u)
        return 0;
    for (index = config->advertising_data_size;
         index < RIN_BLUETOOTH_HCI_MAX_LEGACY_ADV_DATA; ++index) {
        if (config->advertising_data[index] != 0u) return 0;
    }
    for (index = config->scan_response_data_size;
         index < RIN_BLUETOOTH_HCI_MAX_LEGACY_ADV_DATA; ++index) {
        if (config->scan_response_data[index] != 0u) return 0;
    }
    return 1;
}

static int utf8_continuation(uint8_t value)
{
    return (value & 0xc0u) == 0x80u;
}

static int utf8_name_valid(const uint8_t* bytes, uint32_t size)
{
    uint32_t index = 0u;
    while (index < size) {
        uint8_t first = bytes[index++];
        if (first < 0x80u) {
            if (first < 0x20u || first == 0x7fu) return 0;
            continue;
        }
        if (first >= 0xc2u && first <= 0xdfu) {
            if (index >= size || !utf8_continuation(bytes[index])) return 0;
            ++index;
            continue;
        }
        if (first >= 0xe0u && first <= 0xefu) {
            uint8_t second;
            if (index + 1u >= size) return 0;
            second = bytes[index];
            if (!utf8_continuation(second) ||
                !utf8_continuation(bytes[index + 1u]) ||
                (first == 0xe0u && second < 0xa0u) ||
                (first == 0xedu && second >= 0xa0u)) return 0;
            index += 2u;
            continue;
        }
        if (first >= 0xf0u && first <= 0xf4u) {
            uint8_t second;
            if (index + 2u >= size) return 0;
            second = bytes[index];
            if (!utf8_continuation(second) ||
                !utf8_continuation(bytes[index + 1u]) ||
                !utf8_continuation(bytes[index + 2u]) ||
                (first == 0xf0u && second < 0x90u) ||
                (first == 0xf4u && second >= 0x90u)) return 0;
            index += 3u;
            continue;
        }
        return 0;
    }
    return 1;
}

static int parse_advertising_data(const uint8_t* data, uint16_t data_size,
                                  RinBluetoothHciParsedReport* report)
{
    uint32_t offset = 0u;
    int short_name_seen = 0;
    int complete_name_seen = 0;
    int flags_seen = 0;

    report->data_size = data_size;
    report->data = data;
    while (offset < data_size) {
        uint8_t length = data[offset++];
        uint8_t type;
        uint32_t value_size;
        if (length == 0u || length > data_size - offset) return 0;
        type = data[offset];
        value_size = (uint32_t)length - 1u;
        if (type == 0x01u) {
            if (flags_seen || value_size != 1u) return 0;
            flags_seen = 1;
            report->advertising_flags = data[offset + 1u];
            report->advertising_flags_present = 1u;
        } else if (type == 0x08u || type == 0x09u) {
            if (value_size == 0u ||
                value_size >= RIN_BLUETOOTH_HCI_MAX_NAME ||
                !utf8_name_valid(data + offset + 1u, value_size)) return 0;
            if ((type == 0x08u && short_name_seen) ||
                (type == 0x09u && complete_name_seen)) return 0;
            if (type == 0x08u) short_name_seen = 1;
            else complete_name_seen = 1;
            if (type == 0x09u || !complete_name_seen) {
                bytes_copy(report->name, data + offset + 1u, value_size);
                report->name[value_size] = '\0';
                report->name_size = (uint8_t)value_size;
            }
        }
        offset += length;
    }
    return offset == data_size;
}

static int submit_command(RinBluetoothHciControllerV1* controller,
                          uint16_t opcode, const uint8_t* parameters,
                          uint8_t parameter_size,
                          RinBluetoothHciState pending_state)
{
    RinBluetoothHciState previous_state = controller->state;
    int result;
    controller->pending_opcode = opcode;
    controller->state = pending_state;
    result = transport_send_checked(controller, opcode, parameters,
                                    parameter_size);
    if (result == -2) return RIN_BLUETOOTH_HCI_TRANSPORT_FAILURE;
    if (result != 0) {
        controller->pending_opcode = 0u;
        controller->state = previous_state;
        return RIN_BLUETOOTH_HCI_TRANSPORT_FAILURE;
    }
    return RIN_BLUETOOTH_HCI_OK;
}

static void controller_command_failed(RinBluetoothHciControllerV1* controller,
                                      uint8_t status)
{
    RinBluetoothHciState state = controller->state;
    controller->last_controller_status = status;
    controller->pending_opcode = 0u;
    if (state == RIN_BLUETOOTH_HCI_SCAN_PARAMS_PENDING ||
        state == RIN_BLUETOOTH_HCI_SCAN_ENABLE_PENDING) {
        controller->scan_active = 0u;
        controller->state = RIN_BLUETOOTH_HCI_READY;
    } else if (state == RIN_BLUETOOTH_HCI_SCAN_DISABLE_PENDING) {
        if (controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_LEGACY &&
            controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_EXTENDED) {
            controller->scan_active = RIN_BLUETOOTH_HCI_SCAN_INACTIVE;
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
            return;
        }
        controller->state = RIN_BLUETOOTH_HCI_SCANNING;
    } else if (state == RIN_BLUETOOTH_HCI_ADV_DISABLE_PENDING) {
        if (!controller->advertising_active) {
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
            return;
        }
        controller->state = RIN_BLUETOOTH_HCI_READY;
    } else if (state == RIN_BLUETOOTH_HCI_ADV_PARAMS_PENDING ||
               state == RIN_BLUETOOTH_HCI_ADV_DATA_PENDING ||
               state == RIN_BLUETOOTH_HCI_SCAN_RSP_DATA_PENDING ||
               state == RIN_BLUETOOTH_HCI_ADV_ENABLE_PENDING) {
        controller->advertising_active = 0u;
        controller->state = RIN_BLUETOOTH_HCI_READY;
    } else if (state == RIN_BLUETOOTH_HCI_CONNECT_PENDING) {
        controller->connection_pending = 0u;
        controller->connection_cancel_pending = 0u;
        bytes_zero(&controller->pending_connection,
                   sizeof(controller->pending_connection));
        controller->state = RIN_BLUETOOTH_HCI_READY;
    } else if (state == RIN_BLUETOOTH_HCI_CONNECT_CANCEL_PENDING) {
        controller->connection_cancel_pending = 0u;
        controller->state = controller->connection_pending
                                ? RIN_BLUETOOTH_HCI_CONNECTING
                                : RIN_BLUETOOTH_HCI_READY;
    } else if (state == RIN_BLUETOOTH_HCI_DISCONNECT_PENDING) {
        uint32_t index;
        int unmanaged = controller->disconnect_generation == 0u;
        for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS;
             ++index) {
            RinBluetoothHciConnectionV1* connection =
                &controller->connections[index];
            if (connection->present &&
                connection->handle == controller->disconnect_handle &&
                connection->generation == controller->disconnect_generation)
                connection->disconnect_pending = 0u;
        }
        controller->disconnect_pending = 0u;
        controller->disconnect_handle = 0u;
        controller->disconnect_generation = 0u;
        controller->state = unmanaged
                                ? RIN_BLUETOOTH_HCI_FAILED
                                : controller->disconnect_previous_state;
        controller->disconnect_previous_state = RIN_BLUETOOTH_HCI_READY;
    } else if (state == RIN_BLUETOOTH_HCI_ENCRYPTION_START_PENDING) {
        uint32_t index;
        for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS;
             ++index) {
            RinBluetoothHciConnectionV1* connection =
                &controller->connections[index];
            if (connection->encryption_command_pending) {
                connection->encryption_command_pending = 0u;
                connection->encryption_pending = 0u;
                connection->encrypted = 0u;
            }
        }
        controller->state = RIN_BLUETOOTH_HCI_READY;
    } else if (state == RIN_BLUETOOTH_HCI_LTK_REPLY_PENDING) {
        uint32_t index;
        for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS;
             ++index)
            controller->connections[index].ltk_reply_command_pending = 0u;
        controller->state = RIN_BLUETOOTH_HCI_READY;
    } else {
        controller->scan_active = 0u;
        controller->state = RIN_BLUETOOTH_HCI_FAILED;
    }
}

static int submit_event_mask(RinBluetoothHciControllerV1* controller)
{
    /* Connection complete, advertising reports, Encryption Change, LTK
     * Request, enhanced connection complete, and extended reports. */
    const uint8_t mask[8] = {0x93u, 0x12u, 0u, 0u, 0u, 0u, 0u, 0u};
    int result = submit_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_EVENT_MASK, mask,
        sizeof(mask), RIN_BLUETOOTH_HCI_EVENT_MASK_PENDING);
    if (result != RIN_BLUETOOTH_HCI_OK)
        controller->state = RIN_BLUETOOTH_HCI_FAILED;
    return result;
}

static int submit_initialization_command(
    RinBluetoothHciControllerV1* controller, uint16_t opcode,
    const uint8_t* parameters, uint8_t parameter_size,
    RinBluetoothHciState pending_state)
{
    int result = submit_command(controller, opcode, parameters,
                                parameter_size, pending_state);
    if (result != RIN_BLUETOOTH_HCI_OK)
        controller->state = RIN_BLUETOOTH_HCI_FAILED;
    return result;
}

static int submit_privacy_add(RinBluetoothHciControllerV1* controller)
{
    uint8_t parameters[39];
    const RinBluetoothHciPrivacyPeerV1* peer;
    uint32_t index;
    if (controller->privacy_peer_index >= controller->privacy.peer_count)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    peer = &controller->privacy.peers[controller->privacy_peer_index];
    parameters[0] = peer->identity_address_type;
    for (index = 0u; index < 6u; ++index)
        parameters[1u + index] = peer->identity_address[5u - index];
    bytes_copy(parameters + 7u, peer->peer_irk_hci,
               RIN_BLUETOOTH_HCI_IRK_SIZE);
    bytes_copy(parameters + 23u, controller->privacy.local_irk_hci,
               RIN_BLUETOOTH_HCI_IRK_SIZE);
    return submit_initialization_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_ADD_RESOLVING,
        parameters, sizeof(parameters),
        RIN_BLUETOOTH_HCI_PRIVACY_ADD_PENDING);
}

static int submit_privacy_mode(RinBluetoothHciControllerV1* controller)
{
    uint8_t parameters[8];
    const RinBluetoothHciPrivacyPeerV1* peer;
    uint32_t index;
    if (controller->privacy_peer_index >= controller->privacy.peer_count)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    peer = &controller->privacy.peers[controller->privacy_peer_index];
    parameters[0] = peer->identity_address_type;
    for (index = 0u; index < 6u; ++index)
        parameters[1u + index] = peer->identity_address[5u - index];
    parameters[7] = peer->privacy_mode;
    return submit_initialization_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_PRIVACY_MODE,
        parameters, sizeof(parameters),
        RIN_BLUETOOTH_HCI_PRIVACY_MODE_PENDING);
}

static int submit_privacy_timeout(RinBluetoothHciControllerV1* controller)
{
    uint8_t parameters[2];
    store_le16(parameters, controller->privacy.rpa_timeout_seconds);
    return submit_initialization_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_RPA_TIMEOUT,
        parameters, sizeof(parameters),
        RIN_BLUETOOTH_HCI_PRIVACY_TIMEOUT_PENDING);
}

static int submit_privacy_enable(RinBluetoothHciControllerV1* controller)
{
    const uint8_t parameters[1] = {1u};
    return submit_initialization_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_RESOLUTION_ENABLE,
        parameters, sizeof(parameters),
        RIN_BLUETOOTH_HCI_PRIVACY_ENABLE_PENDING);
}

static int submit_privacy_or_event_mask(
    RinBluetoothHciControllerV1* controller)
{
    if (!controller->privacy_configured) return submit_event_mask(controller);
    controller->privacy_peer_index = 0u;
    return submit_initialization_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_CLEAR_RESOLVING,
        NULL, 0u, RIN_BLUETOOTH_HCI_PRIVACY_CLEAR_PENDING);
}

static int submit_advertising_payload(
    RinBluetoothHciControllerV1* controller, uint16_t opcode,
    RinBluetoothHciState pending_state, const uint8_t* data,
    uint8_t data_size)
{
    uint8_t parameters[1u + RIN_BLUETOOTH_HCI_MAX_LEGACY_ADV_DATA] = {0};
    int result;
    parameters[0] = data_size;
    if (data_size != 0u) bytes_copy(parameters + 1u, data, data_size);
    result = submit_command(controller, opcode, parameters,
                            sizeof(parameters), pending_state);
    if (result != RIN_BLUETOOTH_HCI_OK) {
        controller->pending_opcode = 0u;
        controller->advertising_active = 0u;
        if (controller->state != RIN_BLUETOOTH_HCI_FAILED)
            controller->state = RIN_BLUETOOTH_HCI_READY;
    }
    return result;
}

static int submit_advertising_enable(
    RinBluetoothHciControllerV1* controller, uint8_t enable,
    RinBluetoothHciState pending_state)
{
    const uint8_t parameters[1] = {enable};
    int result = submit_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_ENABLE,
        parameters, sizeof(parameters), pending_state);
    if (result != RIN_BLUETOOTH_HCI_OK) {
        controller->pending_opcode = 0u;
        if (controller->state != RIN_BLUETOOTH_HCI_FAILED)
            controller->state = RIN_BLUETOOTH_HCI_READY;
        if (enable != 0u) controller->advertising_active = 0u;
    }
    return result;
}

static int command_complete(RinBluetoothHciControllerV1* controller,
                            const uint8_t* parameters, uint32_t size)
{
    uint16_t opcode;
    uint8_t status;
    if (size < 4u || parameters[0] == 0u) return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    opcode = load_le16(parameters + 1u);
    if (opcode == 0u || opcode != controller->pending_opcode)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    status = parameters[3];

    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_READ_BD_ADDR) {
        uint32_t index;
        uint8_t address[6];
        if (size != 10u) return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        for (index = 0u; index < 6u; ++index)
            address[index] = parameters[9u - index];
        if (status == 0u && address_is_all_ones(address))
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        if (status == 0u && !address_is_zero(address)) {
            bytes_copy(controller->local_address, address, sizeof(address));
            controller->local_address_type = 0u;
            controller->local_address_present = 1u;
        }
    } else if (size != 4u) {
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    }

    if (status != 0u) {
        controller_command_failed(controller, status);
        return RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE;
    }

    controller->last_controller_status = 0u;
    controller->pending_opcode = 0u;
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_LTK_REQUEST_REPLY ||
        opcode ==
            RIN_BLUETOOTH_HCI_OPCODE_LE_LTK_REQUEST_NEGATIVE_REPLY) {
        uint32_t index;
        RinBluetoothHciConnectionV1* pending = NULL;
        if (controller->state != RIN_BLUETOOTH_HCI_LTK_REPLY_PENDING)
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS;
             ++index) {
            RinBluetoothHciConnectionV1* connection =
                &controller->connections[index];
            if (!connection->ltk_reply_command_pending) continue;
            if (pending != NULL) {
                controller->state = RIN_BLUETOOTH_HCI_FAILED;
                return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
            }
            pending = connection;
        }
        if (pending == NULL || !pending->ltk_request_pending) {
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        }
        pending->ltk_reply_command_pending = 0u;
        pending->ltk_request_pending = 0u;
        bytes_zero(pending->ltk_random, sizeof(pending->ltk_random));
        pending->ltk_ediv = 0u;
        pending->encryption_pending =
            opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_LTK_REQUEST_REPLY ? 1u : 0u;
        pending->encrypted = 0u;
        controller->state = RIN_BLUETOOTH_HCI_READY;
        return RIN_BLUETOOTH_HCI_OK;
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_RESET &&
        controller->state == RIN_BLUETOOTH_HCI_RESET_PENDING) {
        int result = submit_command(
            controller, RIN_BLUETOOTH_HCI_OPCODE_READ_BD_ADDR, NULL, 0u,
            RIN_BLUETOOTH_HCI_ADDRESS_PENDING);
        if (result != RIN_BLUETOOTH_HCI_OK)
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
        return result;
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_READ_BD_ADDR &&
        controller->state == RIN_BLUETOOTH_HCI_ADDRESS_PENDING) {
        if (controller->static_random_address_configured) {
            uint8_t parameters[6];
            uint32_t index;
            int result;
            for (index = 0u; index < 6u; ++index)
                parameters[index] = controller->static_random_address[5u - index];
            result = submit_command(
                controller, RIN_BLUETOOTH_HCI_OPCODE_LE_RANDOM_ADDR,
                parameters, sizeof(parameters),
                RIN_BLUETOOTH_HCI_RANDOM_ADDRESS_PENDING);
            if (result != RIN_BLUETOOTH_HCI_OK)
                controller->state = RIN_BLUETOOTH_HCI_FAILED;
            return result;
        }
        if (!controller->local_address_present) {
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
            return RIN_BLUETOOTH_HCI_NOT_FOUND;
        }
        return submit_privacy_or_event_mask(controller);
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_RANDOM_ADDR &&
        controller->state == RIN_BLUETOOTH_HCI_RANDOM_ADDRESS_PENDING) {
        bytes_copy(controller->local_address,
                   controller->static_random_address,
                   sizeof(controller->local_address));
        controller->local_address_type = 1u;
        controller->local_address_present = 1u;
        return submit_privacy_or_event_mask(controller);
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_CLEAR_RESOLVING &&
        controller->state == RIN_BLUETOOTH_HCI_PRIVACY_CLEAR_PENDING) {
        controller->privacy_peer_index = 0u;
        return submit_privacy_add(controller);
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_ADD_RESOLVING &&
        controller->state == RIN_BLUETOOTH_HCI_PRIVACY_ADD_PENDING) {
        ++controller->privacy_peer_index;
        if (controller->privacy_peer_index < controller->privacy.peer_count)
            return submit_privacy_add(controller);
        controller->privacy_peer_index = 0u;
        return submit_privacy_mode(controller);
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_PRIVACY_MODE &&
        controller->state == RIN_BLUETOOTH_HCI_PRIVACY_MODE_PENDING) {
        ++controller->privacy_peer_index;
        if (controller->privacy_peer_index < controller->privacy.peer_count)
            return submit_privacy_mode(controller);
        return submit_privacy_timeout(controller);
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_RPA_TIMEOUT &&
        controller->state == RIN_BLUETOOTH_HCI_PRIVACY_TIMEOUT_PENDING) {
        return submit_privacy_enable(controller);
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_RESOLUTION_ENABLE &&
        controller->state == RIN_BLUETOOTH_HCI_PRIVACY_ENABLE_PENDING) {
        controller->address_resolution_enabled = 1u;
        return submit_event_mask(controller);
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_EVENT_MASK &&
        controller->state == RIN_BLUETOOTH_HCI_EVENT_MASK_PENDING) {
        controller->state = RIN_BLUETOOTH_HCI_READY;
        return RIN_BLUETOOTH_HCI_OK;
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_CREATE_CONNECTION_CANCEL &&
        controller->state == RIN_BLUETOOTH_HCI_CONNECT_CANCEL_PENDING) {
        controller->connection_pending = 0u;
        controller->connection_cancel_pending = 0u;
        bytes_zero(&controller->pending_connection,
                   sizeof(controller->pending_connection));
        controller->state = RIN_BLUETOOTH_HCI_READY;
        return RIN_BLUETOOTH_HCI_OK;
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_PARAMS &&
        controller->state == RIN_BLUETOOTH_HCI_ADV_PARAMS_PENDING)
        return submit_advertising_payload(
            controller, RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_DATA,
            RIN_BLUETOOTH_HCI_ADV_DATA_PENDING,
            controller->advertising.advertising_data,
            controller->advertising.advertising_data_size);
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_DATA &&
        controller->state == RIN_BLUETOOTH_HCI_ADV_DATA_PENDING)
        return submit_advertising_payload(
            controller, RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_RSP_DATA,
            RIN_BLUETOOTH_HCI_SCAN_RSP_DATA_PENDING,
            controller->advertising.scan_response_data,
            controller->advertising.scan_response_data_size);
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_RSP_DATA &&
        controller->state == RIN_BLUETOOTH_HCI_SCAN_RSP_DATA_PENDING)
        return submit_advertising_enable(
            controller, 1u, RIN_BLUETOOTH_HCI_ADV_ENABLE_PENDING);
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_ENABLE &&
        controller->state == RIN_BLUETOOTH_HCI_ADV_ENABLE_PENDING) {
        controller->advertising_active = 1u;
        controller->state = RIN_BLUETOOTH_HCI_READY;
        return RIN_BLUETOOTH_HCI_OK;
    }
    if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_ENABLE &&
        controller->state == RIN_BLUETOOTH_HCI_ADV_DISABLE_PENDING) {
        if (!controller->advertising_active) {
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        }
        controller->advertising_active = 0u;
        controller->state = RIN_BLUETOOTH_HCI_READY;
        return RIN_BLUETOOTH_HCI_OK;
    }
    if ((opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_PARAMS ||
         opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_EXT_SCAN_PARAMS) &&
        controller->state == RIN_BLUETOOTH_HCI_SCAN_PARAMS_PENDING) {
        uint8_t enable[6];
        uint8_t enable_size;
        uint16_t enable_opcode;
        int result;
        enable[0] = 1u;
        enable[1] = controller->scan_filter_duplicates;
        enable[2] = 0u;
        enable[3] = 0u;
        enable[4] = 0u;
        enable[5] = 0u;
        if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_PARAMS) {
            enable_opcode = RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_ENABLE;
            enable_size = 2u;
        } else {
            enable_opcode = RIN_BLUETOOTH_HCI_OPCODE_LE_EXT_SCAN_ENABLE;
            enable_size = sizeof(enable);
        }
        result = submit_command(controller, enable_opcode, enable,
                                enable_size,
                                RIN_BLUETOOTH_HCI_SCAN_ENABLE_PENDING);
        if (result != RIN_BLUETOOTH_HCI_OK) {
            controller->scan_active = 0u;
            controller->state = RIN_BLUETOOTH_HCI_READY;
        }
        return result;
    }
    if ((opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_ENABLE ||
         opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_EXT_SCAN_ENABLE) &&
        controller->state == RIN_BLUETOOTH_HCI_SCAN_ENABLE_PENDING) {
        controller->scan_active =
            opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_ENABLE
                ? RIN_BLUETOOTH_HCI_SCAN_LEGACY
                : RIN_BLUETOOTH_HCI_SCAN_EXTENDED;
        controller->state = RIN_BLUETOOTH_HCI_SCANNING;
        return RIN_BLUETOOTH_HCI_OK;
    }
    if ((opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_ENABLE ||
         opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_EXT_SCAN_ENABLE) &&
        controller->state == RIN_BLUETOOTH_HCI_SCAN_DISABLE_PENDING) {
        if ((opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_ENABLE &&
             controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_LEGACY) ||
            (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_EXT_SCAN_ENABLE &&
             controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_EXTENDED)) {
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
            controller->pending_opcode = 0u;
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        }
        controller->scan_active = RIN_BLUETOOTH_HCI_SCAN_INACTIVE;
        controller->state = RIN_BLUETOOTH_HCI_READY;
        return RIN_BLUETOOTH_HCI_OK;
    }
    controller->state = RIN_BLUETOOTH_HCI_FAILED;
    return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
}

static int command_status(RinBluetoothHciControllerV1* controller,
                          const uint8_t* parameters, uint32_t size)
{
    uint16_t opcode;
    if (size != 4u || parameters[1] == 0u)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    opcode = load_le16(parameters + 2u);
    if (opcode == 0u || opcode != controller->pending_opcode)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    if (parameters[0] == 0u) {
        if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_CREATE_CONNECTION &&
            controller->state == RIN_BLUETOOTH_HCI_CONNECT_PENDING) {
            controller->pending_opcode = 0u;
            controller->state = RIN_BLUETOOTH_HCI_CONNECTING;
            return RIN_BLUETOOTH_HCI_OK;
        }
        if (opcode == RIN_BLUETOOTH_HCI_OPCODE_DISCONNECT) {
            controller->pending_opcode = 0u;
            if (!controller->disconnect_pending) {
                controller->state = controller->disconnect_previous_state;
                controller->disconnect_previous_state =
                    RIN_BLUETOOTH_HCI_READY;
            }
            return RIN_BLUETOOTH_HCI_OK;
        }
        if (opcode == RIN_BLUETOOTH_HCI_OPCODE_LE_START_ENCRYPTION &&
            controller->state ==
                RIN_BLUETOOTH_HCI_ENCRYPTION_START_PENDING) {
            uint32_t index;
            RinBluetoothHciConnectionV1* pending = NULL;
            for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS;
                 ++index) {
                RinBluetoothHciConnectionV1* connection =
                    &controller->connections[index];
                if (!connection->encryption_command_pending) continue;
                if (pending != NULL) {
                    controller->state = RIN_BLUETOOTH_HCI_FAILED;
                    return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
                }
                pending = connection;
            }
            if (pending == NULL || !pending->encryption_pending) {
                controller->state = RIN_BLUETOOTH_HCI_FAILED;
                return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
            }
            pending->encryption_command_pending = 0u;
            controller->pending_opcode = 0u;
            controller->last_controller_status = 0u;
            controller->state = RIN_BLUETOOTH_HCI_READY;
            return RIN_BLUETOOTH_HCI_OK;
        }
        return RIN_BLUETOOTH_HCI_OK;
    }
    controller_command_failed(controller, parameters[0]);
    return RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE;
}

static RinBluetoothHciConnectionV1* connection_by_handle(
    RinBluetoothHciControllerV1* controller, uint16_t handle)
{
    uint32_t index;
    for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS; ++index) {
        RinBluetoothHciConnectionV1* connection =
            &controller->connections[index];
        if (connection->present && connection->handle == handle)
            return connection;
    }
    return NULL;
}

static int connection_handle_present(
    const RinBluetoothHciControllerV1* controller, uint16_t handle)
{
    uint32_t index;
    for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS; ++index) {
        const RinBluetoothHciConnectionV1* connection =
            &controller->connections[index];
        if (connection->present && connection->handle == handle) return 1;
    }
    return 0;
}

static void clear_connection_attempt(
    RinBluetoothHciControllerV1* controller)
{
    controller->connection_pending = 0u;
    bytes_zero(&controller->pending_connection,
               sizeof(controller->pending_connection));
    if (!controller->connection_cancel_pending &&
        controller->state == RIN_BLUETOOTH_HCI_CONNECTING)
        controller->state = RIN_BLUETOOTH_HCI_READY;
}

static int connection_values_valid(uint16_t interval, uint16_t latency,
                                   uint16_t timeout)
{
    uint64_t supervision_window;
    uint64_t required_timeout;
    if (interval < 0x0006u || interval > 0x0c80u || latency > 0x01f3u ||
        timeout < 0x000au || timeout > 0x0c80u)
        return 0;
    supervision_window = (uint64_t)2u * ((uint64_t)latency + 1u) * interval;
    required_timeout = (uint64_t)timeout * 8u;
    return required_timeout > supervision_window;
}

static int privacy_identity_known(
    const RinBluetoothHciControllerV1* controller, uint8_t address_type,
    const uint8_t address[6])
{
    uint32_t index;
    if (address_type < 2u) return 1;
    if (!controller->privacy_configured ||
        !controller->address_resolution_enabled)
        return 0;
    for (index = 0u; index < controller->privacy.peer_count; ++index) {
        const RinBluetoothHciPrivacyPeerV1* peer =
            &controller->privacy.peers[index];
        if (peer->identity_address_type == (uint8_t)(address_type - 2u) &&
            address_equal(peer->identity_address, address))
            return 1;
    }
    return 0;
}

static int connection_peer_matches(
    const RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciConnectionConfigV1* requested,
    uint8_t actual_type, const uint8_t actual_address[6],
    const uint8_t* parameters, int enhanced)
{
    if (actual_type == requested->peer_address_type &&
        address_equal(actual_address, requested->peer_address))
        return 1;
    if (actual_type < 2u ||
        !privacy_identity_known(controller, actual_type, actual_address))
        return 0;
    if (requested->peer_address_type == (uint8_t)(actual_type - 2u) &&
        address_equal(actual_address, requested->peer_address))
        return 1;
    if (enhanced && requested->peer_address_type == 1u) {
        uint8_t peer_rpa[6];
        uint32_t index;
        for (index = 0u; index < 6u; ++index)
            peer_rpa[index] = parameters[23u - index];
        return address_equal(peer_rpa, requested->peer_address);
    }
    return 0;
}

static uint32_t connection_free_slot(
    const RinBluetoothHciControllerV1* controller)
{
    uint32_t index;
    for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS; ++index) {
        if (!controller->connections[index].present) return index;
    }
    return RIN_BLUETOOTH_HCI_MAX_CONNECTIONS;
}

static int submit_disconnect_handle(
    RinBluetoothHciControllerV1* controller, uint16_t handle,
    uint32_t generation, RinBluetoothHciState previous_state)
{
    uint8_t parameters[3];
    RinBluetoothHciConnectionV1* connection;
    int result;
    if (handle > 0x0effu || controller->pending_opcode != 0u ||
        controller->disconnect_pending)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    connection = connection_by_handle(controller, handle);
    if (generation != 0u &&
        (connection == NULL || connection->generation != generation))
        return RIN_BLUETOOTH_HCI_NOT_FOUND;
    store_le16(parameters, handle);
    parameters[2] = 0x13u; /* Remote User Terminated Connection. */
    controller->disconnect_handle = handle;
    controller->disconnect_generation = generation;
    controller->disconnect_previous_state = previous_state;
    controller->disconnect_pending = 1u;
    if (connection != NULL) connection->disconnect_pending = 1u;
    result = submit_command(controller, RIN_BLUETOOTH_HCI_OPCODE_DISCONNECT,
                            parameters, sizeof(parameters),
                            RIN_BLUETOOTH_HCI_DISCONNECT_PENDING);
    if (result != RIN_BLUETOOTH_HCI_OK) {
        if (connection != NULL) connection->disconnect_pending = 0u;
        controller->disconnect_pending = 0u;
        controller->disconnect_handle = 0u;
        controller->disconnect_generation = 0u;
        controller->disconnect_previous_state = RIN_BLUETOOTH_HCI_READY;
        if (controller->state != RIN_BLUETOOTH_HCI_FAILED)
            controller->state = previous_state;
    }
    return result;
}

static int connection_complete(RinBluetoothHciControllerV1* controller,
                               const uint8_t* parameters, uint32_t size,
                               int enhanced)
{
    uint32_t expected_size = enhanced ? 31u : 19u;
    uint16_t handle;
    uint16_t interval;
    uint16_t latency;
    uint16_t timeout;
    uint8_t peer_address[6];
    uint32_t index;
    uint32_t slot;
    RinBluetoothHciConnectionV1* connection;
    if (size != expected_size) {
        if (controller->connection_pending) {
            clear_connection_attempt(controller);
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
        }
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    }
    if (!controller->connection_pending) {
        if (parameters[1] != 0u) return RIN_BLUETOOTH_HCI_NOT_FOUND;
        handle = load_le16(parameters + 2u);
        if (handle <= 0x0effu &&
            !connection_handle_present(controller, handle) &&
            controller->pending_opcode == 0u &&
            (controller->state == RIN_BLUETOOTH_HCI_READY ||
             controller->state == RIN_BLUETOOTH_HCI_CONNECTING)) {
            RinBluetoothHciState previous_state =
                controller->state == RIN_BLUETOOTH_HCI_CONNECTING
                    ? RIN_BLUETOOTH_HCI_READY
                    : controller->state;
            int cleanup;
            if (controller->state == RIN_BLUETOOTH_HCI_CONNECTING)
                controller->state = RIN_BLUETOOTH_HCI_READY;
            cleanup = submit_disconnect_handle(
                controller, handle, 0u, previous_state);
            if (cleanup != RIN_BLUETOOTH_HCI_OK) {
                controller->state = RIN_BLUETOOTH_HCI_FAILED;
                return RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE;
            }
        } else {
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
            return RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE;
        }
        return RIN_BLUETOOTH_HCI_OK;
    }
    if (parameters[1] != 0u) {
        clear_connection_attempt(controller);
        return RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE;
    }
    if (controller->state != RIN_BLUETOOTH_HCI_CONNECTING &&
        controller->state != RIN_BLUETOOTH_HCI_CONNECT_CANCEL_PENDING) {
        controller->state = RIN_BLUETOOTH_HCI_FAILED;
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    }
    handle = load_le16(parameters + 2u);
    interval = load_le16(parameters + (enhanced ? 24u : 12u));
    latency = load_le16(parameters + (enhanced ? 26u : 14u));
    timeout = load_le16(parameters + (enhanced ? 28u : 16u));
    for (index = 0u; index < 6u; ++index)
        peer_address[index] = parameters[11u - index];
    if (handle > 0x0effu || parameters[4] != 0u ||
        parameters[5] > 3u || address_invalid(peer_address) ||
        (parameters[5] == 3u && !static_random_address_valid(peer_address)) ||
        !privacy_identity_known(controller, parameters[5], peer_address) ||
        !connection_peer_matches(controller, &controller->pending_connection,
                                 parameters[5], peer_address, parameters,
                                 enhanced) ||
        !connection_values_valid(interval, latency, timeout) ||
        parameters[enhanced ? 30u : 18u] > 7u ||
        connection_handle_present(controller, handle)) {
        clear_connection_attempt(controller);
        if (handle <= 0x0effu &&
            !connection_handle_present(controller, handle) &&
            controller->pending_opcode == 0u &&
            !controller->connection_cancel_pending) {
            int cleanup;
            controller->state = RIN_BLUETOOTH_HCI_READY;
            cleanup = submit_disconnect_handle(
                controller, handle, 0u, RIN_BLUETOOTH_HCI_READY);
            if (cleanup == RIN_BLUETOOTH_HCI_OK)
                return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        }
        controller->state = RIN_BLUETOOTH_HCI_FAILED;
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    }
    slot = connection_free_slot(controller);
    if (slot == RIN_BLUETOOTH_HCI_MAX_CONNECTIONS ||
        controller->connection_generation == UINT32_MAX) {
        clear_connection_attempt(controller);
        if (!controller->connection_cancel_pending &&
            controller->pending_opcode == 0u) {
            int cleanup = submit_disconnect_handle(
                controller, handle, 0u, RIN_BLUETOOTH_HCI_READY);
            if (cleanup != RIN_BLUETOOTH_HCI_OK) {
                controller->state = RIN_BLUETOOTH_HCI_FAILED;
                return RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE;
            }
        } else {
            controller->state = RIN_BLUETOOTH_HCI_FAILED;
            return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
        }
        return RIN_BLUETOOTH_HCI_OK;
    }
    connection = &controller->connections[slot];
    bytes_zero(connection, sizeof(*connection));
    connection->version = RIN_BLUETOOTH_HCI_CONNECTION_LAYOUT_VERSION;
    connection->generation = controller->connection_generation++;
    connection->handle = handle;
    connection->connection_interval = interval;
    connection->connection_latency = latency;
    connection->supervision_timeout = timeout;
    connection->present = 1u;
    connection->role = parameters[4];
    connection->peer_address_type = parameters[5];
    bytes_copy(connection->peer_address, peer_address,
               sizeof(connection->peer_address));
    ++controller->connection_count;
    clear_connection_attempt(controller);
    if (controller->connection_cancel_pending) {
        /* The LE connection event may race the cancel command's completion. */
        controller->state = RIN_BLUETOOTH_HCI_CONNECT_CANCEL_PENDING;
    } else {
        controller->state = RIN_BLUETOOTH_HCI_READY;
    }
    return RIN_BLUETOOTH_HCI_OK;
}

static int disconnection_complete(RinBluetoothHciControllerV1* controller,
                                  const uint8_t* parameters, uint32_t size)
{
    uint16_t handle;
    RinBluetoothHciConnectionV1* connection;
    int matched_disconnect;
    if (size != 4u) return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    handle = load_le16(parameters + 1u);
    if (handle > 0x0effu) return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    connection = connection_by_handle(controller, handle);
    matched_disconnect = controller->disconnect_pending &&
                         controller->disconnect_handle == handle;
    if (parameters[0] != 0u) {
        if (matched_disconnect) {
            int unmanaged = controller->disconnect_generation == 0u;
            if (connection != NULL &&
                connection->generation == controller->disconnect_generation)
                connection->disconnect_pending = 0u;
            controller->disconnect_pending = 0u;
            controller->disconnect_handle = 0u;
            controller->disconnect_generation = 0u;
            controller->state = unmanaged
                                    ? RIN_BLUETOOTH_HCI_FAILED
                                    : controller->disconnect_previous_state;
            controller->disconnect_previous_state = RIN_BLUETOOTH_HCI_READY;
        }
        return RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE;
    }
    if (connection != NULL) {
        bytes_zero(connection, sizeof(*connection));
        --controller->connection_count;
    }
    if (matched_disconnect) {
        controller->disconnect_pending = 0u;
        controller->disconnect_handle = 0u;
        controller->disconnect_generation = 0u;
        controller->state = controller->disconnect_previous_state;
        controller->disconnect_previous_state = RIN_BLUETOOTH_HCI_READY;
    }
    return connection != NULL || matched_disconnect ||
                   controller->pending_opcode ==
                       RIN_BLUETOOTH_HCI_OPCODE_DISCONNECT
               ? RIN_BLUETOOTH_HCI_OK
               : RIN_BLUETOOTH_HCI_NOT_FOUND;
}

static int long_term_key_request(
    RinBluetoothHciControllerV1* controller, const uint8_t* parameters,
    uint32_t size)
{
    uint16_t handle;
    RinBluetoothHciConnectionV1* connection;
    if (size != 13u ||
        parameters[0] !=
            RIN_BLUETOOTH_HCI_SUBEVENT_LONG_TERM_KEY_REQUEST)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    handle = load_le16(parameters + 1u);
    if (handle > 0x0effu) return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    connection = connection_by_handle(controller, handle);
    if (connection == NULL) return RIN_BLUETOOTH_HCI_NOT_FOUND;
    if (connection->role != 1u || connection->encrypted ||
        connection->encryption_pending || connection->ltk_request_pending ||
        connection->disconnect_pending)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    bytes_copy(connection->ltk_random, parameters + 3u,
               sizeof(connection->ltk_random));
    connection->ltk_ediv = load_le16(parameters + 11u);
    connection->ltk_request_pending = 1u;
    return RIN_BLUETOOTH_HCI_OK;
}

static int encryption_change(RinBluetoothHciControllerV1* controller,
                             const uint8_t* parameters, uint32_t size)
{
    uint16_t handle;
    RinBluetoothHciConnectionV1* connection;
    uint8_t status;
    uint8_t enabled;
    uint8_t was_pending;
    if (size != 5u ||
        parameters[0] != RIN_BLUETOOTH_HCI_SUBEVENT_ENCRYPTION_CHANGE)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    status = parameters[1];
    handle = load_le16(parameters + 2u);
    enabled = parameters[4];
    if (handle > 0x0effu || enabled > 1u)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    connection = connection_by_handle(controller, handle);
    if (connection == NULL) return RIN_BLUETOOTH_HCI_NOT_FOUND;
    if (connection->encryption_command_pending ||
        connection->ltk_reply_command_pending)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    if (status != 0u) {
        connection->encrypted = 0u;
        connection->encryption_pending = 0u;
        connection->encryption_command_pending = 0u;
        connection->ltk_request_pending = 0u;
        connection->ltk_reply_command_pending = 0u;
        bytes_zero(connection->ltk_random, sizeof(connection->ltk_random));
        connection->ltk_ediv = 0u;
        return RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE;
    }
    if (enabled != 0u) {
        if (!connection->encryption_pending ||
            connection->encryption_command_pending ||
            connection->ltk_request_pending ||
            connection->ltk_reply_command_pending)
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        connection->encrypted = 1u;
        connection->encryption_pending = 0u;
        return RIN_BLUETOOTH_HCI_OK;
    }
    was_pending = connection->encryption_pending;
    connection->encrypted = 0u;
    connection->encryption_pending = 0u;
    return was_pending ? RIN_BLUETOOTH_HCI_CONTROLLER_FAILURE
                       : RIN_BLUETOOTH_HCI_OK;
}

static int parse_report(const uint8_t* parameters, uint32_t size,
                        uint32_t* consumed,
                        RinBluetoothHciParsedReport* report)
{
    uint8_t data_size;
    uint32_t index;
    if (size < 10u) return 0;
    bytes_zero(report, sizeof(*report));
    report->report_format = RIN_BLUETOOTH_HCI_REPORT_LEGACY;
    report->data_status = RIN_BLUETOOTH_HCI_DATA_COMPLETE;
    report->primary_phy = 1u;
    report->advertising_sid = 0xffu;
    report->tx_power_dbm = 127;
    report->event_type = parameters[0];
    report->address_type = parameters[1];
    if (report->event_type > 4u || report->address_type > 1u) return 0;
    for (index = 0u; index < 6u; ++index)
        report->address[index] = parameters[7u - index];
    if (address_invalid(report->address)) return 0;
    data_size = parameters[8];
    if (data_size > RIN_BLUETOOTH_HCI_MAX_LEGACY_ADV_DATA ||
        size < 10u + data_size) return 0;
    report->rssi_dbm = (int8_t)parameters[9u + data_size];
    if (report->rssi_dbm < -127 ||
        (report->rssi_dbm > 20 && report->rssi_dbm != 127)) return 0;
    if (!parse_advertising_data(parameters + 9u, data_size, report)) return 0;
    *consumed = 10u + data_size;
    return 1;
}

static int signal_strength_valid(int8_t value)
{
    return value == 127 || (value >= -127 && value <= 20);
}

static int legacy_extended_event_type_valid(uint16_t event_type)
{
    return event_type == 0x0013u || event_type == 0x0015u ||
           event_type == 0x0012u || event_type == 0x0010u ||
           event_type == 0x001bu || event_type == 0x001au;
}

static int extended_address_type_valid(uint8_t address_type)
{
    return address_type <= 3u || address_type == 0xffu;
}

static int extended_direct_address_type_valid(uint8_t address_type)
{
    return address_type <= 3u || address_type == 0xfeu;
}

static int parse_extended_report(const uint8_t* parameters, uint32_t size,
                                 uint32_t* consumed,
                                 RinBluetoothHciParsedReport* report)
{
    uint16_t event_type;
    uint16_t periodic_interval;
    uint8_t data_size;
    uint8_t data_status;
    uint8_t address_type;
    uint8_t primary_phy;
    uint8_t secondary_phy;
    uint8_t advertising_sid;
    uint32_t index;

    if (parameters == NULL || consumed == NULL || report == NULL || size < 24u)
        return 0;
    event_type = load_le16(parameters);
    address_type = parameters[2];
    primary_phy = parameters[9];
    secondary_phy = parameters[10];
    advertising_sid = parameters[11];
    periodic_interval = load_le16(parameters + 14u);
    data_size = parameters[23];
    data_status = (uint8_t)((event_type >> 5u) & 0x03u);

    if ((event_type & 0xff80u) != 0u || data_status == 3u ||
        !extended_address_type_valid(address_type) ||
        (primary_phy != 1u && primary_phy != 3u) ||
        (secondary_phy > 3u) ||
        (advertising_sid > 0x0fu && advertising_sid != 0xffu) ||
        !signal_strength_valid((int8_t)parameters[12]) ||
        !signal_strength_valid((int8_t)parameters[13]) ||
        (periodic_interval != 0u && periodic_interval < 6u) ||
        data_size > RIN_BLUETOOTH_HCI_MAX_REPORT_DATA ||
        size < 24u + data_size) {
        return 0;
    }

    if ((event_type & 0x0010u) != 0u) {
        if (!legacy_extended_event_type_valid(event_type) ||
            data_status != RIN_BLUETOOTH_HCI_DATA_COMPLETE ||
            primary_phy != 1u || secondary_phy != 0u ||
            advertising_sid != 0xffu || periodic_interval != 0u)
            return 0;
    } else if ((event_type & 0x0003u) == 0x0003u) {
        /* Extended PDUs cannot be both connectable and scannable. */
        return 0;
    }
    if ((event_type & 0x0008u) != 0u &&
        (event_type & 0x0002u) == 0u)
        return 0;

    bytes_zero(report, sizeof(*report));
    report->report_format = RIN_BLUETOOTH_HCI_REPORT_EXTENDED;
    report->data_status = data_status;
    report->event_type = (uint8_t)(event_type & 0x001fu);
    report->address_type = address_type;
    report->primary_phy = primary_phy;
    report->secondary_phy = secondary_phy;
    report->advertising_sid = advertising_sid;
    report->tx_power_dbm = (int8_t)parameters[12];
    report->rssi_dbm = (int8_t)parameters[13];
    report->periodic_advertising_interval = periodic_interval;

    if (address_type == 0xffu) {
        /* Anonymous advertisements have no stable address.  Canonicalize the
         * ignored wire bytes; SID remains part of the table identity. */
        if ((event_type & 0x001fu) != 0u) return 0;
    } else {
        for (index = 0u; index < 6u; ++index)
            report->address[index] = parameters[8u - index];
        if (address_invalid(report->address)) return 0;
    }

    if ((event_type & 0x0004u) != 0u) {
        report->direct_address_type = parameters[16];
        if (!extended_direct_address_type_valid(
                report->direct_address_type)) return 0;
        for (index = 0u; index < 6u; ++index)
            report->direct_address[index] = parameters[22u - index];
        if (address_invalid(report->direct_address)) return 0;
        report->direct_address_present = 1u;
    }

    report->data_size = data_size;
    report->data = parameters + 24u;
    *consumed = 24u + data_size;
    return 1;
}

static int fragment_identity_equal(
    const RinBluetoothHciFragmentV1* fragment,
    const RinBluetoothHciParsedReport* report)
{
    return fragment->active &&
           fragment->address_type == report->address_type &&
           fragment->advertising_sid == report->advertising_sid &&
           (report->address_type == 0xffu ||
            address_equal(fragment->address, report->address));
}

static int fragment_metadata_equal(
    const RinBluetoothHciFragmentV1* fragment,
    const RinBluetoothHciParsedReport* report)
{
    return fragment_identity_equal(fragment, report) &&
           fragment->event_type == report->event_type &&
           fragment->primary_phy == report->primary_phy &&
           fragment->secondary_phy == report->secondary_phy &&
           fragment->periodic_advertising_interval ==
               report->periodic_advertising_interval &&
           fragment->direct_address_present ==
               report->direct_address_present &&
           (!fragment->direct_address_present ||
            (fragment->direct_address_type == report->direct_address_type &&
             address_equal(fragment->direct_address,
                           report->direct_address)));
}

static uint32_t fragment_find(
    const RinBluetoothHciFragmentV1 fragments[
        RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS],
    const RinBluetoothHciParsedReport* report)
{
    uint32_t index;
    for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS; ++index) {
        if (fragment_identity_equal(&fragments[index], report)) return index;
    }
    return RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS;
}

static uint32_t fragment_free_slot(
    const RinBluetoothHciFragmentV1 fragments[
        RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS])
{
    uint32_t index;
    for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS; ++index) {
        if (!fragments[index].active) return index;
    }
    return RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS;
}

static void fragment_start(RinBluetoothHciFragmentV1* fragment,
                           const RinBluetoothHciParsedReport* report,
                           uint64_t now_ms)
{
    bytes_zero(fragment, sizeof(*fragment));
    fragment->active = 1u;
    fragment->address_type = report->address_type;
    bytes_copy(fragment->address, report->address,
               sizeof(fragment->address));
    fragment->event_type = report->event_type;
    fragment->primary_phy = report->primary_phy;
    fragment->secondary_phy = report->secondary_phy;
    fragment->advertising_sid = report->advertising_sid;
    fragment->periodic_advertising_interval =
        report->periodic_advertising_interval;
    fragment->direct_address_type = report->direct_address_type;
    fragment->direct_address_present = report->direct_address_present;
    bytes_copy(fragment->direct_address, report->direct_address,
               sizeof(fragment->direct_address));
    fragment->last_update_ms = now_ms;
}

static uint32_t device_slot(RinBluetoothHciControllerV1* controller,
                            const RinBluetoothHciParsedReport* report)
{
    uint32_t index;
    uint32_t free_slot = RIN_BLUETOOTH_HCI_MAX_DEVICES;
    uint32_t oldest_slot = 0u;
    uint64_t oldest_time = UINT64_MAX;
    for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_DEVICES; ++index) {
        RinBluetoothHciDeviceV1* device = &controller->devices[index];
        if (device->present && device->address_type == report->address_type &&
            ((report->address_type == 0xffu &&
              device->advertising_sid == report->advertising_sid) ||
             (report->address_type != 0xffu &&
              address_equal(device->address, report->address)))) return index;
        if (!device->present && free_slot == RIN_BLUETOOTH_HCI_MAX_DEVICES)
            free_slot = index;
        if (device->present && device->last_seen_ms < oldest_time) {
            oldest_time = device->last_seen_ms;
            oldest_slot = index;
        }
    }
    return free_slot != RIN_BLUETOOTH_HCI_MAX_DEVICES ? free_slot : oldest_slot;
}

static void publish_report(RinBluetoothHciControllerV1* controller,
                           const RinBluetoothHciParsedReport* report,
                           uint64_t now_ms)
{
    uint32_t slot = device_slot(controller, report);
    RinBluetoothHciDeviceV1* device = &controller->devices[slot];
    int was_present = device->present != 0u;
    int same_device = was_present &&
        device->address_type == report->address_type &&
        ((report->address_type == 0xffu &&
          device->advertising_sid == report->advertising_sid) ||
         (report->address_type != 0xffu &&
          address_equal(device->address, report->address)));
    char old_name[RIN_BLUETOOTH_HCI_MAX_NAME];
    uint8_t old_name_size = device->name_size;
    uint8_t old_flags = device->advertising_flags;
    uint8_t old_flags_present = device->advertising_flags_present;
    if (same_device && report->name_size == 0u)
        bytes_copy(old_name, device->name, sizeof(old_name));
    bytes_zero(device, sizeof(*device));
    device->version = RIN_BLUETOOTH_HCI_VERSION;
    device->generation = controller->generation;
    device->present = 1u;
    device->address_type = report->address_type;
    device->event_type = report->event_type;
    device->rssi_dbm = report->rssi_dbm;
    bytes_copy(device->address, report->address, sizeof(device->address));
    device->data_size = report->data_size;
    if (report->data_size != 0u)
        bytes_copy(device->data, report->data, report->data_size);
    if (report->name_size != 0u) {
        device->name_size = report->name_size;
        bytes_copy(device->name, report->name, report->name_size + 1u);
    } else if (same_device) {
        device->name_size = old_name_size;
        bytes_copy(device->name, old_name, sizeof(device->name));
    }
    if (report->advertising_flags_present) {
        device->advertising_flags = report->advertising_flags;
        device->advertising_flags_present = 1u;
    } else if (same_device) {
        device->advertising_flags = old_flags;
        device->advertising_flags_present = old_flags_present;
    }
    device->last_seen_ms = now_ms;
    device->report_format = report->report_format;
    device->data_status = report->data_status;
    device->primary_phy = report->primary_phy;
    device->secondary_phy = report->secondary_phy;
    device->advertising_sid = report->advertising_sid;
    device->tx_power_dbm = report->tx_power_dbm;
    device->periodic_advertising_interval =
        report->periodic_advertising_interval;
    device->direct_address_type = report->direct_address_type;
    device->direct_address_present = report->direct_address_present;
    if (report->direct_address_present) {
        bytes_copy(device->direct_address, report->direct_address,
                   sizeof(device->direct_address));
    }
    if (!was_present && controller->device_count < RIN_BLUETOOTH_HCI_MAX_DEVICES)
        ++controller->device_count;
}

static int advertising_report(RinBluetoothHciControllerV1* controller,
                              const uint8_t* parameters, uint32_t size)
{
    RinBluetoothHciParsedReport reports[RIN_BLUETOOTH_HCI_MAX_REPORTS];
    uint32_t report_count;
    uint32_t offset = 2u;
    uint32_t index;
    uint64_t now_ms;
    int transport_mutated;
    if (controller->state != RIN_BLUETOOTH_HCI_SCANNING ||
        controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_LEGACY)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    if (size < 2u || parameters[0] != RIN_BLUETOOTH_HCI_SUBEVENT_ADV_REPORT)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    report_count = parameters[1];
    if (report_count == 0u || report_count > RIN_BLUETOOTH_HCI_MAX_REPORTS)
        return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
    for (index = 0u; index < report_count; ++index) {
        uint32_t consumed = 0u;
        if (!parse_report(parameters + offset, size - offset,
                          &consumed, &reports[index]))
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        offset += consumed;
    }
    if (offset != size) return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    now_ms = monotonic_ms_checked(controller, &transport_mutated);
    if (transport_mutated) return RIN_BLUETOOTH_HCI_TRANSPORT_FAILURE;
    if (advance_report_generation(controller) != RIN_BLUETOOTH_HCI_OK)
        return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
    for (index = 0u; index < report_count; ++index)
        publish_report(controller, &reports[index], now_ms);
    return RIN_BLUETOOTH_HCI_OK;
}

static int extended_advertising_report(
    RinBluetoothHciControllerV1* controller,
    const uint8_t* parameters, uint32_t size)
{
    RinBluetoothHciParsedReport reports[
        RIN_BLUETOOTH_HCI_MAX_EXT_REPORTS];
    RinBluetoothHciParsedReport published[
        RIN_BLUETOOTH_HCI_MAX_EXT_REPORTS];
    RinBluetoothHciFragmentV1* staged_fragments =
        controller->fragment_scratch;
    uint8_t (*assembled_data)[RIN_BLUETOOTH_HCI_MAX_ADV_DATA] =
        controller->fragment_assembled_scratch;
    uint32_t report_count;
    uint32_t publish_count = 0u;
    uint32_t assembled_count = 0u;
    uint32_t fragment_count = 0u;
    uint32_t offset = 2u;
    uint32_t index;
    uint64_t now_ms;
    int transport_mutated;

    if (controller->state != RIN_BLUETOOTH_HCI_SCANNING ||
        controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_EXTENDED)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    if (size < 2u ||
        parameters[0] != RIN_BLUETOOTH_HCI_SUBEVENT_EXT_ADV_REPORT)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    report_count = parameters[1];
    if (report_count == 0u ||
        report_count > RIN_BLUETOOTH_HCI_MAX_EXT_REPORTS)
        return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;

    for (index = 0u; index < report_count; ++index) {
        uint32_t consumed = 0u;
        if (!parse_extended_report(parameters + offset, size - offset,
                                   &consumed, &reports[index]))
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        offset += consumed;
    }
    if (offset != size) return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;

    now_ms = monotonic_ms_checked(controller, &transport_mutated);
    if (transport_mutated) return RIN_BLUETOOTH_HCI_TRANSPORT_FAILURE;
    bytes_copy(staged_fragments, controller->fragments,
               sizeof(controller->fragments));
    for (index = 0u; index < report_count; ++index) {
        RinBluetoothHciParsedReport* report = &reports[index];
        uint32_t slot = fragment_find(staged_fragments, report);

        if (report->data_status == RIN_BLUETOOTH_HCI_DATA_MORE) {
            RinBluetoothHciFragmentV1* fragment;
            if (report->data_size == 0u)
                return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
            if (slot == RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS) {
                slot = fragment_free_slot(staged_fragments);
                if (slot == RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS)
                    return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
                fragment_start(&staged_fragments[slot], report, now_ms);
            } else if (!fragment_metadata_equal(
                           &staged_fragments[slot], report)) {
                return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
            }
            fragment = &staged_fragments[slot];
            if ((uint32_t)fragment->data_size + report->data_size >
                RIN_BLUETOOTH_HCI_MAX_ADV_DATA)
                return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
            bytes_copy(fragment->data + fragment->data_size, report->data,
                       report->data_size);
            fragment->data_size =
                (uint16_t)(fragment->data_size + report->data_size);
            fragment->last_update_ms = now_ms;
            continue;
        }

        if (report->data_status == RIN_BLUETOOTH_HCI_DATA_TRUNCATED) {
            if (slot != RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS) {
                if (!fragment_metadata_equal(
                        &staged_fragments[slot], report))
                    return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
                bytes_zero(&staged_fragments[slot],
                           sizeof(staged_fragments[slot]));
            }
            continue;
        }

        published[publish_count] = *report;
        if (slot != RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS) {
            RinBluetoothHciFragmentV1* fragment = &staged_fragments[slot];
            uint32_t assembled_size;
            if (!fragment_metadata_equal(fragment, report))
                return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
            if (assembled_count == RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS)
                return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
            assembled_size =
                (uint32_t)fragment->data_size + report->data_size;
            if (assembled_size > RIN_BLUETOOTH_HCI_MAX_ADV_DATA)
                return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
            bytes_copy(assembled_data[assembled_count], fragment->data,
                       fragment->data_size);
            bytes_copy(assembled_data[assembled_count] + fragment->data_size,
                       report->data, report->data_size);
            published[publish_count].data = assembled_data[assembled_count];
            published[publish_count].data_size = (uint16_t)assembled_size;
            bytes_zero(fragment, sizeof(*fragment));
            ++assembled_count;
        }
        if (!parse_advertising_data(published[publish_count].data,
                                    published[publish_count].data_size,
                                    &published[publish_count]))
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        ++publish_count;
    }

    for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_FRAGMENT_CHAINS; ++index) {
        if (staged_fragments[index].active) ++fragment_count;
    }
    if (publish_count != 0u) {
        if (advance_report_generation(controller) != RIN_BLUETOOTH_HCI_OK)
            return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
    }
    bytes_copy(controller->fragments, staged_fragments,
               sizeof(controller->fragments));
    controller->fragment_count = fragment_count;
    if (publish_count != 0u) {
        for (index = 0u; index < publish_count; ++index)
            publish_report(controller, &published[index], now_ms);
    }
    return RIN_BLUETOOTH_HCI_OK;
}

int rin_bluetooth_hci_init(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciTransportV1* transport)
{
    if (controller == NULL || !transport_valid(transport))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    bytes_zero(controller, sizeof(*controller));
    controller->version = RIN_BLUETOOTH_HCI_CONTROLLER_LAYOUT_VERSION;
    controller->struct_size = sizeof(*controller);
    controller->state = RIN_BLUETOOTH_HCI_IDLE;
    controller->generation = 1u;
    controller->connection_generation = 1u;
    controller->transport = *transport;
    return RIN_BLUETOOTH_HCI_OK;
}

int rin_bluetooth_hci_start(RinBluetoothHciControllerV1* controller)
{
    if (!controller_valid(controller)) return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_IDLE ||
        controller->pending_opcode != 0u) return RIN_BLUETOOTH_HCI_INVALID_STATE;
    return submit_command(controller, RIN_BLUETOOTH_HCI_OPCODE_RESET, NULL, 0u,
                          RIN_BLUETOOTH_HCI_RESET_PENDING);
}

int rin_bluetooth_hci_configure_static_random_address(
    RinBluetoothHciControllerV1* controller, const uint8_t address[6])
{
    if (!controller_valid(controller) || !static_random_address_valid(address))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_IDLE ||
        controller->pending_opcode != 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    bytes_copy(controller->static_random_address, address,
               sizeof(controller->static_random_address));
    controller->static_random_address_configured = 1u;
    return RIN_BLUETOOTH_HCI_OK;
}

int rin_bluetooth_hci_configure_privacy(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciPrivacyConfigV1* config)
{
    if (!controller_valid(controller) || !privacy_config_valid(config))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_IDLE ||
        controller->pending_opcode != 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    controller->privacy = *config;
    controller->privacy_configured = 1u;
    controller->address_resolution_enabled = 0u;
    controller->privacy_peer_index = 0u;
    return RIN_BLUETOOTH_HCI_OK;
}

int rin_bluetooth_hci_start_scan(
    RinBluetoothHciControllerV1* controller, int active_scan,
    uint16_t interval, uint16_t window, int filter_duplicates)
{
    uint8_t parameters[7];
    int result;
    if (!controller_valid(controller)) return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if ((active_scan != 0 && active_scan != 1) ||
        (filter_duplicates != 0 && filter_duplicates != 1) ||
        interval < 0x0004u || interval > 0x4000u ||
        window < 0x0004u || window > interval)
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_READY ||
        controller->pending_opcode != 0u ||
        controller->advertising_active != 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    if (controller->scan_command_family ==
        RIN_BLUETOOTH_HCI_SCAN_EXTENDED)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    parameters[0] = (uint8_t)active_scan;
    store_le16(parameters + 1u, interval);
    store_le16(parameters + 3u, window);
    parameters[5] = (uint8_t)(
        controller->local_address_type +
        (controller->address_resolution_enabled ? 2u : 0u));
    parameters[6] = 0u;
    controller->scan_filter_duplicates = (uint8_t)filter_duplicates;
    result = submit_command(controller,
                            RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_PARAMS,
                            parameters, sizeof(parameters),
                            RIN_BLUETOOTH_HCI_SCAN_PARAMS_PENDING);
    if (result == RIN_BLUETOOTH_HCI_OK)
        controller->scan_command_family = RIN_BLUETOOTH_HCI_SCAN_LEGACY;
    return result;
}

int rin_bluetooth_hci_start_extended_scan(
    RinBluetoothHciControllerV1* controller, int active_scan,
    uint16_t interval, uint16_t window, int filter_duplicates)
{
    RinBluetoothHciExtendedScanConfigV1 config;
    if ((active_scan != 0 && active_scan != 1) ||
        (filter_duplicates != 0 && filter_duplicates != 1))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    bytes_zero(&config, sizeof(config));
    config.version = RIN_BLUETOOTH_HCI_VERSION;
    config.struct_size = sizeof(config);
    config.scanning_phys = RIN_BLUETOOTH_HCI_SCAN_PHY_1M;
    config.filter_duplicates = (uint8_t)filter_duplicates;
    config.active_1m = (uint8_t)active_scan;
    config.interval_1m = interval;
    config.window_1m = window;
    return rin_bluetooth_hci_start_extended_scan_config(controller, &config);
}

static int extended_scan_phy_valid(int selected, uint8_t active_scan,
                                   uint16_t interval, uint16_t window)
{
    if (!selected)
        return active_scan == 0u && interval == 0u && window == 0u;
    return active_scan <= 1u &&
           interval >= 0x0004u &&
           window >= 0x0004u && window <= interval;
}

int rin_bluetooth_hci_start_extended_scan_config(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciExtendedScanConfigV1* config)
{
    uint8_t parameters[13];
    uint8_t parameter_size = 3u;
    int result;
    if (!controller_valid(controller) || config == NULL ||
        config->version != RIN_BLUETOOTH_HCI_VERSION ||
        config->struct_size != sizeof(*config) ||
        config->scanning_phys == 0u ||
        (config->scanning_phys &
         ~(RIN_BLUETOOTH_HCI_SCAN_PHY_1M |
           RIN_BLUETOOTH_HCI_SCAN_PHY_CODED)) != 0u ||
        config->filter_duplicates > 1u ||
        !extended_scan_phy_valid(
            (config->scanning_phys & RIN_BLUETOOTH_HCI_SCAN_PHY_1M) != 0u,
            config->active_1m, config->interval_1m, config->window_1m) ||
        !extended_scan_phy_valid(
            (config->scanning_phys & RIN_BLUETOOTH_HCI_SCAN_PHY_CODED) != 0u,
            config->active_coded, config->interval_coded,
            config->window_coded))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_READY ||
        controller->pending_opcode != 0u ||
        controller->advertising_active != 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    if (controller->scan_command_family ==
        RIN_BLUETOOTH_HCI_SCAN_LEGACY)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;

    parameters[0] = (uint8_t)(
        controller->local_address_type +
        (controller->address_resolution_enabled ? 2u : 0u));
    parameters[1] = 0u;
    parameters[2] = config->scanning_phys;
    if ((config->scanning_phys & RIN_BLUETOOTH_HCI_SCAN_PHY_1M) != 0u) {
        parameters[parameter_size++] = config->active_1m;
        store_le16(parameters + parameter_size, config->interval_1m);
        parameter_size += 2u;
        store_le16(parameters + parameter_size, config->window_1m);
        parameter_size += 2u;
    }
    if ((config->scanning_phys & RIN_BLUETOOTH_HCI_SCAN_PHY_CODED) != 0u) {
        parameters[parameter_size++] = config->active_coded;
        store_le16(parameters + parameter_size, config->interval_coded);
        parameter_size += 2u;
        store_le16(parameters + parameter_size, config->window_coded);
        parameter_size += 2u;
    }
    controller->scan_filter_duplicates = config->filter_duplicates;
    result = submit_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_EXT_SCAN_PARAMS,
        parameters, parameter_size,
        RIN_BLUETOOTH_HCI_SCAN_PARAMS_PENDING);
    if (result == RIN_BLUETOOTH_HCI_OK)
        controller->scan_command_family = RIN_BLUETOOTH_HCI_SCAN_EXTENDED;
    return result;
}

int rin_bluetooth_hci_stop_scan(RinBluetoothHciControllerV1* controller)
{
    const uint8_t legacy_parameters[2] = {0u, 0u};
    const uint8_t extended_parameters[6] = {0u, 0u, 0u, 0u, 0u, 0u};
    uint16_t opcode;
    const uint8_t* parameters;
    uint8_t parameter_size;
    if (!controller_valid(controller)) return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_SCANNING ||
        (controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_LEGACY &&
         controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_EXTENDED) ||
        controller->pending_opcode != 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    if (controller->scan_active == RIN_BLUETOOTH_HCI_SCAN_LEGACY) {
        opcode = RIN_BLUETOOTH_HCI_OPCODE_LE_SCAN_ENABLE;
        parameters = legacy_parameters;
        parameter_size = sizeof(legacy_parameters);
    } else {
        opcode = RIN_BLUETOOTH_HCI_OPCODE_LE_EXT_SCAN_ENABLE;
        parameters = extended_parameters;
        parameter_size = sizeof(extended_parameters);
    }
    return submit_command(controller, opcode, parameters, parameter_size,
                          RIN_BLUETOOTH_HCI_SCAN_DISABLE_PENDING);
}

int rin_bluetooth_hci_start_connection(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciConnectionConfigV1* config)
{
    uint8_t parameters[25];
    uint32_t index;
    if (!controller_valid(controller) || !connection_config_valid(config) ||
        ranges_overlap(controller, sizeof(*controller), config,
                       sizeof(*config)))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_READY ||
        controller->pending_opcode != 0u ||
        controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_INACTIVE ||
        controller->advertising_active || controller->connection_pending ||
        controller->connection_cancel_pending || controller->disconnect_pending)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    if (controller->connection_count >= RIN_BLUETOOTH_HCI_MAX_CONNECTIONS ||
        controller->connection_generation == UINT32_MAX)
        return RIN_BLUETOOTH_HCI_LIMIT_EXCEEDED;
    if (config->own_address_type <= 1u) {
        if (!controller->local_address_present ||
            config->own_address_type != controller->local_address_type)
            return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    } else if (!controller->privacy_configured ||
               !controller->address_resolution_enabled) {
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    }
    if (config->peer_address_type >= 2u) {
        if (!privacy_identity_known(controller, config->peer_address_type,
                                    config->peer_address))
            return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    }
    for (index = 0u; index < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS; ++index) {
        const RinBluetoothHciConnectionV1* connection =
            &controller->connections[index];
        if (connection->present &&
            connection->peer_address_type == config->peer_address_type &&
            address_equal(connection->peer_address, config->peer_address))
            return RIN_BLUETOOTH_HCI_INVALID_STATE;
    }
    bytes_zero(parameters, sizeof(parameters));
    store_le16(parameters, config->scan_interval);
    store_le16(parameters + 2u, config->scan_window);
    parameters[4] = 0u; /* Direct peer address; no accept-list filter. */
    parameters[5] = config->peer_address_type;
    for (index = 0u; index < 6u; ++index)
        parameters[6u + index] = config->peer_address[5u - index];
    parameters[12] = config->own_address_type;
    store_le16(parameters + 13u, config->connection_interval_min);
    store_le16(parameters + 15u, config->connection_interval_max);
    store_le16(parameters + 17u, config->connection_latency);
    store_le16(parameters + 19u, config->supervision_timeout);
    store_le16(parameters + 21u, config->min_connection_event_length);
    store_le16(parameters + 23u, config->max_connection_event_length);
    controller->pending_connection = *config;
    controller->connection_pending = 1u;
    controller->connection_cancel_pending = 0u;
    if (submit_command(controller,
                       RIN_BLUETOOTH_HCI_OPCODE_LE_CREATE_CONNECTION,
                       parameters, sizeof(parameters),
                       RIN_BLUETOOTH_HCI_CONNECT_PENDING) !=
        RIN_BLUETOOTH_HCI_OK) {
        controller->connection_pending = 0u;
        bytes_zero(&controller->pending_connection,
                   sizeof(controller->pending_connection));
        return RIN_BLUETOOTH_HCI_TRANSPORT_FAILURE;
    }
    return RIN_BLUETOOTH_HCI_OK;
}

int rin_bluetooth_hci_cancel_connection(
    RinBluetoothHciControllerV1* controller)
{
    int result;
    if (!controller_valid(controller)) return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_CONNECTING ||
        controller->pending_opcode != 0u || !controller->connection_pending ||
        controller->connection_cancel_pending)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    controller->connection_cancel_pending = 1u;
    result = submit_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_CREATE_CONNECTION_CANCEL,
        NULL, 0u, RIN_BLUETOOTH_HCI_CONNECT_CANCEL_PENDING);
    if (result != RIN_BLUETOOTH_HCI_OK)
        controller->connection_cancel_pending = 0u;
    return result;
}

int rin_bluetooth_hci_disconnect(
    RinBluetoothHciControllerV1* controller, uint16_t handle,
    uint32_t generation)
{
    RinBluetoothHciConnectionV1* connection;
    if (!controller_valid(controller) || generation == 0u)
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_READY ||
        controller->pending_opcode != 0u || controller->disconnect_pending)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    connection = connection_by_handle(controller, handle);
    if (connection == NULL || connection->generation != generation)
        return RIN_BLUETOOTH_HCI_NOT_FOUND;
    return submit_disconnect_handle(controller, handle, generation,
                                    RIN_BLUETOOTH_HCI_READY);
}

int rin_bluetooth_hci_start_encryption(
    RinBluetoothHciControllerV1* controller, uint16_t handle,
    uint32_t generation, const uint8_t rand[8], uint16_t ediv,
    const uint8_t ltk[16])
{
    uint8_t parameters[28] = {0};
    RinBluetoothHciConnectionV1* connection;
    int result;
    if (!controller_valid(controller) || generation == 0u || rand == NULL ||
        ltk == NULL ||
        ranges_overlap(controller, sizeof(*controller), rand, 8u) ||
        ranges_overlap(controller, sizeof(*controller), ltk, 16u))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_READY ||
        controller->pending_opcode != 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    connection = connection_by_handle(controller, handle);
    if (connection == NULL || connection->generation != generation)
        return RIN_BLUETOOTH_HCI_NOT_FOUND;
    if (connection->role != 0u || connection->encrypted ||
        connection->encryption_pending || connection->ltk_request_pending ||
        connection->disconnect_pending)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    store_le16(parameters, handle);
    bytes_copy(parameters + 2u, rand, 8u);
    store_le16(parameters + 10u, ediv);
    bytes_copy(parameters + 12u, ltk, 16u);
    connection->encryption_pending = 1u;
    connection->encryption_command_pending = 1u;
    result = submit_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_START_ENCRYPTION,
        parameters, sizeof(parameters),
        RIN_BLUETOOTH_HCI_ENCRYPTION_START_PENDING);
    bytes_zero(parameters, sizeof(parameters));
    if (result != RIN_BLUETOOTH_HCI_OK) {
        connection->encryption_pending = 0u;
        connection->encryption_command_pending = 0u;
        connection->encrypted = 0u;
    }
    return result;
}

int rin_bluetooth_hci_reply_ltk_request(
    RinBluetoothHciControllerV1* controller, uint16_t handle,
    uint32_t generation, const uint8_t ltk[16])
{
    uint8_t parameters[18] = {0};
    RinBluetoothHciConnectionV1* connection;
    int result;
    if (!controller_valid(controller) || generation == 0u || ltk == NULL ||
        ranges_overlap(controller, sizeof(*controller), ltk, 16u))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_READY ||
        controller->pending_opcode != 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    connection = connection_by_handle(controller, handle);
    if (connection == NULL || connection->generation != generation)
        return RIN_BLUETOOTH_HCI_NOT_FOUND;
    if (connection->role != 1u || !connection->ltk_request_pending ||
        connection->ltk_reply_command_pending || connection->encrypted ||
        connection->encryption_pending || connection->disconnect_pending)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    store_le16(parameters, handle);
    bytes_copy(parameters + 2u, ltk, 16u);
    connection->ltk_reply_command_pending = 1u;
    result = submit_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_LTK_REQUEST_REPLY,
        parameters, sizeof(parameters), RIN_BLUETOOTH_HCI_LTK_REPLY_PENDING);
    bytes_zero(parameters, sizeof(parameters));
    if (result != RIN_BLUETOOTH_HCI_OK)
        connection->ltk_reply_command_pending = 0u;
    return result;
}

int rin_bluetooth_hci_reject_ltk_request(
    RinBluetoothHciControllerV1* controller, uint16_t handle,
    uint32_t generation)
{
    uint8_t parameters[2] = {0};
    RinBluetoothHciConnectionV1* connection;
    int result;
    if (!controller_valid(controller) || generation == 0u)
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_READY ||
        controller->pending_opcode != 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    connection = connection_by_handle(controller, handle);
    if (connection == NULL || connection->generation != generation)
        return RIN_BLUETOOTH_HCI_NOT_FOUND;
    if (connection->role != 1u || !connection->ltk_request_pending ||
        connection->ltk_reply_command_pending || connection->encrypted ||
        connection->encryption_pending || connection->disconnect_pending)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    store_le16(parameters, handle);
    connection->ltk_reply_command_pending = 1u;
    result = submit_command(
        controller,
        RIN_BLUETOOTH_HCI_OPCODE_LE_LTK_REQUEST_NEGATIVE_REPLY,
        parameters, sizeof(parameters), RIN_BLUETOOTH_HCI_LTK_REPLY_PENDING);
    bytes_zero(parameters, sizeof(parameters));
    if (result != RIN_BLUETOOTH_HCI_OK)
        connection->ltk_reply_command_pending = 0u;
    return result;
}

int rin_bluetooth_hci_get_connection(
    const RinBluetoothHciControllerV1* controller, uint32_t index,
    RinBluetoothHciConnectionV1* connection_out)
{
    uint32_t slot;
    uint32_t visible = 0u;
    if (!controller_valid(controller) || connection_out == NULL ||
        ranges_overlap(controller, sizeof(*controller), connection_out,
                       sizeof(*connection_out)))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    bytes_zero(connection_out, sizeof(*connection_out));
    for (slot = 0u; slot < RIN_BLUETOOTH_HCI_MAX_CONNECTIONS; ++slot) {
        if (!controller->connections[slot].present) continue;
        if (visible++ == index) {
            *connection_out = controller->connections[slot];
            return RIN_BLUETOOTH_HCI_OK;
        }
    }
    return RIN_BLUETOOTH_HCI_NOT_FOUND;
}

int rin_bluetooth_hci_start_advertising(
    RinBluetoothHciControllerV1* controller,
    const RinBluetoothHciAdvertisingConfigV1* config)
{
    uint8_t parameters[15] = {0};
    int result;
    if (!controller_valid(controller) || !advertising_config_valid(config))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_READY ||
        controller->pending_opcode != 0u ||
        controller->scan_active != RIN_BLUETOOTH_HCI_SCAN_INACTIVE ||
        controller->advertising_active != 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    if ((config->own_address_type == 0u &&
         (controller->local_address_present == 0u ||
          controller->local_address_type != 0u)) ||
        (config->own_address_type == 1u &&
         (controller->local_address_present == 0u ||
          controller->local_address_type != 1u)) ||
        ((config->own_address_type == 2u ||
          config->own_address_type == 3u) &&
         controller->address_resolution_enabled == 0u))
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;

    store_le16(parameters, config->interval_min);
    store_le16(parameters + 2u, config->interval_max);
    parameters[4] = config->advertising_type;
    parameters[5] = config->own_address_type;
    /* Peer address type/address are zero because directed types are rejected. */
    parameters[13] = config->channel_map;
    parameters[14] = config->filter_policy;
    controller->advertising = *config;
    controller->advertising_configured = 1u;
    result = submit_command(
        controller, RIN_BLUETOOTH_HCI_OPCODE_LE_ADV_PARAMS,
        parameters, sizeof(parameters), RIN_BLUETOOTH_HCI_ADV_PARAMS_PENDING);
    if (result != RIN_BLUETOOTH_HCI_OK) {
        bytes_zero(&controller->advertising, sizeof(controller->advertising));
        controller->advertising_configured = 0u;
        controller->advertising_active = 0u;
        if (controller->state != RIN_BLUETOOTH_HCI_FAILED)
            controller->state = RIN_BLUETOOTH_HCI_READY;
        controller->pending_opcode = 0u;
    }
    return result;
}

int rin_bluetooth_hci_stop_advertising(
    RinBluetoothHciControllerV1* controller)
{
    if (!controller_valid(controller)) return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (controller->state != RIN_BLUETOOTH_HCI_READY ||
        controller->pending_opcode != 0u ||
        controller->advertising_active == 0u)
        return RIN_BLUETOOTH_HCI_INVALID_STATE;
    return submit_advertising_enable(
        controller, 0u, RIN_BLUETOOTH_HCI_ADV_DISABLE_PENDING);
}

int rin_bluetooth_hci_process_event(
    RinBluetoothHciControllerV1* controller, const uint8_t* event,
    uint32_t event_size)
{
    uint8_t event_code;
    uint32_t parameter_size;
    if (!controller_valid(controller) || event == NULL)
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    if (event_size < 2u) return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    event_code = event[0];
    parameter_size = event[1];
    if (event_size != parameter_size + 2u)
        return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
    if (event_code == RIN_BLUETOOTH_HCI_EVENT_COMMAND_COMPLETE)
        return command_complete(controller, event + 2u, parameter_size);
    if (event_code == RIN_BLUETOOTH_HCI_EVENT_COMMAND_STATUS)
        return command_status(controller, event + 2u, parameter_size);
    if (event_code == RIN_BLUETOOTH_HCI_EVENT_DISCONNECTION_COMPLETE)
        return disconnection_complete(controller, event + 2u,
                                      parameter_size);
    if (event_code == RIN_BLUETOOTH_HCI_EVENT_LE_META) {
        if (parameter_size == 0u)
            return RIN_BLUETOOTH_HCI_MALFORMED_EVENT;
        if (event[2] == RIN_BLUETOOTH_HCI_SUBEVENT_ENCRYPTION_CHANGE)
            return encryption_change(controller, event + 2u,
                                     parameter_size);
        if (event[2] ==
            RIN_BLUETOOTH_HCI_SUBEVENT_LONG_TERM_KEY_REQUEST)
            return long_term_key_request(controller, event + 2u,
                                         parameter_size);
        if (event[2] == RIN_BLUETOOTH_HCI_SUBEVENT_ADV_REPORT)
            return advertising_report(controller, event + 2u,
                                      parameter_size);
        if (event[2] == RIN_BLUETOOTH_HCI_SUBEVENT_CONNECTION_COMPLETE)
            return connection_complete(controller, event + 2u,
                                       parameter_size, 0);
        if (event[2] ==
            RIN_BLUETOOTH_HCI_SUBEVENT_ENHANCED_CONNECTION_COMPLETE)
            return connection_complete(controller, event + 2u,
                                       parameter_size, 1);
        if (event[2] == RIN_BLUETOOTH_HCI_SUBEVENT_EXT_ADV_REPORT)
            return extended_advertising_report(controller, event + 2u,
                                               parameter_size);
        return RIN_BLUETOOTH_HCI_NOT_FOUND;
    }
    return RIN_BLUETOOTH_HCI_NOT_FOUND;
}

int rin_bluetooth_hci_get_device(
    const RinBluetoothHciControllerV1* controller, uint32_t index,
    RinBluetoothHciDeviceV1* device_out)
{
    uint32_t slot;
    uint32_t visible = 0u;
    if (!controller_valid(controller) || device_out == NULL)
        return RIN_BLUETOOTH_HCI_INVALID_ARGUMENT;
    bytes_zero(device_out, sizeof(*device_out));
    for (slot = 0u; slot < RIN_BLUETOOTH_HCI_MAX_DEVICES; ++slot) {
        if (!controller->devices[slot].present) continue;
        if (visible++ == index) {
            *device_out = controller->devices[slot];
            return RIN_BLUETOOTH_HCI_OK;
        }
    }
    return RIN_BLUETOOTH_HCI_NOT_FOUND;
}

void rin_bluetooth_hci_destroy(RinBluetoothHciControllerV1* controller)
{
    volatile uint8_t* bytes;
    uint32_t size;
    if (controller == NULL) return;
    bytes = (volatile uint8_t*)controller;
    size = sizeof(*controller);
    while (size-- != 0u) *bytes++ = 0u;
}
