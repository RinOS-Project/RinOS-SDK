/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_MANAGEMENT_USB4_V1_ABI_H
#define RIN_MANAGEMENT_USB4_V1_ABI_H

#include <stddef.h>
#include <stdint.h>

#include <rin/contract_abi.h>

#define RIN_MANAGEMENT_OPERATION_USB4_DEVICE_COUNT_GET 268u
#define RIN_MANAGEMENT_OPERATION_USB4_DEVICE_GET 269u
#define RIN_MANAGEMENT_OPERATION_USB4_TOPOLOGY_GET 270u
#define RIN_MANAGEMENT_OPERATION_USB4_ROUTER_AUTHORIZE 271u
#define RIN_MANAGEMENT_OPERATION_USB4_PATH_PAIR_CREATE 272u
#define RIN_MANAGEMENT_OPERATION_USB4_PATH_PAIR_REMOVE 273u
#define RIN_MANAGEMENT_OPERATION_USB4_USB3_BANDWIDTH 274u
#define RIN_MANAGEMENT_OPERATION_USB4_PATH_LINK_BANDWIDTH 275u
#define RIN_MANAGEMENT_OPERATION_USB4_PATH_BANDWIDTH 276u

#define RIN_MANAGEMENT_USB4_V1_VERSION 1u
#define RIN_MANAGEMENT_USB4_MAX_DEVICES 4u
#define RIN_MANAGEMENT_USB4_MAX_ROUTERS 32u
#define RIN_MANAGEMENT_USB4_MAX_PORTS 64u
#define RIN_MANAGEMENT_USB4_MAX_PORTS_PER_PAGE 20u
#define RIN_MANAGEMENT_USB4_ROUTER_FLAG_CONFIGURED UINT16_C(0x0001)
#define RIN_MANAGEMENT_USB4_ROUTER_FLAG_AUTHORIZED UINT16_C(0x0002)
#define RIN_MANAGEMENT_USB4_ROUTER_FLAG_UNIQUE_ID_VALID UINT16_C(0x0004)
#define RIN_MANAGEMENT_USB4_TUNNEL_PCIE UINT8_C(0x01)
#define RIN_MANAGEMENT_USB4_TUNNEL_USB3 UINT8_C(0x02)
#define RIN_MANAGEMENT_USB4_USB3_BANDWIDTH_QUERY UINT8_C(0)
#define RIN_MANAGEMENT_USB4_USB3_BANDWIDTH_ALLOCATE UINT8_C(1)
#define RIN_MANAGEMENT_USB4_USB3_BANDWIDTH_RELEASE UINT8_C(2)
#define RIN_MANAGEMENT_USB4_PATH_BANDWIDTH_COVERAGE_MANAGED_PATHS \
    UINT32_C(0x00000001)
#define RIN_MANAGEMENT_USB4_PATH_BANDWIDTH_COVERAGE_DP_UNACCOUNTED \
    UINT32_C(0x00000002)
#define RIN_MANAGEMENT_USB4_PATH_BANDWIDTH_COVERAGE_EXTERNAL_UNACCOUNTED \
    UINT32_C(0x00000004)
#define RIN_MANAGEMENT_USB4_PATH_BANDWIDTH_COVERAGE_MANAGED_USB3_UNACCOUNTED \
    UINT32_C(0x00000008)

typedef struct RinManagementUsb4DeviceCountV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t device_count;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinManagementUsb4DeviceCountV1;

/* device_generation pins the PCI catalog identity; topology_generation pins
 * the successful router scan. Neither field is a kernel object cookie. */
typedef struct RinManagementUsb4DeviceV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t device_index;
    uint16_t segment;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t reserved0;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t router_count;
    uint16_t reserved1;
    /* Keep generation fields at an 8-byte boundary in 32-bit and 64-bit
     * callers; these bytes occupied implicit x64 padding. */
    uint16_t reserved2[3];
    uint64_t device_generation;
    uint64_t topology_generation;
    uint64_t reserved[2];
} RinManagementUsb4DeviceV1;

