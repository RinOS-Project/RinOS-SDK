/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RIN_DRIVERS_USB_AUDIO_UAC1_PACKET_V1_H
#define RIN_DRIVERS_USB_AUDIO_UAC1_PACKET_V1_H

#include "uac1_runtime_v1.h"

#include <stdint.h>

#define RIN_USB_AUDIO_UAC1_PACKET_ABI_VERSION UINT32_C(0x00010000)
#define RIN_USB_AUDIO_UAC1_PACKET_STORAGE_BYTES 384u

#define RIN_USB_AUDIO_UAC1_BUS_FULL_SPEED 1u
#define RIN_USB_AUDIO_UAC1_BUS_HIGH_SPEED 2u

#define RIN_USB_AUDIO_UAC1_PACKET_PLAYBACK UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC1_PACKET_CAPTURE UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC1_PACKET_FEEDBACK_RATE UINT32_C(0x00000004)
#define RIN_USB_AUDIO_UAC1_PACKET_FLAGS                                \
    (RIN_USB_AUDIO_UAC1_PACKET_PLAYBACK |                              \
     RIN_USB_AUDIO_UAC1_PACKET_CAPTURE |                               \
     RIN_USB_AUDIO_UAC1_PACKET_FEEDBACK_RATE)

#define RIN_USB_AUDIO_UAC1_PLANNER_FEEDBACK_REQUIRED UINT32_C(0x00000001)
#define RIN_USB_AUDIO_UAC1_PLANNER_FEEDBACK_ACTIVE UINT32_C(0x00000002)
#define RIN_USB_AUDIO_UAC1_PLANNER_FLAGS                               \
    (RIN_USB_AUDIO_UAC1_PLANNER_FEEDBACK_REQUIRED |                   \
     RIN_USB_AUDIO_UAC1_PLANNER_FEEDBACK_ACTIVE)

typedef enum RinUsbAudioUac1PacketResultV1 {
    RIN_USB_AUDIO_UAC1_PACKET_OK = 0,
    RIN_USB_AUDIO_UAC1_PACKET_INVALID_ARGUMENT = -1,
    RIN_USB_AUDIO_UAC1_PACKET_INVALID_PROFILE = -2,
    RIN_USB_AUDIO_UAC1_PACKET_INACTIVE_PROFILE = -3,
    RIN_USB_AUDIO_UAC1_PACKET_UNSUPPORTED_SPEED = -4,
    RIN_USB_AUDIO_UAC1_PACKET_PAYLOAD_EXCEEDED = -5,
    RIN_USB_AUDIO_UAC1_PACKET_FEEDBACK_UNAVAILABLE = -6,
    RIN_USB_AUDIO_UAC1_PACKET_FEEDBACK_MALFORMED = -7,
    RIN_USB_AUDIO_UAC1_PACKET_FEEDBACK_OUT_OF_RANGE = -8,
    RIN_USB_AUDIO_UAC1_PACKET_STALE_GENERATION = -9,
    RIN_USB_AUDIO_UAC1_PACKET_BUSY = -10,
    RIN_USB_AUDIO_UAC1_PACKET_COMMIT_MISMATCH = -11,
    RIN_USB_AUDIO_UAC1_PACKET_COUNTER_EXHAUSTED = -12
} RinUsbAudioUac1PacketResultV1;

typedef struct RinUsbAudioUac1PacketPlanV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t planner_generation;
    uint32_t flags;
    uint32_t profile_index;
    uint32_t runtime_generation;
    uint32_t frame_count;
    uint32_t byte_count;
    uint64_t sequence;
    uint64_t total_frames_before;
    uint64_t rate_numerator;
    uint64_t rate_denominator;
    uint64_t remainder_after;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t endpoint_address;
    uint8_t reserved1[5];
    uint32_t reserved[4];
} RinUsbAudioUac1PacketPlanV1;

