#ifndef RIN_SDK_ABI_H
#define RIN_SDK_ABI_H

#include <stddef.h>
#include <stdint.h>

/* Keep the public umbrella source-compatible while the individual contracts
 * remain independently includable. */
#include "formats_v3.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_SDK_ABI_VERSION UINT32_C(0x00010009)
#define RIN_SDK_STRUCT_VERSION_1 UINT32_C(1)
#define RIN_SDK_INFINITE UINT64_MAX

#define RIN_ABI_MAJOR 1u
#define RIN_ABI_MINOR 0u
#define RIN_CREDENTIALS_VERSION_1 1u
#define RIN_CREDENTIALS_MAX_GROUPS 8u
#define RIN_PROCESS_CWD_MAX 256u

#if defined(_WIN32) && defined(RIN_SDK_BUILD_SHARED)
#define RIN_SDK_API __declspec(dllexport)
#elif defined(_WIN32) && defined(RIN_SDK_USE_SHARED)
#define RIN_SDK_API __declspec(dllimport)
#elif defined(_WIN32)
#define RIN_SDK_API
#else
#define RIN_SDK_API __attribute__((visibility("default")))
#endif

#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
typedef int32_t RinResult;
#endif
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

#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
typedef uint64_t RinHandle;
#endif
typedef uint64_t RinObjectHandle;
#define RIN_OBJECT_HANDLE_INVALID UINT64_C(0)

/* Rights are attached to handle-table entries. Duplicating a handle may only
 * narrow its rights; the encoded object value carries no authority. */
typedef uint32_t RinObjectRights;
#define RIN_OBJECT_RIGHT_READ      (UINT32_C(1) << 0)
#define RIN_OBJECT_RIGHT_WRITE     (UINT32_C(1) << 1)
#define RIN_OBJECT_RIGHT_EXECUTE   (UINT32_C(1) << 2)
#define RIN_OBJECT_RIGHT_DUPLICATE (UINT32_C(1) << 3)
#define RIN_OBJECT_RIGHT_WAIT      (UINT32_C(1) << 4)
#define RIN_OBJECT_RIGHT_SIGNAL    (UINT32_C(1) << 5)
#define RIN_OBJECT_RIGHT_CONTROL   (UINT32_C(1) << 6)
#define RIN_OBJECT_RIGHT_QUERY     (UINT32_C(1) << 7)
#define RIN_OBJECT_RIGHT_ALL       (RIN_OBJECT_RIGHT_READ | \
                                    RIN_OBJECT_RIGHT_WRITE | \
                                    RIN_OBJECT_RIGHT_EXECUTE | \
                                    RIN_OBJECT_RIGHT_DUPLICATE | \
                                    RIN_OBJECT_RIGHT_WAIT | \
                                    RIN_OBJECT_RIGHT_SIGNAL | \
                                    RIN_OBJECT_RIGHT_CONTROL | \
                                    RIN_OBJECT_RIGHT_QUERY)

typedef enum RinObjectType {
    RIN_OBJECT_NONE = 0,
    RIN_OBJECT_FILE = 1,
    RIN_OBJECT_PROCESS = 2,
    RIN_OBJECT_THREAD = 3,
    RIN_OBJECT_WINDOW = 4,
    RIN_OBJECT_SOCKET = 5,
    RIN_OBJECT_GPU = 6,
    RIN_OBJECT_SERVICE = 7,
    RIN_OBJECT_EVENT = 8,
    RIN_OBJECT_MUTEX = 9,
    RIN_OBJECT_SECTION = 10,
    RIN_OBJECT_LIBRARY = 11,
    RIN_OBJECT_INSTALL_SOURCE = 12,
    RIN_OBJECT_INSTALL_TARGET = 13
} RinObjectType;

typedef enum RinProcessPersonality {
    RIN_NATIVE = 0,
    RIN_WIN32 = 1,
    RIN_WIN64 = 2,
    RIN_WIN_DRIVER_HOST = 3
} RinProcessPersonality;

