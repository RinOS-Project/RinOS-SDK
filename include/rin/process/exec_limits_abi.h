/* SPDX-License-Identifier: MIT */
/* libcと各kernel ABIが共有するexec引数上限。 */

#ifndef RIN_SDK_PROCESS_EXEC_LIMITS_ABI_H
#define RIN_SDK_PROCESS_EXEC_LIMITS_ABI_H

#define RIN_EXEC_LIMITS_ABI_VERSION 1u

/* 文字列容量には終端NULを含む。vectorの件数には終端null pointerを含まない。
 * ARG_MAXはargvとenvpの文字列を各終端NUL込みで数え、実行pathは別上限とする。 */
#define RIN_EXEC_ARGUMENT_PATH_MAX 512u
#define RIN_EXEC_ARGUMENT_COUNT_MAX 64u
#define RIN_EXEC_ENVIRONMENT_COUNT_MAX 128u
#define RIN_EXEC_ARGUMENT_STRING_MAX 4096u
#define RIN_EXEC_ARGUMENT_ENV_BYTES_MAX (256u * 1024u)

#endif /* RIN_SDK_PROCESS_EXEC_LIMITS_ABI_H */
