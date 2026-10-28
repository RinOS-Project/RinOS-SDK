/* SPDX-License-Identifier: MIT */

#include <rin/driver/wifi/association.h>

#include <stddef.h>
#include <stdint.h>

static void wifi_assoc_zero(void* destination, size_t size)
{
    uint8_t* bytes = (uint8_t*)destination;
    size_t index;
    for (index = 0u; index < size; ++index) bytes[index] = 0u;
}

static void wifi_assoc_copy(void* destination, const void* source, size_t size)
{
    uint8_t* output = (uint8_t*)destination;
    const uint8_t* input = (const uint8_t*)source;
    size_t index;
    for (index = 0u; index < size; ++index) output[index] = input[index];
}

static int wifi_assoc_equal(const void* left, const void* right, size_t size)
{
    const uint8_t* left_bytes = (const uint8_t*)left;
    const uint8_t* right_bytes = (const uint8_t*)right;
    uint8_t difference = 0u;
    size_t index;
    for (index = 0u; index < size; ++index)
        difference |= (uint8_t)(left_bytes[index] ^ right_bytes[index]);
    return difference == 0u;
}

static int wifi_assoc_ranges_overlap(
    const void* left, size_t left_size, const void* right, size_t right_size)
{
    uintptr_t left_begin;
    uintptr_t right_begin;
    if (left == NULL || right == NULL || left_size == 0u || right_size == 0u)
        return 0;
    left_begin = (uintptr_t)left;
    right_begin = (uintptr_t)right;
    if (left_begin > UINTPTR_MAX - left_size ||
        right_begin > UINTPTR_MAX - right_size)
        return 1;
    return left_begin < right_begin + right_size &&
           right_begin < left_begin + left_size;
}

static int wifi_assoc_bssid_valid(const uint8_t bssid[6])
{
    uint8_t combined = 0u;
    uint32_t index;
    if ((bssid[0] & 1u) != 0u) return 0;
    for (index = 0u; index < 6u; ++index) combined |= bssid[index];
    return combined != 0u;
}

static int wifi_assoc_5ghz_channel_valid(uint8_t channel)
{
    if (channel >= 36u && channel <= 64u)
        return (channel & 3u) == 0u;
    if (channel >= 100u && channel <= 144u)
        return (channel & 3u) == 0u;
    if (channel >= 149u && channel <= 177u)
        return (uint8_t)(channel - 149u) % 4u == 0u;
    return 0;
}

static int wifi_assoc_6ghz_primary_valid(uint8_t channel)
{
    return channel >= 1u && channel <= 233u &&
           (uint8_t)(channel - 1u) % 4u == 0u;
}

static int wifi_assoc_6ghz_plan_valid(
    const RinWifiDiscoveredNetworkV1* network)
{
    uint8_t distance;
    if (!wifi_assoc_6ghz_primary_valid(network->channel) ||
        !(network->regulatory_info <= 4u ||
          network->regulatory_info == 7u ||
          network->regulatory_info == 8u))
        return 0;
    if (network->channel_width_mhz == RIN_WIFI_CHANNEL_WIDTH_20_MHZ)
        return network->center_channel0 == network->channel &&
               network->center_channel1 == 0u;
    if (network->channel_width_mhz == RIN_WIFI_CHANNEL_WIDTH_40_MHZ) {
        if (network->center_channel0 < 3u ||
            network->center_channel0 > 227u ||
            (uint8_t)(network->center_channel0 - 3u) % 8u != 0u ||
            network->center_channel1 != 0u)
            return 0;
        distance = network->channel > network->center_channel0
                       ? (uint8_t)(network->channel - network->center_channel0)
                       : (uint8_t)(network->center_channel0 - network->channel);
        return distance == 2u;
    }
    if (network->channel_width_mhz == RIN_WIFI_CHANNEL_WIDTH_80_MHZ) {
        if (network->center_channel0 < 7u ||
            network->center_channel0 > 215u ||
            (uint8_t)(network->center_channel0 - 7u) % 16u != 0u ||
            network->center_channel1 != 0u)
            return 0;
        distance = network->channel > network->center_channel0
                       ? (uint8_t)(network->channel - network->center_channel0)
                       : (uint8_t)(network->center_channel0 - network->channel);
        return distance == 2u || distance == 6u;
    }
    if (network->channel_width_mhz == RIN_WIFI_CHANNEL_WIDTH_160_MHZ) {
        uint8_t expected_center0;
        if (network->center_channel1 < 15u ||
            network->center_channel1 > 207u ||
            (uint8_t)(network->center_channel1 - 15u) % 32u != 0u)
            return 0;
        expected_center0 = network->channel < network->center_channel1
                               ? (uint8_t)(network->center_channel1 - 8u)
                               : (uint8_t)(network->center_channel1 + 8u);
        if (network->center_channel0 != expected_center0) return 0;
        distance = network->channel > network->center_channel0
                       ? (uint8_t)(network->channel - network->center_channel0)
                       : (uint8_t)(network->center_channel0 - network->channel);
        return distance == 2u || distance == 6u;
    }
    return 0;
}

