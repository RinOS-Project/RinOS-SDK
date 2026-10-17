/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVERS_USB_AUDIO_UAC1_ISO_V1_H
#define RIN_DRIVERS_USB_AUDIO_UAC1_ISO_V1_H

#include "packet_planner_v1.h"

#include <stdint.h>

#define RIN_USB_AUDIO_UAC1_ISO_ABI_VERSION UINT32_C(0x00010000)
#define RIN_USB_AUDIO_UAC1_ISO_STORAGE_BYTES 1024u

#define RIN_USB_AUDIO_UAC1_ISO_PLAYBACK UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC1_ISO_CAPTURE UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC1_ISO_FEEDBACK_RATE UINT32_C(0x00000004)
#define RIN_USB_AUDIO_UAC1_ISO_ZERO_LENGTH UINT32_C(0x00000008)
#define RIN_USB_AUDIO_UAC1_ISO_TRANSFER_FLAGS                         \
    (RIN_USB_AUDIO_UAC1_ISO_PLAYBACK |                               \
     RIN_USB_AUDIO_UAC1_ISO_CAPTURE |                                \
     RIN_USB_AUDIO_UAC1_ISO_FEEDBACK_RATE |                          \
     RIN_USB_AUDIO_UAC1_ISO_ZERO_LENGTH)

#define RIN_USB_AUDIO_UAC1_ISO_PENDING UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC1_ISO_RECOVERY_REQUIRED UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC1_ISO_STATE_FLAGS                            \
    (RIN_USB_AUDIO_UAC1_ISO_PENDING |                                \
     RIN_USB_AUDIO_UAC1_ISO_RECOVERY_REQUIRED)

#define RIN_USB_AUDIO_UAC1_ISO_COMPLETION_NONE 0u
#define RIN_USB_AUDIO_UAC1_ISO_COMPLETION_SUCCESS 1u
#define RIN_USB_AUDIO_UAC1_ISO_COMPLETION_CANCELLED 2u
#define RIN_USB_AUDIO_UAC1_ISO_COMPLETION_DEVICE_ERROR 3u

typedef enum RinUsbAudioUac1IsoResultV1 {
    RIN_USB_AUDIO_UAC1_ISO_OK = 0,
    RIN_USB_AUDIO_UAC1_ISO_INVALID_ARGUMENT = -1,
    RIN_USB_AUDIO_UAC1_ISO_INVALID_BACKEND = -2,
    RIN_USB_AUDIO_UAC1_ISO_INVALID_STATE = -3,
    RIN_USB_AUDIO_UAC1_ISO_STALE_GENERATION = -4,
    RIN_USB_AUDIO_UAC1_ISO_BUSY = -5,
    RIN_USB_AUDIO_UAC1_ISO_BUFFER_TOO_SMALL = -6,
    RIN_USB_AUDIO_UAC1_ISO_BACKEND_FAILURE = -7,
    RIN_USB_AUDIO_UAC1_ISO_CALLBACK_CORRUPTION = -8,
    RIN_USB_AUDIO_UAC1_ISO_TRANSFER_PENDING = -9,
    RIN_USB_AUDIO_UAC1_ISO_NOT_PENDING = -10,
    RIN_USB_AUDIO_UAC1_ISO_COMPLETION_MISMATCH = -11,
    RIN_USB_AUDIO_UAC1_ISO_CANCEL_FAILURE = -12,
    RIN_USB_AUDIO_UAC1_ISO_RECOVERY_REQUIRED_RESULT = -13,
    RIN_USB_AUDIO_UAC1_ISO_GENERATION_EXHAUSTED = -14,
    RIN_USB_AUDIO_UAC1_ISO_PACKET_FAILURE = -15
} RinUsbAudioUac1IsoResultV1;

typedef struct RinUsbAudioUac1IsoTransferV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t engine_generation;
    uint32_t flags;
    uint32_t planner_generation;
    uint32_t runtime_generation;
    uint32_t profile_index;
    uint32_t frame_count;
    uint32_t byte_count;
    uint32_t buffer_capacity;
    uint64_t sequence;
    uint64_t total_frames_before;
    uint64_t rate_numerator;
    uint64_t rate_denominator;
    uint64_t remainder_after;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t endpoint_address;
    uint8_t reserved1[5];
    uint32_t reserved[2];
} RinUsbAudioUac1IsoTransferV1;

typedef int (*RinUsbAudioUac1IsoSubmitV1)(
    void* context,
    const RinUsbAudioUac1IsoTransferV1* transfer,
    uint8_t* buffer,
    uint32_t buffer_capacity,
    uint64_t* submission_id,
    uint32_t* accepted_length);

typedef int (*RinUsbAudioUac1IsoCancelV1)(
    void* context,
    uint64_t submission_id);

