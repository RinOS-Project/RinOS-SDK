// SPDX-License-Identifier: MIT
/* Fixed-width little-endian wire contract for the RinNT user-space service. */
#ifndef RIN_SDK_COMPAT_NT_SERVICE_ABI_H
#define RIN_SDK_COMPAT_NT_SERVICE_ABI_H

#include <stdint.h>

#define RINNT_SERVICE_ABI_MAGIC UINT32_C(0x31534e52) /* "RNS1" */
#define RINNT_SERVICE_ABI_VERSION 1u

/* Canonical RinNT service endpoint. Its path is routing information only;
 * connection identity is supplied by the authenticated local transport. */
#define RINNT_SERVICE_SOCKET_PATH "/run/rin/rinnt.sock"

#define RINNT_SERVICE_MAX_MESSAGE_SIZE (64u * 1024u)
#define RINNT_SERVICE_MAX_PATH_SIZE 1024u
#define RINNT_SERVICE_MAX_COMMAND_LINE_SIZE 32768u
#define RINNT_SERVICE_MAX_REGISTRY_KEY_SIZE 512u
#define RINNT_SERVICE_MAX_REGISTRY_VALUE_NAME_SIZE 255u
#define RINNT_SERVICE_MAX_REGISTRY_VALUE_DATA_SIZE 4096u
#define RINNT_SERVICE_REGISTRY_KEY_MARKER_NAME "\x01RinOSKeyV1"
#define RINNT_SERVICE_REGISTRY_AUDIT_PAGE_MAX_EVENTS 32u
#define RINNT_SERVICE_REGISTRY_AUDIT_EVENT_VERSION 1u
#define RINNT_SERVICE_REGISTRY_AUDIT_MAX_EVENTS_PER_FILE 8192u
#define RINNT_SERVICE_REGISTRY_AUDIT_MAX_RETAINED_EVENTS \
    (RINNT_SERVICE_REGISTRY_AUDIT_MAX_EVENTS_PER_FILE * 2u)

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
    RINNT_SERVICE_COMMAND_RAW_INPUT_READ = 14,
    RINNT_SERVICE_COMMAND_ENUM_REGISTRY_KEY = 15,
    RINNT_SERVICE_COMMAND_ENUM_REGISTRY_VALUE = 16,
    RINNT_SERVICE_COMMAND_DELETE_REGISTRY_TREE = 17,
    RINNT_SERVICE_COMMAND_FLUSH_REGISTRY_HIVE = 18,
    RINNT_SERVICE_COMMAND_QUERY_REGISTRY_HIVE = 19,
    RINNT_SERVICE_COMMAND_QUERY_REGISTRY_AUDIT = 20,
    RINNT_SERVICE_COMMAND_REGISTER_REGISTRY_NOTIFY = 21,
    RINNT_SERVICE_COMMAND_POLL_REGISTRY_NOTIFY = 22,
    RINNT_SERVICE_COMMAND_CANCEL_REGISTRY_NOTIFY = 23,
    RINNT_SERVICE_COMMAND_RENAME_REGISTRY_KEY = 24,
    RINNT_SERVICE_COMMAND_COPY_REGISTRY_TREE = 25
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
    RINNT_SERVICE_ERROR_COMMIT_UNKNOWN = -1217,
    RINNT_SERVICE_ERROR_EXISTS = -1218
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
#define RINNT_SERVICE_FEATURE_REGISTRY_TREE UINT64_C(64)
#define RINNT_SERVICE_FEATURE_REGISTRY_AUDIT UINT64_C(128)
#define RINNT_SERVICE_FEATURE_REGISTRY_NOTIFY UINT64_C(256)

#define RINNT_SERVICE_XINPUT_MAX_USERS 4u
#define RINNT_SERVICE_XINPUT_BUTTON_MASK UINT16_C(0xf3ff)

#define RINNT_SERVICE_RAW_INPUT_CLASS_KEYBOARD UINT32_C(1)
#define RINNT_SERVICE_RAW_INPUT_CLASS_MOUSE UINT32_C(2)
#define RINNT_SERVICE_RAW_INPUT_CLASS_HID UINT32_C(4)
#define RINNT_SERVICE_RAW_INPUT_CLASS_MASK UINT32_C(7)
#define RINNT_SERVICE_RAW_INPUT_HID_INLINE_REPORT_BYTES UINT32_C(20)
#define RINNT_SERVICE_RAW_INPUT_HID_MAX_REPORT_BYTES UINT32_C(64)
#define RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_BYTES UINT32_C(20)

