// SPDX-License-Identifier: MIT
#ifndef RIN_SDK_WEB_WEBCONTENT_PROTOCOL_H
#define RIN_SDK_WEB_WEBCONTENT_PROTOCOL_H

#include <stdint.h>

#include "../ipc/shm_abi.h"
#include "webauthn_abi.h"

#define RIN_WEBCONTENT_SERVICE_ID "webcontent"
#define RIN_WEBCONTENT_RPC_TIMEOUT_MS UINT64_C(250)
#define RIN_WEBCONTENT_MAGIC 0x57434231u /* "WCB1" */
#define RIN_WEBCONTENT_VERSION 1u
#define RIN_WEBCONTENT_INLINE_THRESHOLD 16384u
#define RIN_WEBCONTENT_URL_MAX 2048
#define RIN_WEBCONTENT_TITLE_MAX 256
#define RIN_WEBCONTENT_REASON_MAX 128
#define RIN_WEBCONTENT_TEXT_MAX 64
#define RIN_WEBCONTENT_EXTENSION_ABI_VERSION 1u
#define RIN_WEBCONTENT_WEBAUTHN_TIMEOUT_MAX_MS UINT32_C(120000)
#define RIN_WEBCONTENT_WEBAUTHN_AUTHENTICATOR_DATA_MAX UINT32_C(4096)
#define RIN_WEBCONTENT_WEBAUTHN_SIGNATURE_MAX UINT32_C(128)
#define RIN_WEBCONTENT_FILE_PICKER_NAME_MAX UINT32_C(256)
#define RIN_WEBCONTENT_FILE_PICKER_MAX_SELECTIONS UINT32_C(8)
#define RIN_WEBCONTENT_PERMISSION_TYPE_MAX UINT32_C(32)
#define RIN_WEBCONTENT_PERMISSION_DESCRIPTION_MAX UINT32_C(192)
#define RIN_WEBCONTENT_ACCESSIBILITY_MAX_NODES UINT32_C(128)
#define RIN_WEBCONTENT_ACCESSIBILITY_MAX_TEXT_BYTES UINT16_C(192)

enum {
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_WINDOW = 0,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_GROUP,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_BUTTON,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_LABEL,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_TEXT_FIELD,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_TEXT_AREA,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_CHECK_BOX,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_SLIDER,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_PROGRESS_BAR,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_TAB_LIST,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_COMBO_BOX,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_RADIO_BUTTON,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_MENU,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_MENU_ITEM,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_DIALOG,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_LIST,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_TREE,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_TABLE,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_SCROLL_VIEW,
    RIN_WEBCONTENT_ACCESSIBILITY_ROLE_MAX =
        RIN_WEBCONTENT_ACCESSIBILITY_ROLE_SCROLL_VIEW
};

enum {
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_VISIBLE = UINT32_C(0x00000001),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_ENABLED = UINT32_C(0x00000002),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_FOCUSABLE = UINT32_C(0x00000004),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_FOCUSED = UINT32_C(0x00000008),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_EDITABLE = UINT32_C(0x00000010),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_CHECKED = UINT32_C(0x00000020),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_EXPANDED = UINT32_C(0x00000040),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_SELECTED = UINT32_C(0x00000080),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_MODAL = UINT32_C(0x00000100),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_SENSITIVE = UINT32_C(0x00000200),
    RIN_WEBCONTENT_ACCESSIBILITY_STATE_FOCUS_VISIBLE = UINT32_C(0x00000400)
};

enum {
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_NONE = UINT32_C(0x00000000),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_FOCUS = UINT32_C(0x00000001),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_ACTIVATE = UINT32_C(0x00000002),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_SET_VALUE = UINT32_C(0x00000004),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_INCREMENT = UINT32_C(0x00000008),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_DECREMENT = UINT32_C(0x00000010),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_EXPAND = UINT32_C(0x00000020),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_COLLAPSE = UINT32_C(0x00000040),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_SELECT = UINT32_C(0x00000080),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_DISMISS = UINT32_C(0x00000100),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_SCROLL_FORWARD = UINT32_C(0x00000200),
    RIN_WEBCONTENT_ACCESSIBILITY_ACTION_SCROLL_BACKWARD = UINT32_C(0x00000400)
};

#define RIN_WEBCONTENT_CAPABILITY_WEBAUTHN_V1 (UINT64_C(1) << 0)
#define RIN_WEBCONTENT_CAPABILITY_PERMISSIONS_V1 (UINT64_C(1) << 1)
#define RIN_WEBCONTENT_CAPABILITY_SERVICE_WORKER_OWNER_V1 (UINT64_C(1) << 2)
#define RIN_WEBCONTENT_CAPABILITY_SERIAL_PORTAL_V1 (UINT64_C(1) << 3)