typedef struct RinUsbAudioUac1IsoBackendV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    void* context;
    RinUsbAudioUac1IsoSubmitV1 submit;
    RinUsbAudioUac1IsoCancelV1 cancel;
    uint32_t reserved[8];
} RinUsbAudioUac1IsoBackendV1;

typedef struct RinUsbAudioUac1IsoSnapshotV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t generation;
    uint32_t flags;
    uint32_t planner_generation;
    uint32_t runtime_generation;
    uint32_t profile_index;
    uint32_t pending_bytes;
    uint32_t last_completion;
    uint32_t pending_flags;
    uint64_t pending_submission_id;
    uint64_t pending_sequence;
    uint64_t submitted_packets;
    uint64_t completed_packets;
    uint64_t failed_packets;
    uint64_t cancelled_packets;
    uint64_t completed_frames;
    uint64_t completed_bytes;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t endpoint_address;
    uint8_t reserved1[5];
    uint32_t reserved[4];
} RinUsbAudioUac1IsoSnapshotV1;

typedef union RinUsbAudioUac1IsoEngineV1 {
    uint64_t alignment;
    uint8_t bytes[RIN_USB_AUDIO_UAC1_ISO_STORAGE_BYTES];
} RinUsbAudioUac1IsoEngineV1;

/* Initialization owns a private packet planner and snapshots the backend. */
RinUsbAudioUac1IsoResultV1 rin_usb_audio_uac1_iso_init_v1(
    RinUsbAudioUac1IsoEngineV1* engine,
    const RinUsbAudioUac1ProfileV1* profile,
    uint32_t configuration_length,
    uint32_t profile_index,
    const RinUsbAudioUac1RuntimeSnapshotV1* runtime_snapshot,
    uint32_t bus_speed,
    const RinUsbAudioUac1IsoBackendV1* backend);

RinUsbAudioUac1IsoResultV1 rin_usb_audio_uac1_iso_snapshot_v1(
    RinUsbAudioUac1IsoEngineV1* engine,
    RinUsbAudioUac1IsoSnapshotV1* snapshot);

RinUsbAudioUac1IsoResultV1 rin_usb_audio_uac1_iso_feedback_v1(
    RinUsbAudioUac1IsoEngineV1* engine,
    const uint8_t* feedback,
    uint32_t feedback_size,
    uint32_t expected_generation);

/* Apply explicit feedback with a caller-selected fixed-point denominator.
 * UAC2 uses 16.16 feedback at both full and high speed; this entry point
 * lets a protocol adapter preserve that rule while sharing the ISO owner. */
RinUsbAudioUac1IsoResultV1 rin_usb_audio_uac1_iso_feedback_format_v1(
    RinUsbAudioUac1IsoEngineV1* engine,
    const uint8_t* feedback,
    uint32_t feedback_size,
    uint64_t feedback_denominator,
    uint32_t expected_generation);

/* The buffer is leased to the backend until exact completion or cancellation. */
RinUsbAudioUac1IsoResultV1 rin_usb_audio_uac1_iso_submit_v1(
    RinUsbAudioUac1IsoEngineV1* engine,
    uint8_t* buffer,
    uint32_t buffer_capacity,
    uint32_t expected_generation,
    RinUsbAudioUac1IsoTransferV1* transfer);

RinUsbAudioUac1IsoResultV1 rin_usb_audio_uac1_iso_complete_v1(
    RinUsbAudioUac1IsoEngineV1* engine,
    uint64_t submission_id,
    uint32_t transferred,
    uint32_t completion,
    uint32_t expected_generation);

RinUsbAudioUac1IsoResultV1 rin_usb_audio_uac1_iso_cancel_v1(
    RinUsbAudioUac1IsoEngineV1* engine,
    uint64_t submission_id,
    uint32_t expected_generation);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbAudioUac1IsoTransferV1) == 96u,
              "UAC1 isochronous transfer ABI drift");
static_assert(sizeof(RinUsbAudioUac1IsoSnapshotV1) == 128u,
              "UAC1 isochronous snapshot ABI drift");
static_assert(sizeof(RinUsbAudioUac1IsoEngineV1) ==
                  RIN_USB_AUDIO_UAC1_ISO_STORAGE_BYTES,
              "UAC1 isochronous engine storage ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbAudioUac1IsoTransferV1) == 96u,
               "UAC1 isochronous transfer ABI drift");
_Static_assert(sizeof(RinUsbAudioUac1IsoSnapshotV1) == 128u,
               "UAC1 isochronous snapshot ABI drift");
_Static_assert(sizeof(RinUsbAudioUac1IsoEngineV1) ==
                   RIN_USB_AUDIO_UAC1_ISO_STORAGE_BYTES,
               "UAC1 isochronous engine storage ABI drift");
#endif

#endif /* RIN_DRIVERS_USB_AUDIO_UAC1_ISO_V1_H */