typedef struct RinManagementUsb4PortV1 {
    uint8_t port_number;
    uint8_t type;
    uint8_t scan_state;
    uint8_t reserved0;
    uint16_t child_router_index;
    uint16_t adapter_capability;
    uint16_t max_in_hop_id;
    uint16_t max_out_hop_id;
    uint32_t usb4_status;
} RinManagementUsb4PortV1;

/* Returns one router and a page of ports, keeping the operation within the
 * fixed Management request payload. Port indices start at one; zero means the
 * router exposes no ports. */
typedef struct RinManagementUsb4TopologyV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t device_index;
    uint16_t router_index;
    uint16_t first_port;
    uint16_t returned_port_count;
    uint16_t total_port_count;
    uint16_t router_count;
    uint16_t reserved1;
    uint64_t device_generation;
    uint64_t topology_generation;
    uint64_t route;
    uint64_t unique_id;
    uint16_t parent_router_index;
    uint8_t parent_port_number;
    uint8_t depth;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t maximum_port_number;
    uint8_t revision;
    uint8_t usb4_version;
    uint8_t configuration_state;
    uint8_t tunnel_flags;
    uint8_t authorization_state;
    uint8_t unique_id_valid;
    uint16_t reserved0;
    RinManagementUsb4PortV1 ports[
        RIN_MANAGEMENT_USB4_MAX_PORTS_PER_PAGE];
    /* Keep the trailing reservation at the same offset on 32-bit callers. */
    uint16_t reserved2[2];
    uint64_t reserved[2];
} RinManagementUsb4TopologyV1;

/* The pair token is scoped to device_index and device_generation. Applications
 * must preserve the complete record and return it when requesting teardown. */
typedef struct RinManagementUsb4PathPairV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t device_index;
    uint32_t reserved0;
    uint64_t device_generation;
    uint64_t topology_generation;
    uint64_t forward_token;
    uint64_t reverse_token;
    uint16_t source_router;
    uint16_t destination_router;
    uint8_t source_port;
    uint8_t destination_port;
    uint8_t tunnel_type;
    uint8_t state;
    uint64_t reserved[2];
} RinManagementUsb4PathPairV1;

/* Host USB3 adapter isochronous bandwidth, in Mb/s. Query fills allocation,
 * consumption, and adapter link ceiling; allocation cannot undercut live
 * consumption, and release retains 900 Mb/s in each direction for xHCI. */
typedef struct RinManagementUsb4Usb3BandwidthV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t device_index;
    uint16_t router_index;
    uint8_t port_number;
    uint8_t action;
    uint64_t device_generation;
    uint64_t topology_generation;
    uint32_t requested_upstream_mbps;
    uint32_t requested_downstream_mbps;
    uint32_t allocated_upstream_mbps;
    uint32_t allocated_downstream_mbps;
    uint32_t consumed_upstream_mbps;
    uint32_t consumed_downstream_mbps;
    uint32_t maximum_link_rate_mbps;
    uint32_t allocation_granularity_mbps;
    uint32_t reserved[2];
} RinManagementUsb4Usb3BandwidthV1;

/* Physical link ceiling for a router-to-router route. This reports the
 * generation-pinned minimum of every live link on the route, including the
 * USB4 connection-manager guard band. It does not represent unallocated
 * bandwidth after tunneled traffic reservations. */
typedef struct RinManagementUsb4PathLinkBandwidthV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t device_index;
    uint16_t source_router;
    uint16_t destination_router;
    uint64_t device_generation;
    uint64_t topology_generation;
    uint32_t maximum_upstream_mbps;
    uint32_t maximum_downstream_mbps;
    uint32_t minimum_link_speed_mbps;
    uint8_t link_count;
    uint8_t minimum_link_width;
    uint16_t reserved0;
    uint64_t reserved[2];
} RinManagementUsb4PathLinkBandwidthV1;

/* Route capacity after reservations known to this software Connection
 * Manager. Upstream/downstream are the link's system-oriented directions.
 * coverage_flags bits are the RIN_MANAGEMENT_USB4_PATH_BANDWIDTH_COVERAGE_*
 * values above. Managed-path scope excludes firmware or other-Connection-
 * Manager tunnels. USB3 allocations on non-host adapters are also not
 * accounted. available_* is not an admission ceiling while an
 * unaccounted-traffic bit is set. */