#define RIN_WEBCONTENT_PERMISSION_EXTENSION_ABI_VERSION 1u

enum {
    RIN_WEBCONTENT_PERMISSION_EVENT_NONE = 0,
    RIN_WEBCONTENT_PERMISSION_EVENT_REQUEST = 1,
    RIN_WEBCONTENT_PERMISSION_EVENT_CANCEL = 2
};

enum {
    RIN_WEBCONTENT_PERMISSION_RESULT_DENIED = 0,
    RIN_WEBCONTENT_PERMISSION_RESULT_ALLOW = 1,
    RIN_WEBCONTENT_PERMISSION_RESULT_BLOCK = 2,
    RIN_WEBCONTENT_PERMISSION_RESULT_CANCELLED = 3
};

#define RIN_WEBCONTENT_WEBAUTHN_EVENT_BEGIN 1u
#define RIN_WEBCONTENT_WEBAUTHN_EVENT_CANCEL 2u
#define RIN_WEBCONTENT_WEBAUTHN_FLAG_CROSS_ORIGIN (1u << 0)
#define RIN_WEBCONTENT_WEBAUTHN_FLAG_PERMISSIONS_POLICY_ALLOWED (1u << 1)
#define RIN_WEBCONTENT_WEBAUTHN_FLAG_ANCESTORS_VALIDATED (1u << 2)

enum {
    RIN_WEBCONTENT_CMD_CREATE_PAGE_V1 = 1,
    RIN_WEBCONTENT_CMD_DESTROY_PAGE_V1 = 2,
    RIN_WEBCONTENT_CMD_NAVIGATE_V1 = 3,
    RIN_WEBCONTENT_CMD_LOAD_MARKUP_V1 = 4,
    RIN_WEBCONTENT_CMD_RESIZE_V1 = 5,
    RIN_WEBCONTENT_CMD_PUMP_EVENTS_V1 = 6,
    RIN_WEBCONTENT_CMD_PAINT_V1 = 7,
    RIN_WEBCONTENT_CMD_DISPATCH_POINTER_V1 = 8,
    RIN_WEBCONTENT_CMD_DISPATCH_KEY_OR_TEXT_V1 = 9,
    RIN_WEBCONTENT_CMD_SCROLL_TO_V1 = 10,
    RIN_WEBCONTENT_CMD_GET_PAGE_STATE_V1 = 11,
    RIN_WEBCONTENT_CMD_GET_CAPABILITIES_V1 = 12,
    RIN_WEBCONTENT_CMD_POLL_WEBAUTHN_V1 = 13,
    RIN_WEBCONTENT_CMD_COMPLETE_WEBAUTHN_V1 = 14,
    RIN_WEBCONTENT_CMD_COMPLETE_FILE_PICKER_V1 = 15,
    RIN_WEBCONTENT_CMD_GET_ACCESSIBILITY_TREE_V1 = 16,
    RIN_WEBCONTENT_CMD_PERFORM_ACCESSIBILITY_ACTION_V1 = 17,
    RIN_WEBCONTENT_CMD_GET_DOWNLOAD_EVENT_V1 = 18,
    RIN_WEBCONTENT_CMD_CANCEL_DOWNLOAD_V1 = 19,
    RIN_WEBCONTENT_CMD_RETRY_DOWNLOAD_V1 = 20,
    RIN_WEBCONTENT_CMD_COMPLETE_FILE_PICKER_V2 = 21,
    RIN_WEBCONTENT_CMD_POLL_PERMISSION_V1 = 22,
    RIN_WEBCONTENT_CMD_COMPLETE_PERMISSION_V1 = 23,
    /* A separate, long-lived Browser-owned channel.  It is deliberately
     * outside the page-control request range: page RPCs remain one-shot,
     * while WebContent may synchronously ask the Browser for durable
     * profile state during a renderer operation. */
    RIN_WEBCONTENT_CMD_OPEN_SERVICE_WORKER_OWNER_V1 = 24,
    RIN_WEBCONTENT_CMD_SERVICE_WORKER_OWNER_V1 = 25,
    /* The authenticated owner channel also carries Serial Portal frames. */
    RIN_WEBCONTENT_CMD_SERIAL_PORTAL_V1 = 26
};

enum {
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_REGISTER = 1,
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_MATCH = 2,
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_UNREGISTER = 3,
    /* Authorize the page's CacheStorage profile before the WebContent side
     * opens its durable Caches storage bottle. */
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_AUTHORIZE_CACHE = 4,
    /* Authorize one exact Cache.add()/addAll() fetch URL before network I/O. */
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_AUTHORIZE_CACHE_FETCH = 5
};