typedef enum RinNTServiceRawInputEventTypeV1 {
    RINNT_SERVICE_RAW_INPUT_EVENT_KEY_DOWN = 1,
    RINNT_SERVICE_RAW_INPUT_EVENT_KEY_UP = 2,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_MOVE = 3,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_BUTTON = 4,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_WHEEL = 5,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_HORIZONTAL_WHEEL = 6,
    RINNT_SERVICE_RAW_INPUT_EVENT_MOUSE_ABSOLUTE = 7,
    RINNT_SERVICE_RAW_INPUT_EVENT_HID_REPORT = 8,
    RINNT_SERVICE_RAW_INPUT_EVENT_HID_SENSOR_SAMPLE = 9,
    RINNT_SERVICE_RAW_INPUT_EVENT_DEVICE_RESET = 10,
    RINNT_SERVICE_RAW_INPUT_EVENT_HID_USAGE = 11,
    RINNT_SERVICE_RAW_INPUT_EVENT_HID_REPORT_FRAGMENT = 12
} RinNTServiceRawInputEventTypeV1;

#define RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_FIRST UINT32_C(1)
#define RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_LAST UINT32_C(2)
#define RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_FLAGS_MASK UINT32_C(3)

/* A fragment's data[0..2] contain total report length, byte offset, and
 * fragment length. The 20 payload bytes occupy data[3..5] followed by the
 * event's reserved word in little-endian byte order. */
static inline uint8_t rinnt_service_raw_input_hid_fragment_byte(
    const int32_t data[6], uint64_t reserved, uint32_t index)
{
    if (!data || index >= RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_BYTES)
        return 0u;
    if (index < 12u) {
        uint32_t word = (uint32_t)data[3u + index / 4u];
        return (uint8_t)(word >> ((index % 4u) * 8u));
    }
    return (uint8_t)(reserved >> ((index - 12u) * 8u));
}

static inline int rinnt_service_raw_input_hid_fragment_valid(
    uint32_t flags, const int32_t data[6], uint64_t reserved)
{
    uint32_t total_length;
    uint32_t offset;
    uint32_t fragment_length;
    uint32_t expected_length;
    uint32_t expected_flags;
    if (!data) return 0;
    total_length = (uint32_t)data[0];
    offset = (uint32_t)data[1];
    fragment_length = (uint32_t)data[2];
    if (total_length <= RINNT_SERVICE_RAW_INPUT_HID_INLINE_REPORT_BYTES ||
        total_length > RINNT_SERVICE_RAW_INPUT_HID_MAX_REPORT_BYTES ||
        offset >= total_length ||
        offset % RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_BYTES != 0u)
        return 0;
    expected_length = total_length - offset;
    if (expected_length > RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_BYTES)
        expected_length = RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_BYTES;
    if (fragment_length != expected_length) return 0;
    expected_flags = (offset == 0u
                          ? RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_FIRST
                          : 0u) |
        (offset + fragment_length == total_length
             ? RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_LAST
             : 0u);
    if (flags != expected_flags ||
        (flags & ~RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_FLAGS_MASK) != 0u)
        return 0;
    for (uint32_t index = fragment_length;
         index < RINNT_SERVICE_RAW_INPUT_HID_FRAGMENT_BYTES; ++index) {
        if (rinnt_service_raw_input_hid_fragment_byte(data, reserved, index) !=
            0u)
            return 0;
    }
    return 1;
}

/* Generic decoded HID usage event. data[0] is the HID Usage Page, data[1]
 * is the 16-bit Usage ID, and data[2] is 1 for asserted or 0 for released.
 * This keeps Consumer Control usages distinct from keyboard scan codes. */

#define RINNT_SERVICE_RAW_INPUT_RESET_REASON_REMOVED UINT32_C(1)
#define RINNT_SERVICE_RAW_INPUT_RESET_REASON_OVERFLOW UINT32_C(2)
#define RINNT_SERVICE_RAW_INPUT_RESET_REASON_REVOKED UINT32_C(3)

/* HID sensor sample metadata is packed into NativeEvent.flags; data[0] is the
 * sensor type, data[1..3] are the logical values, data[4] is the HID Unit and
 * data[5] is the sensor status. */
#define RINNT_SERVICE_RAW_INPUT_SENSOR_VALUE_COUNT_MASK UINT32_C(0x00000003)
#define RINNT_SERVICE_RAW_INPUT_SENSOR_REPORT_ID_SHIFT UINT32_C(8)
#define RINNT_SERVICE_RAW_INPUT_SENSOR_REPORT_ID_MASK UINT32_C(0x0000ff00)
#define RINNT_SERVICE_RAW_INPUT_SENSOR_UNIT_EXPONENT_SHIFT UINT32_C(16)
#define RINNT_SERVICE_RAW_INPUT_SENSOR_UNIT_EXPONENT_MASK UINT32_C(0x00ff0000)
#define RINNT_SERVICE_RAW_INPUT_SENSOR_FLAGS_MASK \
    (RINNT_SERVICE_RAW_INPUT_SENSOR_VALUE_COUNT_MASK | \
     RINNT_SERVICE_RAW_INPUT_SENSOR_REPORT_ID_MASK | \
     RINNT_SERVICE_RAW_INPUT_SENSOR_UNIT_EXPONENT_MASK)

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

