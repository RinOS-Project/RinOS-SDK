/* SPDX-License-Identifier: MIT */
#ifndef RIN_LEGACY_NDRV_V1_RUNTIME_ABI_H
#define RIN_LEGACY_NDRV_V1_RUNTIME_ABI_H

#include <stdint.h>

/* Legacy-only NDRV v1/v2 runtime table.  This namespace is intentionally not
 * reachable through the NDRV v3 public headers. */
#define RIN_NDRV_V1_RUNTIME_API_MAGIC UINT32_C(0x32495041) /* API2 */
#define RIN_NDRV_V1_RUNTIME_API_VERSION UINT16_C(2)
#define RIN_NDRV_V1_MAX_IRQS UINT32_C(16)
#define RIN_NDRV_V1_IRQ_ENABLE UINT32_C(1)
#define RIN_NDRV_V1_IRQ_DISABLE UINT32_C(2)
#define RIN_NDRV_V1_IRQ_FREE UINT32_C(3)
#define RIN_NDRV_V1_PENDING_IRQ_DISABLED UINT32_C(0x00000001)
#define RIN_NDRV_V1_API_OK 0
#define RIN_NDRV_V1_API_NOT_FOUND -2
#define RIN_NDRV_V1_API_NO_MEMORY -3
#define RIN_NDRV_V1_API_IO -4
#define RIN_NDRV_V1_API_BUSY -5
#define RIN_NDRV_V1_API_INVALID -7

#if defined(_MSC_VER)
#define RIN_NDRV_V1_ALIGN8 __declspec(align(8))
#elif defined(__GNUC__) || defined(__clang__)
#define RIN_NDRV_V1_ALIGN8 __attribute__((aligned(8)))
#else
#define RIN_NDRV_V1_ALIGN8
#endif

typedef struct RIN_NDRV_V1_ALIGN8 RinNdrvV1RuntimeApiV2 {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint64_t owner_cookie;
    uint64_t owner_generation;
    uint64_t map_mmio64;
    uint64_t unmap_mmio64;
    uint64_t irq_control;
    uint64_t monotonic_ms;
    uint64_t irq_request;
} RinNdrvV1RuntimeApiV2;

#define RIN_NDRV_V1_CHAR_EXTENSION_MAGIC UINT32_C(0x31415243) /* CAR1 */
#define RIN_NDRV_V1_CHAR_EXTENSION_VERSION UINT16_C(1)

typedef struct RIN_NDRV_V1_ALIGN8 RinNdrvV1CharExtensionV1 {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint64_t register_chardev;
    uint64_t reserved;
} RinNdrvV1CharExtensionV1;

#if defined(__cplusplus)
static_assert(sizeof(RinNdrvV1RuntimeApiV2) == 64u,
              "NDRV v1 runtime API ABI drift");
static_assert(alignof(RinNdrvV1RuntimeApiV2) == 8u,
              "NDRV v1 runtime API alignment drift");
static_assert(sizeof(RinNdrvV1CharExtensionV1) == 24u,
              "NDRV v1 CAR1 ABI drift");
static_assert(alignof(RinNdrvV1CharExtensionV1) == 8u,
              "NDRV v1 CAR1 alignment drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinNdrvV1RuntimeApiV2) == 64u,
               "NDRV v1 runtime API ABI drift");
_Static_assert(_Alignof(RinNdrvV1RuntimeApiV2) == 8u,
               "NDRV v1 runtime API alignment drift");
_Static_assert(sizeof(RinNdrvV1CharExtensionV1) == 24u,
               "NDRV v1 CAR1 ABI drift");
_Static_assert(_Alignof(RinNdrvV1CharExtensionV1) == 8u,
               "NDRV v1 CAR1 alignment drift");
#endif

#undef RIN_NDRV_V1_ALIGN8

#endif /* RIN_LEGACY_NDRV_V1_RUNTIME_ABI_H */