typedef struct RinManagementUsb4PathBandwidthV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t device_index;
    uint16_t source_router;
    uint16_t destination_router;
    uint64_t device_generation;
    uint64_t topology_generation;
    uint32_t maximum_upstream_mbps;
    uint32_t maximum_downstream_mbps;
    uint32_t reserved_upstream_mbps;
    uint32_t reserved_downstream_mbps;
    uint32_t available_upstream_mbps;
    uint32_t available_downstream_mbps;
    uint8_t link_count;
    uint8_t reserved0[3];
    uint32_t coverage_flags;
    uint64_t reserved[2];
} RinManagementUsb4PathBandwidthV1;

#if defined(__cplusplus)
static_assert(sizeof(RinManagementUsb4DeviceCountV1) == 32u,
              "RinManagementUsb4DeviceCountV1 ABI drift");
static_assert(sizeof(RinManagementUsb4DeviceV1) == 64u,
              "RinManagementUsb4DeviceV1 ABI drift");
static_assert(offsetof(RinManagementUsb4DeviceV1, device_generation) == 32u,
              "RinManagementUsb4DeviceV1 generation offset drift");
static_assert(sizeof(RinManagementUsb4PortV1) == 16u,
              "RinManagementUsb4PortV1 ABI drift");
static_assert(sizeof(RinManagementUsb4TopologyV1) == 416u,
              "RinManagementUsb4TopologyV1 ABI drift");
static_assert(offsetof(RinManagementUsb4TopologyV1, reserved) == 400u,
              "RinManagementUsb4TopologyV1 reserved offset drift");
static_assert(sizeof(RinManagementUsb4PathPairV1) == 72u,
              "RinManagementUsb4PathPairV1 ABI drift");
static_assert(sizeof(RinManagementUsb4Usb3BandwidthV1) == 72u,
              "RinManagementUsb4Usb3BandwidthV1 ABI drift");
static_assert(sizeof(RinManagementUsb4PathLinkBandwidthV1) == 64u,
              "RinManagementUsb4PathLinkBandwidthV1 ABI drift");
static_assert(sizeof(RinManagementUsb4PathBandwidthV1) == 80u,
              "RinManagementUsb4PathBandwidthV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinManagementUsb4DeviceCountV1) == 32u,
               "RinManagementUsb4DeviceCountV1 ABI drift");
_Static_assert(sizeof(RinManagementUsb4DeviceV1) == 64u,
               "RinManagementUsb4DeviceV1 ABI drift");
_Static_assert(offsetof(RinManagementUsb4DeviceV1, device_generation) == 32u,
               "RinManagementUsb4DeviceV1 generation offset drift");
_Static_assert(sizeof(RinManagementUsb4PortV1) == 16u,
               "RinManagementUsb4PortV1 ABI drift");
_Static_assert(sizeof(RinManagementUsb4TopologyV1) == 416u,
               "RinManagementUsb4TopologyV1 ABI drift");
_Static_assert(offsetof(RinManagementUsb4TopologyV1, reserved) == 400u,
               "RinManagementUsb4TopologyV1 reserved offset drift");
_Static_assert(sizeof(RinManagementUsb4PathPairV1) == 72u,
               "RinManagementUsb4PathPairV1 ABI drift");
_Static_assert(sizeof(RinManagementUsb4Usb3BandwidthV1) == 72u,
               "RinManagementUsb4Usb3BandwidthV1 ABI drift");
_Static_assert(sizeof(RinManagementUsb4PathLinkBandwidthV1) == 64u,
               "RinManagementUsb4PathLinkBandwidthV1 ABI drift");
_Static_assert(sizeof(RinManagementUsb4PathBandwidthV1) == 80u,
               "RinManagementUsb4PathBandwidthV1 ABI drift");
#endif

#endif /* RIN_MANAGEMENT_USB4_V1_ABI_H */
