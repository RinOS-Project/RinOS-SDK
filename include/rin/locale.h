/* SPDX-License-Identifier: MIT */
/* セッション単位のlocale設定API */
#ifndef RIN_SDK_LOCALE_H
#define RIN_SDK_LOCALE_H

#include <rin/contract_abi.h>
#include <rin/syscall_abi.h>
#include <sys/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GET／SETは呼出processに紐づく前景sessionを対象とする。SETは
 * info->generationで更新競合を検出する。system-wide設定には
 * RIN_LOCALE_INFO_FLAG_SYSTEM_DEFAULTを指定し、認可拒否時はsyscallの
 * RinResultCodeを返す。 */
static inline RinResultCode rin_locale_info_get_v1(RinLocaleInfoV1* info)
{
    if (info == 0) return RIN_RESULT_INVALID_ARGUMENT;
    return (RinResultCode)syscall(RIN_SYS_LOCALE_GET, info);
}

static inline RinResultCode rin_locale_info_set_v1(
    const RinLocaleInfoV1* info)
{
    if (info == 0) return RIN_RESULT_INVALID_ARGUMENT;
    return (RinResultCode)syscall(RIN_SYS_LOCALE_SET, info);
}

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_LOCALE_H */