enum {
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_RESULT_REJECTED = 0,
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_RESULT_COMMITTED = 1,
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_RESULT_NOT_FOUND = 2
};

enum {
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_FLAG_PRIVATE = UINT32_C(1) << 0
};

/* Cache fetch policy is carried in update_via_cache only for operation 5.
 * The field is already present on the fixed-size owner request, so this keeps
 * the wire ABI stable while binding network credential/mode admission to the
 * exact URL owner check. */
enum {
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_CACHE_FETCH_POLICY_CREDENTIALS_INCLUDE = UINT32_C(1) << 0,
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_CACHE_FETCH_POLICY_MODE_CORS = UINT32_C(1) << 1,
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_CACHE_FETCH_POLICY_MODE_NO_CORS = UINT32_C(1) << 2,
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_CACHE_FETCH_POLICY_MODE_SAME_ORIGIN = UINT32_C(1) << 3,
    RIN_WEBCONTENT_SERVICE_WORKER_OWNER_CACHE_FETCH_POLICY_MODE_MASK =
        RIN_WEBCONTENT_SERVICE_WORKER_OWNER_CACHE_FETCH_POLICY_MODE_CORS |
        RIN_WEBCONTENT_SERVICE_WORKER_OWNER_CACHE_FETCH_POLICY_MODE_NO_CORS |
        RIN_WEBCONTENT_SERVICE_WORKER_OWNER_CACHE_FETCH_POLICY_MODE_SAME_ORIGIN
};

enum {
    RIN_WEBCONTENT_STORAGE_INLINE = 0,
    RIN_WEBCONTENT_STORAGE_SHM = 1
};

enum {
    RIN_WEBCONTENT_POINTER_DOWN = 1,
    RIN_WEBCONTENT_POINTER_UP = 2,
    RIN_WEBCONTENT_POINTER_MOVE = 3
};

/* The wire representation intentionally uses DOM-style button identities,
 * rather than GUI button bitmasks. A move carries NONE; a press or release
 * identifies the button whose state changed. */
enum {
    RIN_WEBCONTENT_POINTER_BUTTON_NONE = 0,
    RIN_WEBCONTENT_POINTER_BUTTON_PRIMARY = 1,
    RIN_WEBCONTENT_POINTER_BUTTON_MIDDLE = 2,
    RIN_WEBCONTENT_POINTER_BUTTON_SECONDARY = 3
};

enum {
    RIN_WEBCONTENT_KEY_ACTION_NONE = 0,
    RIN_WEBCONTENT_KEY_ACTION_BACKSPACE = 1,
    RIN_WEBCONTENT_KEY_ACTION_ENTER = 2,
    RIN_WEBCONTENT_KEY_ACTION_TAB = 3,
    RIN_WEBCONTENT_KEY_ACTION_ESCAPE = 4
};

enum {
    RIN_WEBCONTENT_STATE_FLAG_LOADING = 1u << 0,
    RIN_WEBCONTENT_STATE_FLAG_CRASHED = 1u << 1,
    RIN_WEBCONTENT_STATE_FLAG_DIRTY = 1u << 2,
    /* P5-2: ロード中フェーズ情報が load_* フィールドに埋まっていることを示す */
    RIN_WEBCONTENT_STATE_FLAG_HAS_LOAD_PHASE = 1u << 3,
    /* P5-2: パーサ/スクリプトが停滞している疑いがある */
    RIN_WEBCONTENT_STATE_FLAG_SUSPECTED_STALL = 1u << 4,
    /* フォーカス中のDOM要素がテキスト入力を受け付ける。 */
    RIN_WEBCONTENT_STATE_FLAG_TEXT_INPUT_ENABLED = 1u << 5,
    /* Browser が局所化して表示するダウンロード失敗状態がある。 */
    RIN_WEBCONTENT_STATE_FLAG_HAS_DOWNLOAD_STATUS = 1u << 6,
    /* WebContent が authenticated Browser/File Portal chooser を待っている。 */
    RIN_WEBCONTENT_STATE_FLAG_HAS_FILE_PICKER_REQUEST = 1u << 7
};

/* Download failures are an enumerated bridge status, never renderer- or
 * server-supplied presentation text. File Manager remains the owner of any
 * durable-success confirmation. */
