/* SPDX-License-Identifier: MIT */

#ifndef RIN_DOTNET_NATIVEAOT_THUNK_ABI_H
#define RIN_DOTNET_NATIVEAOT_THUNK_ABI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_NATIVEAOT_THUNK_ABI_VERSION UINT16_C(1)

#define RIN_NATIVEAOT_THUNK_CAP_EXECUTABLE_MAPPING UINT32_C(0x00000001)
#define RIN_NATIVEAOT_THUNK_CAP_COMMON_STUB UINT32_C(0x00000002)
#define RIN_NATIVEAOT_THUNK_KNOWN_CAPABILITIES \
    (RIN_NATIVEAOT_THUNK_CAP_EXECUTABLE_MAPPING | \
     RIN_NATIVEAOT_THUNK_CAP_COMMON_STUB)

#define RIN_NATIVEAOT_THUNK_S_OK INT32_C(0)
#define RIN_NATIVEAOT_THUNK_E_FAIL ((int32_t)UINT32_C(0x80004005))
#define RIN_NATIVEAOT_THUNK_E_INVALIDARG ((int32_t)UINT32_C(0x80070057))
#define RIN_NATIVEAOT_THUNK_E_NOTIMPL ((int32_t)UINT32_C(0x80004001))
#define RIN_NATIVEAOT_THUNK_E_OUTOFMEMORY ((int32_t)UINT32_C(0x8007000e))

/*
 * Product-owned NativeAOT thunk services. The callback table is intentionally
 * opaque to PortableRuntime: the owner supplies executable mapping policy,
 * thunk layout, and the interop common stub for one target ABI.
 *
 * A provider must advertise every callback and both capabilities before the
 * runtime will dispatch to it. A missing or malformed provider is not a
 * partial implementation; PortableRuntime remains fail-closed.
 */
typedef struct RinNativeAotThunkProviderV1 {
    uint32_t struct_size;
    uint16_t abi_version;
    uint16_t reserved0;
    uint32_t capability_mask;
    void* context;

    int32_t (*allocate_thunks_mapping)(void* context, void** mapping_out);
    void* (*get_thunks_base)(void* context);
    int32_t (*get_num_thunk_blocks_per_mapping)(void* context);
    int32_t (*get_num_thunks_per_block)(void* context);
    int32_t (*get_thunk_size)(void* context);
    void* (*get_thunk_data_block_address)(void* context, void* thunk_stub);
    void* (*get_thunk_stubs_block_address)(void* context, void* thunk_data);
    int32_t (*get_thunk_block_size)(void* context);
    void* (*get_common_stub_address)(void* context);
    void* (*get_current_thunk_context)(void* context);
} RinNativeAotThunkProviderV1;

const RinNativeAotThunkProviderV1* rin_nativeaot_thunk_provider_get_v1(void);

#ifdef __cplusplus
}

static_assert(sizeof(RinNativeAotThunkProviderV1) >= 80,
              "RinNativeAotThunkProviderV1 unexpectedly shrank");
#endif

#endif