typedef struct RinNTServiceRegistryEnumerateRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t ordinal;
    uint32_t key_offset;
    uint32_t key_length;
    uint32_t reserved;
    uint32_t reserved2;
} RinNTServiceRegistryEnumerateRequestV1;

typedef struct RinNTServiceRegistryEnumerateResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t owner_uid;
    uint32_t ordinal;
    uint32_t name_offset;
    uint32_t name_length;
    uint32_t value_type;
    uint32_t data_offset;
    uint32_t data_length;
    uint32_t flags;
    uint32_t reserved;
    uint64_t hive_generation;
    uint64_t reserved2;
} RinNTServiceRegistryEnumerateResponseV1;

typedef struct RinNTServiceRegistryTreeMutationRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t flags;
    uint64_t expected_generation;
    uint32_t key_offset;
    uint32_t key_length;
    uint32_t reserved;
    uint32_t reserved2;
} RinNTServiceRegistryTreeMutationRequestV1;

typedef struct RinNTServiceRegistryRenameKeyRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t reserved;
    uint64_t expected_generation;
    uint32_t old_key_offset;
    uint32_t old_key_length;
    uint32_t new_name_offset;
    uint32_t new_name_length;
    uint32_t reserved2;
    uint32_t reserved3;
} RinNTServiceRegistryRenameKeyRequestV1;

typedef struct RinNTServiceRegistryCopyTreeRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t source_hive;
    uint32_t destination_hive;
    uint64_t expected_source_generation;
    uint64_t expected_destination_generation;
    uint32_t source_key_offset;
    uint32_t source_key_length;
    uint32_t destination_key_offset;
    uint32_t destination_key_length;
    uint32_t reserved;
    uint32_t reserved2;
} RinNTServiceRegistryCopyTreeRequestV1;

typedef struct RinNTServiceRegistryFlushRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t reserved;
} RinNTServiceRegistryFlushRequestV1;

typedef struct RinNTServiceRegistryHiveRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t reserved;
} RinNTServiceRegistryHiveRequestV1;

typedef struct RinNTServiceRegistryHiveResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t owner_uid;
    uint64_t hive_generation;
    uint64_t reserved;
} RinNTServiceRegistryHiveResponseV1;

typedef enum RinNTServiceRegistryNotifyOperationV1 {
    RINNT_SERVICE_REGISTRY_NOTIFY_REGISTER = 1,
    RINNT_SERVICE_REGISTRY_NOTIFY_POLL = 2,
    RINNT_SERVICE_REGISTRY_NOTIFY_CANCEL = 3
} RinNTServiceRegistryNotifyOperationV1;

typedef struct RinNTServiceRegistryNotifyRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t operation;
    uint32_t hive;
    uint32_t flags;
    uint32_t filter;
    uint32_t key_offset;
    uint32_t key_length;
    uint64_t subscription;
    uint64_t reserved;
} RinNTServiceRegistryNotifyRequestV1;

typedef struct RinNTServiceRegistryNotifyResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t signaled;
    uint32_t reserved;
    uint64_t subscription;
    uint64_t reserved2;
} RinNTServiceRegistryNotifyResponseV1;

#define RINNT_SERVICE_REGISTRY_NOTIFY_FLAG_SUBTREE UINT32_C(1)
#define RINNT_SERVICE_REGISTRY_NOTIFY_FILTER_NAME UINT32_C(1)
#define RINNT_SERVICE_REGISTRY_NOTIFY_FILTER_LAST_SET UINT32_C(4)
#define RINNT_SERVICE_REGISTRY_NOTIFY_FILTER_MASK \
    (RINNT_SERVICE_REGISTRY_NOTIFY_FILTER_NAME | \
     RINNT_SERVICE_REGISTRY_NOTIFY_FILTER_LAST_SET)

/* Administrator audit reads are paged oldest-first.  The first request uses
 * zero file identities and event count; each response returns a snapshot
 * cursor.  Later pages must return those values unchanged or receive STALE
 * after log rotation.  Appends to the active file do not invalidate a cursor
 * and are excluded from its fixed event count. */
typedef struct RinNTServiceRegistryAuditQueryRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t archive_identity;
    uint64_t active_identity;
    uint32_t first_event;
    uint32_t snapshot_event_count;
    uint32_t maximum_events;
    uint32_t reserved;
} RinNTServiceRegistryAuditQueryRequestV1;

typedef struct RinNTServiceRegistryAuditQueryResponseV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t archive_identity;
    uint64_t active_identity;
    uint32_t snapshot_event_count;
    uint32_t first_event;
    uint32_t event_count;
    uint32_t flags;
} RinNTServiceRegistryAuditQueryResponseV1;