enum {
    RIN_WEBCONTENT_DOWNLOAD_STATUS_NONE = 0,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_CANCELLED = 1,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_UNSAFE_URL = 2,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_UNSAFE_FILENAME = 3,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_SIZE_REJECTED = 4,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_FILE_MANAGER_UNAVAILABLE = 5,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_DURABILITY_FAILED = 6,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_NETWORK_FAILED = 7,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_HTTP_FAILED = 8,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_RESPONSE_INVALID = 9,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_MEMORY_FAILED = 10,
    RIN_WEBCONTENT_DOWNLOAD_STATUS_TRANSFER_FAILED = 11,
    /* The network stack rejected TLS before any download byte was accepted.
     * This grants no certificate exception or bypass. */
    RIN_WEBCONTENT_DOWNLOAD_STATUS_TLS_FAILED = 12
};

/* Browser polls this bounded event stream after WebContent has validated a
 * download response. Transfer bytes and File Portal authority stay inside
 * FileDownloader; only display metadata and terminal outcome cross here. */
enum {
    RIN_WEBCONTENT_DOWNLOAD_EVENT_NONE = 0,
    RIN_WEBCONTENT_DOWNLOAD_EVENT_STARTED = 1,
    RIN_WEBCONTENT_DOWNLOAD_EVENT_COMPLETED = 2,
    RIN_WEBCONTENT_DOWNLOAD_EVENT_FAILED = 3
};

/* P5-2: ロードフェーズ定数。load_phase に格納。 */
enum {
    RIN_WEBCONTENT_LOAD_PHASE_NONE = 0,
    RIN_WEBCONTENT_LOAD_PHASE_DNS = 1,
    RIN_WEBCONTENT_LOAD_PHASE_HTTP = 2,
    RIN_WEBCONTENT_LOAD_PHASE_PARSE = 3,
    RIN_WEBCONTENT_LOAD_PHASE_SCRIPT = 4,
    RIN_WEBCONTENT_LOAD_PHASE_PAINT = 5,
    RIN_WEBCONTENT_LOAD_PHASE_STALLED = 6,
    RIN_WEBCONTENT_LOAD_PHASE_COMPLETE = 7
};

typedef struct RinWebContentMsgHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t command;
    int32_t status;
    uint32_t page_id;
    uint32_t payload_len;
    uint32_t reserved0;
    uint32_t reserved1;
} RinWebContentMsgHeader;

typedef struct RinWebContentShmRegion {
    uint32_t size;
    char name[RIN_SHM_NAME_MAX];
} RinWebContentShmRegion;

typedef struct RinWebContentCreatePageRequest {
    uint32_t viewport_width;
    uint32_t viewport_height;
} RinWebContentCreatePageRequest;

typedef struct RinWebContentNavigateRequest {
    char url[RIN_WEBCONTENT_URL_MAX];
} RinWebContentNavigateRequest;

typedef struct RinWebContentLoadMarkupRequest {
    uint32_t markup_storage_kind;
    uint32_t markup_len;
    RinWebContentShmRegion markup_region;
    char base_url[RIN_WEBCONTENT_URL_MAX];
} RinWebContentLoadMarkupRequest;

typedef struct RinWebContentResizeRequest {
    uint32_t viewport_width;
    uint32_t viewport_height;
} RinWebContentResizeRequest;

typedef struct RinWebContentPointerRequest {
    int32_t x;
    int32_t y;
    int32_t button;
    int32_t pointer_type;
} RinWebContentPointerRequest;

/* Keep invalid button transitions out of the authenticated WebContent IPC.
 * In particular, NONE for a press/release would become a misleading
 * no-button MouseEvent in the renderer. */
static inline int rin_webcontent_pointer_request_valid(
    const RinWebContentPointerRequest* request)
{
    if (request == NULL)
        return 0;
    if (request->pointer_type == RIN_WEBCONTENT_POINTER_MOVE)
        return request->button == RIN_WEBCONTENT_POINTER_BUTTON_NONE;
    if (request->pointer_type != RIN_WEBCONTENT_POINTER_DOWN &&
        request->pointer_type != RIN_WEBCONTENT_POINTER_UP)
        return 0;
    return request->button >= RIN_WEBCONTENT_POINTER_BUTTON_PRIMARY &&
           request->button <= RIN_WEBCONTENT_POINTER_BUTTON_SECONDARY;
}

typedef struct RinWebContentKeyOrTextRequest {
    uint32_t action;
    /* UTF-8 text must contain a NUL terminator within this fixed array. */
    char text[RIN_WEBCONTENT_TEXT_MAX];
} RinWebContentKeyOrTextRequest;

typedef struct RinWebContentScrollRequest {
    int32_t x;
    int32_t y;
} RinWebContentScrollRequest;

