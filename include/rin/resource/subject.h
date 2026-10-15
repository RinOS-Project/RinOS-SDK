/* SPDX-License-Identifier: MIT */
/* Kernel-owned process identity used at application-resource boundaries. */

#ifndef RIN_API_RIN_RESOURCE_SUBJECT_H
#define RIN_API_RIN_RESOURCE_SUBJECT_H

#include <stdint.h>

#include <rin/abi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_RESOURCE_SUBJECT_VERSION 1u
#define RIN_RESOURCE_SUBJECT_APPLICATION_ID_SIZE 32u

typedef struct RinResourceSubjectV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t owner_uid;
    uint32_t process_id;
    uint64_t process_instance_cookie;
    RinCapabilitySet current_capabilities;
    uint8_t application_id[RIN_RESOURCE_SUBJECT_APPLICATION_ID_SIZE];
    uint64_t reserved[2];
} RinResourceSubjectV1;

typedef enum RinResourceSubjectStatus {
    RIN_RESOURCE_SUBJECT_OK = 0,
    RIN_RESOURCE_SUBJECT_INVALID_ARGUMENT = -1,
    RIN_RESOURCE_SUBJECT_DENIED = -2
} RinResourceSubjectStatus;

/* A product boundary must build this value from the current kernel process;
 * it must never copy a caller-supplied subject from user memory. */
RinResourceSubjectStatus rin_resource_subject_validate(
    const RinResourceSubjectV1* subject);

/* Revalidates the complete subject and its current capability set for exactly
 * one RinAppResource. Capability drops therefore invalidate existing data
 * operations even when a broker handle remains open for cleanup. */
RinResourceSubjectStatus rin_resource_subject_authorize(
    const RinResourceSubjectV1* subject,
    uint32_t resource);

#if defined(__cplusplus)
static_assert(sizeof(RinResourceSubjectV1) == 80u,
              "RinResourceSubjectV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinResourceSubjectV1) == 80u,
               "RinResourceSubjectV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_API_RIN_RESOURCE_SUBJECT_H */
