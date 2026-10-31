/* SPDX-License-Identifier: MIT */
/* Fixed-width ABI for POSIX spawn file actions. */

#ifndef RIN_SDK_POSIX_SPAWN_ABI_H
#define RIN_SDK_POSIX_SPAWN_ABI_H

#include <stdint.h>

#define RIN_POSIX_SPAWN_ACTION_ABI_VERSION 1u
#define RIN_POSIX_SPAWN_ACTION_MAX 32u
#define RIN_POSIX_SPAWN_ACTION_PATH_MAX 256u

/* Values intentionally match the public posix_spawn_file_actions API. */
#define RIN_POSIX_SPAWN_ACTION_OPEN  1u
#define RIN_POSIX_SPAWN_ACTION_CLOSE 2u
#define RIN_POSIX_SPAWN_ACTION_DUP2  3u
#define RIN_POSIX_SPAWN_ACTION_ATTRIBUTES 4u

#define RIN_POSIX_SPAWN_ATTRIBUTE_ABI_VERSION 1u
#define RIN_POSIX_SPAWN_ATTRIBUTE_SETPGROUP  UINT32_C(0x00000001)
#define RIN_POSIX_SPAWN_ATTRIBUTE_SETSIGDEF  UINT32_C(0x00000002)
#define RIN_POSIX_SPAWN_ATTRIBUTE_SETSIGMASK UINT32_C(0x00000004)
#define RIN_POSIX_SPAWN_ATTRIBUTE_SETSCHEDPARAM UINT32_C(0x00000008)
#define RIN_POSIX_SPAWN_ATTRIBUTE_SETSCHEDULER  UINT32_C(0x00000010)

/* Attribute payload carried by the reserved action record.  Keeping the
 * payload fixed-width lets native and compat32 spawn share the same copy-in
 * and validation path without widening the six-word syscall. */
typedef struct RinPosixSpawnAttributesV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t reserved0;
    uint32_t flags;
    uint32_t process_group;
    uint64_t signal_default;
    uint64_t signal_mask;
    int32_t sched_policy;
    int32_t sched_priority;
} RinPosixSpawnAttributesV1;

#if defined(__cplusplus)
static_assert(sizeof(RinPosixSpawnAttributesV1) == 40u,
              "RinPosixSpawnAttributesV1 ABI size");
#else
_Static_assert(sizeof(RinPosixSpawnAttributesV1) == 40u,
               "RinPosixSpawnAttributesV1 ABI size");
#endif

/* Spawn attributes carried in the existing syscall flags word.  SETSID is
 * kept in the scalar word because it has no payload.  Attributes with data
 * use the reserved attributes action record below, preserving the fixed
 * six-word syscall ABI.  Scheduler policy/priority use the same payload and
 * are applied by the child scheduler owner before activation. */
#define RIN_POSIX_SPAWN_FLAG_RESETIDS 1u
#define RIN_POSIX_SPAWN_FLAG_SETSID 2u

/* The record contains no target-width long or pointer.  The kernel receives
 * an array of these records through the six-word spawn syscall and validates
 * every unused byte in path before mutating the blocked child. */
typedef struct RinPosixSpawnFileActionV1 {
    uint32_t type;
    int32_t fd;
    int32_t src_fd;
    uint32_t oflag;
    uint32_t mode;
    char path[RIN_POSIX_SPAWN_ACTION_PATH_MAX];
} RinPosixSpawnFileActionV1;

#if defined(__cplusplus)
static_assert(sizeof(RinPosixSpawnFileActionV1) == 276u,
              "RinPosixSpawnFileActionV1 ABI size");
#else
_Static_assert(sizeof(RinPosixSpawnFileActionV1) == 276u,
               "RinPosixSpawnFileActionV1 ABI size");
#endif

#endif /* RIN_SDK_POSIX_SPAWN_ABI_H */