static int wifi_assoc_band_valid(const RinWifiDiscoveredNetworkV1* network)
{
    if (network->band == RIN_WIFI_BAND_2_4_GHZ)
        return network->channel >= 1u && network->channel <= 14u &&
               network->channel_width_mhz ==
                   RIN_WIFI_CHANNEL_WIDTH_UNKNOWN &&
               network->center_channel0 == 0u &&
               network->center_channel1 == 0u &&
               network->regulatory_info == 0u &&
               network->minimum_rate_mbps == 0u;
    if (network->band == RIN_WIFI_BAND_5_GHZ)
        return wifi_assoc_5ghz_channel_valid(network->channel) &&
               network->channel_width_mhz ==
                   RIN_WIFI_CHANNEL_WIDTH_UNKNOWN &&
               network->center_channel0 == 0u &&
               network->center_channel1 == 0u &&
               network->regulatory_info == 0u &&
               network->minimum_rate_mbps == 0u;
    if (network->band != RIN_WIFI_BAND_6_GHZ ||
        !wifi_assoc_6ghz_plan_valid(network))
        return 0;
    return network->security == RIN_WIFI_SECURITY_WPA3_SAE &&
           network->pmf_capable == 1u && network->pmf_required == 1u &&
           network->sae_h2e == 1u;
}

static int wifi_assoc_network_valid(const RinWifiDiscoveredNetworkV1* network)
{
    uint32_t index;
    if (network == NULL || network->struct_size != sizeof(*network) ||
        network->version != RIN_WIFI_DISCOVERY_VERSION ||
        !wifi_assoc_bssid_valid(network->bssid) ||
        network->ssid_length == 0u ||
        network->ssid_length > RIN_WIFI_DISCOVERY_MAX_SSID_SIZE ||
        network->hidden != 0u || network->channel == 0u ||
        network->channel > 233u ||
        network->security > RIN_WIFI_SECURITY_WPA2_WPA3_TRANSITION ||
        network->pmf_capable > 1u || network->pmf_required > 1u ||
        (network->pmf_required != 0u && network->pmf_capable == 0u) ||
        ((network->security == RIN_WIFI_SECURITY_WPA3_SAE ||
          network->security == RIN_WIFI_SECURITY_WPA2_WPA3_TRANSITION) &&
         network->pmf_capable == 0u) ||
        network->signal_dbm > 0 || network->signal_dbm < -127 ||
        (network->frame_subtype != 5u && network->frame_subtype != 8u) ||
        network->beacon_interval_tu == 0u ||
        network->information_element_count == 0u ||
        network->information_element_count >
            RIN_WIFI_DISCOVERY_MAX_INFORMATION_ELEMENTS ||
        network->sae_h2e > 1u || network->reserved0 != 0u ||
        !wifi_assoc_band_valid(network))
        return 0;
    for (index = 0u; index < 2u; ++index)
        if (network->reserved[index] != 0u) return 0;
    return 1;
}

static int wifi_assoc_controller_valid(
    const RinWifiAssociationControllerV1* controller)
{
    return controller != NULL &&
           controller->struct_size == sizeof(*controller) &&
           controller->version == RIN_WIFI_ASSOCIATION_VERSION &&
           controller->transport.struct_size == sizeof(controller->transport) &&
           controller->transport.version == RIN_WIFI_ASSOCIATION_VERSION &&
           controller->transport.submit != NULL &&
           controller->transport.monotonic_ms != NULL &&
           controller->state >= RIN_WIFI_ASSOCIATION_IDLE &&
           controller->state <= RIN_WIFI_ASSOCIATION_SUSPENDED &&
           controller->roam_pending <= 1u &&
           controller->resume_required <= 1u &&
           controller->resume_target_valid <= 1u &&
           (controller->roam_pending == 0u ||
            (controller->state == RIN_WIFI_ASSOCIATION_DISCONNECTING &&
             wifi_assoc_network_valid(&controller->roam_network))) &&
           (controller->resume_target_valid == 0u ||
            wifi_assoc_network_valid(&controller->resume_network)) &&
           (controller->resume_required == 0u ||
            (controller->state == RIN_WIFI_ASSOCIATION_IDLE ||
             controller->state == RIN_WIFI_ASSOCIATION_SUSPENDED)) &&
           (controller->state != RIN_WIFI_ASSOCIATION_SUSPENDED ||
            (controller->resume_required != 0u &&
             controller->resume_target_valid != 0u &&
             controller->deadline_ms == 0u &&
             controller->selected_security == 0u &&
             controller->association_id == 0u &&
             controller->network.struct_size == 0u));
}

static void wifi_assoc_clear_selection(
    RinWifiAssociationControllerV1* controller)
{
    controller->deadline_ms = 0u;
    controller->last_replay_counter = 0u;
    controller->selected_security = 0u;
    controller->association_id = 0u;
    controller->last_status = RIN_WIFI_ASSOCIATION_OK;
    controller->roam_pending = 0u;
    wifi_assoc_zero(&controller->network, sizeof(controller->network));
    wifi_assoc_zero(&controller->roam_network,
                    sizeof(controller->roam_network));
}

