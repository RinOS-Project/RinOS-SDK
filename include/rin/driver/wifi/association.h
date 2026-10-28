/* SPDX-License-Identifier: MIT */
/* Fail-closed Wi-Fi authentication, association and key-install sequencing. */

#ifndef RIN_DRIVER_WIFI_ASSOCIATION_H
#define RIN_DRIVER_WIFI_ASSOCIATION_H

#include <stdint.h>

#include <rin/driver/wifi/discovery.h>

#define RIN_WIFI_ASSOCIATION_VERSION 1u
#define RIN_WIFI_ASSOCIATION_MIN_TIMEOUT_MS UINT32_C(1000)
#define RIN_WIFI_ASSOCIATION_MAX_TIMEOUT_MS UINT32_C(30000)
#define RIN_WIFI_ASSOCIATION_MIN_ROAM_AGE_MS UINT32_C(100)
#define RIN_WIFI_ASSOCIATION_MAX_ROAM_AGE_MS UINT32_C(60000)
#define RIN_WIFI_ASSOCIATION_MIN_ROAM_GAIN_DB UINT32_C(1)
#define RIN_WIFI_ASSOCIATION_MAX_ROAM_GAIN_DB UINT32_C(60)
/* A resume must use a strictly newer trusted scan generation. This is an
 * owner-to-owner value, not a beacon sequence number supplied by the AP. */
#define RIN_WIFI_ASSOCIATION_MIN_SCAN_GENERATION UINT64_C(1)
#define RIN_WIFI_ASSOCIATION_MAX_RESUME_CANDIDATES UINT32_C(32)

#define RIN_WIFI_ASSOCIATION_KEY_PAIRWISE UINT32_C(0x00000001)
#define RIN_WIFI_ASSOCIATION_KEY_GROUP    UINT32_C(0x00000002)
#define RIN_WIFI_ASSOCIATION_PMF_ACTIVE   UINT32_C(0x00000004)
#define RIN_WIFI_ASSOCIATION_EVENT_FLAGS  UINT32_C(0x00000007)

typedef enum RinWifiAssociationStatus {
    RIN_WIFI_ASSOCIATION_OK = 0,
    RIN_WIFI_ASSOCIATION_INVALID_ARGUMENT = -1,
    RIN_WIFI_ASSOCIATION_INVALID_STATE = -2,
    RIN_WIFI_ASSOCIATION_TRANSPORT_FAILURE = -3,
    RIN_WIFI_ASSOCIATION_MALFORMED_EVENT = -4,
    RIN_WIFI_ASSOCIATION_TIMEOUT = -5,
    RIN_WIFI_ASSOCIATION_STALE_EVENT = -6,
    RIN_WIFI_ASSOCIATION_UNSUPPORTED = -7,
    RIN_WIFI_ASSOCIATION_SECURITY_FAILURE = -8,
    RIN_WIFI_ASSOCIATION_GENERATION_EXHAUSTED = -9
} RinWifiAssociationStatus;

typedef enum RinWifiAssociationState {
    RIN_WIFI_ASSOCIATION_UNINITIALIZED = 0,
    RIN_WIFI_ASSOCIATION_IDLE = 1,
    RIN_WIFI_ASSOCIATION_AUTHENTICATING = 2,
    RIN_WIFI_ASSOCIATION_ASSOCIATING = 3,
    RIN_WIFI_ASSOCIATION_KEY_EXCHANGE = 4,
    RIN_WIFI_ASSOCIATION_CONNECTED = 5,
    RIN_WIFI_ASSOCIATION_DISCONNECTING = 6,
    RIN_WIFI_ASSOCIATION_FAILED = 7,
    RIN_WIFI_ASSOCIATION_SUSPENDING = 8,
    RIN_WIFI_ASSOCIATION_SUSPENDED = 9
} RinWifiAssociationState;

typedef enum RinWifiAssociationOperation {
    RIN_WIFI_ASSOCIATION_OP_AUTHENTICATE = 1,
    RIN_WIFI_ASSOCIATION_OP_ASSOCIATE = 2,
    RIN_WIFI_ASSOCIATION_OP_KEY_EXCHANGE = 3,
    RIN_WIFI_ASSOCIATION_OP_DISCONNECT = 4,
    /* The driver must stop data traffic and remove installed key material
     * before acknowledging this operation with QUIESCED. */
    RIN_WIFI_ASSOCIATION_OP_QUIESCE = 5
} RinWifiAssociationOperation;

typedef enum RinWifiAuthenticationMode {
    RIN_WIFI_AUTH_OPEN_SYSTEM = 1,
    RIN_WIFI_AUTH_SAE = 2
} RinWifiAuthenticationMode;

typedef enum RinWifiAssociationEventType {
    RIN_WIFI_ASSOCIATION_EVENT_AUTHENTICATED = 1,
    RIN_WIFI_ASSOCIATION_EVENT_ASSOCIATED = 2,
    RIN_WIFI_ASSOCIATION_EVENT_KEYS_INSTALLED = 3,
    RIN_WIFI_ASSOCIATION_EVENT_FAILED = 4,
    RIN_WIFI_ASSOCIATION_EVENT_DISCONNECTED = 5,
    RIN_WIFI_ASSOCIATION_EVENT_QUIESCED = 6
} RinWifiAssociationEventType;

typedef struct RinWifiAssociationCommandV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t generation;
    uint32_t operation;
    uint32_t authentication_mode;
    uint32_t selected_security;
    uint32_t reason_code;
    RinWifiDiscoveredNetworkV1 network;
} RinWifiAssociationCommandV1;