#define RINNT_SERVICE_REGISTRY_AUDIT_RESPONSE_MORE UINT32_C(1)

typedef struct RinNTServiceRegistryAuditEventV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t hive;
    uint32_t result;
    uint32_t uid;
    uint32_t desired_access;
    uint32_t key_length;
    uint32_t reserved;
    uint64_t process_id;
    uint64_t process_cookie;
    uint64_t connection_id;
    char key[RINNT_SERVICE_MAX_REGISTRY_KEY_SIZE + 1u];
    uint8_t reserved_tail[7u];
} RinNTServiceRegistryAuditEventV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNTServiceRegistryAuditEventV1) == 576u,
              "RinNT service registry audit event ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinNTServiceRegistryAuditEventV1) == 576u,
               "RinNT service registry audit event ABI drift");
#endif

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
 * words. Bytes after the declared length must be zero.
 * HID_REPORT_FRAGMENT: for opaque HID reports of 21..64 bytes, data[0] is
 * total length, data[1] is byte offset, and data[2] is this fragment's
 * length. The 20 fragment bytes are packed into data[3..5] and reserved
 * (low byte first); flags contain FIRST and/or LAST. Fragments for one report
 * are consecutive events with the same device, generation, and timestamp.
 * A device reset or sequence gap invalidates any incomplete report.
 * HID_USAGE: data[0] is the HID Usage Page, data[1] is the 16-bit Usage ID,
 * and data[2] is 1 when asserted or 0 when released; remaining words are
 * zero. Generic usage events require the raw-input permission.
 * DEVICE_RESET: flags is a nonzero raw input class mask and data[0] is a
 * RINNT_SERVICE_RAW_INPUT_RESET_REASON_* value; data[1..5] are zero. A
 * removal/revocation carries the removed device handle and generation. An
 * overflow reset has both identity fields zero and clears state for every
 * requested class through this sequence. */
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
static_assert(sizeof(RinNTServiceRegistryEnumerateRequestV1) == 32u,
              "RinNT service registry enumeration request ABI drift");
static_assert(sizeof(RinNTServiceRegistryEnumerateResponseV1) == 64u,
              "RinNT service registry enumeration response ABI drift");
static_assert(sizeof(RinNTServiceRegistryTreeMutationRequestV1) == 40u,
              "RinNT service registry tree mutation ABI drift");
static_assert(sizeof(RinNTServiceRegistryRenameKeyRequestV1) == 48u,
              "RinNT service registry rename ABI drift");
static_assert(sizeof(RinNTServiceRegistryCopyTreeRequestV1) == 56u,
              "RinNT service registry copy ABI drift");
static_assert(sizeof(RinNTServiceRegistryFlushRequestV1) == 16u,
              "RinNT service registry flush ABI drift");
static_assert(sizeof(RinNTServiceRegistryHiveRequestV1) == 16u,
              "RinNT service registry hive request ABI drift");
static_assert(sizeof(RinNTServiceRegistryHiveResponseV1) == 32u,
              "RinNT service registry hive response ABI drift");
static_assert(sizeof(RinNTServiceRegistryNotifyRequestV1) == 48u,
              "RinNT service registry notify request ABI drift");
static_assert(sizeof(RinNTServiceRegistryNotifyResponseV1) == 32u,
              "RinNT service registry notify response ABI drift");
static_assert(sizeof(RinNTServiceRegistryAuditQueryRequestV1) == 40u,
              "RinNT service registry audit query ABI drift");
static_assert(sizeof(RinNTServiceRegistryAuditQueryResponseV1) == 40u,
              "RinNT service registry audit response ABI drift");
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
_Static_assert(sizeof(RinNTServiceRegistryEnumerateRequestV1) == 32u,
               "RinNT service registry enumeration request ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryEnumerateResponseV1) == 64u,
               "RinNT service registry enumeration response ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryTreeMutationRequestV1) == 40u,
               "RinNT service registry tree mutation ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryRenameKeyRequestV1) == 48u,
               "RinNT service registry rename ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryCopyTreeRequestV1) == 56u,
               "RinNT service registry copy ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryFlushRequestV1) == 16u,
               "RinNT service registry flush ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryHiveRequestV1) == 16u,
               "RinNT service registry hive request ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryHiveResponseV1) == 32u,
               "RinNT service registry hive response ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryNotifyRequestV1) == 48u,
               "RinNT service registry notify request ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryNotifyResponseV1) == 32u,
               "RinNT service registry notify response ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryAuditQueryRequestV1) == 40u,
               "RinNT service registry audit query ABI drift");
_Static_assert(sizeof(RinNTServiceRegistryAuditQueryResponseV1) == 40u,
               "RinNT service registry audit response ABI drift");
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
