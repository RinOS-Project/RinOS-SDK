/* SPDX-License-Identifier: MIT */
/* Shared dynamic-loader policy bits.  These values are the wire contract
 * used by libc and the native RIN64 loader; they intentionally match POSIX
 * dlfcn.h without making kernel code include a libc header. */
#ifndef RIN_SDK_DYNLINK_FLAGS_H
#define RIN_SDK_DYNLINK_FLAGS_H

#define RIN_DYNLINK_FLAG_LAZY      0x0001u
#define RIN_DYNLINK_FLAG_NOW       0x0002u
#define RIN_DYNLINK_FLAG_NOLOAD    0x0004u
#define RIN_DYNLINK_FLAG_DEEPBIND  0x0008u
#define RIN_DYNLINK_FLAG_GLOBAL    0x0100u
#define RIN_DYNLINK_FLAG_NODELETE  0x1000u

#define RIN_DYNLINK_FLAG_PUBLIC_MASK \
    (RIN_DYNLINK_FLAG_LAZY | RIN_DYNLINK_FLAG_NOW | \
     RIN_DYNLINK_FLAG_NOLOAD | RIN_DYNLINK_FLAG_DEEPBIND | \
     RIN_DYNLINK_FLAG_GLOBAL | RIN_DYNLINK_FLAG_NODELETE)

#endif /* RIN_SDK_DYNLINK_FLAGS_H */
