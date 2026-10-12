#ifndef RIN_SDK_DEVICE_H
#define RIN_SDK_DEVICE_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Values accepted by rin_device_enumerate_v1(). Keep these in lockstep with
 * the public RinOS device-class ABI; the SDK prefix avoids colliding with
 * kernel RinBus declarations in mixed kernel translation units. */
#define RIN_SDK_DEVICE_CLASS_ANY UINT32_C(0)
#define RIN_SDK_DEVICE_CLASS_STORAGE UINT32_C(1)
#define RIN_SDK_DEVICE_CLASS_NETWORK UINT32_C(2)
#define RIN_SDK_DEVICE_CLASS_AUDIO UINT32_C(3)
#define RIN_SDK_DEVICE_CLASS_GPU UINT32_C(4)
#define RIN_SDK_DEVICE_CLASS_INPUT UINT32_C(5)
#define RIN_SDK_DEVICE_CLASS_USB_CONTROLLER UINT32_C(6)
#define RIN_SDK_DEVICE_CLASS_BUS UINT32_C(7)
#define RIN_SDK_DEVICE_CLASS_POWER UINT32_C(8)

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