typedef uint64_t RinCapabilitySet;
#define RIN_CAP_DAC_OVERRIDE     (UINT64_C(1) << 0)
#define RIN_CAP_CHOWN            (UINT64_C(1) << 1)
#define RIN_CAP_SYSTEM_ADMIN     (UINT64_C(1) << 2)
#define RIN_CAP_DRIVER_BROKER    (UINT64_C(1) << 3)
#define RIN_CAP_SERVICE_CONTROL  (UINT64_C(1) << 4)
#define RIN_CAP_GUI_CONTROL      (UINT64_C(1) << 5)
#define RIN_CAP_PROCESS_CONTROL  (UINT64_C(1) << 6)
#define RIN_CAP_NETWORK_ADMIN    (UINT64_C(1) << 7)
#define RIN_CAP_CRASH_DIAGNOSTIC (UINT64_C(1) << 8)
#define RIN_CAP_STORAGE_MANAGE   (UINT64_C(1) << 9)
#define RIN_CAP_PACKAGE_MANAGE   (UINT64_C(1) << 10)
#define RIN_CAP_RIN_PASS         (UINT64_C(1) << 11)
#define RIN_CAP_KEYRING_MASTER   (UINT64_C(1) << 12)
#define RIN_CAP_POWER_CONTROL    (UINT64_C(1) << 13)
#define RIN_CAP_SYSTEM_INFO      (UINT64_C(1) << 14)
#define RIN_CAP_NETWORK_ACCESS   (UINT64_C(1) << 15)
#define RIN_CAP_CLIPBOARD        (UINT64_C(1) << 16)
#define RIN_CAP_NOTIFICATION     (UINT64_C(1) << 17)
#define RIN_CAP_GPU_ACCESS       (UINT64_C(1) << 18)
#define RIN_CAP_MICROPHONE      (UINT64_C(1) << 19)
#define RIN_CAP_CAMERA           (UINT64_C(1) << 20)
#define RIN_CAP_MIDI             (UINT64_C(1) << 21)
#define RIN_CAP_LOCATION         (UINT64_C(1) << 22)
#define RIN_CAP_FILE_PORTAL      (UINT64_C(1) << 23)
#define RIN_CAP_THEME_CONTROL    (UINT64_C(1) << 24)
#define RIN_CAP_ACCESSIBILITY    (UINT64_C(1) << 25)
#define RIN_CAP_DISPLAY          (UINT64_C(1) << 26)
#define RIN_CAP_DESKTOP          (UINT64_C(1) << 27)
#define RIN_CAP_AUDIO_OUTPUT     (UINT64_C(1) << 28)
#define RIN_CAP_APP_RESOURCES    (RIN_CAP_NETWORK_ACCESS | \
                                  RIN_CAP_CLIPBOARD | RIN_CAP_NOTIFICATION | \
                                  RIN_CAP_GPU_ACCESS | RIN_CAP_MICROPHONE | \
                                  RIN_CAP_CAMERA | RIN_CAP_MIDI | \
                                  RIN_CAP_LOCATION)
#define RIN_CAP_ALL              (RIN_CAP_DAC_OVERRIDE | RIN_CAP_CHOWN | \
                                  RIN_CAP_SYSTEM_ADMIN | RIN_CAP_DRIVER_BROKER | \
                                  RIN_CAP_SERVICE_CONTROL | RIN_CAP_GUI_CONTROL | \
                                  RIN_CAP_PROCESS_CONTROL | RIN_CAP_NETWORK_ADMIN | \
                                  RIN_CAP_CRASH_DIAGNOSTIC | \
                                  RIN_CAP_STORAGE_MANAGE | RIN_CAP_PACKAGE_MANAGE | \
                                  RIN_CAP_RIN_PASS | RIN_CAP_KEYRING_MASTER | \
                                  RIN_CAP_POWER_CONTROL | RIN_CAP_SYSTEM_INFO | \
                                  RIN_CAP_APP_RESOURCES | RIN_CAP_FILE_PORTAL | \
                                  RIN_CAP_THEME_CONTROL | RIN_CAP_ACCESSIBILITY | \
                                  RIN_CAP_DISPLAY | RIN_CAP_DESKTOP | \
                                  RIN_CAP_AUDIO_OUTPUT)

