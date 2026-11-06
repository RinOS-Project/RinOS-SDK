/* SPDX-License-Identifier: MIT */
#ifndef RIN_FS_FUSE_EXTENSION_DISPATCH_V1_H
#define RIN_FS_FUSE_EXTENSION_DISPATCH_V1_H

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <rin/fs/fuse_extension_v1.h>

#define RIN_FUSE_EXTENSION_DISPATCH_VERSION UINT32_C(1)

/* name pointers refer to the request payload and are valid only for the
 * duration of the callback. The backend must compare identity and mutate the
 * directory entry in one transaction; a separate lookup followed by rename or
 * unlink is not sufficient. Return 0 or a negative errno value. */
typedef struct RinFuseExpectedRenameRequestV1 {
    uint64_t old_directory_node;
    uint64_t new_directory_node;
    uint64_t expected_node_id;
    uint64_t expected_inode;
    uint64_t expected_generation;
    uint32_t flags;
    const char* old_name;
    size_t old_name_length;
    const char* new_name;
    size_t new_name_length;
} RinFuseExpectedRenameRequestV1;

typedef struct RinFuseExpectedUnlinkRequestV1 {
    uint64_t directory_node;
    uint64_t expected_node_id;
    uint64_t expected_inode;
    uint64_t expected_generation;
    const char* name;
    size_t name_length;
} RinFuseExpectedUnlinkRequestV1;

typedef int32_t (*RinFuseExpectedRenameHandlerV1)(
    void* context, const RinFuseExpectedRenameRequestV1* request);
typedef int32_t (*RinFuseExpectedUnlinkHandlerV1)(
    void* context, const RinFuseExpectedUnlinkRequestV1* request);

typedef struct RinFuseExtensionDispatchOpsV1 {
    uint32_t struct_size;
    uint32_t version;
    RinFuseExpectedRenameHandlerV1 expected_rename;
    RinFuseExpectedUnlinkHandlerV1 expected_unlink;
} RinFuseExtensionDispatchOpsV1;

/* Parse exactly one NUL-terminated path component. Slash, empty, ".", and
 * ".." names are rejected so a backend cannot accidentally treat the input
 * as a path. */
static inline int rin_fuse_extension_dispatch_name_v1(
    const uint8_t* payload, size_t payload_size, size_t* offset,
    const char** name_out, size_t* length_out)
{
    size_t start;
    size_t cursor;

    if (payload == NULL || offset == NULL || name_out == NULL ||
        length_out == NULL || *offset >= payload_size)
        return -EINVAL;
    start = *offset;
    for (cursor = start; cursor < payload_size; ++cursor) {
        uint8_t value = payload[cursor];
        if (value == (uint8_t)'/') return -EINVAL;
        if (value == 0u) {
            size_t length = cursor - start;
            if (length == 0u ||
                (length == 1u && payload[start] == (uint8_t)'.') ||
                (length == 2u && payload[start] == (uint8_t)'.' &&
                 payload[start + 1u] == (uint8_t)'.'))
                return -EINVAL;
            *name_out = (const char*)(payload + start);
            *length_out = length;
            *offset = cursor + 1u;
            return 0;
        }
    }
    return -EINVAL;
}

static inline int32_t rin_fuse_extension_dispatch_result_v1(int32_t result)
{
    if (result > 0 || result < -4095) return -EIO;
    return result;
}

/* Dispatch a RinOS FUSE extension request. parent_node is the node ID from
 * the FUSE request header. Unknown or unimplemented operations return
 * -ENOSYS, which is the required reply for an unsupported extension. */
