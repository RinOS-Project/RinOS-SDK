#ifndef RIN_SDK_ABI_H
#define RIN_SDK_ABI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_SDK_ABI_VERSION UINT32_C(0x00010009)
#define RIN_SDK_STRUCT_VERSION_1 UINT32_C(1)
#define RIN_SDK_INFINITE UINT64_MAX

#if defined(_WIN32) && defined(RIN_SDK_BUILD_SHARED)
#define RIN_SDK_API __declspec(dllexport)
#elif defined(_WIN32) && defined(RIN_SDK_USE_SHARED)
#define RIN_SDK_API __declspec(dllimport)
#elif defined(_WIN32)
#define RIN_SDK_API
#else
#define RIN_SDK_API __attribute__((visibility("default")))
#endif

typedef int32_t RinResult;
enum {
    RIN_SUCCESS = 0,
    RIN_ERROR_INVALID_ARGUMENT = -1,
    RIN_ERROR_NOT_SUPPORTED = -2,
    RIN_ERROR_NOT_FOUND = -3,
    RIN_ERROR_ACCESS_DENIED = -4,
    RIN_ERROR_NO_MEMORY = -5,
    RIN_ERROR_BUSY = -6,
    RIN_ERROR_TIMEOUT = -7,
    RIN_ERROR_CANCELLED = -8,
    RIN_ERROR_IO = -9,
    RIN_ERROR_WOULD_BLOCK = -10,
    RIN_ERROR_ABI_MISMATCH = -11,
    RIN_ERROR_STALE_HANDLE = -12,
    RIN_ERROR_INTEGRITY = -13
};

typedef uint64_t RinHandle;
typedef uint64_t RinObject;
typedef uint64_t RinProcess;
typedef uint64_t RinThread;
typedef uint64_t RinChannel;
typedef uint64_t RinEvent;
typedef uint64_t RinWaitSet;
typedef uint64_t RinSharedMemory;
typedef uint64_t RinService;
typedef uint64_t RinFile;
typedef uint64_t RinDirectory;
typedef uint64_t RinFileWatch;
typedef uint64_t RinIoRequest;
typedef uint64_t RinSocket;
typedef uint64_t RinTlsSession;
typedef uint64_t RinWindow;
typedef uint64_t RinSurface;
typedef uint64_t RinMediaObject;
typedef uint64_t RinConfigStore;
typedef uint64_t RinPackage;
typedef uint64_t RinDevice;

#define RIN_HANDLE_INVALID UINT64_C(0)

typedef struct RinStringV1 {
    uint64_t address;
    uint64_t size;
} RinStringV1;

typedef struct RinSliceV1 {
    uint64_t address;
    uint64_t size;
} RinSliceV1;

typedef struct RinVersionedV1 {
    uint32_t struct_size;
    uint32_t version;
} RinVersionedV1;

typedef RinResult (*RinSdkInvokeV1)(
    uint64_t context, uint32_t library_id, uint32_t operation,
    const void* request, uint32_t request_size,
    void* response, uint32_t response_size);

typedef struct RinSdkBackendV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t context;
    /* Fixed-width code address. Its high 32 bits must be zero on i686. */
    uint64_t invoke;
    uint64_t reserved[5];
} RinSdkBackendV1;

enum {
    RIN_SDK_LIBRARY_BASE = 1,
    RIN_SDK_LIBRARY_IPC = 2,
    RIN_SDK_LIBRARY_FS = 3,
    RIN_SDK_LIBRARY_NET = 4,
    RIN_SDK_LIBRARY_GUI = 5,
    RIN_SDK_LIBRARY_MEDIA = 6,
    RIN_SDK_LIBRARY_CONFIG = 7,
    RIN_SDK_LIBRARY_PKG = 8,
    RIN_SDK_LIBRARY_DEVICE = 9
};

RIN_SDK_API RinResult rin_sdk_bind_backend_v1(const RinSdkBackendV1* backend);
RIN_SDK_API RinResult rin_sdk_invoke_v1(
    uint32_t library_id, uint32_t operation,
    const void* request, uint32_t request_size,
    void* response, uint32_t response_size);

#if defined(__cplusplus)
static_assert(sizeof(RinStringV1) == 16, "RinStringV1 ABI drift");
static_assert(sizeof(RinSliceV1) == 16, "RinSliceV1 ABI drift");
static_assert(sizeof(RinSdkBackendV1) == 64, "RinSdkBackendV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinStringV1) == 16, "RinStringV1 ABI drift");
_Static_assert(sizeof(RinSliceV1) == 16, "RinSliceV1 ABI drift");
_Static_assert(sizeof(RinSdkBackendV1) == 64, "RinSdkBackendV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif
#endif
