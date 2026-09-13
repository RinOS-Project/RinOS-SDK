#ifndef RIN_SDK_FS_H
#define RIN_SDK_FS_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct RinFileOpenV1 {
    uint32_t struct_size;
    uint32_t version;
    RinDirectory directory;
    RinStringV1 path;
    uint32_t access;
    uint32_t disposition;
    uint32_t flags;
    uint32_t mode;
    uint64_t reserved[3];
} RinFileOpenV1;

typedef struct RinFileIoV1 {
    uint32_t struct_size;
    uint32_t version;
    RinFile file;
    uint64_t offset;
    RinSliceV1 buffer;
    RinEvent completion_event;
    uint64_t user_tag;
    uint64_t reserved[2];
} RinFileIoV1;

typedef struct RinDirectoryEntryV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t file_id;
    uint64_t size;
    uint64_t modified_ns;
    uint32_t type;
    uint32_t flags;
    RinStringV1 name;
    uint64_t reserved[2];
} RinDirectoryEntryV1;

typedef struct RinDirectoryOpenV1 {
    uint32_t struct_size;
    uint32_t version;
    RinDirectory directory;
    RinStringV1 path;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t reserved[3];
} RinDirectoryOpenV1;

typedef struct RinFileWatchEventV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t events;
    uint32_t flags;
    uint64_t file_id;
    uint64_t parent_file_id;
    RinStringV1 name;
    uint64_t cookie;
    uint64_t reserved[3];
} RinFileWatchEventV1;

RIN_SDK_API RinResult rin_file_open_v1(const RinFileOpenV1* request, RinFile* file);
RIN_SDK_API RinResult rin_file_read_v1(RinFileIoV1* request, uint64_t* transferred);
RIN_SDK_API RinResult rin_file_write_v1(RinFileIoV1* request, uint64_t* transferred);
RIN_SDK_API RinResult rin_file_flush_v1(RinFile file);
RIN_SDK_API RinResult rin_directory_next_v1(RinDirectory directory, RinDirectoryEntryV1* entry);
RIN_SDK_API RinResult rin_path_normalize_v1(RinStringV1 input, RinSliceV1 output, uint64_t* required);
RIN_SDK_API RinResult rin_file_watch_v1(RinStringV1 path, uint32_t events, RinFileWatch* watch);
RIN_SDK_API RinResult rin_file_read_async_v1(const RinFileIoV1* request, RinIoRequest* operation);
RIN_SDK_API RinResult rin_file_write_async_v1(const RinFileIoV1* request, RinIoRequest* operation);
RIN_SDK_API RinResult rin_file_io_result_v1(RinIoRequest operation, uint64_t* transferred);
RIN_SDK_API RinResult rin_file_io_cancel_v1(RinIoRequest operation);
RIN_SDK_API RinResult rin_directory_open_v1(const RinDirectoryOpenV1* request, RinDirectory* directory);
RIN_SDK_API RinResult rin_directory_rewind_v1(RinDirectory directory);
RIN_SDK_API RinResult rin_file_watch_next_v1(RinFileWatch watch, uint64_t timeout_ns, RinFileWatchEventV1* event);

#ifdef __cplusplus
}
#endif
#endif