typedef struct RinWebContentPageState {
    uint32_t page_id;
    uint32_t flags;
    uint32_t progress_percent;
    uint32_t state_revision;
    uint32_t paint_revision;
    int32_t scroll_x;
    int32_t scroll_y;
    int32_t max_scroll_x;
    int32_t max_scroll_y;
    uint32_t viewport_width;
    uint32_t viewport_height;
    uint32_t content_width;
    uint32_t content_height;
    char committed_url[RIN_WEBCONTENT_URL_MAX];
    char pending_url[RIN_WEBCONTENT_URL_MAX];
    char title[RIN_WEBCONTENT_TITLE_MAX];
    char crash_reason[RIN_WEBCONTENT_REASON_MAX];

    /* P5-2: ロード中ユーザフィードバック用。
     * RIN_WEBCONTENT_STATE_FLAG_HAS_LOAD_PHASE が立っているときのみ有効。*/
    uint32_t load_phase;             /* RIN_WEBCONTENT_LOAD_PHASE_* */
    uint32_t load_phase_elapsed_ms;  /* そのフェーズに入ってからの経過 ms */
    uint32_t load_bytes_received;
    uint32_t load_bytes_total;
    uint32_t load_scripts_pending;
    uint32_t load_scripts_total;
    char load_current_url[RIN_WEBCONTENT_URL_MAX];

    /* DOMテキスト入力状態。座標はWebContentビューポート基準。 */
    uint32_t text_input_content_type; /* RIN_TEXT_INPUT_CONTENT_* */
    int32_t text_input_x;
    int32_t text_input_y;
    int32_t text_input_width;
    int32_t text_input_height;

    /* Valid only with HAS_DOWNLOAD_STATUS. The revision and timestamp make a
     * stale replay fail closed in Browser UI; there is intentionally no text
     * payload in this renderer-to-browser ABI. */
    uint32_t download_status; /* RIN_WEBCONTENT_DOWNLOAD_STATUS_* */
    uint32_t download_status_revision;
    uint64_t download_status_timestamp_ms;

    /* Valid only with HAS_FILE_PICKER_REQUEST. The request id is generated by
     * the bridge and must be echoed by the Browser completion transaction.
     * The bridge currently supports one selected file; multiple selection is
     * reported so the Browser can fail closed instead of silently truncating. */
    uint64_t file_picker_request_id;
    uint32_t file_picker_allow_multiple;
    uint32_t file_picker_reserved0;
} RinWebContentPageState;

typedef struct RinWebContentPaintResponse {
    uint32_t width;
    uint32_t height;
    uint32_t pixel_storage_kind;
    uint32_t paint_revision;
    RinWebContentShmRegion pixel_region;
} RinWebContentPaintResponse;

/* Optional extensions are discovered explicitly. An old peer rejects command
 * 12 and is therefore treated as exposing no capabilities. */
typedef struct RinWebContentCapabilitiesV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t capabilities;
    uint32_t webauthn_min_version;
    uint32_t webauthn_max_version;
    uint64_t reserved[1];
} RinWebContentCapabilitiesV1;

/* Session material is issued by the Browser when it opens the durable owner
 * channel and delivered once to WebContent.  Serial Portal frames must echo
 * these values; they are not caller-selectable operation arguments. */
typedef struct RinWebContentOwnerChannelSessionV1 {
    uint64_t session_id;
    uint64_t session_generation;
    uint64_t reserved[2];
} RinWebContentOwnerChannelSessionV1;

/* Requests on the durable owner channel.  URL strings are copied from the
 * parsed LibWeb URL objects and are validated again by the Browser owner;
 * no renderer-provided profile identifier or filesystem path crosses this
 * boundary.  Register uses client_url/script_url/scope, match uses only
 * client_url, and unregister uses origin/scope. */
typedef struct RinWebContentServiceWorkerOwnerRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t operation;
    uint32_t update_via_cache;
    uint32_t flags;
    uint32_t reserved0;
    char client_url[RIN_WEBCONTENT_URL_MAX];
    char origin[RIN_WEBCONTENT_URL_MAX];
    char script_url[RIN_WEBCONTENT_URL_MAX];
    char scope[RIN_WEBCONTENT_URL_MAX];
    /* Keep the following u64 array at offset 8216 on both i386 and x86_64.
     * Without explicit padding, the i386 ABI gives u64 only 4-byte alignment
     * and the request becomes four bytes shorter than its wire contract. */
    uint32_t reserved1;
    uint64_t reserved[2];
} RinWebContentServiceWorkerOwnerRequestV1;

