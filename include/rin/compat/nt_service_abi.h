// SPDX-License-Identifier: MIT
/* Fixed-width little-endian wire contract for the RinNT user-space service. */
#ifndef RIN_SDK_COMPAT_NT_SERVICE_ABI_H
#define RIN_SDK_COMPAT_NT_SERVICE_ABI_H

#include <stdint.h>

#define RINNT_SERVICE_ABI_MAGIC UINT32_C(0x31534e52) /* "RNS1" */
#define RINNT_SERVICE_ABI_VERSION 1u

#define RINNT_SERVICE_MAX_MESSAGE_SIZE (64u * 1024u)
#define RINNT_SERVICE_MAX_PATH_SIZE 1024u
#define RINNT_SERVICE_MAX_COMMAND_LINE_SIZE 32768u
#define RINNT_SERVICE_MAX_REGISTRY_KEY_SIZE 512u
#define RINNT_SERVICE_MAX_REGISTRY_VALUE_NAME_SIZE 255u
#define RINNT_SERVICE_MAX_REGISTRY_VALUE_DATA_SIZE 4096u

typedef uint64_t RinNTServiceSessionHandle;

typedef enum RinNTServicePeerScopeV1 {
    RINNT_SERVICE_PEER_SCOPE_SYSTEM = 1,
    RINNT_SERVICE_PEER_SCOPE_USER = 2
} RinNTServicePeerScopeV1;

typedef enum RinNTServiceCommandV1 {
    RINNT_SERVICE_COMMAND_HELLO = 1,
    RINNT_SERVICE_COMMAND_CREATE_PROCESS = 2,
    RINNT_SERVICE_COMMAND_QUERY_PROCESS = 3,
    RINNT_SERVICE_COMMAND_LOAD_LIBRARY = 4,
    RINNT_SERVICE_COMMAND_FREE_LIBRARY = 5,
    RINNT_SERVICE_COMMAND_RETRY_CLEANUP = 6,
    RINNT_SERVICE_COMMAND_TERMINATE_PROCESS = 7,
    RINNT_SERVICE_COMMAND_QUERY_REGISTRY_VALUE = 8,
    RINNT_SERVICE_COMMAND_SET_REGISTRY_VALUE = 9,
    RINNT_SERVICE_COMMAND_DELETE_REGISTRY_VALUE = 10,
    RINNT_SERVICE_COMMAND_XINPUT_GET_STATE = 11,
    RINNT_SERVICE_COMMAND_XINPUT_SET_VIBRATION = 12,
    RINNT_SERVICE_COMMAND_RAW_INPUT_SET_FILTER = 13,
    RINNT_SERVICE_COMMAND_RAW_INPUT_READ = 14
} RinNTServiceCommandV1;

typedef enum RinNTServiceResult {
    RINNT_SERVICE_OK = 0,
    RINNT_SERVICE_ERROR_INVALID_ARGUMENT = -1200,
    RINNT_SERVICE_ERROR_PROTOCOL = -1201,
    RINNT_SERVICE_ERROR_VERSION = -1202,
    RINNT_SERVICE_ERROR_SIZE = -1203,
    RINNT_SERVICE_ERROR_REPLAY = -1204,
    RINNT_SERVICE_ERROR_ACCESS = -1205,
    RINNT_SERVICE_ERROR_CAPACITY = -1206,
    RINNT_SERVICE_ERROR_NOT_FOUND = -1207,
    RINNT_SERVICE_ERROR_STATE = -1208,
    RINNT_SERVICE_ERROR_PATH = -1209,
    RINNT_SERVICE_ERROR_PERSONALITY = -1210,
    RINNT_SERVICE_ERROR_PROFILE = -1211,
    RINNT_SERVICE_ERROR_BACKEND = -1212,
    RINNT_SERVICE_ERROR_BUSY = -1213,
    RINNT_SERVICE_ERROR_STALE = -1214,
    RINNT_SERVICE_ERROR_CORRUPT = -1215,
    RINNT_SERVICE_ERROR_NOT_SUPPORTED = -1216,
    RINNT_SERVICE_ERROR_COMMIT_UNKNOWN = -1217
} RinNTServiceResult;

