/* SPDX-License-Identifier: MIT */
#ifndef RIN_FS_FUSE_EXTENSION_DISPATCH_V2_H
#define RIN_FS_FUSE_EXTENSION_DISPATCH_V2_H

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <rin/fs/fuse_extension_dispatch_v1.h>
#include <rin/fs/fuse_extension_v2.h>

#define RIN_FUSE_EXTENSION_DISPATCH_VERSION_2 UINT32_C(2)

typedef int32_t (*RinFuseExpectedRenameHandlerV2)(
    void* context, const RinFuseExpectedRenameRequestV2* request);

typedef struct RinFuseExtensionDispatchOpsV2 {
    uint32_t struct_size;
    uint32_t version;
    RinFuseExpectedRenameHandlerV1 expected_rename_v1;
    RinFuseExpectedRenameHandlerV2 expected_rename_v2;
    RinFuseExpectedUnlinkHandlerV1 expected_unlink;
} RinFuseExtensionDispatchOpsV2;

/* Dispatch both wire versions. Version 2 callbacks must compare the source
 * identity and destination identity or absence in the same backend
 * transaction that performs the rename. */
static inline int32_t rin_fuse_extension_dispatch_v2(
    uint32_t opcode, uint64_t parent_node, const void* payload,
    size_t payload_size, const RinFuseExtensionDispatchOpsV2* operations,
    void* context)
{
    if (opcode != RIN_FUSE_EXPECTED_RENAME_OPCODE &&
        opcode != RIN_FUSE_EXPECTED_UNLINK_OPCODE)
        return -ENOSYS;
    if (operations == NULL) return -ENOSYS;
    if (operations->struct_size != sizeof(*operations) ||
        operations->version != RIN_FUSE_EXTENSION_DISPATCH_VERSION_2)
        return -EINVAL;
    if (opcode == RIN_FUSE_EXPECTED_UNLINK_OPCODE) {
        RinFuseExtensionDispatchOpsV1 v1;
        v1.struct_size = sizeof(v1);
        v1.version = RIN_FUSE_EXTENSION_DISPATCH_VERSION;
        v1.expected_rename = NULL;
        v1.expected_unlink = operations->expected_unlink;
        return rin_fuse_extension_dispatch_v1(
            opcode, parent_node, payload, payload_size, &v1, context);
    }
    if (payload == NULL || payload_size < 8u || parent_node == 0u)
        return -EINVAL;
    {
        uint32_t version;
        memcpy(&version, (const uint8_t*)payload + sizeof(uint32_t),
               sizeof(version));
        if (version == RIN_FUSE_EXPECTED_RENAME_VERSION) {
            RinFuseExtensionDispatchOpsV1 v1;
            v1.struct_size = sizeof(v1);
            v1.version = RIN_FUSE_EXTENSION_DISPATCH_VERSION;
            v1.expected_rename = operations->expected_rename_v1;
            v1.expected_unlink = operations->expected_unlink;
            return rin_fuse_extension_dispatch_v1(
                opcode, parent_node, payload, payload_size, &v1, context);
        }
        if (version == RIN_FUSE_EXPECTED_RENAME_VERSION_2) {
            RinFuseExpectedRenameV2 input;
            RinFuseExpectedRenameRequestV2 request;
            size_t offset = sizeof(input);
            if (payload_size < sizeof(input)) return -EINVAL;
            memcpy(&input, payload, sizeof(input));
            if (input.struct_size != sizeof(input) ||
                input.version != RIN_FUSE_EXPECTED_RENAME_VERSION_2 ||
                input.reserved != 0u || input.reserved_v2 != 0u ||
                input.new_directory_node == 0u ||
                input.expected_node_id == 0u ||
                input.expected_inode == 0u ||
                input.destination_present > 1u ||
                (input.flags & ~(RIN_FUSE_EXPECTED_RENAME_NOREPLACE |
                                 RIN_FUSE_EXPECTED_RENAME_EXCHANGE |
                                 RIN_FUSE_EXPECTED_RENAME_WHITEOUT)) != 0u ||
                ((input.flags & RIN_FUSE_EXPECTED_RENAME_NOREPLACE) != 0u &&
                 (input.flags & RIN_FUSE_EXPECTED_RENAME_EXCHANGE) != 0u) ||
                ((input.flags & RIN_FUSE_EXPECTED_RENAME_EXCHANGE) != 0u &&
                 (input.flags & RIN_FUSE_EXPECTED_RENAME_WHITEOUT) != 0u) ||
                ((input.flags & RIN_FUSE_EXPECTED_RENAME_EXCHANGE) != 0u &&
                 input.destination_present == 0u) ||
                (input.destination_present == 0u &&
                 (input.expected_destination_node_id != 0u ||
                  input.expected_destination_inode != 0u ||
                  input.expected_destination_generation != 0u)) ||
                (input.destination_present != 0u &&
                 (input.expected_destination_node_id == 0u ||
                  input.expected_destination_inode == 0u)))
                return -EINVAL;
            memset(&request, 0, sizeof(request));
            request.old_directory_node = parent_node;
            request.new_directory_node = input.new_directory_node;
            request.expected_node_id = input.expected_node_id;
            request.expected_inode = input.expected_inode;
            request.expected_generation = input.expected_generation;
            request.expected_destination_node_id =
                input.expected_destination_node_id;
            request.expected_destination_inode =
                input.expected_destination_inode;
            request.expected_destination_generation =
                input.expected_destination_generation;
            request.destination_present = input.destination_present;
            request.flags = input.flags;
            if (rin_fuse_extension_dispatch_name_v1(
                    (const uint8_t*)payload, payload_size, &offset,
                    &request.old_name, &request.old_name_length) != 0 ||
                rin_fuse_extension_dispatch_name_v1(
                    (const uint8_t*)payload, payload_size, &offset,
                    &request.new_name, &request.new_name_length) != 0 ||
                offset != payload_size)
                return -EINVAL;
            if (operations->expected_rename_v2 == NULL) return -ENOSYS;
            return rin_fuse_extension_dispatch_result_v1(
                operations->expected_rename_v2(context, &request));
        }
    }
    return -EINVAL;
}

#endif /* RIN_FS_FUSE_EXTENSION_DISPATCH_V2_H */