typedef struct RinWebContentServiceWorkerOwnerResponseV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t result;
    uint32_t state;
    uint32_t reserved0;
    uint64_t generation;
    char origin[RIN_WEBCONTENT_URL_MAX];
    char script_url[RIN_WEBCONTENT_URL_MAX];
    char scope[RIN_WEBCONTENT_URL_MAX];
    uint64_t reserved[2];
} RinWebContentServiceWorkerOwnerResponseV1;

/* The Browser polls a page so WebContent never writes unsolicited bytes into
 * an in-flight request/response exchange. after_request_id prevents replay of
 * an event that the Browser already consumed. */
typedef struct RinWebContentWebAuthnPollV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t navigation_generation;
    uint32_t reserved0;
    uint64_t after_request_id;
    uint64_t reserved[1];
} RinWebContentWebAuthnPollV1;

/* document_url and rp_id are untrusted WebContent claims. The Browser TCB
 * canonicalizes and validates them again before invoking Rin Pass. The nested
 * request contains only public WebAuthn options; PINs, private keys, TPM
 * handles, and raw CTAP frames are never valid in this protocol. */
typedef struct RinWebContentWebAuthnEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t event;
    uint64_t request_id;
    uint32_t navigation_generation;
    uint32_t operation;
    uint32_t timeout_ms;
    uint32_t flags;
    char document_url[RIN_WEBCONTENT_URL_MAX];
    char rp_id[RIN_PASS_WEBAUTHN_RP_ID_STORAGE];
    RinPassWebAuthnRequestV1 request;
    uint64_t reserved[4];
} RinWebContentWebAuthnEventV1;

/* A successful completion carries the already bounded, public result stream
 * beginning with RinPassWebAuthnResultHeaderV1. Errors and cancellation carry
 * no stream. */
typedef struct RinWebContentWebAuthnCompleteV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    int32_t result;
    uint32_t flags;
    uint64_t request_id;
    uint32_t navigation_generation;
    uint32_t result_stream_size;
    uint8_t result_stream[RIN_PASS_WEBAUTHN_RESULT_STREAM_MAX];
    uint64_t reserved[2];
} RinWebContentWebAuthnCompleteV1;

/* Permission requests are polled, never pushed, so a renderer cannot write
 * unsolicited prompt data into the Browser's request/response stream.  The
 * Browser re-authenticates origin/profile/generation before showing UI. */
typedef struct RinWebContentPermissionPollV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t navigation_generation;
    uint32_t reserved0;
    uint64_t after_request_id;
    uint64_t reserved[1];
} RinWebContentPermissionPollV1;

typedef struct RinWebContentPermissionEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t event;
    uint32_t flags;
    uint32_t navigation_generation;
    uint64_t request_id;
    char permission_type[RIN_WEBCONTENT_PERMISSION_TYPE_MAX];
    char origin[RIN_WEBCONTENT_URL_MAX];
    char description[RIN_WEBCONTENT_PERMISSION_DESCRIPTION_MAX];
    uint64_t reserved[2];
} RinWebContentPermissionEventV1;

typedef struct RinWebContentPermissionCompleteV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    int32_t result;
    uint32_t reserved1;
    uint64_t request_id;
    uint32_t navigation_generation;
    uint32_t reserved2;
    uint64_t reserved[2];
} RinWebContentPermissionCompleteV1;

enum {
    RIN_WEBCONTENT_FILE_PICKER_RESULT_OK = 0,
    RIN_WEBCONTENT_FILE_PICKER_RESULT_CANCELLED = 1
};

enum {
    RIN_WEBCONTENT_FILE_PICKER_FLAG_HAS_DESCRIPTOR = 1u << 0
};

/* A completion carries a single already-open File Portal descriptor through
 * SCM_RIGHTS. No pathname crosses the Browser/WebContent boundary. */
typedef struct RinWebContentFilePickerCompleteV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    int32_t result;
    uint32_t reserved0;
    uint64_t request_id;
    uint32_t display_name_size;
    char display_name[RIN_WEBCONTENT_FILE_PICKER_NAME_MAX];
    uint32_t reserved1;
    uint64_t reserved[2];
} RinWebContentFilePickerCompleteV1;

/* Multiple-file completion.  Each entry is paired, in order, with one
 * SCM_RIGHTS descriptor.  The fixed bound keeps the Browser/WebContent
 * boundary allocation-free and prevents a renderer from turning a chooser
 * response into an unbounded descriptor fan-out. */
typedef struct RinWebContentFilePickerSelectionV2 {
    uint32_t display_name_size;
    uint32_t reserved;
    char display_name[RIN_WEBCONTENT_FILE_PICKER_NAME_MAX];
} RinWebContentFilePickerSelectionV2;