typedef enum RinNTServiceProcessStateV1 {
    RINNT_SERVICE_PROCESS_ACTIVE = 1,
    RINNT_SERVICE_PROCESS_ORPHANED = 2
} RinNTServiceProcessStateV1;

#define RINNT_SERVICE_FEATURE_PE32 UINT64_C(1)
#define RINNT_SERVICE_FEATURE_PE32_PLUS UINT64_C(2)
#define RINNT_SERVICE_FEATURE_DYNAMIC_LIBRARY UINT64_C(4)
#define RINNT_SERVICE_FEATURE_FAIL_CLOSED_SESSIONS UINT64_C(8)
#define RINNT_SERVICE_FEATURE_REGISTRY_HIVE UINT64_C(16)
#define RINNT_SERVICE_FEATURE_INPUT_TRANSLATION UINT64_C(32)

#define RINNT_SERVICE_XINPUT_MAX_USERS 4u
#define RINNT_SERVICE_XINPUT_BUTTON_MASK UINT16_C(0xf3ff)

#define RINNT_SERVICE_RAW_INPUT_CLASS_KEYBOARD UINT32_C(1)
#define RINNT_SERVICE_RAW_INPUT_CLASS_MOUSE UINT32_C(2)
#define RINNT_SERVICE_RAW_INPUT_CLASS_HID UINT32_C(4)
#define RINNT_SERVICE_RAW_INPUT_CLASS_MASK UINT32_C(7)
#define RINNT_SERVICE_RAW_INPUT_HID_MAX_REPORT_BYTES UINT32_C(20)

typedef enum RinNTServiceRawInputEventTypeV1 {
    RINNT_SERVICE_RAW_INPUT_EVENT_KEY_DOWN = 1,
    RINNT_SERVICE_RAW_INPUT_EVENT_KEY_UP = 2,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_MOVE = 3,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_BUTTON = 4,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_WHEEL = 5,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_HORIZONTAL_WHEEL = 6,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_ABSOLUTE = 7,
    RINNT_SERVICE_RAW_INPUT_EVENT_HID_REPORT = 8
} RinNTServiceRawInputEventTypeV1;

#define RINNT_SERVICE_RAW_KEY_FLAG_E0 UINT32_C(1)
#define RINNT_SERVICE_RAW_KEY_FLAG_E1 UINT32_C(2)
#define RINNT_SERVICE_RAW_KEY_FLAG_REPEAT UINT32_C(4)
#define RINNT_SERVICE_RAW_KEY_FLAG_MASK UINT32_C(7)
#define RINNT_SERVICE_RAW_MOUSE_BUTTON_MASK UINT32_C(0x1f)

/* Requests never carry a PID, UID, target address, PEB/TEB pointer or VMA.
 * Those values belong to the authenticated transport and service backend. */
typedef struct RinNTServiceRequestHeaderV1 {
    uint32_t magic;
    uint16_t version;
    uint16_t command;
    uint32_t struct_size;
    uint32_t payload_size;
    uint32_t response_capacity;
    uint32_t flags;
    uint64_t request_id;
    RinNTServiceSessionHandle session;
    uint64_t reserved;
} RinNTServiceRequestHeaderV1;

typedef struct RinNTServiceResponseHeaderV1 {
    uint32_t magic;
    uint16_t version;
    uint16_t command;
    uint32_t struct_size;
    uint32_t payload_size;
    int32_t status;
    uint32_t flags;
    uint64_t request_id;
    RinNTServiceSessionHandle session;
} RinNTServiceResponseHeaderV1;

typedef struct RinNTServiceHelloResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t features;
    uint32_t maximum_message_size;
    uint32_t maximum_path_size;
} RinNTServiceHelloResponseV1;

/* Strings are concatenated directly after this structure in image, command,
 * cwd order. Offsets are relative to the start of the payload. They are UTF-8
 * byte strings without a trailing NUL. */
typedef struct RinNTServiceCreateProcessRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t personality;
    uint32_t windows_profile;
    uint32_t flags;
    uint32_t reserved;
    uint32_t image_offset;
    uint32_t image_length;
    uint32_t command_offset;
    uint32_t command_length;
    uint32_t cwd_offset;
    uint32_t cwd_length;
} RinNTServiceCreateProcessRequestV1;

typedef struct RinNTServiceCreateProcessResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t process_id;
    uint32_t personality;
    uint32_t windows_profile;
} RinNTServiceCreateProcessResponseV1;

typedef struct RinNTServiceQueryProcessResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t process_id;
    uint32_t personality;
    uint32_t windows_profile;
    uint32_t state;
    uint32_t reserved;
} RinNTServiceQueryProcessResponseV1;

typedef struct RinNTServiceLoadLibraryRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t path_offset;
    uint32_t path_length;
    uint64_t owner_thread_id;
    uint64_t reserved;
} RinNTServiceLoadLibraryRequestV1;

typedef struct RinNTServiceModuleResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t module_handle;
} RinNTServiceModuleResponseV1;

typedef struct RinNTServiceFreeLibraryRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t module_handle;
    uint64_t owner_thread_id;
} RinNTServiceFreeLibraryRequestV1;

/* Registry requests are sessionless. key/name/data byte views are packed in
 * that order immediately after this structure. Hive ownership is never a
 * payload field: HKCU always means the authenticated peer UID and HKLM writes
 * require the peer's captured administrator capability. QUERY requires zero
 * expected_generation/type/data; DELETE requires type NONE and zero data. */
typedef struct RinNTServiceRegistryRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t flags;
    uint64_t expected_generation;
    uint32_t key_offset;
    uint32_t key_length;
    uint32_t name_offset;
    uint32_t name_length;
    uint32_t value_type;
    uint32_t data_offset;
    uint32_t data_length;
    uint32_t reserved;
    uint64_t reserved2;
} RinNTServiceRegistryRequestV1;

/* Value bytes follow this fixed header at data_offset. */
typedef struct RinNTServiceRegistryValueResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t owner_uid;
    uint32_t value_type;
    uint32_t data_offset;
    uint32_t data_length;
    uint32_t flags;
    uint64_t hive_generation;
    uint64_t reserved;
} RinNTServiceRegistryValueResponseV1;

typedef struct RinNTServiceRegistryMutationResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t owner_uid;
    uint64_t hive_generation;
    uint64_t reserved;
} RinNTServiceRegistryMutationResponseV1;

/* Input translation is process-session bound. No native pointer, provider
 * cookie or device object crosses this ABI. XInput user slots keep the public
 * 0..3 contract; Raw Input filters and devices are opaque service handles. */
typedef struct RinNTServiceXInputGetStateRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t user_index;
    uint32_t reserved;
} RinNTServiceXInputGetStateRequestV1;

typedef struct RinNTServiceXInputStateResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t packet_number;
    uint16_t buttons;
    uint8_t left_trigger;
    uint8_t right_trigger;
    int16_t thumb_lx;
    int16_t thumb_ly;
    int16_t thumb_rx;
    int16_t thumb_ry;
    uint32_t flags;
    uint32_t reserved;
    uint64_t reserved2;
} RinNTServiceXInputStateResponseV1;

typedef struct RinNTServiceXInputSetVibrationRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t user_index;
    uint32_t flags;
    uint16_t left_motor_speed;
    uint16_t right_motor_speed;
    uint32_t reserved;
} RinNTServiceXInputSetVibrationRequestV1;

/* previous_filter_handle is zero for create. A nonzero class mask replaces
 * the exact current filter; a zero mask deletes it. Both are CAS operations. */
typedef struct RinNTServiceRawInputFilterRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t class_mask;
    uint32_t flags;
    uint64_t previous_filter_handle;
    uint64_t reserved;
} RinNTServiceRawInputFilterRequestV1;

typedef struct RinNTServiceRawInputFilterResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t filter_handle;
    uint64_t reserved;
} RinNTServiceRawInputFilterResponseV1;

typedef struct RinNTServiceRawInputReadRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t filter_handle;
    uint64_t after_sequence;
    uint64_t reserved;
} RinNTServiceRawInputReadRequestV1;

