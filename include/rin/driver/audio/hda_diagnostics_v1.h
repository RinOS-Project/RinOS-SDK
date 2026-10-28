/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_AUDIO_HDA_DIAGNOSTICS_V1_H
#define RIN_DRIVER_AUDIO_HDA_DIAGNOSTICS_V1_H

#include <stdint.h>

/* Pointer-free HDA owner status. It reports bounded stream and lifecycle
 * state, not register contents, callback contexts, or hardware authority. */
#define RIN_HDA_DIAGNOSTIC_SNAPSHOT_VERSION UINT32_C(0x00010000)
#define RIN_HDA_DIAGNOSTIC_FLAG_MMIO_PRESENT UINT32_C(0x00000001)
#define RIN_HDA_DIAGNOSTIC_FLAG_COMMAND_RINGS_ENABLED UINT32_C(0x00000002)
#define RIN_HDA_DIAGNOSTIC_FLAG_CODEC_ROUTE_VALID UINT32_C(0x00000004)
#define RIN_HDA_DIAGNOSTIC_FLAG_OUTPUT_STREAM_PRESENT UINT32_C(0x00000008)
#define RIN_HDA_DIAGNOSTIC_FLAG_INPUT_STREAM_PRESENT UINT32_C(0x00000010)
#define RIN_HDA_DIAGNOSTIC_FLAG_MASK \
    (RIN_HDA_DIAGNOSTIC_FLAG_MMIO_PRESENT | \
     RIN_HDA_DIAGNOSTIC_FLAG_COMMAND_RINGS_ENABLED | \
     RIN_HDA_DIAGNOSTIC_FLAG_CODEC_ROUTE_VALID | \
     RIN_HDA_DIAGNOSTIC_FLAG_OUTPUT_STREAM_PRESENT | \
     RIN_HDA_DIAGNOSTIC_FLAG_INPUT_STREAM_PRESENT)

typedef struct RinHdaDiagnosticSnapshotV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t codec_vendor_id;
    uint32_t codec_revision_id;
    uint32_t codec_profile_id;
    uint32_t widget_count;
    uint32_t codec_power_state_count;
    uint32_t output_stream_index;
    uint32_t input_stream_index;
    uint32_t active;
    uint32_t active_prepared;
    uint32_t configured;
    uint32_t suspended;
    uint32_t removed;
    uint32_t quarantined;
    uint32_t transport_stopped;
    uint32_t irq_pending;
    uint32_t jack_event_pending;
    uint32_t headphone_present;
    uint32_t capture_pin_present;
    uint32_t playback_configured;
    uint32_t capture_configured;
    uint32_t direction;
    uint32_t volume_percent;
    uint32_t muted;
    uint32_t capture_volume_percent;
    uint32_t capture_muted;
    uint32_t output_frame_count;
    uint32_t input_frame_count;
    uint32_t active_frame_count;
    uint32_t active_segment_count;
    uint32_t reserved[4];
} RinHdaDiagnosticSnapshotV1;

#if defined(__cplusplus)
static_assert(sizeof(RinHdaDiagnosticSnapshotV1) == 144u,
              "HDA diagnostic snapshot ABI drift");
#else
_Static_assert(sizeof(RinHdaDiagnosticSnapshotV1) == 144u,
               "HDA diagnostic snapshot ABI drift");
#endif

#endif /* RIN_DRIVER_AUDIO_HDA_DIAGNOSTICS_V1_H */
