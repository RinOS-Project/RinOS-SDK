/* SPDX-License-Identifier: MIT */
/* Transitional source aliases. New code must use the resource or handle
 * namespace directly. */
#ifndef RIN_SDK_LEGACY_DEVICE_RIGHTS_H
#define RIN_SDK_LEGACY_DEVICE_RIGHTS_H

#include "device.h"
#include "resource/capability.h"

/* Historical bit values are unchanged. The unqualified ALL alias describes
 * resource grants; MASK describes generic device handles. */
#define RIN_DEVICE_RIGHT_READ RIN_RESOURCE_RIGHT_READ
#define RIN_DEVICE_RIGHT_WRITE RIN_RESOURCE_RIGHT_WRITE
#define RIN_DEVICE_RIGHT_CONTROL RIN_RESOURCE_RIGHT_CONTROL
#define RIN_DEVICE_RIGHT_MAP RIN_RESOURCE_RIGHT_MAP
#define RIN_DEVICE_RIGHT_QUERY RIN_DEVICE_HANDLE_RIGHT_QUERY
#define RIN_DEVICE_RIGHT_DUPLICATE RIN_DEVICE_HANDLE_RIGHT_DUPLICATE
#define RIN_DEVICE_RIGHT_ALL RIN_RESOURCE_RIGHT_ALL
#define RIN_DEVICE_RIGHT_MASK RIN_DEVICE_HANDLE_RIGHT_MASK

#endif
