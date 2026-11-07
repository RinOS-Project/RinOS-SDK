/* SPDX-License-Identifier: MIT */
#ifndef RIN_FS_FUSE_EXTENSION_V2_H
#define RIN_FS_FUSE_EXTENSION_V2_H

#include <stddef.h>
#include <stdint.h>

#include <rin/fs/fuse_extension_v1.h>

/* Version 2 binds both sides of a rename, including an explicitly absent
 * destination, to the state observed before VFS mount-route publication. */
#define RIN_FUSE_EXPECTED_RENAME_VERSION_2 UINT32_C(2)

typedef struct RinFuseExpectedRenameV2 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t reserved;
    uint64_t new_directory_node;
    uint64_t expected_node_id;
    uint64_t expected_inode;
    uint64_t expected_generation;
    uint64_t expected_destination_node_id;
    uint64_t expected_destination_inode;
    uint64_t expected_destination_generation;
    uint32_t destination_present;
    uint32_t reserved_v2;
} RinFuseExpectedRenameV2;

typedef struct RinFuseExpectedRenameRequestV2 {
    uint64_t old_directory_node;
    uint64_t new_directory_node;
    uint64_t expected_node_id;
    uint64_t expected_inode;
    uint64_t expected_generation;
    uint64_t expected_destination_node_id;
    uint64_t expected_destination_inode;
    uint64_t expected_destination_generation;
    uint32_t destination_present;
    uint32_t flags;
    const char* old_name;
    size_t old_name_length;
    const char* new_name;
    size_t new_name_length;
} RinFuseExpectedRenameRequestV2;

#endif /* RIN_FS_FUSE_EXTENSION_V2_H */
