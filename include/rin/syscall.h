// SPDX-License-Identifier: MIT
/* Public declaration shared by the typed syscall wrappers. The syscall ABI
 * header defines numbers; libc feature-test macros may hide the function
 * prototype while an SDK inline wrapper is parsed. */
#ifndef RIN_SDK_SYSCALL_H
#define RIN_SDK_SYSCALL_H

#ifdef __cplusplus
extern "C" long int syscall(long int syscall_number, ...) noexcept;
#else
long int syscall(long int syscall_number, ...);
#endif

#endif /* RIN_SDK_SYSCALL_H */
