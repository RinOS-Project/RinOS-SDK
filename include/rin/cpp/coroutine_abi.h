/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_CPP_COROUTINE_ABI_H
#define RIN_SDK_CPP_COROUTINE_ABI_H

#include <stdint.h>

/* Shared fixed-size prefix used by RinCompiler-generated coroutine frames and
 * the libcxx coroutine_handle runtime.  All slots are 64-bit so offsets stay
 * identical for i686 and x86_64 targets.  Function addresses are stored as
 * zero-extended uintptr_t values. */
#define RIN_CXX_CORO_FRAME_HEADER_SIZE 64u
#define RIN_CXX_CORO_FRAME_ALIGNMENT 8u
#define RIN_CXX_CORO_FRAME_MAGIC UINT64_C(0x52494e434f524f31)
#define RIN_CXX_CORO_FRAME_DONE UINT64_C(1)
#define RIN_CXX_CORO_FRAME_NOOP UINT64_C(2)

typedef struct RinCxxCoroutineFrameHeader {
    uint64_t magic;
    uint64_t frame_size;
    uint64_t promise_offset;
    uint64_t state;
    uint64_t resume_entry;
    uint64_t destroy_entry;
    uint64_t flags;
    uint64_t reserved;
} RinCxxCoroutineFrameHeader;

#endif
