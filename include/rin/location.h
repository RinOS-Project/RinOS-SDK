/* SPDX-License-Identifier: MIT */
/* Fixed ABI for privacy-gated location fixes. */

#ifndef RIN_API_RIN_LOCATION_H
#define RIN_API_RIN_LOCATION_H

#include <stdint.h>

#define RIN_LOCATION_FIX_VERSION UINT32_C(0x00010000)
#define RIN_LOCATION_COMMAND_GET_FIX UINT32_C(1)

#define RIN_LOCATION_FIX_VALID_ALTITUDE UINT32_C(0x00000001)
#define RIN_LOCATION_FIX_VALID_SPEED UINT32_C(0x00000002)
#define RIN_LOCATION_FIX_VALID_HEADING UINT32_C(0x00000004)
#define RIN_LOCATION_FIX_KNOWN_FLAGS                                      \
    (RIN_LOCATION_FIX_VALID_ALTITUDE | RIN_LOCATION_FIX_VALID_SPEED |     \
     RIN_LOCATION_FIX_VALID_HEADING)

typedef enum RinLocationSourceV1 {
    RIN_LOCATION_SOURCE_NONE = 0,
    RIN_LOCATION_SOURCE_GNSS = 1,
    RIN_LOCATION_SOURCE_NETWORK = 2,
    RIN_LOCATION_SOURCE_FUSED = 3
} RinLocationSourceV1;

/* Coordinates use signed degrees times 10^7. Optional values and their
 * accuracy are zero when the corresponding validity flag is absent. */
typedef struct RinLocationFixV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t monotonic_time_ns;
    int64_t latitude_e7;
    int64_t longitude_e7;
    int64_t altitude_mm;
    uint32_t horizontal_accuracy_mm;
    uint32_t vertical_accuracy_mm;
    uint32_t speed_mm_per_second;
    uint32_t heading_millidegrees;
    uint32_t source;
    uint32_t flags;
} RinLocationFixV1;

static inline int rin_location_fix_v1_is_valid(const RinLocationFixV1* fix)
{
    if (fix == 0 || fix->struct_size != sizeof(*fix) ||
        fix->version != RIN_LOCATION_FIX_VERSION ||
        fix->monotonic_time_ns == 0u ||
        fix->latitude_e7 < INT64_C(-900000000) ||
        fix->latitude_e7 > INT64_C(900000000) ||
        fix->longitude_e7 < INT64_C(-1800000000) ||
        fix->longitude_e7 > INT64_C(1800000000) ||
        fix->horizontal_accuracy_mm == 0u ||
        fix->source < RIN_LOCATION_SOURCE_GNSS ||
        fix->source > RIN_LOCATION_SOURCE_FUSED ||
        (fix->flags & ~RIN_LOCATION_FIX_KNOWN_FLAGS) != 0u)
        return 0;
    if ((fix->flags & RIN_LOCATION_FIX_VALID_ALTITUDE) == 0u) {
        if (fix->altitude_mm != 0 || fix->vertical_accuracy_mm != 0u)
            return 0;
    } else if (fix->vertical_accuracy_mm == 0u) {
        return 0;
    }
    if ((fix->flags & RIN_LOCATION_FIX_VALID_SPEED) == 0u &&
        fix->speed_mm_per_second != 0u)
        return 0;
    if ((fix->flags & RIN_LOCATION_FIX_VALID_HEADING) == 0u) {
        if (fix->heading_millidegrees != 0u) return 0;
    } else if (fix->heading_millidegrees >= 360000u) {
        return 0;
    }
    return 1;
}

#if defined(__cplusplus)
static_assert(sizeof(RinLocationFixV1) == 64u,
              "RinLocationFixV1 ABI size changed");
#else
_Static_assert(sizeof(RinLocationFixV1) == 64u,
               "RinLocationFixV1 ABI size changed");
#endif

#endif /* RIN_API_RIN_LOCATION_H */