typedef struct RinWebContentFilePickerCompleteV2 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    int32_t result;
    uint32_t reserved0;
    uint64_t request_id;
    uint32_t selection_count;
    uint32_t reserved1;
    RinWebContentFilePickerSelectionV2 selections[RIN_WEBCONTENT_FILE_PICKER_MAX_SELECTIONS];
    uint64_t reserved[2];
} RinWebContentFilePickerCompleteV2;

typedef struct RinWebContentDownloadEventRequestV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t after_revision;
    uint32_t reserved1;
} RinWebContentDownloadEventRequestV1;

/* Browser controls an existing WebContent-owned download by its opaque
 * transfer identity.  The command itself selects cancel or retry, so the
 * payload carries no mutable state, pathname, or descriptor. */
typedef struct RinWebContentDownloadControlV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint64_t transfer_id;
    uint64_t reserved[2];
} RinWebContentDownloadControlV1;

typedef struct RinWebContentDownloadEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t event;
    uint32_t revision;
    uint64_t transfer_id;
    uint64_t timestamp_ms;
    uint64_t size_bytes;
    uint32_t failure_status;
    uint32_t reserved0;
    char url[RIN_WEBCONTENT_URL_MAX];
    char filename[RIN_WEBCONTENT_FILE_PICKER_NAME_MAX];
    uint64_t reserved[2];
} RinWebContentDownloadEventV1;

/* WebContent publishes a renderer-owned semantic snapshot as a response to
 * an explicit Browser poll.  This keeps the socket request/response model
 * intact: the renderer never writes unsolicited bytes into the Browser's
 * connection.  The representation mirrors the desktop wire fields but is
 * owned by the WebContent ABI so the renderer cannot access desktop-service
 * internals directly. */
typedef struct RinWebContentAccessibilityTextV1 {
    uint16_t size;
    uint16_t reserved;
    uint8_t bytes[RIN_WEBCONTENT_ACCESSIBILITY_MAX_TEXT_BYTES];
} RinWebContentAccessibilityTextV1;

typedef struct RinWebContentAccessibilityRequestV1 {
    /* Browser supplies its native window identity so a renderer cannot
     * publish a snapshot that belongs to another chrome window. */
    uint64_t window;
} RinWebContentAccessibilityRequestV1;

typedef struct RinWebContentAccessibilityNodeV1 {
    uint64_t id;
    uint64_t parent_id;
    uint16_t role;
    uint16_t reserved0;
    uint32_t state;
    uint32_t actions;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    uint32_t cursor_start;
    uint32_t cursor_end;
    uint32_t selection_start;
    uint32_t selection_end;
    uint32_t editable_start;
    uint32_t editable_end;
    RinWebContentAccessibilityTextV1 name;
    RinWebContentAccessibilityTextV1 description;
    RinWebContentAccessibilityTextV1 value;
    uint64_t reserved[2];
} RinWebContentAccessibilityNodeV1;

typedef struct RinWebContentAccessibilitySnapshotV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t window;
    uint64_t generation;
    uint32_t node_count;
    uint32_t reserved0;
    RinWebContentAccessibilityNodeV1 nodes[RIN_WEBCONTENT_ACCESSIBILITY_MAX_NODES];
} RinWebContentAccessibilitySnapshotV1;

typedef struct RinWebContentAccessibilityActionV1 {
    uint64_t window;
    uint64_t generation;
    uint64_t node_id;
    uint32_t action;
    uint32_t reserved0;
    RinWebContentAccessibilityTextV1 value;
    uint32_t reserved1;
    uint64_t reserved[2];
} RinWebContentAccessibilityActionV1;

#if defined(__cplusplus)
static_assert(sizeof(RinWebContentCapabilitiesV1) == 32u,
              "WebContent capabilities ABI drift");
static_assert(sizeof(RinWebContentOwnerChannelSessionV1) == 32u,
              "WebContent owner channel session ABI drift");
static_assert(sizeof(RinWebContentServiceWorkerOwnerRequestV1) == 8232u,
              "WebContent ServiceWorker owner request ABI drift");
static_assert(sizeof(RinWebContentServiceWorkerOwnerResponseV1) == 6184u,
              "WebContent ServiceWorker owner response ABI drift");
static_assert(sizeof(RinWebContentWebAuthnPollV1) == 32u,
              "WebContent WebAuthn poll ABI drift");
static_assert(sizeof(RinWebContentWebAuthnEventV1) == 4608u,
              "WebContent WebAuthn event ABI drift");
static_assert(sizeof(RinWebContentWebAuthnCompleteV1) == 4512u,
              "WebContent WebAuthn completion ABI drift");
