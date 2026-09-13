/* SPDX-License-Identifier: MIT */
/* Bounded per-application audio policy shared by Settings and audio owners. */
#ifndef RIN_AUDIO_POLICY_ABI_H
#define RIN_AUDIO_POLICY_ABI_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define RINRUNTIME_AUDIO_APPLICATION_POLICY_VERSION UINT32_C(1)
#define RINRUNTIME_AUDIO_APPLICATION_POLICY_MAX UINT32_C(16)
#define RINRUNTIME_AUDIO_POLICY_OUTPUT_BIT UINT32_C(0x00000001)
#define RINRUNTIME_AUDIO_POLICY_INPUT_BIT UINT32_C(0x00000002)
#define RINRUNTIME_AUDIO_POLICY_FLAGS_MASK \
    (RINRUNTIME_AUDIO_POLICY_OUTPUT_BIT | RINRUNTIME_AUDIO_POLICY_INPUT_BIT)

typedef struct RinRuntimeAudioApplicationPolicyV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t application_id;
    uint64_t application_generation;
    uint32_t flags;
    uint32_t max_output_volume;
    uint32_t reserved0;
    uint32_t reserved1;
} RinRuntimeAudioApplicationPolicyV1;

typedef struct RinRuntimeAudioPolicyCatalogV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t generation;
    uint32_t policy_count;
    uint32_t reserved;
    RinRuntimeAudioApplicationPolicyV1 policies[
        RINRUNTIME_AUDIO_APPLICATION_POLICY_MAX];
} RinRuntimeAudioPolicyCatalogV1;

typedef struct RinRuntimeAudioPolicySelectionV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t catalog_generation;
    uint64_t application_id;
    uint64_t application_generation;
    uint32_t flags;
    uint32_t max_output_volume;
} RinRuntimeAudioPolicySelectionV1;

#if defined(__cplusplus)
static_assert(sizeof(RinRuntimeAudioApplicationPolicyV1) == 40u,
              "audio application policy ABI drift");
static_assert(sizeof(RinRuntimeAudioPolicyCatalogV1) == 664u,
              "audio policy catalog ABI drift");
static_assert(sizeof(RinRuntimeAudioPolicySelectionV1) == 40u,
              "audio policy selection ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinRuntimeAudioApplicationPolicyV1) == 40u,
               "audio application policy ABI drift");
_Static_assert(sizeof(RinRuntimeAudioPolicyCatalogV1) == 664u,
               "audio policy catalog ABI drift");
_Static_assert(sizeof(RinRuntimeAudioPolicySelectionV1) == 40u,
               "audio policy selection ABI drift");
#endif

static inline int rinruntime_audio_application_policy_valid(
    const RinRuntimeAudioApplicationPolicyV1* policy)
{
    return policy != NULL && policy->struct_size == sizeof(*policy) &&
           policy->version == RINRUNTIME_AUDIO_APPLICATION_POLICY_VERSION &&
           policy->application_id != 0u &&
           policy->application_generation != 0u &&
           policy->flags != 0u &&
           (policy->flags & ~RINRUNTIME_AUDIO_POLICY_FLAGS_MASK) == 0u &&
           policy->max_output_volume <= 100u && policy->reserved0 == 0u &&
           policy->reserved1 == 0u;
}

static inline int rinruntime_audio_policy_catalog_valid(
    const RinRuntimeAudioPolicyCatalogV1* catalog)
{
    uint32_t index;
    if (catalog == NULL || catalog->struct_size != sizeof(*catalog) ||
        catalog->version != RINRUNTIME_AUDIO_APPLICATION_POLICY_VERSION ||
        catalog->generation == 0u || catalog->policy_count == 0u ||
        catalog->policy_count > RINRUNTIME_AUDIO_APPLICATION_POLICY_MAX ||
        catalog->reserved != 0u)
        return 0;
    for (index = 0u; index < catalog->policy_count; ++index) {
        uint32_t other;
        if (!rinruntime_audio_application_policy_valid(
                &catalog->policies[index]))
            return 0;
        for (other = 0u; other < index; ++other)
            if (catalog->policies[other].application_id ==
                catalog->policies[index].application_id)
                return 0;
    }
    for (index = catalog->policy_count;
         index < RINRUNTIME_AUDIO_APPLICATION_POLICY_MAX; ++index) {
        const uint8_t* bytes = (const uint8_t*)&catalog->policies[index];
        uint32_t byte;
        for (byte = 0u; byte < sizeof(catalog->policies[index]); ++byte)
            if (bytes[byte] != 0u) return 0;
    }
    return 1;
}

static inline int rinruntime_audio_policy_select(
    const RinRuntimeAudioPolicyCatalogV1* catalog, uint64_t expected_generation,
    uint64_t application_id, uint64_t application_generation,
    RinRuntimeAudioPolicySelectionV1* output)
{
    uint32_t index;
    if (output == NULL) return 0;
    memset(output, 0, sizeof(*output));
    if (!rinruntime_audio_policy_catalog_valid(catalog) ||
        expected_generation == 0u || expected_generation != catalog->generation ||
        application_id == 0u || application_generation == 0u)
        return 0;
    for (index = 0u; index < catalog->policy_count; ++index) {
        const RinRuntimeAudioApplicationPolicyV1* policy =
            &catalog->policies[index];
        if (policy->application_id == application_id &&
            policy->application_generation == application_generation) {
            output->struct_size = sizeof(*output);
            output->version = RINRUNTIME_AUDIO_APPLICATION_POLICY_VERSION;
            output->catalog_generation = catalog->generation;
            output->application_id = application_id;
            output->application_generation = application_generation;
            output->flags = policy->flags;
            output->max_output_volume = policy->max_output_volume;
            return 1;
        }
    }
    return 0;
}

/* Return 1 when output may be used at requested_volume, 0 when policy denies
 * it, and -1 when the selection itself is malformed. */
static inline int rinruntime_audio_policy_admit_output(
    const RinRuntimeAudioPolicySelectionV1* selection,
    uint64_t expected_catalog_generation, uint32_t requested_volume)
{
    if (selection == NULL || selection->struct_size != sizeof(*selection) ||
        selection->version != RINRUNTIME_AUDIO_APPLICATION_POLICY_VERSION ||
        selection->catalog_generation == 0u || selection->application_id == 0u ||
        selection->application_generation == 0u || selection->flags == 0u ||
        (selection->flags & ~RINRUNTIME_AUDIO_POLICY_FLAGS_MASK) != 0u ||
        selection->max_output_volume > 100u)
        return -1;
    if (expected_catalog_generation == 0u ||
        selection->catalog_generation != expected_catalog_generation)
        return -1;
    if ((selection->flags & RINRUNTIME_AUDIO_POLICY_OUTPUT_BIT) == 0u ||
        requested_volume > selection->max_output_volume)
        return 0;
    return 1;
}

#endif /* RIN_AUDIO_POLICY_ABI_H */
