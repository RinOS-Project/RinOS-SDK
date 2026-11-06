/* SPDX-License-Identifier: MIT */
#ifndef RIN_FS_FUSE_EXTENSION_V1_H
#define RIN_FS_FUSE_EXTENSION_V1_H

#include <stdint.h>

/* RinOS FUSE拡張要求。Linux FUSE標準opcodeと衝突しない番号を使う。
 * daemonは照合と変更を同一transactionで行うこと。拡張未対応時は
 * ENOSYSを返し、kernelは通常のRENAME/UNLINKへ置き換えてはならない。 */
#define RIN_FUSE_EXPECTED_RENAME_OPCODE UINT32_C(0x52494e52)
#define RIN_FUSE_EXPECTED_RENAME_VERSION UINT32_C(1)
#define RIN_FUSE_EXPECTED_UNLINK_OPCODE UINT32_C(0x52494e55)
#define RIN_FUSE_EXPECTED_UNLINK_VERSION UINT32_C(1)

#define RIN_FUSE_EXPECTED_RENAME_NOREPLACE UINT32_C(1)
#define RIN_FUSE_EXPECTED_RENAME_EXCHANGE UINT32_C(2)
#define RIN_FUSE_EXPECTED_RENAME_WHITEOUT UINT32_C(4)

/* Version 1はstruct_sizeの一致、reserved=0、未知flagなしを要求する。
 * payloadの後にNUL終端したold/new nameが続く。request headerのnodeidは
 * old parent。daemonはentryのnode ID・inode・generationを同一transaction
 * 内で照合してrenameする。不一致はENOENTとし、directoryを変更しない。 */
typedef struct RinFuseExpectedRenameV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t reserved;
    uint64_t new_directory_node;
    uint64_t expected_node_id;
    uint64_t expected_inode;
    uint64_t expected_generation;
} RinFuseExpectedRenameV1;

/* daemonはentry identityを同一transaction内で照合して削除する。
 * 不一致はENOENTとし、通常のUNLINKへfallbackしてはならない。 */
typedef struct RinFuseExpectedUnlinkV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t reserved0;
    uint32_t reserved1;
    uint64_t expected_node_id;
    uint64_t expected_inode;
    uint64_t expected_generation;
} RinFuseExpectedUnlinkV1;

#endif /* RIN_FS_FUSE_EXTENSION_V1_H */
