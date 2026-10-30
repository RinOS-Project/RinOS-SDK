/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_PKG_H
#define RIN_SDK_PKG_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif

#define RIN_PACKAGE_INSTALL_FLAG_RESTORE_ARCHIVED_DATA UINT32_C(0x00000001)
#define RIN_PACKAGE_INSTALL_FLAG_DISCARD_ARCHIVED_DATA UINT32_C(0x00000002)
#define RIN_PACKAGE_INSTALL_FLAG_MASK UINT32_C(0x00000003)
#define RIN_PACKAGE_SOURCE_REPOSITORY_PREFIX "repo:"
#define RIN_PACKAGE_REMOVE_FLAG_PRESERVE_USER_DATA UINT32_C(0x00000001)
#define RIN_PACKAGE_REMOVE_FLAG_MASK RIN_PACKAGE_REMOVE_FLAG_PRESERVE_USER_DATA
#define RIN_PACKAGE_INFO_NAME_CAPACITY 64u
#define RIN_PACKAGE_INFO_VERSION_CAPACITY 32u

typedef struct RinPackageInfoV1 {
    uint32_t struct_size;
    uint32_t version;
    char name[RIN_PACKAGE_INFO_NAME_CAPACITY];
    char package_version[RIN_PACKAGE_INFO_VERSION_CAPACITY];
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
/* Authenticates the exact immutable bytes retained by the caller against an
 * installed package file identity. Only the wincompatregd system service may
 * invoke this broker operation. */
RIN_SDK_API RinResult rin_package_verify_installed_file_content_v1(
    RinStringV1 canonical_path, const void* content, uint64_t content_size);
#ifdef __cplusplus
}
#endif
#endif
