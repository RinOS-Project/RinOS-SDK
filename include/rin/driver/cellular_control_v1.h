/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Shared NDRV/networkd cellular control request and result contract. */
#ifndef RIN_DRIVER_CELLULAR_CONTROL_V1_H
#define RIN_DRIVER_CELLULAR_CONTROL_V1_H

#include <stdint.h>

#define RIN_NETWORK_MBIM_CONTROL_VERSION UINT32_C(1)
#define RIN_NETWORK_MBIM_APN_MAX UINT32_C(100)
#define RIN_NETWORK_MBIM_CREDENTIAL_MAX UINT32_C(55)
#define RIN_NETWORK_MBIM_SIM_PIN_MAX UINT32_C(16)
#define RIN_NETWORK_MBIM_ADDRESS_MAX UINT32_C(4)

enum RinNetworkMbimOperationV1 {
    RIN_NETWORK_MBIM_OPERATION_CONNECT = 1u,
    RIN_NETWORK_MBIM_OPERATION_DISCONNECT = 2u,
    RIN_NETWORK_MBIM_OPERATION_QUERY = 3u,
};

enum RinNetworkMbimIpTypeV1 {
    RIN_NETWORK_MBIM_IP_IPV4 = 1u,
    RIN_NETWORK_MBIM_IP_IPV6 = 2u,
    RIN_NETWORK_MBIM_IP_IPV4V6 = 3u,
};

enum RinNetworkMbimAuthV1 {
    RIN_NETWORK_MBIM_AUTH_NONE = 0u,
    RIN_NETWORK_MBIM_AUTH_PAP = 1u,
    RIN_NETWORK_MBIM_AUTH_CHAP = 2u,
    RIN_NETWORK_MBIM_AUTH_MSCHAP = 3u,
};

#define RIN_NETWORK_MBIM_RESULT_CONNECTED UINT32_C(0x00000001)
#define RIN_NETWORK_MBIM_RESULT_IPV4 UINT32_C(0x00000002)
#define RIN_NETWORK_MBIM_RESULT_IPV6 UINT32_C(0x00000004)
#define RIN_NETWORK_MBIM_RESULT_KNOWN_FLAGS UINT32_C(0x00000007)
#define RIN_NETWORK_MBIM_IP_AVAILABLE_ADDRESS UINT32_C(0x00000001)
#define RIN_NETWORK_MBIM_IP_AVAILABLE_GATEWAY UINT32_C(0x00000002)
#define RIN_NETWORK_MBIM_IP_AVAILABLE_DNS UINT32_C(0x00000004)
#define RIN_NETWORK_MBIM_IP_AVAILABLE_MTU UINT32_C(0x00000008)
#define RIN_NETWORK_MBIM_IP_AVAILABLE_KNOWN_FLAGS UINT32_C(0x0000000f)

#pragma pack(push, 1)
typedef struct RinNetworkMbimControlRequestV1 {
    uint32_t version;
    uint32_t struct_size;
    uint32_t operation;
    uint32_t ip_type;
    uint32_t auth_protocol;
    uint32_t reserved[3];
    char apn[RIN_NETWORK_MBIM_APN_MAX + 1u];
    char username[RIN_NETWORK_MBIM_CREDENTIAL_MAX + 1u];
    char password[RIN_NETWORK_MBIM_CREDENTIAL_MAX + 1u];
    char sim_pin[RIN_NETWORK_MBIM_SIM_PIN_MAX + 1u];
} RinNetworkMbimControlRequestV1;

typedef struct RinNetworkMbimIPv4AddressV1 {
    uint8_t address[4];
    uint8_t prefix_length;
    uint8_t reserved[3];
} RinNetworkMbimIPv4AddressV1;

typedef struct RinNetworkMbimIPv6AddressV1 {
    uint8_t address[16];
    uint8_t prefix_length;
    uint8_t reserved[3];
} RinNetworkMbimIPv6AddressV1;

typedef struct RinNetworkMbimControlResultV1 {
    uint32_t version;
    uint32_t struct_size;
    uint32_t operation;
    uint32_t flags;
    uint32_t ipv4_available;
    uint32_t ipv6_available;
    uint32_t ipv4_address_count;
    uint32_t ipv6_address_count;
    RinNetworkMbimIPv4AddressV1 ipv4_addresses[
        RIN_NETWORK_MBIM_ADDRESS_MAX];
    uint8_t ipv4_gateway[4];
    uint32_t ipv4_dns_count;
    uint8_t ipv4_dns[RIN_NETWORK_MBIM_ADDRESS_MAX][4];
    RinNetworkMbimIPv6AddressV1 ipv6_addresses[
        RIN_NETWORK_MBIM_ADDRESS_MAX];
    uint8_t ipv6_gateway[16];
    uint32_t ipv6_dns_count;
    uint8_t ipv6_dns[RIN_NETWORK_MBIM_ADDRESS_MAX][16];
    uint32_t ipv4_mtu;
    uint32_t ipv6_mtu;
    uint32_t reserved;
} RinNetworkMbimControlResultV1;

typedef struct RinNetworkMbimControlIoctlV1 {
    RinNetworkMbimControlRequestV1 request;
    RinNetworkMbimControlResultV1 result;
} RinNetworkMbimControlIoctlV1;
#pragma pack(pop)

#if defined(__cplusplus)
static_assert(sizeof(RinNetworkMbimControlRequestV1) == 262u,
              "MBIM control request ABI drift");
static_assert(sizeof(RinNetworkMbimControlResultV1) == 264u,
              "MBIM control result ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinNetworkMbimControlRequestV1) == 262u,
               "MBIM control request ABI drift");
_Static_assert(sizeof(RinNetworkMbimControlResultV1) == 264u,
               "MBIM control result ABI drift");
#endif

#endif /* RIN_DRIVER_CELLULAR_CONTROL_V1_H */
