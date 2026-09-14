// SPDX-License-Identifier: MIT
#ifndef RIN_SDK_RIN_AUDIO_ABI_H
#define RIN_SDK_RIN_AUDIO_ABI_H

#include <stdint.h>

#include "../syscall_abi.h"

/* Version 1 is deliberately fixed-width and has the same layout on i386 and x86_64. */
typedef struct RinAudioStatusV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t state;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t sample_format;
    uint32_t capacity_frames;
    uint32_t queued_frames;
    uint64_t submitted_frames;
    uint64_t played_frames;
    uint64_t underrun_count;
    uint32_t volume_percent;
    uint32_t muted;
} RinAudioStatusV1;

#if defined(__cplusplus)
static_assert(sizeof(RinAudioStatusV1) == 64, "RinAudioStatusV1 ABI size changed");
#else
_Static_assert(sizeof(RinAudioStatusV1) == 64, "RinAudioStatusV1 ABI size changed");
#endif

/* Hardware period information is separate so the fixed V1 status record
 * remains binary compatible with existing applications. */
#define RIN_AUDIO_PERIOD_VERSION 1u
typedef struct RinAudioPeriodV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t period_frames;
    uint32_t period_bytes;
    uint32_t buffer_frames;
    uint32_t reserved[3];
} RinAudioPeriodV1;

#if defined(__cplusplus)
static_assert(sizeof(RinAudioPeriodV1) == 32, "RinAudioPeriodV1 ABI size changed");
#else
_Static_assert(sizeof(RinAudioPeriodV1) == 32, "RinAudioPeriodV1 ABI size changed");
#endif

#endif /* RIN_SDK_RIN_AUDIO_ABI_H */
