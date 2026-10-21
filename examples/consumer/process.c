/* SPDX-License-Identifier: MIT */
#include <rin/base.h>

int main(int argc, char** argv)
{
    RinProcessSpawnV1 request = {0};
    RinProcess child = RIN_HANDLE_INVALID;
    int32_t exit_code = 0;
    RinResult result;

    if (argc != 2 || !argv[1]) return 2;
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.image.address = (uint64_t)(uintptr_t)argv[1];
    request.image.size = 0u;
    while (argv[1][request.image.size] != '\0') ++request.image.size;
    request.arguments = (RinSliceV1){0u, 0u};
    request.environment = (RinSliceV1){0u, 0u};
    request.capability_mask = 0u;

    result = rin_process_spawn_v1(&request, &child);
    if (result != RIN_SUCCESS) return 1;
    result = rin_process_wait_v1(child, RIN_SDK_INFINITE, &exit_code);
    if (rin_object_close_v1((RinObject)child) != RIN_SUCCESS &&
        result == RIN_SUCCESS)
        result = RIN_ERROR_IO;
    return result == RIN_SUCCESS ? exit_code : 1;
}
