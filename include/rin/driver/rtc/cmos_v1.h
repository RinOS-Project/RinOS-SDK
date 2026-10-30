/* SPDX-License-Identifier: MIT */
#ifndef RIN_DRIVER_RTC_CMOS_V1_H
#define RIN_DRIVER_RTC_CMOS_V1_H

#include <stdint.h>

#include <rin/driver/rtc/backend_v1.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_RTC_CMOS_VERSION UINT32_C(0x00010000)
#define RIN_RTC_CMOS_DEFAULT_UIP_POLLS UINT32_C(100000)
#define RIN_RTC_CMOS_MAX_UIP_POLLS UINT32_C(1000000)

typedef int (*RinRtcCmosReadRegisterV1)(
    void* context, uint8_t reg, uint8_t* value);
/* A nonzero result does not prove the device left the register unchanged.
 * Callers must assume the write may have committed and restore or reread. */
typedef int (*RinRtcCmosWriteRegisterV1)(
    void* context, uint8_t reg, uint8_t value);
typedef void (*RinRtcCmosLockV1)(void* context);
typedef void (*RinRtcCmosUnlockV1)(void* context);

typedef struct RinRtcCmosConfigV1 {
    uint32_t struct_size;
    uint32_t version;
    void* context;
    RinRtcCmosReadRegisterV1 read_register;
    RinRtcCmosWriteRegisterV1 write_register;
    RinRtcCmosLockV1 lock;
    RinRtcCmosUnlockV1 unlock;
    uint32_t uip_poll_limit;
    uint8_t century_register;
    uint8_t has_century_register;
    uint16_t reserved0;
    uint64_t reserved[3];
} RinRtcCmosConfigV1;

typedef struct RinRtcCmosBackendV1 {
    uint32_t struct_size;
    uint32_t version;
    RinRtcCmosConfigV1 config;
    RinRtcBackendV1 backend;
    uint64_t reserved[2];
} RinRtcCmosBackendV1;

RinRtcCorrectionStatus rin_rtc_cmos_backend_initialize(
    RinRtcCmosBackendV1* product, const RinRtcCmosConfigV1* config);
RinRtcCorrectionStatus rin_rtc_cmos_backend_destroy(
    RinRtcCmosBackendV1* product);

#if defined(__cplusplus)
static_assert(sizeof(RinRtcBackendV1) ==
                  (sizeof(void*) == 8u ? 48u : 36u),
              "RTC backend callback ABI drift");
static_assert(sizeof(RinRtcCmosConfigV1) ==
                  (sizeof(void*) == 8u ? 80u : 60u),
              "CMOS config ABI drift");
static_assert(sizeof(RinRtcCmosBackendV1) ==
                  (sizeof(void*) == 8u ? 152u : 120u),
              "CMOS backend ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinRtcBackendV1) ==
                   (sizeof(void*) == 8u ? 48u : 36u),
               "RTC backend callback ABI drift");
_Static_assert(sizeof(RinRtcCmosConfigV1) ==
                   (sizeof(void*) == 8u ? 80u : 60u),
               "CMOS config ABI drift");
_Static_assert(sizeof(RinRtcCmosBackendV1) ==
                   (sizeof(void*) == 8u ? 152u : 120u),
               "CMOS backend ABI drift");
#endif

#ifdef __cplusplus
}
#endif

#endif /* RIN_DRIVER_RTC_CMOS_V1_H */