static void wifi_assoc_clear_resume_target(
    RinWifiAssociationControllerV1* controller)
{
    controller->resume_target_valid = 0u;
    wifi_assoc_zero(&controller->resume_network,
                    sizeof(controller->resume_network));
}

static void wifi_assoc_clear_roam(
    RinWifiAssociationControllerV1* controller)
{
    controller->roam_pending = 0u;
    wifi_assoc_zero(&controller->roam_network,
                    sizeof(controller->roam_network));
}

static int wifi_assoc_fail(
    RinWifiAssociationControllerV1* controller, int status)
{
    wifi_assoc_clear_roam(controller);
    wifi_assoc_clear_resume_target(controller);
    controller->state = RIN_WIFI_ASSOCIATION_FAILED;
    controller->last_status = status;
    return status;
}

static uint32_t wifi_assoc_selected_security(
    const RinWifiDiscoveredNetworkV1* network)
{
    return network->security == RIN_WIFI_SECURITY_WPA2_WPA3_TRANSITION
               ? RIN_WIFI_SECURITY_WPA3_SAE
               : network->security;
}

static int wifi_assoc_same_ssid(
    const RinWifiDiscoveredNetworkV1* left,
    const RinWifiDiscoveredNetworkV1* right)
{
    return left->ssid_length == right->ssid_length &&
           wifi_assoc_equal(left->ssid, right->ssid, left->ssid_length);
}

static uint32_t wifi_assoc_authentication_mode(
    const RinWifiAssociationControllerV1* controller)
{
    return controller->selected_security == RIN_WIFI_SECURITY_WPA3_SAE
               ? RIN_WIFI_AUTH_SAE
               : RIN_WIFI_AUTH_OPEN_SYSTEM;
}

static int wifi_assoc_now_ms(
    RinWifiAssociationControllerV1* controller, uint64_t* result_out)
{
    RinWifiAssociationControllerV1 snapshot;
    RinWifiAssociationMonotonicMsFn monotonic_ms =
        controller->transport.monotonic_ms;
    void* context = controller->transport.context;
    uint64_t result;
    wifi_assoc_copy(&snapshot, controller, sizeof(snapshot));
    controller->callback_active = 1u;
    result = monotonic_ms(context);
    controller->callback_active = 0u;
    if (!wifi_assoc_equal(controller, &snapshot, sizeof(snapshot))) {
        wifi_assoc_copy(controller, &snapshot, sizeof(snapshot));
        return wifi_assoc_fail(
            controller, RIN_WIFI_ASSOCIATION_SECURITY_FAILURE);
    }
    *result_out = result;
    return RIN_WIFI_ASSOCIATION_OK;
}

static int wifi_assoc_submit(
    RinWifiAssociationControllerV1* controller, uint32_t operation,
    uint16_t reason_code)
{
    RinWifiAssociationCommandV1 command;
    RinWifiAssociationCommandV1 snapshot;
    RinWifiAssociationSubmitFn submit;
    void* context;
    int result;

    wifi_assoc_zero(&command, sizeof(command));
    command.struct_size = sizeof(command);
    command.version = RIN_WIFI_ASSOCIATION_VERSION;
    command.generation = controller->generation;
    command.operation = operation;
    command.authentication_mode = wifi_assoc_authentication_mode(controller);
    command.selected_security = controller->selected_security;
    command.reason_code = reason_code;
    wifi_assoc_copy(&command.network, &controller->network,
                    sizeof(command.network));
    wifi_assoc_copy(&snapshot, &command, sizeof(snapshot));
    submit = controller->transport.submit;
    context = controller->transport.context;
    controller->callback_active = 1u;
    result = submit(context, &command);
    controller->callback_active = 0u;
    if (!wifi_assoc_equal(&command, &snapshot, sizeof(command)))
        return wifi_assoc_fail(
            controller, RIN_WIFI_ASSOCIATION_SECURITY_FAILURE);
    if (result != 0)
        return wifi_assoc_fail(
            controller, RIN_WIFI_ASSOCIATION_TRANSPORT_FAILURE);
    return RIN_WIFI_ASSOCIATION_OK;
}

static int wifi_assoc_event_base_valid(
    const RinWifiAssociationControllerV1* controller,
    const RinWifiAssociationEventV1* event)
{
    return event->struct_size == sizeof(*event) &&
           event->version == RIN_WIFI_ASSOCIATION_VERSION &&
           event->generation == controller->generation &&
           event->selected_security == controller->selected_security &&
           wifi_assoc_equal(event->bssid, controller->network.bssid, 6u) &&
           (event->flags & ~RIN_WIFI_ASSOCIATION_EVENT_FLAGS) == 0u &&
           event->reserved0 == 0u && event->reserved[0] == 0u &&
           event->reserved[1] == 0u;
}

int rin_wifi_association_init(
    RinWifiAssociationControllerV1* controller,
    const RinWifiAssociationTransportV1* transport)
{
    RinWifiAssociationTransportV1 transport_snapshot;
    if (controller == NULL || transport == NULL ||
        wifi_assoc_ranges_overlap(controller, sizeof(*controller), transport,
                                  sizeof(*transport)) ||
        transport->struct_size != sizeof(*transport) ||
        transport->version != RIN_WIFI_ASSOCIATION_VERSION ||
        transport->submit == NULL || transport->monotonic_ms == NULL)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    wifi_assoc_copy(&transport_snapshot, transport, sizeof(transport_snapshot));
    wifi_assoc_zero(controller, sizeof(*controller));
    controller->struct_size = sizeof(*controller);
    controller->version = RIN_WIFI_ASSOCIATION_VERSION;
    controller->state = RIN_WIFI_ASSOCIATION_IDLE;
    wifi_assoc_copy(&controller->transport, &transport_snapshot,
                    sizeof(controller->transport));
    return RIN_WIFI_ASSOCIATION_OK;
}

static int wifi_assoc_start(
    RinWifiAssociationControllerV1* controller,
    const RinWifiDiscoveredNetworkV1* network, uint64_t scan_generation,
    uint32_t timeout_ms, int scan_bound)
{
    RinWifiDiscoveredNetworkV1 network_snapshot;
    uint64_t now_ms;
    int time_status;
    if (!wifi_assoc_controller_valid(controller) || network == NULL ||
        controller->callback_active != 0u ||
        wifi_assoc_ranges_overlap(controller, sizeof(*controller), network,
                                  sizeof(*network)))
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (controller->state != RIN_WIFI_ASSOCIATION_IDLE)
        return RIN_WIFI_ASSOCIATION_INVALID_STATE;
    if (controller->resume_required == 0u &&
        controller->resume_target_valid != 0u)
        return RIN_WIFI_ASSOCIATION_INVALID_STATE;
    if (controller->resume_required != 0u) {
        if (scan_bound == 0)
            return RIN_WIFI_ASSOCIATION_INVALID_STATE;
        if (scan_generation <= controller->scan_generation_floor)
            return RIN_WIFI_ASSOCIATION_STALE_EVENT;
    } else if (scan_bound != 0 &&
               scan_generation < RIN_WIFI_ASSOCIATION_MIN_SCAN_GENERATION) {
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    }
    if (timeout_ms < RIN_WIFI_ASSOCIATION_MIN_TIMEOUT_MS ||
        timeout_ms > RIN_WIFI_ASSOCIATION_MAX_TIMEOUT_MS ||
        !wifi_assoc_network_valid(network))
        return RIN_WIFI_ASSOCIATION_UNSUPPORTED;
    if (controller->generation == UINT64_MAX)
        return RIN_WIFI_ASSOCIATION_GENERATION_EXHAUSTED;
    wifi_assoc_copy(&network_snapshot, network, sizeof(network_snapshot));
    time_status = wifi_assoc_now_ms(controller, &now_ms);
    if (time_status != RIN_WIFI_ASSOCIATION_OK) return time_status;
    if (now_ms > UINT64_MAX - timeout_ms)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;

    wifi_assoc_clear_selection(controller);
    wifi_assoc_clear_resume_target(controller);
    ++controller->generation;
    controller->deadline_ms = now_ms + timeout_ms;
    if (scan_bound != 0)
        controller->scan_generation_floor = scan_generation;
    controller->resume_required = 0u;
    controller->selected_security =
        wifi_assoc_selected_security(&network_snapshot);
    wifi_assoc_copy(&controller->network, &network_snapshot,
                    sizeof(controller->network));
    controller->state = RIN_WIFI_ASSOCIATION_AUTHENTICATING;
    return wifi_assoc_submit(
        controller, RIN_WIFI_ASSOCIATION_OP_AUTHENTICATE, 0u);
}

int rin_wifi_association_start(
    RinWifiAssociationControllerV1* controller,
    const RinWifiDiscoveredNetworkV1* network, uint32_t timeout_ms)
{
    return wifi_assoc_start(controller, network, 0u, timeout_ms, 0);
}

int rin_wifi_association_start_from_scan(
    RinWifiAssociationControllerV1* controller,
    const RinWifiDiscoveredNetworkV1* network, uint64_t scan_generation,
    uint32_t timeout_ms)
{
    return wifi_assoc_start(controller, network, scan_generation, timeout_ms,
                            1);
}

