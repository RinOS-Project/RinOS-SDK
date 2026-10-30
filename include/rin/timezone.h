/* SPDX-License-Identifier: MIT */
/* timezone設定API */
#ifndef RIN_SDK_TIMEZONE_H
#define RIN_SDK_TIMEZONE_H

#include <rin/contract_abi.h>
#include <rin/syscall_abi.h>
#include <sys/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GETはmachine-wide既定値、GET_EFFECTIVEは呼出processに対する有効値を返す。
 * SETはinfo->generationで更新競合を検出する。machine-wide更新には
 * RIN_TIME_ZONE_INFO_FLAG_SYSTEM_DEFAULTを指定し、それ以外は呼出userの
 * overrideを要求する。結果はsyscallのRinResultCode。 */
static inline RinResultCode rin_timezone_info_get_v1(
    RinTimeZoneInfoV1* info)
{
    if (info == 0) return RIN_RESULT_INVALID_ARGUMENT;
    return (RinResultCode)syscall(RIN_SYS_TIMEZONE_GET, info);
}

static inline RinResultCode rin_timezone_info_get_effective_v1(
    RinTimeZoneInfoV1* info)
{
    if (info == 0) return RIN_RESULT_INVALID_ARGUMENT;
    return (RinResultCode)syscall(RIN_SYS_TIMEZONE_GET_EFFECTIVE, info);
}

static inline RinResultCode rin_timezone_info_set_v1(
    const RinTimeZoneInfoV1* info)
{
    if (info == 0) return RIN_RESULT_INVALID_ARGUMENT;
    return (RinResultCode)syscall(RIN_SYS_TIMEZONE_SET, info);
}

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_TIMEZONE_H */