#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
typedef uint64_t RinObject;
#endif
#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
typedef uint64_t RinProcess;
#endif
#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
typedef uint64_t RinThread;
#endif
typedef uint64_t RinChannel;
typedef uint64_t RinEvent;
typedef uint64_t RinWaitSet;
typedef uint64_t RinSharedMemory;
typedef uint64_t RinService;
#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
typedef uint64_t RinFile;
#endif
typedef uint64_t RinDirectory;
typedef uint64_t RinFileWatch;
typedef uint64_t RinIoRequest;
#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
typedef uint64_t RinSocket;
#endif
typedef uint64_t RinTlsSession;
typedef uint64_t RinWindow;
typedef uint64_t RinSurface;
typedef uint64_t RinMediaObject;
typedef uint64_t RinConfigStore;
typedef uint64_t RinPackage;
typedef uint64_t RinDevice;

#define RIN_HANDLE_INVALID UINT64_C(0)

typedef struct RinCredentialsV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t uid;
    uint32_t gid;
    uint32_t effective_uid;
    uint32_t effective_gid;
    uint32_t groups[RIN_CREDENTIALS_MAX_GROUPS];
    uint32_t group_count;
    uint32_t umask;
    RinCapabilitySet capabilities;
    uint32_t personality;
    uint32_t flags;
    uint64_t reserved[4];
} RinCredentialsV1;

#if defined(__cplusplus)
static_assert(sizeof(RinCredentialsV1) == 112u,
              "RinCredentialsV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinCredentialsV1) == 112u,
               "RinCredentialsV1 ABI drift");
#endif

#define RIN_SESSION_IDENTITY_VERSION_1 1u
#ifndef RIN_SESSION_IDENTITY_V1_DEFINED
#define RIN_SESSION_IDENTITY_V1_DEFINED 1
typedef struct RinSessionIdentityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t session_id;
    uint32_t uid;
    uint64_t instance_cookie;
    uint64_t reserved[2];
} RinSessionIdentityV1;
#endif

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

#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
typedef RinResult (*RinSdkInvokeV1)(
    uint64_t context, uint32_t library_id, uint32_t operation,
    const void* request, uint32_t request_size,
    void* response, uint32_t response_size);

/* Fixed-width argument carrier used by SDK operations whose request is only
 * a handle and scalar values.  Keeping this in the public ABI lets an
 * OS-owned backend validate the exact request shape instead of depending on
 * the private SDK implementation's local type. */
typedef struct RinSdkArgsV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t value[6];
} RinSdkArgsV1;

#if defined(__cplusplus)
static_assert(sizeof(RinSdkArgsV1) == 56u, "SDK args ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinSdkArgsV1) == 56u, "SDK args ABI drift");
#endif

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
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinStringV1) == 16, "RinStringV1 ABI drift");
static_assert(sizeof(RinSliceV1) == 16, "RinSliceV1 ABI drift");
#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
static_assert(sizeof(RinSdkBackendV1) == 64, "RinSdkBackendV1 ABI drift");
#endif
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinStringV1) == 16, "RinStringV1 ABI drift");
_Static_assert(sizeof(RinSliceV1) == 16, "RinSliceV1 ABI drift");
#if !defined(RIN_SDK_KERNEL_INTERNAL_ABI)
_Static_assert(sizeof(RinSdkBackendV1) == 64, "RinSdkBackendV1 ABI drift");
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif
