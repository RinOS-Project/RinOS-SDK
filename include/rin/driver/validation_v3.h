/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVER_VALIDATION_V3_H
#define RIN_DRIVER_VALIDATION_V3_H

#include <stddef.h>
#include <rin/driver/module_api_v3.h>

static inline int rin_ndrv_v3_api_valid(const RinDriverApiV3* api,
                                        const RinDriverContextV3* context,
                                        uint64_t capabilities) {
    if (!api || !context || api->version != RIN_DRIVER_V3_ABI_VERSION ||
        context->version != RIN_DRIVER_V3_ABI_VERSION ||
        api->struct_size < offsetof(RinDriverApiV3, reserved) ||
        context->struct_size < sizeof(RinDriverContextV3) ||
        context->session == 0u || context->device == 0u ||
        (context->architecture != RIN_ARCH_X86 &&
         context->architecture != RIN_ARCH_X86_64) ||
        (api->capabilities & capabilities) != capabilities ||
        !api->dma_allocate || !api->bar_map ||
        !api->lease_release || !api->register_device ||
        (((api->capabilities &
           (RIN_DRIVER_V3_CAP_MSI | RIN_DRIVER_V3_CAP_MSIX)) != 0u) &&
         !api->irq_acquire) ||
        (((capabilities & RIN_DRIVER_V3_CAP_PCI_CONFIG) != 0u) &&
         (!api->pci_config_read || !api->pci_config_write)) ||
        (((capabilities & RIN_DRIVER_V3_CAP_DMA_START) != 0u) &&
         !api->device_dma_start)) {
        return 0;
    }
    for (uint32_t index = 0u; index < 4u; ++index) {
        if (api->reserved[index] != 0u) return 0;
    }
    for (uint32_t index = 0u; index < 8u; ++index) {
        if (context->reserved[index] != 0u) return 0;
    }
#if UINTPTR_MAX == UINT32_MAX
    if (api->dma_allocate_padding != 0u ||
        api->dma_map_sg_padding != 0u || api->dma_sync_padding != 0u ||
        api->bar_map_padding != 0u || api->irq_acquire_padding != 0u ||
        api->lease_release_padding != 0u || api->timer_arm_padding != 0u ||
        api->timer_poll_padding != 0u ||
        api->timer_set_handler_padding != 0u ||
        api->register_device_padding != 0u ||
        api->pci_config_read_padding != 0u ||
        api->pci_config_write_padding != 0u || api->log_padding != 0u ||
        api->monotonic_time_ns_padding != 0u ||
        api->device_dma_start_padding != 0u)
        return 0;
#endif
    return 1;
}

static inline void rin_ndrv_v3_zero(void* output, uint32_t size) {
    if (!output) return;
    uint8_t* bytes = (uint8_t*)output;
    for (uint32_t i = 0; i < size; i++) bytes[i] = 0u;
}

static inline void rin_ndrv_v3_copy(void* output, const void* input,
                                    uint32_t size) {
    if (!output || !input) return;
    uint8_t* destination = (uint8_t*)output;
    const uint8_t* source = (const uint8_t*)input;
    for (uint32_t i = 0; i < size; i++) destination[i] = source[i];
}

static inline int rin_ndrv_v3_dma_valid(const RinDriverV3DmaLeaseInfo* dma,
                                        uint64_t minimum_length,
                                        int below_4g) {
    uint64_t end;
    if (!dma || dma->lease == 0u || dma->driver_virtual_address == 0u ||
        dma->driver_virtual_address > (uint64_t)UINTPTR_MAX ||
        dma->iova == 0u || dma->length < minimum_length ||
        dma->length > (uint64_t)UINTPTR_MAX - dma->driver_virtual_address ||
        dma->iova > UINT64_MAX - dma->length ||
        dma->alignment == 0u ||
        (dma->alignment & (dma->alignment - 1u)) != 0u ||
        (dma->driver_virtual_address & (dma->alignment - 1u)) != 0u ||
        (dma->iova & (dma->alignment - 1u)) != 0u ||
        (dma->flags & ~RIN_DRIVER_V3_DMA_FLAG_MASK) != 0u ||
        (dma->flags & (RIN_DRIVER_V3_DMA_READ | RIN_DRIVER_V3_DMA_WRITE)) == 0u ||
        dma->segment_count > RIN_DRIVER_V3_MAX_SG_SEGMENTS ||
        dma->segment_count == 0u || dma->reserved != 0u ||
        (below_4g != 0 && below_4g != 1)) {
        return 0;
    }
    end = dma->iova + dma->length;
    return !below_4g || end <= UINT64_C(0x100000000);
}

