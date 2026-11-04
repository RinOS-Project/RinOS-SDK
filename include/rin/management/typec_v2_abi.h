/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_MANAGEMENT_TYPEC_V2_ABI_H
#define RIN_MANAGEMENT_TYPEC_V2_ABI_H

#include <stddef.h>
#include <stdint.h>

#include <rin/contract_abi.h>

/* V2 adds ACPI-proven xHCI root-port links without changing the frozen V1
 * connector snapshot or consuming its reserved fields. */
#define RIN_MANAGEMENT_OPERATION_TYPEC_CONNECTOR_GET_V2 267u
#define RIN_MANAGEMENT_TYPEC_CONNECTOR_V2_VERSION 2u
#define RIN_MANAGEMENT_TYPEC_CONNECTOR_V2_FLAG_PORT_MAP_VALID UINT32_C(1)
#define RIN_MANAGEMENT_TYPEC_CONNECTOR_V2_FLAG_MASK \
    RIN_MANAGEMENT_TYPEC_CONNECTOR_V2_FLAG_PORT_MAP_VALID
#define RIN_MANAGEMENT_TYPEC_MAX_MAPPED_USB_PORTS 16u

typedef struct RinManagementTypeCUsbPortV1 {
    uint16_t segment;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t root_port;
    uint8_t reserved[2];
} RinManagementTypeCUsbPortV1;

typedef struct RinManagementTypeCConnectorV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    RinManagementTypeCConnectorV1 connector;
    uint32_t mapping_flags;
    uint32_t mapped_port_count;
    RinManagementTypeCUsbPortV1 mapped_ports[
        RIN_MANAGEMENT_TYPEC_MAX_MAPPED_USB_PORTS];
    uint64_t reserved[2];
} RinManagementTypeCConnectorV2;

#if defined(__cplusplus)
static_assert(sizeof(RinManagementTypeCUsbPortV1) == 8u,
              "RinManagementTypeCUsbPortV1 ABI drift");
static_assert(sizeof(RinManagementTypeCConnectorV2) == 248u,
              "RinManagementTypeCConnectorV2 ABI drift");
static_assert(offsetof(RinManagementTypeCConnectorV2, connector) == 8u,
              "RinManagementTypeCConnectorV2.connector ABI drift");
static_assert(offsetof(RinManagementTypeCConnectorV2, mapped_ports) == 104u,
              "RinManagementTypeCConnectorV2.mapped_ports ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinManagementTypeCUsbPortV1) == 8u,
               "RinManagementTypeCUsbPortV1 ABI drift");
_Static_assert(sizeof(RinManagementTypeCConnectorV2) == 248u,
               "RinManagementTypeCConnectorV2 ABI drift");
_Static_assert(offsetof(RinManagementTypeCConnectorV2, connector) == 8u,
               "RinManagementTypeCConnectorV2.connector ABI drift");
_Static_assert(offsetof(RinManagementTypeCConnectorV2, mapped_ports) == 104u,
               "RinManagementTypeCConnectorV2.mapped_ports ABI drift");
#endif

#endif /* RIN_MANAGEMENT_TYPEC_V2_ABI_H */
