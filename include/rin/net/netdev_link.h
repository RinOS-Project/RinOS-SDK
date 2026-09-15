/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_RIN_NETDEV_LINK_H
#define RIN_SDK_RIN_NETDEV_LINK_H

#include <stdint.h>

/* The legacy NetDevOps record has a fixed seven-pointer ABI.  Link state is
 * consequently queried through its pre-existing ioctl slot instead of
 * appending a callback that older signed drivers do not own. */
#define RIN_NETDEV_LINK_STATUS_VERSION UINT32_C(1)
#define RIN_NETDEV_IOCTL_LINK_STATUS UINT32_C(0x524e4c01)
#define RIN_NETDEV_IOCTL_WIFI_INFO UINT32_C(0x524e4c02)

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

#if defined(__cplusplus)
static_assert(sizeof(RinNetdevLinkStatusV1) == 24u,
              "network link-status ABI drift");
#else
_Static_assert(sizeof(RinNetdevLinkStatusV1) == 24u,
               "network link-status ABI drift");
#endif

#endif /* RIN_SDK_RIN_NETDEV_LINK_H */