typedef struct RinWifiAssociationEventV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t generation;
    uint32_t type;
    uint32_t selected_security;
    uint8_t bssid[6];
    uint8_t flags;
    uint8_t reserved0;
    uint16_t status_code;
    uint16_t reason_code;
    uint32_t association_id;
    uint64_t replay_counter;
    uint64_t reserved[2];
} RinWifiAssociationEventV1;

typedef int (*RinWifiAssociationSubmitFn)(
    void* context, const RinWifiAssociationCommandV1* command);
typedef uint64_t (*RinWifiAssociationMonotonicMsFn)(void* context);

typedef struct RinWifiAssociationTransportV1 {
    uint32_t struct_size;
    uint32_t version;
    RinWifiAssociationSubmitFn submit;
    RinWifiAssociationMonotonicMsFn monotonic_ms;
    void* context;
} RinWifiAssociationTransportV1;

/* The owner serializes calls. Callbacks must copy the command before returning
 * if it is queued, and callback re-entry into this controller is rejected. */

typedef struct RinWifiAssociationControllerV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t callback_active;
    uint64_t generation;
    uint64_t deadline_ms;
    uint64_t last_replay_counter;
    uint32_t selected_security;
    uint32_t association_id;
    int32_t last_status;
    uint32_t roam_pending;
    uint32_t resume_required;
    uint32_t resume_target_valid;
    uint64_t scan_generation_floor;
    RinWifiAssociationTransportV1 transport;
#if UINTPTR_MAX == UINT32_MAX
    uint32_t reserved_alignment;
#endif
    RinWifiDiscoveredNetworkV1 network;
    RinWifiDiscoveredNetworkV1 roam_network;
    RinWifiDiscoveredNetworkV1 resume_network;
} RinWifiAssociationControllerV1;

int rin_wifi_association_init(
    RinWifiAssociationControllerV1* controller,
    const RinWifiAssociationTransportV1* transport);
int rin_wifi_association_start(
    RinWifiAssociationControllerV1* controller,
    const RinWifiDiscoveredNetworkV1* network, uint32_t timeout_ms);
/* Starts from a trusted scanner generation.  After resume, this is the only
 * start entry point: it rejects the scan that was current before suspend. */
int rin_wifi_association_start_from_scan(
    RinWifiAssociationControllerV1* controller,
    const RinWifiDiscoveredNetworkV1* network, uint64_t scan_generation,
    uint32_t timeout_ms);
int rin_wifi_association_process_event(
    RinWifiAssociationControllerV1* controller,
    const RinWifiAssociationEventV1* event);
int rin_wifi_association_poll(RinWifiAssociationControllerV1* controller);
int rin_wifi_association_disconnect(
    RinWifiAssociationControllerV1* controller, uint16_t reason_code);
/* Quiesce is asynchronous.  It reaches SUSPENDED only after the driver has
 * acknowledged key removal.  A timeout or transport failure is never treated
 * as a suspend success. */
int rin_wifi_association_suspend(
    RinWifiAssociationControllerV1* controller, uint64_t scan_generation,
    uint32_t timeout_ms);
/* Makes the controller IDLE but retains the scan generation floor. The caller
 * must next use start_from_scan with a strictly newer trusted scan result. */
int rin_wifi_association_resume(RinWifiAssociationControllerV1* controller);
/* Resume and choose the strongest same-SSID/same-security BSS from a fresh,
 * trusted scan. The saved resume target contains no key material. */
int rin_wifi_association_resume_from_scan(
    RinWifiAssociationControllerV1* controller,
    const RinWifiDiscoveredNetworkV1* networks, uint32_t network_count,
    uint64_t scan_generation, uint32_t timeout_ms);
/* Discard all association state after a device/firmware reset.  The upper
 * generation advances so a translated event from the old association cannot
 * be accepted by a newly started connection. */
int rin_wifi_association_invalidate_for_device_reset(
    RinWifiAssociationControllerV1* controller);
int rin_wifi_association_roam(
    RinWifiAssociationControllerV1* controller,
    const RinWifiDiscoveredNetworkV1* candidate,
    uint64_t observed_monotonic_ms, uint32_t maximum_age_ms,
    uint32_t minimum_signal_gain_db, uint32_t timeout_ms,
    uint16_t disconnect_reason_code);

/* Commands and events cross the driver boundary and therefore have a fixed
 * layout. Transport and controller objects contain native pointers and are
 * deliberately architecture-local. */
#if defined(__cplusplus)
static_assert(sizeof(RinWifiAssociationCommandV1) == 128u,
              "Wi-Fi association command ABI drift");
static_assert(sizeof(RinWifiAssociationEventV1) == 64u,
              "Wi-Fi association event ABI drift");
static_assert(sizeof(RinWifiAssociationTransportV1) ==
                  (sizeof(void*) == 8u ? 32u : 20u),
              "Wi-Fi association transport layout drift");
static_assert(sizeof(RinWifiAssociationControllerV1) ==
                  (sizeof(void*) == 8u ? 392u : 384u),
              "Wi-Fi association controller layout drift");
#else
_Static_assert(sizeof(RinWifiAssociationCommandV1) == 128u,
               "Wi-Fi association command ABI drift");
_Static_assert(sizeof(RinWifiAssociationEventV1) == 64u,
               "Wi-Fi association event ABI drift");
_Static_assert(sizeof(RinWifiAssociationTransportV1) ==
                   (sizeof(void*) == 8u ? 32u : 20u),
               "Wi-Fi association transport layout drift");
_Static_assert(sizeof(RinWifiAssociationControllerV1) ==
                   (sizeof(void*) == 8u ? 392u : 384u),
               "Wi-Fi association controller layout drift");
#endif

#endif /* RIN_DRIVER_WIFI_ASSOCIATION_H */