int rin_wifi_association_process_event(
    RinWifiAssociationControllerV1* controller,
    const RinWifiAssociationEventV1* event)
{
    if (!wifi_assoc_controller_valid(controller) || event == NULL ||
        controller->callback_active != 0u ||
        wifi_assoc_ranges_overlap(controller, sizeof(*controller), event,
                                  sizeof(*event)))
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (controller->state == RIN_WIFI_ASSOCIATION_IDLE ||
        controller->state == RIN_WIFI_ASSOCIATION_SUSPENDED ||
        (controller->state == RIN_WIFI_ASSOCIATION_FAILED &&
         event->type != RIN_WIFI_ASSOCIATION_EVENT_DISCONNECTED))
        return RIN_WIFI_ASSOCIATION_INVALID_STATE;
    if (controller->state == RIN_WIFI_ASSOCIATION_AUTHENTICATING ||
        controller->state == RIN_WIFI_ASSOCIATION_ASSOCIATING ||
        controller->state == RIN_WIFI_ASSOCIATION_KEY_EXCHANGE ||
        controller->state == RIN_WIFI_ASSOCIATION_SUSPENDING ||
        (controller->state == RIN_WIFI_ASSOCIATION_DISCONNECTING &&
         controller->roam_pending != 0u)) {
        int polled = rin_wifi_association_poll(controller);
        if (polled != RIN_WIFI_ASSOCIATION_OK) return polled;
    }
    if (event->generation != controller->generation)
        return RIN_WIFI_ASSOCIATION_STALE_EVENT;
    if (!wifi_assoc_event_base_valid(controller, event))
        return RIN_WIFI_ASSOCIATION_MALFORMED_EVENT;

    if (event->type == RIN_WIFI_ASSOCIATION_EVENT_FAILED) {
        if (event->status_code == 0u || event->flags != 0u ||
            event->association_id != 0u || event->replay_counter != 0u)
            return RIN_WIFI_ASSOCIATION_MALFORMED_EVENT;
        return wifi_assoc_fail(
            controller, RIN_WIFI_ASSOCIATION_SECURITY_FAILURE);
    }
    if (controller->state == RIN_WIFI_ASSOCIATION_SUSPENDING) {
        if (event->type != RIN_WIFI_ASSOCIATION_EVENT_QUIESCED)
            return RIN_WIFI_ASSOCIATION_INVALID_STATE;
        if (event->flags != 0u || event->status_code != 0u ||
            event->reason_code != 0u || event->association_id != 0u ||
            event->replay_counter != 0u) {
            return RIN_WIFI_ASSOCIATION_MALFORMED_EVENT;
        }
        wifi_assoc_clear_selection(controller);
        controller->resume_required = 1u;
        controller->state = RIN_WIFI_ASSOCIATION_SUSPENDED;
        return RIN_WIFI_ASSOCIATION_OK;
    }
    if (event->type == RIN_WIFI_ASSOCIATION_EVENT_DISCONNECTED) {
        RinWifiDiscoveredNetworkV1 roam_network;
        if (event->flags != 0u || event->status_code != 0u ||
            event->association_id != 0u || event->replay_counter != 0u)
            return RIN_WIFI_ASSOCIATION_MALFORMED_EVENT;
        if (controller->roam_pending != 0u) {
            wifi_assoc_copy(&roam_network, &controller->roam_network,
                            sizeof(roam_network));
            wifi_assoc_clear_roam(controller);
            if (controller->generation == UINT64_MAX)
                return wifi_assoc_fail(
                    controller,
                    RIN_WIFI_ASSOCIATION_GENERATION_EXHAUSTED);
            ++controller->generation;
            controller->last_replay_counter = 0u;
            controller->association_id = 0u;
            controller->last_status = RIN_WIFI_ASSOCIATION_OK;
            controller->selected_security =
                wifi_assoc_selected_security(&roam_network);
            wifi_assoc_copy(&controller->network, &roam_network,
                            sizeof(controller->network));
            controller->state = RIN_WIFI_ASSOCIATION_AUTHENTICATING;
            return wifi_assoc_submit(
                controller, RIN_WIFI_ASSOCIATION_OP_AUTHENTICATE, 0u);
        }
        wifi_assoc_clear_selection(controller);
        controller->state = RIN_WIFI_ASSOCIATION_IDLE;
        return RIN_WIFI_ASSOCIATION_OK;
    }
    if (event->status_code != 0u || event->reason_code != 0u)
        return RIN_WIFI_ASSOCIATION_MALFORMED_EVENT;

    if (controller->state == RIN_WIFI_ASSOCIATION_AUTHENTICATING &&
        event->type == RIN_WIFI_ASSOCIATION_EVENT_AUTHENTICATED) {
        if (event->flags != 0u || event->association_id != 0u ||
            event->replay_counter != 0u)
            return RIN_WIFI_ASSOCIATION_MALFORMED_EVENT;
        controller->state = RIN_WIFI_ASSOCIATION_ASSOCIATING;
        return wifi_assoc_submit(
            controller, RIN_WIFI_ASSOCIATION_OP_ASSOCIATE, 0u);
    }
    if (controller->state == RIN_WIFI_ASSOCIATION_ASSOCIATING &&
        event->type == RIN_WIFI_ASSOCIATION_EVENT_ASSOCIATED) {
        if ((event->flags & (RIN_WIFI_ASSOCIATION_KEY_PAIRWISE |
                             RIN_WIFI_ASSOCIATION_KEY_GROUP)) != 0u ||
            event->association_id == 0u || event->replay_counter != 0u ||
            ((controller->selected_security == RIN_WIFI_SECURITY_WPA3_SAE ||
              controller->network.pmf_required != 0u) &&
             (event->flags & RIN_WIFI_ASSOCIATION_PMF_ACTIVE) == 0u))
            return wifi_assoc_fail(
                controller, RIN_WIFI_ASSOCIATION_SECURITY_FAILURE);
        controller->association_id = event->association_id;
        if (controller->selected_security == RIN_WIFI_SECURITY_OPEN) {
            controller->state = RIN_WIFI_ASSOCIATION_CONNECTED;
            controller->deadline_ms = 0u;
            return RIN_WIFI_ASSOCIATION_OK;
        }
        controller->state = RIN_WIFI_ASSOCIATION_KEY_EXCHANGE;
        return wifi_assoc_submit(
            controller, RIN_WIFI_ASSOCIATION_OP_KEY_EXCHANGE, 0u);
    }
    if (controller->state == RIN_WIFI_ASSOCIATION_KEY_EXCHANGE &&
        event->type == RIN_WIFI_ASSOCIATION_EVENT_KEYS_INSTALLED) {
        uint32_t required = RIN_WIFI_ASSOCIATION_KEY_PAIRWISE |
                            RIN_WIFI_ASSOCIATION_KEY_GROUP;
        if (controller->selected_security == RIN_WIFI_SECURITY_WPA3_SAE ||
            controller->network.pmf_required != 0u)
            required |= RIN_WIFI_ASSOCIATION_PMF_ACTIVE;
        if (((uint32_t)event->flags & required) != required ||
            event->association_id != controller->association_id ||
            event->replay_counter == 0u ||
            event->replay_counter <= controller->last_replay_counter)
            return wifi_assoc_fail(
                controller, RIN_WIFI_ASSOCIATION_SECURITY_FAILURE);
        controller->last_replay_counter = event->replay_counter;
        controller->deadline_ms = 0u;
        controller->state = RIN_WIFI_ASSOCIATION_CONNECTED;
        return RIN_WIFI_ASSOCIATION_OK;
    }
    return RIN_WIFI_ASSOCIATION_INVALID_STATE;
}

