/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_RIN_NETDEV_LINK_H
#define RIN_SDK_RIN_NETDEV_LINK_H

#include <stddef.h>
#include <stdint.h>

/* The legacy NetDevOps record has a fixed seven-pointer ABI.  Link state is
 * consequently queried through its pre-existing ioctl slot instead of
 * appending a callback that older signed drivers do not own. */
#define RIN_NETDEV_LINK_STATUS_VERSION UINT32_C(1)
#define RIN_NETDEV_IOCTL_LINK_STATUS UINT32_C(0x524e4c01)
#define RIN_NETDEV_IOCTL_WIFI_INFO UINT32_C(0x524e4c02)
#define RIN_NETDEV_INTERFACE_PROPERTIES_VERSION UINT32_C(1)
#define RIN_NETDEV_IOCTL_INTERFACE_PROPERTIES UINT32_C(0x524e4c06)
#define RIN_NETDEV_INTERFACE_FLAG_ADMIN_KNOWN UINT32_C(0x00000001)
#define RIN_NETDEV_INTERFACE_FLAG_ADMIN_UP UINT32_C(0x00000002)
#define RIN_NETDEV_INTERFACE_FLAG_TYPE_KNOWN UINT32_C(0x00000004)
#define RIN_NETDEV_INTERFACE_FLAG_CAPABILITIES_KNOWN UINT32_C(0x00000008)
#define RIN_NETDEV_INTERFACE_KNOWN_FLAGS UINT32_C(0x0000000f)
#define RIN_NETDEV_INTERFACE_CAP_BROADCAST UINT32_C(0x00000001)
#define RIN_NETDEV_INTERFACE_CAP_MULTICAST UINT32_C(0x00000002)
#define RIN_NETDEV_INTERFACE_CAP_POINT_TO_POINT UINT32_C(0x00000004)
#define RIN_NETDEV_INTERFACE_KNOWN_CAPABILITIES UINT32_C(0x00000007)
#define RIN_NETDEV_INTERFACE_TYPE_UNKNOWN UINT32_C(0)
#define RIN_NETDEV_INTERFACE_TYPE_OTHER UINT32_C(1)
#define RIN_NETDEV_INTERFACE_TYPE_LOOPBACK UINT32_C(2)
#define RIN_NETDEV_INTERFACE_TYPE_BROADCAST UINT32_C(3)
#define RIN_NETDEV_INTERFACE_TYPE_POINT_TO_POINT UINT32_C(4)
#define RIN_NETDEV_INTERFACE_TYPE_MAX \
    RIN_NETDEV_INTERFACE_TYPE_POINT_TO_POINT

/* Stable link classification is supplied by the network owner.  Consumers
 * must not infer it from an interface name. */
#define RIN_NET_LINK_TYPE_UNKNOWN   UINT32_C(0)
#define RIN_NET_LINK_TYPE_ETHERNET  UINT32_C(1)
#define RIN_NET_LINK_TYPE_WIFI      UINT32_C(2)
#define RIN_NET_LINK_TYPE_CELLULAR  UINT32_C(3)
#define RIN_NET_LINK_TYPE_BLUETOOTH_PAN UINT32_C(4)
#define RIN_NET_LINK_TYPE_VIRTUAL   UINT32_C(5)
#define RIN_NET_LINK_TYPE_MAX       RIN_NET_LINK_TYPE_VIRTUAL
/* A strength value is a percentage only when it is <= 100.  Providers that
 * do not expose radio telemetry use this sentinel instead of publishing a
 * fabricated zero-strength reading. */
#define RIN_NET_SIGNAL_UNKNOWN      UINT32_MAX

typedef struct RinNetdevLinkStatusV1 {
    uint32_t version;
    uint32_t struct_size;
    uint32_t link_up;
    uint32_t speed_mbps;
    uint32_t link_type;
    uint32_t signal_percent;
} RinNetdevLinkStatusV1;

/* Optional driver-owned L3/admin properties. TYPE_KNOWN and
 * CAPABILITIES_KNOWN independently qualify their values. A nonzero peer is
 * explicit even when capability metadata is unknown. Unsupported ioctl leaves
 * the snapshot fields unknown; consumers must not infer them from link_type. */
