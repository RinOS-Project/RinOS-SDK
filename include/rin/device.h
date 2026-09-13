#ifndef RIN_SDK_DEVICE_H
#define RIN_SDK_DEVICE_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct RinDeviceInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t device_id;
    uint32_t device_class;
    uint32_t state;
    RinStringV1 name;
    uint64_t capabilities;
    uint64_t generation;
    uint64_t reserved[3];
} RinDeviceInfoV1;
RIN_SDK_API RinResult rin_device_enumerate_v1(uint32_t device_class, uint64_t index, RinDeviceInfoV1* info);
RIN_SDK_API RinResult rin_device_open_v1(uint64_t device_id, uint32_t rights, RinDevice* device);
RIN_SDK_API RinResult rin_device_control_v1(RinDevice device, uint32_t operation, RinSliceV1 input, RinSliceV1 output, uint64_t* transferred);
RIN_SDK_API RinResult rin_device_query_v1(RinDevice device, RinDeviceInfoV1* info);
#ifdef __cplusplus
}
#endif
#endif
