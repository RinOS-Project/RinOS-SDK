/* SPDX-License-Identifier: MIT */
/* Public helpers for the per-session input-method preference syscalls. */
#ifndef RIN_SDK_INPUT_METHOD_H
#define RIN_SDK_INPUT_METHOD_H

#include <rin/contract_abi.h>
#include <rin/syscall_abi.h>
#include <sys/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int rin_input_method_get(RinInputMethodConfigV1* config) {
    if (config == 0) return (int)RIN_RESULT_INVALID_ARGUMENT;
    return (int)syscall(RIN_SYS_INPUT_METHOD_GET, config);
}

static inline int rin_input_method_set(
    const RinInputMethodConfigV1* config) {
    if (config == 0) return (int)RIN_RESULT_INVALID_ARGUMENT;
    return (int)syscall(RIN_SYS_INPUT_METHOD_SET, config);
}

#ifdef __cplusplus
}
#endif

#endif /* RIN_SDK_INPUT_METHOD_H */
