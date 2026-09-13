#ifndef RIN_SDK_PKG_H
#define RIN_SDK_PKG_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct RinPackageInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    RinStringV1 name;
    RinStringV1 package_version;
    uint8_t identity_hash[32];
    uint32_t state;
    uint32_t flags;
    uint64_t reserved[3];
} RinPackageInfoV1;
RIN_SDK_API RinResult rin_package_open_v1(RinStringV1 name, RinPackage* package);
RIN_SDK_API RinResult rin_package_query_v1(RinPackage package, RinPackageInfoV1* info);
RIN_SDK_API RinResult rin_package_install_v1(RinStringV1 source, uint32_t flags, RinEvent completion);
RIN_SDK_API RinResult rin_package_remove_v1(RinStringV1 name, uint32_t flags, RinEvent completion);
RIN_SDK_API RinResult rin_package_enumerate_v1(uint64_t index, RinPackageInfoV1* info);
#ifdef __cplusplus
}
#endif
#endif
