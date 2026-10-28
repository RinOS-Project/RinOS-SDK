/* SPDX-License-Identifier: MIT */
/* Public Driver entrypoint for OS-Core-owned capture transport registration. */

#ifndef RIN_DRIVER_CAPTURE_TRANSPORT_SERVICE_V1_H
#define RIN_DRIVER_CAPTURE_TRANSPORT_SERVICE_V1_H

#include <stdint.h>

#include <rin/driver/device_resource_v1.h>
#include <rinruntime/rin_capture_transport.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Requests one permanent CAMERA or MICROPHONE provider registration. OS-Core
 * snapshots the callback table and owns the capability endpoint adapter and
 * all per-consumer sessions. The transport context must remain valid for the
 * lifetime of the registration. */
RinDeviceResourceEndpointStatus
rin_device_resource_product_register_capture_transport_v1(
    uint32_t resource, const RinCaptureTransportV1* transport);

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_CAPTURE_TRANSPORT_SERVICE_V1_H */
