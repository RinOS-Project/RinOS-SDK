/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_CCID_H
#define RIN_SDK_CCID_H

#include "rin/device.h"
#include "rin/ccid_abi.h"

#ifdef __cplusplus
extern "C" {
#endif

/* input is RinCcidControlRequestV1 followed by request bytes; output is
 * RinCcidControlResponseV1 followed by response bytes. transferred_out
 * reports the response payload length, excluding the response header. */
RIN_SDK_API RinResult rin_ccid_control_v1(
    RinDevice device, RinSliceV1 input, RinSliceV1 output,
    uint64_t* transferred_out);
RIN_SDK_API RinResult rin_ccid_device_query_v1(
    RinDevice device, RinCcidDeviceInfoV1* info);

#ifdef __cplusplus
}
#endif
#endif /* RIN_SDK_CCID_H */