static inline int32_t rin_fuse_extension_dispatch_v1(
    uint32_t opcode, uint64_t parent_node, const void* payload,
    size_t payload_size, const RinFuseExtensionDispatchOpsV1* operations,
    void* context)
{
    const uint8_t* bytes = (const uint8_t*)payload;
    if (opcode == RIN_FUSE_EXPECTED_RENAME_OPCODE) {
        RinFuseExpectedRenameV1 input;
        RinFuseExpectedRenameRequestV1 request;
        size_t offset = sizeof(input);
        int32_t result;
        if (payload == NULL || parent_node == 0u ||
            payload_size < sizeof(input))
            return -EINVAL;
        memcpy(&input, payload, sizeof(input));
        if (input.struct_size != sizeof(input) ||
            input.version != RIN_FUSE_EXPECTED_RENAME_VERSION ||
            input.reserved != 0u || input.new_directory_node == 0u ||
            input.expected_node_id == 0u || input.expected_inode == 0u ||
            (input.flags & ~(RIN_FUSE_EXPECTED_RENAME_NOREPLACE |
                             RIN_FUSE_EXPECTED_RENAME_EXCHANGE |
                             RIN_FUSE_EXPECTED_RENAME_WHITEOUT)) != 0u ||
            ((input.flags & RIN_FUSE_EXPECTED_RENAME_NOREPLACE) != 0u &&
             (input.flags & RIN_FUSE_EXPECTED_RENAME_EXCHANGE) != 0u) ||
            ((input.flags & RIN_FUSE_EXPECTED_RENAME_EXCHANGE) != 0u &&
             (input.flags & RIN_FUSE_EXPECTED_RENAME_WHITEOUT) != 0u))
            return -EINVAL;
        memset(&request, 0, sizeof(request));
        request.old_directory_node = parent_node;
        request.new_directory_node = input.new_directory_node;
        request.expected_node_id = input.expected_node_id;
        request.expected_inode = input.expected_inode;
        request.expected_generation = input.expected_generation;
        request.flags = input.flags;
        if (rin_fuse_extension_dispatch_name_v1(
                bytes, payload_size, &offset, &request.old_name,
                &request.old_name_length) != 0 ||
            rin_fuse_extension_dispatch_name_v1(
                bytes, payload_size, &offset, &request.new_name,
                &request.new_name_length) != 0 ||
            offset != payload_size)
            return -EINVAL;
        if (operations == NULL) return -ENOSYS;
        if (operations->struct_size != sizeof(*operations) ||
            operations->version != RIN_FUSE_EXTENSION_DISPATCH_VERSION)
            return -EINVAL;
        if (operations->expected_rename == NULL) return -ENOSYS;
        result = operations->expected_rename(context, &request);
        return rin_fuse_extension_dispatch_result_v1(result);
    }
    if (opcode == RIN_FUSE_EXPECTED_UNLINK_OPCODE) {
        RinFuseExpectedUnlinkV1 input;
        RinFuseExpectedUnlinkRequestV1 request;
        size_t offset = sizeof(input);
        int32_t result;
        if (payload == NULL || parent_node == 0u ||
            payload_size < sizeof(input))
            return -EINVAL;
        memcpy(&input, payload, sizeof(input));
        if (input.struct_size != sizeof(input) ||
            input.version != RIN_FUSE_EXPECTED_UNLINK_VERSION ||
            input.reserved0 != 0u || input.reserved1 != 0u ||
            input.expected_node_id == 0u || input.expected_inode == 0u)
            return -EINVAL;
        memset(&request, 0, sizeof(request));
        request.directory_node = parent_node;
        request.expected_node_id = input.expected_node_id;
        request.expected_inode = input.expected_inode;
        request.expected_generation = input.expected_generation;
        if (rin_fuse_extension_dispatch_name_v1(
                bytes, payload_size, &offset, &request.name,
                &request.name_length) != 0 ||
            offset != payload_size)
            return -EINVAL;
        if (operations == NULL) return -ENOSYS;
        if (operations->struct_size != sizeof(*operations) ||
            operations->version != RIN_FUSE_EXTENSION_DISPATCH_VERSION)
            return -EINVAL;
        if (operations->expected_unlink == NULL) return -ENOSYS;
        result = operations->expected_unlink(context, &request);
        return rin_fuse_extension_dispatch_result_v1(result);
    }
    return -ENOSYS;
}

#endif /* RIN_FS_FUSE_EXTENSION_DISPATCH_V1_H */