static inline int rin_ndrv_v3_bar_valid(const RinDriverV3BarLeaseInfo* bar) {
    return bar && bar->lease != 0u && bar->driver_virtual_address != 0u &&
           bar->driver_virtual_address <= (uint64_t)UINTPTR_MAX &&
           bar->length != 0u && bar->bar_index < 6u &&
           (bar->flags & ~RIN_DRIVER_V3_BAR_FLAG_MASK) == 0u &&
           (bar->flags & RIN_DRIVER_V3_BAR_FLAG_MASK) != 0u &&
           bar->length <= (uint64_t)UINTPTR_MAX - bar->driver_virtual_address;
}

static inline int rin_ndrv_v3_irq_valid(const RinDriverV3IrqLeaseInfo* irq,
                                        uint32_t expected_mode) {
    return irq && irq->lease != 0u && irq->mode == expected_mode &&
           (expected_mode == RIN_DRIVER_V3_IRQ_LEGACY ||
            expected_mode == RIN_DRIVER_V3_IRQ_MSI ||
            expected_mode == RIN_DRIVER_V3_IRQ_MSIX) &&
           irq->vector >= 32u && irq->vector <= 255u;
}

static inline int rin_ndrv_v3_status_valid(
    const RinDriverV3Status* status) {
    if (!status || status->struct_size < sizeof(*status) ||
        status->version != RIN_DRIVER_V3_STATUS_VERSION ||
        status->health_state < RIN_DRIVER_V3_HEALTH_ACTIVE ||
        status->health_state > RIN_DRIVER_V3_HEALTH_LOST ||
        status->failure_reason > RIN_DRIVER_V3_FAILURE_EXTERNAL_FATAL ||
        (status->flags & ~(RIN_DRIVER_V3_STATUS_FLAG_QUARANTINED |
                           RIN_DRIVER_V3_STATUS_FLAG_REMOVING |
                           RIN_DRIVER_V3_STATUS_FLAG_DMA_STARTED)) != 0u ||
        status->device_generation == 0u || status->reserved[0] != 0u ||
        status->reserved[1] != 0u || status->reserved[2] != 0u)
        return 0;
    if (status->health_state == RIN_DRIVER_V3_HEALTH_ACTIVE)
        return status->failure_reason == RIN_DRIVER_V3_FAILURE_NONE &&
               status->failure_generation == 0u;
    return status->failure_reason != RIN_DRIVER_V3_FAILURE_NONE &&
           status->failure_generation != 0u;
}

static inline int rin_ndrv_v3_timer_event_valid(
    const RinDriverV3TimerEvent* event, uint64_t generation) {
    uint32_t kind;
    if (!event || event->lease == 0u || generation == 0u ||
        event->device_generation != generation || event->deadline_ns == 0u ||
        event->expirations == 0u ||
        (event->flags & ~RIN_DRIVER_V3_TIMER_EVENT_FLAG_MASK) != 0u)
        return 0;
    kind = event->flags & (RIN_DRIVER_V3_TIMER_EVENT_FLAG_ONE_SHOT |
                           RIN_DRIVER_V3_TIMER_EVENT_FLAG_PERIODIC);
    return (kind == RIN_DRIVER_V3_TIMER_EVENT_FLAG_ONE_SHOT &&
            event->expirations == 1u &&
            (event->flags & RIN_DRIVER_V3_TIMER_EVENT_FLAG_COALESCED) == 0u) ||
           (kind == RIN_DRIVER_V3_TIMER_EVENT_FLAG_PERIODIC);
}