typedef struct RinNetdevInterfacePropertiesV1 {
    uint32_t version;
    uint32_t struct_size;
    uint32_t flags;
    uint32_t interface_type;
    uint32_t capabilities;
    uint8_t ipv4_peer[4];
    uint32_t reserved;
} RinNetdevInterfacePropertiesV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNetdevLinkStatusV1) == 24u,
              "network link-status ABI drift");
static_assert(sizeof(RinNetdevInterfacePropertiesV1) == 28u,
              "network interface-properties ABI drift");
#else
_Static_assert(sizeof(RinNetdevLinkStatusV1) == 24u,
               "network link-status ABI drift");
_Static_assert(sizeof(RinNetdevInterfacePropertiesV1) == 28u,
               "network interface-properties ABI drift");
#endif

static inline int rin_netdev_interface_properties_v1_valid(
    const RinNetdevInterfacePropertiesV1* properties)
{
    uint8_t peer_nonzero = 0u;
    uint32_t index;
    if (properties == NULL ||
        properties->version != RIN_NETDEV_INTERFACE_PROPERTIES_VERSION ||
        properties->struct_size != sizeof(*properties) ||
        (properties->flags & ~RIN_NETDEV_INTERFACE_KNOWN_FLAGS) != 0u ||
        properties->interface_type > RIN_NETDEV_INTERFACE_TYPE_MAX ||
        (properties->capabilities &
         ~RIN_NETDEV_INTERFACE_KNOWN_CAPABILITIES) != 0u ||
        properties->reserved != 0u ||
        ((properties->flags & RIN_NETDEV_INTERFACE_FLAG_ADMIN_UP) != 0u &&
         (properties->flags & RIN_NETDEV_INTERFACE_FLAG_ADMIN_KNOWN) == 0u) ||
        (((properties->flags & RIN_NETDEV_INTERFACE_FLAG_TYPE_KNOWN) != 0u) !=
         (properties->interface_type != RIN_NETDEV_INTERFACE_TYPE_UNKNOWN)) ||
        ((properties->flags &
          RIN_NETDEV_INTERFACE_FLAG_CAPABILITIES_KNOWN) == 0u &&
         properties->capabilities != 0u) ||
        ((properties->flags &
          RIN_NETDEV_INTERFACE_FLAG_CAPABILITIES_KNOWN) != 0u &&
         properties->interface_type ==
             RIN_NETDEV_INTERFACE_TYPE_BROADCAST &&
         (properties->capabilities &
          RIN_NETDEV_INTERFACE_CAP_BROADCAST) == 0u) ||
        ((properties->flags & RIN_NETDEV_INTERFACE_FLAG_TYPE_KNOWN) != 0u &&
         properties->interface_type ==
             RIN_NETDEV_INTERFACE_TYPE_POINT_TO_POINT &&
         (properties->flags &
          RIN_NETDEV_INTERFACE_FLAG_CAPABILITIES_KNOWN) != 0u &&
         ((properties->capabilities &
           RIN_NETDEV_INTERFACE_CAP_POINT_TO_POINT) == 0u ||
          (properties->capabilities &
           RIN_NETDEV_INTERFACE_CAP_BROADCAST) != 0u)) ||
        ((properties->capabilities &
          RIN_NETDEV_INTERFACE_CAP_POINT_TO_POINT) != 0u &&
         (((properties->flags & RIN_NETDEV_INTERFACE_FLAG_TYPE_KNOWN) != 0u &&
           properties->interface_type !=
               RIN_NETDEV_INTERFACE_TYPE_POINT_TO_POINT) ||
          (properties->capabilities &
           RIN_NETDEV_INTERFACE_CAP_BROADCAST) != 0u)))
        return 0;
    for (index = 0u; index < sizeof(properties->ipv4_peer); ++index)
        peer_nonzero |= properties->ipv4_peer[index];
    return peer_nonzero == 0u ||
           (((properties->flags & RIN_NETDEV_INTERFACE_FLAG_TYPE_KNOWN) == 0u ||
             properties->interface_type ==
                 RIN_NETDEV_INTERFACE_TYPE_POINT_TO_POINT) &&
            ((properties->flags &
              RIN_NETDEV_INTERFACE_FLAG_CAPABILITIES_KNOWN) == 0u ||
             (properties->capabilities &
              RIN_NETDEV_INTERFACE_CAP_POINT_TO_POINT) != 0u));
}

#endif /* RIN_SDK_RIN_NETDEV_LINK_H */