typedef struct RinUsbAudioUac1PacketSnapshotV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t generation;
    uint32_t flags;
    uint32_t profile_index;
    uint32_t runtime_generation;
    uint32_t sample_rate;
    uint32_t bus_speed;
    uint32_t base_ticks_per_second;
    uint32_t interval_ticks;
    uint32_t bytes_per_audio_frame;
    uint32_t maximum_payload;
    uint64_t sequence;
    uint64_t total_frames;
    uint64_t rate_numerator;
    uint64_t rate_denominator;
    uint64_t remainder;
    uint8_t interface_number;
    uint8_t alternate_setting;
    uint8_t endpoint_address;
    uint8_t reserved1[5];
    uint32_t reserved[8];
} RinUsbAudioUac1PacketSnapshotV1;

typedef union RinUsbAudioUac1PacketPlannerV1 {
    uint64_t alignment;
    uint8_t bytes[RIN_USB_AUDIO_UAC1_PACKET_STORAGE_BYTES];
} RinUsbAudioUac1PacketPlannerV1;

RinUsbAudioUac1PacketResultV1 rin_usb_audio_uac1_packet_init_v1(
    RinUsbAudioUac1PacketPlannerV1* planner,
    const RinUsbAudioUac1ProfileV1* profile,
    uint32_t configuration_length,
    uint32_t profile_index,
    const RinUsbAudioUac1RuntimeSnapshotV1* runtime_snapshot,
    uint32_t bus_speed);

RinUsbAudioUac1PacketResultV1 rin_usb_audio_uac1_packet_snapshot_v1(
    RinUsbAudioUac1PacketPlannerV1* planner,
    RinUsbAudioUac1PacketSnapshotV1* snapshot);

/* UAC1 full-speed feedback is 10.14 in three bytes; high-speed is 16.16. */
RinUsbAudioUac1PacketResultV1 rin_usb_audio_uac1_packet_feedback_v1(
    RinUsbAudioUac1PacketPlannerV1* planner,
    const uint8_t* feedback,
    uint32_t feedback_size,
    uint32_t expected_generation);

/* Apply an explicit feedback value with a caller-selected fixed-point
 * denominator.  UAC1 uses 10.14 at full speed and 16.16 at high speed;
 * UAC2 uses 16.16 for both bus speeds. */
RinUsbAudioUac1PacketResultV1 rin_usb_audio_uac1_packet_feedback_format_v1(
    RinUsbAudioUac1PacketPlannerV1* planner,
    const uint8_t* feedback,
    uint32_t feedback_size,
    uint64_t feedback_denominator,
    uint32_t expected_generation);

/* Peek is non-mutating. Commit advances only an exactly recomputed plan. */
RinUsbAudioUac1PacketResultV1 rin_usb_audio_uac1_packet_peek_v1(
    RinUsbAudioUac1PacketPlannerV1* planner,
    uint32_t expected_generation,
    RinUsbAudioUac1PacketPlanV1* packet);

RinUsbAudioUac1PacketResultV1 rin_usb_audio_uac1_packet_commit_v1(
    RinUsbAudioUac1PacketPlannerV1* planner,
    const RinUsbAudioUac1PacketPlanV1* packet,
    uint32_t expected_generation);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbAudioUac1PacketPlanV1) == 96u,
              "UAC1 packet plan ABI drift");
static_assert(sizeof(RinUsbAudioUac1PacketSnapshotV1) == 128u,
              "UAC1 packet snapshot ABI drift");
static_assert(sizeof(RinUsbAudioUac1PacketPlannerV1) ==
                  RIN_USB_AUDIO_UAC1_PACKET_STORAGE_BYTES,
              "UAC1 packet storage ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbAudioUac1PacketPlanV1) == 96u,
               "UAC1 packet plan ABI drift");
_Static_assert(sizeof(RinUsbAudioUac1PacketSnapshotV1) == 128u,
               "UAC1 packet snapshot ABI drift");
_Static_assert(sizeof(RinUsbAudioUac1PacketPlannerV1) ==
                   RIN_USB_AUDIO_UAC1_PACKET_STORAGE_BYTES,
               "UAC1 packet storage ABI drift");
#endif

#endif /* RIN_DRIVERS_USB_AUDIO_UAC1_PACKET_V1_H */