static inline int rin_ndrv_v3_power_of_two_u32(uint32_t value) {
    return value != 0u && (value & (value - 1u)) == 0u;
}

static inline int rin_ndrv_v3_block_geometry_valid(
    const RinDriverV3BlockGeometry* geometry) {
    return geometry && geometry->struct_size >= sizeof(*geometry) &&
           geometry->version == RIN_DRIVER_V3_OPS_VERSION &&
           geometry->block_count != 0u &&
           geometry->logical_block_size >= 512u &&
           geometry->logical_block_size <= 65536u &&
           rin_ndrv_v3_power_of_two_u32(geometry->logical_block_size) &&
           geometry->physical_block_size >= geometry->logical_block_size &&
           geometry->physical_block_size % geometry->logical_block_size == 0u &&
           rin_ndrv_v3_power_of_two_u32(geometry->physical_block_size) &&
           geometry->optimal_transfer_blocks != 0u &&
           geometry->maximum_transfer_blocks != 0u &&
           geometry->optimal_transfer_blocks <=
               geometry->maximum_transfer_blocks &&
           geometry->maximum_transfer_blocks <= 65535u &&
           (geometry->flags & ~RIN_DRIVER_V3_BLOCK_FLAG_MASK) == 0u &&
           (geometry->flags & RIN_DRIVER_V3_BLOCK_FLAG_CONTROLLED_IO) != 0u &&
           geometry->reserved == 0u;
}

static inline int rin_ndrv_v3_block_request_valid(
    const RinDriverV3BlockRequest* request,
    const RinDriverV3MemorySlice* slices,
    const RinDriverV3BlockGeometry* geometry) {
    uint64_t expected_bytes;
    uint64_t supplied_bytes = 0u;
    if (!request || !rin_ndrv_v3_block_geometry_valid(geometry) ||
        request->struct_size < sizeof(*request) ||
        request->version != RIN_DRIVER_V3_OPS_VERSION ||
        request->request_id == 0u || request->deadline_ns == 0u ||
        (request->flags & ~RIN_DRIVER_V3_BLOCK_REQUEST_FLAG_MASK) != 0u ||
        request->reserved[0] != 0u || request->reserved[1] != 0u) {
        return 0;
    }
    if (request->operation == RIN_DRIVER_V3_BLOCK_FLUSH) {
        return request->lba == 0u && request->block_count == 0u &&
               request->slice_count == 0u && slices == NULL &&
               (request->flags & RIN_DRIVER_V3_BLOCK_REQUEST_FUA) == 0u &&
               (geometry->flags & RIN_DRIVER_V3_BLOCK_FLAG_FLUSH) != 0u;
    }
    if (request->operation == RIN_DRIVER_V3_BLOCK_DISCARD) {
        return request->flags == 0u && request->block_count != 0u &&
               request->block_count <= geometry->maximum_transfer_blocks &&
               request->lba < geometry->block_count &&
               request->block_count <= geometry->block_count - request->lba &&
               request->slice_count == 0u && slices == NULL &&
               (geometry->flags & RIN_DRIVER_V3_BLOCK_FLAG_DISCARD) != 0u;
    }
    if ((request->operation != RIN_DRIVER_V3_BLOCK_READ &&
         request->operation != RIN_DRIVER_V3_BLOCK_WRITE) ||
        request->block_count == 0u ||
        request->block_count > geometry->maximum_transfer_blocks ||
        request->lba >= geometry->block_count ||
        request->block_count > geometry->block_count - request->lba ||
        request->slice_count == 0u ||
        request->slice_count > RIN_DRIVER_V3_MAX_SG_SEGMENTS || !slices) {
        return 0;
    }
    if (request->operation == RIN_DRIVER_V3_BLOCK_READ &&
        request->flags != 0u) return 0;
    if (request->operation == RIN_DRIVER_V3_BLOCK_WRITE &&
        ((geometry->flags & RIN_DRIVER_V3_BLOCK_FLAG_READ_ONLY) != 0u ||
         (((request->flags & RIN_DRIVER_V3_BLOCK_REQUEST_FUA) != 0u) &&
          ((geometry->flags & RIN_DRIVER_V3_BLOCK_FLAG_FUA) == 0u)))) {
        return 0;
    }
    expected_bytes = (uint64_t)request->block_count *
                     geometry->logical_block_size;
    for (uint32_t index = 0u; index < request->slice_count; ++index) {
        if (slices[index].memory_lease == 0u || slices[index].length == 0u ||
            slices[index].offset > UINT64_MAX - slices[index].length ||
            supplied_bytes > UINT64_MAX - slices[index].length) {
            return 0;
        }
        supplied_bytes += slices[index].length;
    }
    return supplied_bytes == expected_bytes;
}