static_assert(sizeof(RinWebContentPermissionPollV1) == 32u,
              "WebContent permission poll ABI drift");
static_assert(sizeof(RinWebContentPermissionEventV1) == 2312u,
              "WebContent permission event ABI drift");
static_assert(sizeof(RinWebContentPermissionCompleteV1) == 48u,
              "WebContent permission completion ABI drift");
static_assert(sizeof(RinWebContentFilePickerCompleteV1) == 304u,
              "WebContent file picker completion ABI drift");
static_assert(sizeof(RinWebContentFilePickerSelectionV2) == 264u,
              "WebContent file picker selection ABI drift");
static_assert(sizeof(RinWebContentFilePickerCompleteV2) == 2160u,
              "WebContent multiple file picker completion ABI drift");
static_assert(sizeof(RinWebContentDownloadEventRequestV1) == 16u,
              "WebContent download event request ABI drift");
static_assert(sizeof(RinWebContentDownloadControlV1) == 32u,
              "WebContent download control ABI drift");
static_assert(sizeof(RinWebContentDownloadEventV1) == 2368u,
              "WebContent download event ABI drift");
static_assert(sizeof(RinWebContentAccessibilityTextV1) == 196u,
              "WebContent accessibility text ABI drift");
static_assert(sizeof(RinWebContentAccessibilityRequestV1) == 8u,
              "WebContent accessibility request ABI drift");
static_assert(sizeof(RinWebContentAccessibilityNodeV1) == 672u,
              "WebContent accessibility node ABI drift");
static_assert(sizeof(RinWebContentAccessibilitySnapshotV1) == 86048u,
              "WebContent accessibility snapshot ABI drift");
static_assert(sizeof(RinWebContentAccessibilityActionV1) == 248u,
              "WebContent accessibility action ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinWebContentCapabilitiesV1) == 32u,
               "WebContent capabilities ABI drift");
_Static_assert(sizeof(RinWebContentOwnerChannelSessionV1) == 32u,
               "WebContent owner channel session ABI drift");
_Static_assert(sizeof(RinWebContentServiceWorkerOwnerRequestV1) == 8232u,
               "WebContent ServiceWorker owner request ABI drift");
_Static_assert(sizeof(RinWebContentServiceWorkerOwnerResponseV1) == 6184u,
               "WebContent ServiceWorker owner response ABI drift");
_Static_assert(sizeof(RinWebContentWebAuthnPollV1) == 32u,
               "WebContent WebAuthn poll ABI drift");
_Static_assert(sizeof(RinWebContentWebAuthnEventV1) == 4608u,
               "WebContent WebAuthn event ABI drift");
_Static_assert(sizeof(RinWebContentWebAuthnCompleteV1) == 4512u,
               "WebContent WebAuthn completion ABI drift");
_Static_assert(sizeof(RinWebContentPermissionPollV1) == 32u,
               "WebContent permission poll ABI drift");
_Static_assert(sizeof(RinWebContentPermissionEventV1) == 2312u,
               "WebContent permission event ABI drift");
_Static_assert(sizeof(RinWebContentPermissionCompleteV1) == 48u,
               "WebContent permission completion ABI drift");
_Static_assert(sizeof(RinWebContentFilePickerCompleteV1) == 304u,
               "WebContent file picker completion ABI drift");
_Static_assert(sizeof(RinWebContentFilePickerSelectionV2) == 264u,
               "WebContent file picker selection ABI drift");
_Static_assert(sizeof(RinWebContentFilePickerCompleteV2) == 2160u,
               "WebContent multiple file picker completion ABI drift");
_Static_assert(sizeof(RinWebContentDownloadEventRequestV1) == 16u,
               "WebContent download event request ABI drift");
_Static_assert(sizeof(RinWebContentDownloadControlV1) == 32u,
               "WebContent download control ABI drift");
_Static_assert(sizeof(RinWebContentDownloadEventV1) == 2368u,
               "WebContent download event ABI drift");
_Static_assert(sizeof(RinWebContentAccessibilityTextV1) == 196u,
               "WebContent accessibility text ABI drift");
_Static_assert(sizeof(RinWebContentAccessibilityRequestV1) == 8u,
               "WebContent accessibility request ABI drift");
_Static_assert(sizeof(RinWebContentAccessibilityNodeV1) == 672u,
               "WebContent accessibility node ABI drift");
_Static_assert(sizeof(RinWebContentAccessibilitySnapshotV1) == 86048u,
               "WebContent accessibility snapshot ABI drift");
_Static_assert(sizeof(RinWebContentAccessibilityActionV1) == 248u,
               "WebContent accessibility action ABI drift");
#endif

#endif
