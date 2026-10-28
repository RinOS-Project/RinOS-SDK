/* SPDX-License-Identifier: MIT */
/* Public exec argument limits shared by libc and every kernel ABI. */

#ifndef RIN_SDK_PROCESS_EXEC_LIMITS_ABI_H
#define RIN_SDK_PROCESS_EXEC_LIMITS_ABI_H

#define RIN_EXEC_LIMITS_ABI_VERSION 1u

/* String capacities include the terminating NUL byte. Vector counts exclude
 * the terminating null pointer. ARG_MAX counts argv and envp string bytes,
 * including each terminator; the executable path has its own bound. */
#define RIN_EXEC_ARGUMENT_PATH_MAX 512u
#define RIN_EXEC_ARGUMENT_COUNT_MAX 64u
#define RIN_EXEC_ENVIRONMENT_COUNT_MAX 128u
#define RIN_EXEC_ARGUMENT_STRING_MAX 4096u
#define RIN_EXEC_ARGUMENT_ENV_BYTES_MAX (256u * 1024u)

#endif /* RIN_SDK_PROCESS_EXEC_LIMITS_ABI_H */