static inline int rin_ndrv_v3_block_completion_valid(
    const RinDriverV3BlockCompletion* completion,
    const RinDriverV3BlockRequest* request) {
    if (!completion || !request ||
        completion->struct_size < sizeof(*completion) ||
        completion->version != RIN_DRIVER_V3_OPS_VERSION ||
        request->request_id == 0u ||
        request->operation < RIN_DRIVER_V3_BLOCK_READ ||
        request->operation > RIN_DRIVER_V3_BLOCK_DISCARD ||
        completion->request_id != request->request_id ||
        completion->result > RIN_DRIVER_V3_OK ||
        completion->result < RIN_DRIVER_V3_UNSUPPORTED ||
        completion->flags != 0u || completion->reserved[0] != 0u ||
        completion->reserved[1] != 0u ||
        completion->transferred_blocks > request->block_count)
        return 0;
    if (completion->result != RIN_DRIVER_V3_OK)
        return completion->transferred_blocks == 0u;
    if (request->operation == RIN_DRIVER_V3_BLOCK_READ ||
        request->operation == RIN_DRIVER_V3_BLOCK_WRITE)
        return completion->transferred_blocks == request->block_count;
    return completion->transferred_blocks == 0u;
}

static inline int rin_ndrv_v3_usb_transfer_valid(
    const RinDriverV3UsbTransfer* transfer) {
    uint32_t endpoint;
    uint32_t direction_in;
    uint32_t setup_length;
    if (!transfer || transfer->struct_size < sizeof(*transfer) ||
        transfer->version != RIN_DRIVER_V3_OPS_VERSION ||
        transfer->transfer_id == 0u || transfer->deadline_ns == 0u ||
        transfer->port_generation == 0u ||
        transfer->device_address > 127u ||
        transfer->endpoint_address > 255u ||
        (transfer->flags & ~RIN_DRIVER_V3_USB_TRANSFER_FLAG_MASK) != 0u ||
        transfer->length > UINT32_C(1048576) ||
        transfer->reserved0 != 0u || transfer->reserved[0] != 0u ||
        transfer->reserved[1] != 0u) {
        return 0;
    }
    if (transfer->length == 0u) {
        if (transfer->memory_lease != 0u || transfer->offset != 0u)
            return 0;
    } else if (transfer->memory_lease == 0u ||
               transfer->offset > UINT64_MAX - transfer->length) {
        return 0;
    }
    endpoint = transfer->endpoint_address;
    if (transfer->transfer_type == RIN_DRIVER_V3_USB_TRANSFER_CONTROL) {
        setup_length = (uint32_t)(transfer->setup_packet >> 48u);
        direction_in = (uint32_t)(transfer->setup_packet >> 7u) & 1u;
        if (endpoint != 0u || setup_length != transfer->length)
            return 0;
    } else if (transfer->transfer_type ==
                   RIN_DRIVER_V3_USB_TRANSFER_INTERRUPT ||
               transfer->transfer_type ==
                   RIN_DRIVER_V3_USB_TRANSFER_BULK ||
               transfer->transfer_type ==
                   RIN_DRIVER_V3_USB_TRANSFER_ISOCHRONOUS) {
        direction_in = endpoint >> 7u;
        if (transfer->device_address == 0u ||
            (endpoint & UINT32_C(0x70)) != 0u ||
            (endpoint & UINT32_C(0x0f)) == 0u ||
            transfer->setup_packet != 0u || transfer->length == 0u)
            return 0;
    } else {
        return 0;
    }
    return direction_in != 0u ||
           (transfer->flags & RIN_DRIVER_V3_USB_TRANSFER_SHORT_OK) == 0u;
}

