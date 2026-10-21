/* SPDX-License-Identifier: MIT */
#include <errno.h>
#include <stdlib.h>

#include <rin/file_portal.h>
#include <rin/fs.h>

int main(int argc, char** argv)
{
    char* end = NULL;
    unsigned long long parsed_grant;
    uint64_t handle = RIN_HANDLE_INVALID;
    RinFsPortalOpenV1 open_request = {0};
    RinFileIoV1 read_request = {0};
    RinFile file = RIN_HANDLE_INVALID;
    unsigned char buffer[256];
    uint64_t transferred = 0u;
    RinResult result;

    if (argc != 2 || !argv[1] || argv[1][0] == '-') return 2;
    errno = 0;
    parsed_grant = strtoull(argv[1], &end, 10);
    if (errno != 0 || !end || *end != '\0' || parsed_grant == 0u)
        return 2;

    open_request.struct_size = sizeof(open_request);
    open_request.version = RIN_SDK_STRUCT_VERSION_1;
    open_request.grant_id = (uint64_t)parsed_grant;
    open_request.object_type = RIN_FS_PORTAL_OPEN_FILE;
    open_request.rights = RIN_FILE_PORTAL_RIGHT_READ;
    result = rin_fs_portal_open_v1(&open_request, &handle);
    if (result != RIN_SUCCESS) return 1;
    file = (RinFile)handle;

    read_request.struct_size = sizeof(read_request);
    read_request.version = RIN_SDK_STRUCT_VERSION_1;
    read_request.file = file;
    read_request.buffer.address = (uint64_t)(uintptr_t)buffer;
    read_request.buffer.size = sizeof(buffer);
    read_request.completion_event = RIN_HANDLE_INVALID;
    result = rin_file_read_v1(&read_request, &transferred);
    if (rin_file_close_v1(file) != RIN_SUCCESS && result == RIN_SUCCESS)
        result = RIN_ERROR_IO;
    return result == RIN_SUCCESS && transferred <= sizeof(buffer) ? 0 : 1;
}