/* data interpretation by event_type:
 * KEY: scan code, virtual key, repeat count, then zeroes.
 * MOVE: relative x/y, then zeroes.
 * BUTTON: current mask, changed mask, then zeroes.
 * WHEEL/HORIZONTAL_WHEEL: signed delta, then zeroes.
 * ABSOLUTE: x, y, minimum x, maximum x, minimum y, maximum y.
 * HID_REPORT: byte length 1..20, then five little-endian packed report
 * words.  Bytes after the declared length must be zero. */
typedef struct RinNTServiceRawInputEventResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t event_type;
    uint32_t flags;
    uint64_t sequence;
    uint64_t timestamp_ns;
    uint64_t device_handle;
    uint64_t device_generation;
    int32_t data[6];
    uint64_t reserved;
} RinNTServiceRawInputEventResponseV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNTServiceRequestHeaderV1) == 48u,
              "RinNT service request ABI drift");
static_assert(sizeof(RinNTServiceResponseHeaderV1) == 40u,
              "RinNT service response ABI drift");
static_assert(sizeof(RinNTServiceCreateProcessRequestV1) == 48u,
              "RinNT service create ABI drift");
static_assert(sizeof(RinNTServiceLoadLibraryRequestV1) == 32u,
              "RinNT service load ABI drift");
static_assert(sizeof(RinNTServiceRegistryRequestV1) == 64u,
              "RinNT service registry request ABI drift");
static_assert(sizeof(RinNTServiceRegistryValueResponseV1) == 48u,
              "RinNT service registry value ABI drift");
static_assert(sizeof(RinNTServiceRegistryMutationResponseV1) == 32u,
              "RinNT service registry mutation ABI drift");
static_assert(sizeof(RinNTServiceXInputGetStateRequestV1) == 16u,
              "RinNT service XInput query ABI drift");
static_assert(sizeof(RinNTServiceXInputStateResponseV1) == 40u,
              "RinNT service XInput state ABI drift");
static_assert(sizeof(RinNTServiceXInputSetVibrationRequestV1) == 24u,
              "RinNT service XInput vibration ABI drift");
static_assert(sizeof(RinNTServiceRawInputFilterRequestV1) == 32u,
              "RinNT service raw filter request ABI drift");
static_assert(sizeof(RinNTServiceRawInputFilterResponseV1) == 24u,
              "RinNT service raw filter response ABI drift");
static_assert(sizeof(RinNTServiceRawInputReadRequestV1) == 32u,
              "RinNT service raw read request ABI drift");
static_assert(sizeof(RinNTServiceRawInputEventResponseV1) == 80u,
              "RinNT service raw event ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinNTServiceRequestHeaderV1) == 48u,
               "RinNT service request ABI drift");
_Static_assert(sizeof(RinNTServiceResponseHeaderV1) == 40u,
               "RinNT service response ABI drift");
_Static_assert(sizeof(RinNTServiceCreateProcessRequestV1) == 48u,
               "RinNT service create ABI drift");
_Static_assert(sizeof(RinNTServiceLoadLibraryRequestV1) == 32u,
               "RinNT service load ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryRequestV1) == 64u,
               "RinNT service registry request ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryValueResponseV1) == 48u,
               "RinNT service registry value ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryMutationResponseV1) == 32u,
               "RinNT service registry mutation ABI drift");
_Static_assert(sizeof(RinNTServiceXInputGetStateRequestV1) == 16u,
               "RinNT service XInput query ABI drift");
_Static_assert(sizeof(RinNTServiceXInputStateResponseV1) == 40u,
               "RinNT service XInput state ABI drift");
_Static_assert(sizeof(RinNTServiceXInputSetVibrationRequestV1) == 24u,
               "RinNT service XInput vibration ABI drift");
_Static_assert(sizeof(RinNTServiceRawInputFilterRequestV1) == 32u,
               "RinNT service raw filter request ABI drift");
_Static_assert(sizeof(RinNTServiceRawInputFilterResponseV1) == 24u,
               "RinNT service raw filter response ABI drift");
_Static_assert(sizeof(RinNTServiceRawInputReadRequestV1) == 32u,
               "RinNT service raw read request ABI drift");
_Static_assert(sizeof(RinNTServiceRawInputEventResponseV1) == 80u,
               "RinNT service raw event ABI drift");
#endif

#endif /* RIN_SDK_COMPAT_NT_SERVICE_ABI_H */