static inline int rin_ndrv_v3_usb_completion_valid(
    const RinDriverV3UsbTransferCompletion* completion,
    const RinDriverV3UsbTransfer* transfer) {
    if (!completion || !rin_ndrv_v3_usb_transfer_valid(transfer) ||
        completion->struct_size < sizeof(*completion) ||
        completion->version != RIN_DRIVER_V3_OPS_VERSION ||
        completion->transfer_id != transfer->transfer_id ||
        completion->result > RIN_DRIVER_V3_OK ||
        completion->result < RIN_DRIVER_V3_UNSUPPORTED ||
        completion->transferred_length > transfer->length ||
        completion->port_index != transfer->port_index ||
        completion->port_generation != transfer->port_generation ||
        completion->flags != 0u || completion->reserved[0] != 0u ||
        completion->reserved[1] != 0u) {
        return 0;
    }
    if (completion->result != RIN_DRIVER_V3_OK)
        return completion->transferred_length == 0u;
    return completion->transferred_length == transfer->length ||
           (transfer->flags & RIN_DRIVER_V3_USB_TRANSFER_SHORT_OK) != 0u;
}

static inline int rin_ndrv_v3_usb_ep0_packet_size_valid(
    uint32_t packet_size) {
    return packet_size == 8u || packet_size == 16u ||
           packet_size == 32u || packet_size == 64u ||
           packet_size == 512u;
}

static inline int rin_ndrv_v3_usb_speed_valid(uint32_t speed) {
    return speed >= RIN_DRIVER_V3_USB_SPEED_LOW &&
           speed <= RIN_DRIVER_V3_USB_SPEED_SUPER_PLUS;
}

static inline int rin_ndrv_v3_usb_port_status_valid(
    const RinDriverV3UsbPortStatus* status, uint32_t expected_port) {
    if (!status || status->struct_size < sizeof(*status) ||
        status->version != RIN_DRIVER_V3_OPS_VERSION ||
        status->port_index != expected_port || status->generation == 0u ||
        (status->flags & ~RIN_DRIVER_V3_USB_PORT_FLAG_MASK) != 0u ||
        (status->change_flags & ~RIN_DRIVER_V3_USB_PORT_CHANGE_FLAG_MASK) != 0u ||
        status->reserved0 != 0u || status->reserved[0] != 0u ||
        ((status->flags & RIN_DRIVER_V3_USB_PORT_ENABLED) != 0u &&
         (status->flags & RIN_DRIVER_V3_USB_PORT_CONNECTED) == 0u))
        return 0;
    if ((status->flags & RIN_DRIVER_V3_USB_PORT_CONNECTED) == 0u)
        return status->speed == 0u;
    return rin_ndrv_v3_usb_speed_valid(status->speed);
}

static inline int rin_ndrv_v3_usb_route_string_valid(uint32_t route_string,
                                                      uint32_t parent_port) {
    uint32_t remaining = route_string;
    uint32_t last_port = 0u;
    if (route_string == 0u || route_string > UINT32_C(0x000fffff) ||
        parent_port == 0u || parent_port > 15u)
        return 0;
    for (uint32_t tier = 0u; tier < 5u; ++tier) {
        uint32_t port = remaining & UINT32_C(0x0f);
        if (port == 0u) return 0;
        last_port = port;
        remaining >>= 4u;
        if (remaining == 0u) break;
    }
    return remaining == 0u && last_port == parent_port;
}

static inline int rin_ndrv_v3_usb_root_topology_valid(
    const RinDriverV3UsbDeviceRequest* request) {
    return request->topology.route_string == 0u &&
           request->topology.device_speed == 0u &&
           request->topology.parent_device_address == 0u &&
           request->topology.parent_port == 0u;
}