int rin_wifi_association_poll(RinWifiAssociationControllerV1* controller)
{
    uint64_t now_ms;
    int submitted;
    int time_status;
    if (!wifi_assoc_controller_valid(controller) ||
        controller->callback_active != 0u)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (controller->state == RIN_WIFI_ASSOCIATION_SUSPENDING) {
        time_status = wifi_assoc_now_ms(controller, &now_ms);
        if (time_status != RIN_WIFI_ASSOCIATION_OK) return time_status;
        if (now_ms < controller->deadline_ms)
            return RIN_WIFI_ASSOCIATION_OK;
        return wifi_assoc_fail(controller, RIN_WIFI_ASSOCIATION_TIMEOUT);
    }
    if (controller->state == RIN_WIFI_ASSOCIATION_DISCONNECTING &&
        controller->roam_pending != 0u) {
        time_status = wifi_assoc_now_ms(controller, &now_ms);
        if (time_status != RIN_WIFI_ASSOCIATION_OK) return time_status;
        if (now_ms < controller->deadline_ms)
            return RIN_WIFI_ASSOCIATION_OK;
        return wifi_assoc_fail(controller, RIN_WIFI_ASSOCIATION_TIMEOUT);
    }
    if (controller->state != RIN_WIFI_ASSOCIATION_AUTHENTICATING &&
        controller->state != RIN_WIFI_ASSOCIATION_ASSOCIATING &&
        controller->state != RIN_WIFI_ASSOCIATION_KEY_EXCHANGE)
        return RIN_WIFI_ASSOCIATION_OK;
    time_status = wifi_assoc_now_ms(controller, &now_ms);
    if (time_status != RIN_WIFI_ASSOCIATION_OK) return time_status;
    if (now_ms < controller->deadline_ms) return RIN_WIFI_ASSOCIATION_OK;
    controller->state = RIN_WIFI_ASSOCIATION_DISCONNECTING;
    submitted = wifi_assoc_submit(
        controller, RIN_WIFI_ASSOCIATION_OP_DISCONNECT, 0u);
    if (submitted != RIN_WIFI_ASSOCIATION_OK) return submitted;
    controller->last_status = RIN_WIFI_ASSOCIATION_TIMEOUT;
    return RIN_WIFI_ASSOCIATION_TIMEOUT;
}

int rin_wifi_association_disconnect(
    RinWifiAssociationControllerV1* controller, uint16_t reason_code)
{
    if (!wifi_assoc_controller_valid(controller) ||
        controller->callback_active != 0u)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (controller->state != RIN_WIFI_ASSOCIATION_CONNECTED)
        return RIN_WIFI_ASSOCIATION_INVALID_STATE;
    controller->state = RIN_WIFI_ASSOCIATION_DISCONNECTING;
    return wifi_assoc_submit(
        controller, RIN_WIFI_ASSOCIATION_OP_DISCONNECT, reason_code);
}

