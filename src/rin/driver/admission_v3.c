/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <rin/driver/admission_v3.h>

static int admission_reserved_zero(const uint64_t* values, uint32_t count)
{
    for (uint32_t index = 0u; index < count; ++index) {
        if (values[index] != 0u) return 0;
    }
    return 1;
}

static int admission_resources_valid(uint64_t resources)
{
    const uint64_t allowed = RIN_DRIVER_RESOURCE_PCI |
                             RIN_DRIVER_RESOURCE_MMIO |
                             RIN_DRIVER_RESOURCE_IOPORT |
                             RIN_DRIVER_RESOURCE_DMA |
                             RIN_DRIVER_RESOURCE_IRQ |
                             RIN_DRIVER_RESOURCE_BLOCK |
                             RIN_DRIVER_RESOURCE_NETWORK |
                             RIN_DRIVER_RESOURCE_INPUT |
                             RIN_DRIVER_RESOURCE_DISPLAY |
                             RIN_DRIVER_RESOURCE_AUDIO |
                             RIN_DRIVER_RESOURCE_USB;
    return (resources & ~allowed) == 0u;
}

static int admission_match_device(const RinDriverMatchV3* match,
                                  const RinDriverDeviceIdentityV3* device)
{
    if (match->bus_type != device->bus_type ||
        (match->required_resources & ~device->available_resources) != 0u) {
        return 0;
    }
    if ((match->flags & RIN_DRIVER_MATCH_VENDOR_DEVICE) != 0u &&
        (match->vendor_id != device->vendor_id ||
         match->device_id != device->device_id)) {
        return 0;
    }
    if ((match->flags & RIN_DRIVER_MATCH_SUBSYSTEM) != 0u &&
        (match->subsystem_vendor_id != device->subsystem_vendor_id ||
         match->subsystem_device_id != device->subsystem_device_id)) {
        return 0;
    }
    if ((match->flags & RIN_DRIVER_MATCH_CLASS) != 0u &&
        (match->class_code & match->class_mask) !=
            (device->class_code & match->class_mask)) {
        return 0;
    }
    if ((match->flags & RIN_DRIVER_MATCH_REVISION) != 0u &&
        (match->revision & match->revision_mask) !=
            (device->revision & match->revision_mask)) {
        return 0;
    }
    return 1;
}

RinDriverAdmissionV3Result rin_driver_admission_v3_match_image(
    const RinDriverHeaderV3* image_header,
    const RinDriverMatchV3* image_matches,
    const RinDriverDeviceIdentityV3* device,
    const RinDriverAdmissionPolicyV3* policy,
    uint32_t* matched_index)
{
    if (matched_index) *matched_index = UINT32_MAX;
    if (!image_header || !image_matches || image_header->match_count == 0u ||
        image_header->match_count > RIN_DRIVER_ADMISSION_V3_MAX_MATCHES ||
        !device || !policy ||
        device->struct_size < sizeof(*device) ||
        device->version != RIN_DRIVER_ADMISSION_V3_VERSION ||
        device->device_cookie == 0u || device->generation == 0u ||
        !admission_resources_valid(device->available_resources) ||
        device->bus_type <= RIN_DRIVER_BUS_ANY ||
        device->bus_type > RIN_DRIVER_BUS_VIRTIO ||
        device->reserved0 != 0u || device->reserved1 != 0u ||
        device->reserved != 0u ||
        policy->struct_size < sizeof(*policy) ||
        policy->version != RIN_DRIVER_ADMISSION_V3_VERSION ||
        (policy->architecture != RIN_ARCH_X86 &&
         policy->architecture != RIN_ARCH_X86_64) ||
        policy->allow_experimental > 1u || policy->reserved0 != 0u ||
        !admission_reserved_zero(policy->reserved, 2u)) {
        return RIN_DRIVER_ADMISSION_V3_INVALID_ARGUMENT;
    }
    if (image_header->architecture != policy->architecture) {
        return RIN_DRIVER_ADMISSION_V3_ARCHITECTURE_MISMATCH;
    }
    if ((image_header->flags & RIN_DRIVER_IMAGE_EXPERIMENTAL) != 0u &&
        policy->allow_experimental == 0u) {
        return RIN_DRIVER_ADMISSION_V3_EXPERIMENTAL_DENIED;
    }
    if ((image_header->required_cpu_features &
         ~policy->available_cpu_features) != 0u ||
        (image_header->required_resources &
         ~device->available_resources) != 0u) {
        return RIN_DRIVER_ADMISSION_V3_REQUIREMENTS_UNMET;
    }
    for (uint32_t index = 0u; index < image_header->match_count; ++index) {
        if (admission_match_device(&image_matches[index], device)) {
            if (matched_index) *matched_index = index;
            return RIN_DRIVER_ADMISSION_V3_OK;
        }
    }
    return RIN_DRIVER_ADMISSION_V3_NO_MATCH;
}