static inline int rin_ndrv_v3_usb_endpoint_configuration_valid(
    const RinDriverV3UsbEndpointConfiguration* endpoint) {
    uint32_t address;
    uint32_t transactions;
    uint64_t interval_capacity;
    if (!endpoint || endpoint->struct_size < sizeof(*endpoint) ||
        endpoint->version != RIN_DRIVER_V3_OPS_VERSION ||
        (endpoint->transfer_type != RIN_DRIVER_V3_USB_TRANSFER_INTERRUPT &&
         endpoint->transfer_type != RIN_DRIVER_V3_USB_TRANSFER_BULK &&
         endpoint->transfer_type != RIN_DRIVER_V3_USB_TRANSFER_ISOCHRONOUS) ||
        endpoint->max_packet_size == 0u ||
        endpoint->max_packet_size > 1024u || endpoint->interval > 255u ||
        (endpoint->transfer_type == RIN_DRIVER_V3_USB_TRANSFER_INTERRUPT &&
         endpoint->interval == 0u) ||
        (endpoint->transfer_type == RIN_DRIVER_V3_USB_TRANSFER_BULK &&
         endpoint->interval != 0u) ||
        (endpoint->transfer_type == RIN_DRIVER_V3_USB_TRANSFER_ISOCHRONOUS &&
         (endpoint->interval == 0u || endpoint->interval > 16u)) ||
        endpoint->max_burst > 15u ||
        endpoint->transactions_per_interval > 3u ||
        endpoint->reserved[1] != 0u)
        return 0;
    transactions = endpoint->transactions_per_interval == 0u
        ? 1u : endpoint->transactions_per_interval;
    if (endpoint->transfer_type != RIN_DRIVER_V3_USB_TRANSFER_ISOCHRONOUS &&
        (transactions != 1u || endpoint->reserved[0] != 0u))
        return 0;
    if (endpoint->transfer_type == RIN_DRIVER_V3_USB_TRANSFER_ISOCHRONOUS) {
        interval_capacity = (uint64_t)endpoint->max_packet_size *
                            (endpoint->max_burst + 1u) * transactions;
        if (endpoint->reserved[0] == 0u) {
            /* Legacy USB 2.0 endpoint geometry. */
            if (endpoint->max_burst != 0u ||
                transactions * endpoint->max_packet_size > 3072u)
                return 0;
        } else if (endpoint->reserved[0] < 2u ||
                   endpoint->reserved[0] >
                       RIN_DRIVER_V3_USB_MAX_ISOCH_ESIT_PAYLOAD ||
                   endpoint->reserved[0] > interval_capacity) {
            return 0;
        }
    }
    address = endpoint->endpoint_address;
    return (address & UINT32_C(0xffffff70)) == 0u &&
           (address & UINT32_C(0x0f)) != 0u;
}

