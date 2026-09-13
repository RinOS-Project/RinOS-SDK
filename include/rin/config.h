#ifndef RIN_SDK_CONFIG_H
#define RIN_SDK_CONFIG_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif
RIN_SDK_API RinResult rin_config_open_v1(RinStringV1 domain, uint32_t flags, RinConfigStore* store);
RIN_SDK_API RinResult rin_config_get_v1(RinConfigStore store, RinStringV1 key, RinSliceV1 value, uint64_t* required);
RIN_SDK_API RinResult rin_config_set_v1(RinConfigStore store, RinStringV1 key, RinSliceV1 value, uint32_t flags);
RIN_SDK_API RinResult rin_config_commit_v1(RinConfigStore store);
RIN_SDK_API RinResult rin_config_remove_v1(RinConfigStore store, RinStringV1 key, uint32_t flags);
#ifdef __cplusplus
}
#endif
#endif
