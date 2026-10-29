/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_CONFIG_H
#define RIN_SDK_CONFIG_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif

#define RIN_CONFIG_OPEN_READ_ONLY UINT32_C(0x00000001)
#define RIN_CONFIG_OPEN_FLAG_MASK RIN_CONFIG_OPEN_READ_ONLY
#define RIN_CONFIG_SET_FLAG_MASK UINT32_C(0)
#define RIN_CONFIG_REMOVE_FLAG_MASK UINT32_C(0)

RIN_SDK_API RinResult rin_config_open_v1(RinStringV1 domain, uint32_t flags, RinConfigStore* store);
RIN_SDK_API RinResult rin_config_get_v1(RinConfigStore store, RinStringV1 key, RinSliceV1 value, uint64_t* required);
RIN_SDK_API RinResult rin_config_set_v1(RinConfigStore store, RinStringV1 key, RinSliceV1 value, uint32_t flags);
RIN_SDK_API RinResult rin_config_commit_v1(RinConfigStore store);
RIN_SDK_API RinResult rin_config_remove_v1(RinConfigStore store, RinStringV1 key, uint32_t flags);
#ifdef __cplusplus
}
#endif
#endif