int rin_wifi_association_suspend(
    RinWifiAssociationControllerV1* controller, uint64_t scan_generation,
    uint32_t timeout_ms)
{
    uint64_t now_ms;
    int time_status;

    if (!wifi_assoc_controller_valid(controller) ||
        controller->callback_active != 0u)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (controller->state != RIN_WIFI_ASSOCIATION_CONNECTED)
        return RIN_WIFI_ASSOCIATION_INVALID_STATE;
    if (scan_generation < RIN_WIFI_ASSOCIATION_MIN_SCAN_GENERATION ||
        scan_generation < controller->scan_generation_floor ||
        timeout_ms < RIN_WIFI_ASSOCIATION_MIN_TIMEOUT_MS ||
        timeout_ms > RIN_WIFI_ASSOCIATION_MAX_TIMEOUT_MS) {
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    }
    time_status = wifi_assoc_now_ms(controller, &now_ms);
    if (time_status != RIN_WIFI_ASSOCIATION_OK) return time_status;
    if (now_ms > UINT64_MAX - timeout_ms)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;

    wifi_assoc_copy(&controller->resume_network, &controller->network,
                    sizeof(controller->resume_network));
    controller->resume_target_valid = 1u;
    controller->scan_generation_floor = scan_generation;
    controller->deadline_ms = now_ms + timeout_ms;
    controller->state = RIN_WIFI_ASSOCIATION_SUSPENDING;
    return wifi_assoc_submit(controller, RIN_WIFI_ASSOCIATION_OP_QUIESCE, 0u);
}

int rin_wifi_association_resume(RinWifiAssociationControllerV1* controller)
{
    if (!wifi_assoc_controller_valid(controller) ||
        controller->callback_active != 0u)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (controller->state != RIN_WIFI_ASSOCIATION_SUSPENDED ||
        controller->resume_required == 0u ||
        controller->scan_generation_floor == 0u) {
        return RIN_WIFI_ASSOCIATION_INVALID_STATE;
    }
    controller->state = RIN_WIFI_ASSOCIATION_IDLE;
    return RIN_WIFI_ASSOCIATION_OK;
}

static int wifi_assoc_bssid_less(
    const uint8_t left[6], const uint8_t right[6])
{
    uint32_t index;
    for (index = 0u; index < 6u; ++index) {
        if (left[index] < right[index]) return 1;
        if (left[index] > right[index]) return 0;
    }
    return 0;
}

static int wifi_assoc_resume_candidate_better(
    const RinWifiDiscoveredNetworkV1* candidate,
    const RinWifiDiscoveredNetworkV1* best)
{
    if (candidate->signal_dbm > best->signal_dbm) return 1;
    if (candidate->signal_dbm < best->signal_dbm) return 0;
    return wifi_assoc_bssid_less(candidate->bssid, best->bssid);
}

int rin_wifi_association_resume_from_scan(
    RinWifiAssociationControllerV1* controller,
    const RinWifiDiscoveredNetworkV1* networks, uint32_t network_count,
    uint64_t scan_generation, uint32_t timeout_ms)
{
    RinWifiDiscoveredNetworkV1 candidate;
    uint32_t index;
    uint8_t found = 0u;
    int status;

    if (!wifi_assoc_controller_valid(controller) || networks == NULL ||
        network_count == 0u ||
        network_count > RIN_WIFI_ASSOCIATION_MAX_RESUME_CANDIDATES ||
        wifi_assoc_ranges_overlap(
            controller, sizeof(*controller), networks,
            (size_t)network_count * sizeof(*networks)))
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (controller->state != RIN_WIFI_ASSOCIATION_SUSPENDED ||
        controller->resume_required == 0u ||
        controller->resume_target_valid == 0u)
        return RIN_WIFI_ASSOCIATION_INVALID_STATE;
    if (scan_generation < RIN_WIFI_ASSOCIATION_MIN_SCAN_GENERATION ||
        scan_generation <= controller->scan_generation_floor)
        return RIN_WIFI_ASSOCIATION_STALE_EVENT;

    for (index = 0u; index < network_count; ++index) {
        const RinWifiDiscoveredNetworkV1* network = &networks[index];
        if (!wifi_assoc_network_valid(network))
            return RIN_WIFI_ASSOCIATION_UNSUPPORTED;
        if (!wifi_assoc_same_ssid(network, &controller->resume_network) ||
            wifi_assoc_selected_security(network) !=
                wifi_assoc_selected_security(&controller->resume_network))
            continue;
        if (found == 0u ||
            wifi_assoc_resume_candidate_better(network, &candidate)) {
            wifi_assoc_copy(&candidate, network, sizeof(candidate));
            found = 1u;
        }
    }
    if (found == 0u) return RIN_WIFI_ASSOCIATION_UNSUPPORTED;

    status = rin_wifi_association_resume(controller);
    if (status != RIN_WIFI_ASSOCIATION_OK) return status;
    return rin_wifi_association_start_from_scan(
        controller, &candidate, scan_generation, timeout_ms);
}

