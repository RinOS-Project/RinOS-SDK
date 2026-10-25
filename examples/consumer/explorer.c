/* SPDX-License-Identifier: MIT */
#include <stdio.h>
#include <string.h>

#include <rin/base.h>

typedef struct ExampleEntry {
    const char* id;
    const char* package_id;
    const char* api;
    const char* capability;
} ExampleEntry;

static const ExampleEntry examples[] = {
    {"hello", "org.rinos.sdk.hello", "rin_log", "none"},
    {"process", "org.rinos.sdk.process",
     "rin_process_spawn_v1 / rin_process_wait_v1", "process-control"},
    {"channel", "org.rinos.sdk.channel",
     "rin_channel_create_v1 / send_v1 / receive_v1", "none"},
    {"file-portal", "org.rinos.sdk.file-portal",
     "rin_fs_portal_open_v1 / rin_file_read_v1", "file-portal"},
    {"window", "org.rinos.sdk.window",
     "rin_window_create_v1 / rin_window_next_event_v1", "gui-control"},
    {"media", "org.rinos.sdk.media",
     "rin_media_open_v1 / image and audio metadata/decode",
     "none"},
    {"keyring", "org.rinos.sdk.keyring",
     "RIN_KEYRING_OP_STATUS / authenticated Unix service identity", "none"},
};

static const ExampleEntry* find_example(const char* id)
{
    uint32_t index;
    if (id == NULL) return NULL;
    for (index = 0u; index < sizeof(examples) / sizeof(examples[0]); ++index) {
        if (strcmp(id, examples[index].id) == 0) return &examples[index];
    }
    return NULL;
}

static int list_examples(void)
{
    uint32_t index;
    puts("RinOS SDK examples");
    puts("------------------");
    for (index = 0u; index < sizeof(examples) / sizeof(examples[0]); ++index) {
        printf("%-12s  %-28s  capability: %s\n",
               examples[index].id, examples[index].api,
               examples[index].capability);
        printf("              package: %s\n", examples[index].package_id);
    }
    puts("Run: sdk-explorer --run <example-id> <installed-image-path>");
    puts("The child receives no capabilities from the Explorer process.");
    return 0;
}

static int run_example(const char* id, const char* image_path)
{
    const ExampleEntry* entry = find_example(id);
    RinProcessSpawnV1 request = {0};
    RinProcess child = RIN_HANDLE_INVALID;
    RinResult result;
    int32_t exit_code = 0;

    if (entry == NULL || image_path == NULL || image_path[0] == '\0') {
        puts("unknown example or empty image path");
        return 2;
    }

    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.image.address = (uint64_t)(uintptr_t)image_path;
    request.image.size = strlen(image_path);
    request.capability_mask = 0u;

    result = rin_process_spawn_v1(&request, &child);
    if (result != RIN_SUCCESS) {
        printf("process spawn failed: %d\n", (int)result);
        return 1;
    }

    printf("Running %s (%s)\n", entry->id, entry->package_id);
    result = rin_process_wait_v1(child, RIN_SDK_INFINITE, &exit_code);
    if (rin_object_close_v1((RinObject)child) != RIN_SUCCESS &&
        result == RIN_SUCCESS)
        result = RIN_ERROR_IO;
    if (result != RIN_SUCCESS) {
        printf("process wait/close failed: %d\n", (int)result);
        return 1;
    }
    return exit_code < 0 || exit_code > 255 ? 1 : (int)exit_code;
}

int main(int argc, char** argv)
{
    if (argc == 1 || (argc == 2 && strcmp(argv[1], "--list") == 0))
        return list_examples();
    if (argc == 4 && strcmp(argv[1], "--run") == 0)
        return run_example(argv[2], argv[3]);
    puts("usage: sdk-explorer [--list | --run <example-id> <image-path>]");
    return 2;
}