static inline int rin_ndrv_v3_usb_device_request_valid(
    const RinDriverV3UsbDeviceRequest* request,
    const RinDriverV3UsbEndpointConfiguration* endpoints) {
    uint32_t endpoint_mask = 0u;
    if (!request || request->struct_size < sizeof(*request) ||
        request->version != RIN_DRIVER_V3_OPS_VERSION ||
        request->deadline_ns == 0u || request->port_generation == 0u)
        return 0;
    if (request->operation == RIN_DRIVER_V3_USB_DEVICE_ADDRESS) {
        int root = rin_ndrv_v3_usb_root_topology_valid(request);
        int descendant =
            rin_ndrv_v3_usb_route_string_valid(
                request->topology.route_string,
                request->topology.parent_port) &&
            rin_ndrv_v3_usb_speed_valid(
                request->topology.device_speed) &&
            request->topology.parent_device_address != 0u &&
            request->topology.parent_device_address <= 127u;
        return request->flags == 0u && (root || descendant) &&
               request->device_address == 0u &&
               request->configuration_value == 0u &&
               request->ep0_max_packet_size == 0u &&
               request->endpoint_count == 0u && !endpoints;
    }
    if (request->device_address == 0u || request->device_address > 127u)
        return 0;
    if (request->operation == RIN_DRIVER_V3_USB_DEVICE_DECLARE_HUB) {
        return (request->flags & ~RIN_DRIVER_V3_USB_DEVICE_HUB_FLAG_MASK) ==
                   0u &&
               request->configuration_value == 0u &&
               request->ep0_max_packet_size == 0u &&
               request->endpoint_count == 0u && !endpoints &&
               request->hub.port_count != 0u &&
               request->hub.port_count <= 255u &&
               request->hub.tt_think_time <= 3u &&
               request->hub.reserved0 == 0u &&
               request->hub.reserved1 == 0u;
    }
    if (request->flags != 0u ||
        !rin_ndrv_v3_usb_root_topology_valid(request))
        return 0;
    if (request->operation == RIN_DRIVER_V3_USB_DEVICE_UPDATE_EP0) {
        return request->configuration_value == 0u &&
               rin_ndrv_v3_usb_ep0_packet_size_valid(
                   request->ep0_max_packet_size) &&
               request->endpoint_count == 0u && !endpoints;
    }
    if (request->operation == RIN_DRIVER_V3_USB_DEVICE_RELEASE) {
        return request->configuration_value == 0u &&
               request->ep0_max_packet_size == 0u &&
               request->endpoint_count == 0u && !endpoints;
    }
    if (request->operation != RIN_DRIVER_V3_USB_DEVICE_CONFIGURE ||
        request->configuration_value == 0u ||
        request->configuration_value > 255u ||
        request->ep0_max_packet_size != 0u || !endpoints ||
        request->endpoint_count == 0u ||
        request->endpoint_count > RIN_DRIVER_V3_USB_DEVICE_ENDPOINTS_MAX)
        return 0;
    for (uint32_t index = 0u; index < request->endpoint_count; ++index) {
        uint32_t address;
        uint32_t device_context_index;
        uint32_t bit;
        if (!rin_ndrv_v3_usb_endpoint_configuration_valid(
                &endpoints[index]))
            return 0;
        address = endpoints[index].endpoint_address;
        device_context_index = ((address & UINT32_C(0x0f)) << 1u) |
                               ((address >> 7u) & 1u);
        bit = UINT32_C(1) << device_context_index;
        if ((endpoint_mask & bit) != 0u) return 0;
        endpoint_mask |= bit;
    }
    return 1;
}

static inline int rin_ndrv_v3_usb_device_response_valid(
    const RinDriverV3UsbDeviceResponse* response,
    const RinDriverV3UsbDeviceRequest* request) {
    if (!response || !request || response->struct_size < sizeof(*response) ||
        response->version != RIN_DRIVER_V3_OPS_VERSION ||
        response->port_index != request->port_index ||
        response->port_generation != request->port_generation ||
        response->device_address == 0u || response->device_address > 127u ||
        response->flags != 0u || response->reserved[0] != 0u ||
        response->reserved[1] != 0u)
        return 0;
    if (request->operation == RIN_DRIVER_V3_USB_DEVICE_ADDRESS)
        return response->configuration_value == 0u &&
               response->endpoint_count == 0u;
    if (response->device_address != request->device_address) return 0;
    if (request->operation == RIN_DRIVER_V3_USB_DEVICE_CONFIGURE)
        return response->configuration_value ==
                   request->configuration_value &&
               response->endpoint_count == request->endpoint_count;
    return response->configuration_value == 0u &&
           response->endpoint_count == 0u;
}

/* Freestanding i686 must not acquire a hidden libgcc __udivdi3 import. */
static inline uint64_t rin_ndrv_v3_udiv64_u32(uint64_t numerator,
                                              uint32_t denominator) {
    uint64_t quotient = 0u;
    uint64_t remainder = 0u;
    if (denominator == 0u) return 0u;
    for (int bit = 63; bit >= 0; --bit) {
        remainder = (remainder << 1u) | ((numerator >> bit) & 1u);
        if (remainder >= denominator) {
            remainder -= denominator;
            quotient |= UINT64_C(1) << bit;
        }
    }
    return quotient;
}

static inline void rin_ndrv_v3_release_if(const RinDriverApiV3* api,
                                          uint64_t session,
                                          RinDriverV3Lease* lease) {
    if (*lease != 0u) {
        (void)api->lease_release(api->api_context, session, *lease);
        *lease = 0u;
    }
}

#endif