int rin_wifi_association_invalidate_for_device_reset(
    RinWifiAssociationControllerV1* controller)
{
    if (!wifi_assoc_controller_valid(controller) ||
        controller->callback_active != 0u)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (controller->generation == UINT64_MAX) {
        wifi_assoc_clear_selection(controller);
        wifi_assoc_clear_resume_target(controller);
        controller->state = RIN_WIFI_ASSOCIATION_FAILED;
        controller->last_status = RIN_WIFI_ASSOCIATION_GENERATION_EXHAUSTED;
        return RIN_WIFI_ASSOCIATION_GENERATION_EXHAUSTED;
    }
    ++controller->generation;
    wifi_assoc_clear_selection(controller);
    wifi_assoc_clear_resume_target(controller);
    controller->scan_generation_floor = 0u;
    controller->resume_required = 0u;
    controller->state = RIN_WIFI_ASSOCIATION_IDLE;
    return RIN_WIFI_ASSOCIATION_OK;
}

int rin_wifi_association_roam(
    RinWifiAssociationControllerV1* controller,
    const RinWifiDiscoveredNetworkV1* candidate,
    uint64_t observed_monotonic_ms, uint32_t maximum_age_ms,
    uint32_t minimum_signal_gain_db, uint32_t timeout_ms,
    uint16_t disconnect_reason_code)
{
    RinWifiDiscoveredNetworkV1 candidate_snapshot;
    uint64_t now_ms;
    int32_t signal_gain;
    int time_status;
    if (!wifi_assoc_controller_valid(controller) || candidate == NULL ||
        controller->callback_active != 0u ||
        wifi_assoc_ranges_overlap(controller, sizeof(*controller), candidate,
                                  sizeof(*candidate)))
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (controller->state != RIN_WIFI_ASSOCIATION_CONNECTED)
        return RIN_WIFI_ASSOCIATION_INVALID_STATE;
    if (!wifi_assoc_network_valid(&controller->network) ||
        wifi_assoc_selected_security(&controller->network) !=
            controller->selected_security ||
        controller->association_id == 0u || controller->deadline_ms != 0u)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    if (maximum_age_ms < RIN_WIFI_ASSOCIATION_MIN_ROAM_AGE_MS ||
        maximum_age_ms > RIN_WIFI_ASSOCIATION_MAX_ROAM_AGE_MS ||
        minimum_signal_gain_db < RIN_WIFI_ASSOCIATION_MIN_ROAM_GAIN_DB ||
        minimum_signal_gain_db > RIN_WIFI_ASSOCIATION_MAX_ROAM_GAIN_DB ||
        timeout_ms < RIN_WIFI_ASSOCIATION_MIN_TIMEOUT_MS ||
        timeout_ms > RIN_WIFI_ASSOCIATION_MAX_TIMEOUT_MS ||
        !wifi_assoc_network_valid(candidate))
        return RIN_WIFI_ASSOCIATION_UNSUPPORTED;
    if (controller->generation == UINT64_MAX)
        return RIN_WIFI_ASSOCIATION_GENERATION_EXHAUSTED;
    wifi_assoc_copy(&candidate_snapshot, candidate,
                    sizeof(candidate_snapshot));
    time_status = wifi_assoc_now_ms(controller, &now_ms);
    if (time_status != RIN_WIFI_ASSOCIATION_OK) return time_status;
    if (observed_monotonic_ms > now_ms ||
        now_ms - observed_monotonic_ms > maximum_age_ms)
        return RIN_WIFI_ASSOCIATION_STALE_EVENT;
    if (now_ms > UINT64_MAX - timeout_ms)
        return RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT;
    signal_gain = (int32_t)candidate_snapshot.signal_dbm -
                  (int32_t)controller->network.signal_dbm;
    if (wifi_assoc_equal(candidate_snapshot.bssid,
                         controller->network.bssid, 6u) ||
        !wifi_assoc_same_ssid(&candidate_snapshot, &controller->network) ||
        wifi_assoc_selected_security(&candidate_snapshot) !=
            controller->selected_security ||
        (controller->network.pmf_required != 0u &&
         candidate_snapshot.pmf_required == 0u) ||
        (controller->network.sae_h2e != 0u &&
         candidate_snapshot.sae_h2e == 0u) ||
        signal_gain < (int32_t)minimum_signal_gain_db)
        return RIN_WIFI_ASSOCIATION_UNSUPPORTED;

    wifi_assoc_copy(&controller->roam_network, &candidate_snapshot,
                    sizeof(controller->roam_network));
    controller->roam_pending = 1u;
    controller->deadline_ms = now_ms + timeout_ms;
    controller->state = RIN_WIFI_ASSOCIATION_DISCONNECTING;
    return wifi_assoc_submit(
        controller, RIN_WIFI_ASSOCIATION_OP_DISCONNECT,
        disconnect_reason_code);
}
