#ifndef RIN_SDK_FS_H
#define RIN_SDK_FS_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Operations added after the original FS v1 surface.  Existing operation
 * numbers remain unchanged; these requests are still dispatched by the
 * authenticated RinSdkBackendV1 owner. */
enum {
    RIN_FS_OPERATION_FILE_OPEN = 1,
    RIN_FS_OPERATION_FILE_READ = 2,
    RIN_FS_OPERATION_FILE_WRITE = 3,
    RIN_FS_OPERATION_FILE_FLUSH = 4,
    RIN_FS_OPERATION_DIRECTORY_NEXT = 5,
    RIN_FS_OPERATION_PATH_NORMALIZE = 6,
    RIN_FS_OPERATION_FILE_WATCH = 7,
    RIN_FS_OPERATION_FILE_READ_ASYNC = 8,
    RIN_FS_OPERATION_FILE_WRITE_ASYNC = 9,
    RIN_FS_OPERATION_FILE_IO_RESULT = 10,
    RIN_FS_OPERATION_FILE_IO_CANCEL = 11,
    RIN_FS_OPERATION_DIRECTORY_OPEN = 12,
    RIN_FS_OPERATION_DIRECTORY_REWIND = 13,
    RIN_FS_OPERATION_FILE_WATCH_NEXT = 14,
    RIN_FS_OPERATION_FILE_CLOSE = 15,
    RIN_FS_OPERATION_DIRECTORY_CLOSE = 16,
    RIN_FS_OPERATION_FILE_SEEK = 17,
    RIN_FS_OPERATION_FILE_TRUNCATE = 18,
    RIN_FS_OPERATION_DIRECTORY_SYNC = 19,
    RIN_FS_OPERATION_PATH_STAT = 20,
    RIN_FS_OPERATION_PATH_MKDIR = 21,
    RIN_FS_OPERATION_PATH_UNLINK = 22,
    RIN_FS_OPERATION_PATH_RENAME = 23,
    RIN_FS_OPERATION_PORTAL_OPEN = 24
};

#define RIN_FS_PATH_UNLINK_DIRECTORY UINT32_C(1)
#define RIN_FS_PORTAL_OPEN_FILE UINT32_C(1)
#define RIN_FS_PORTAL_OPEN_DIRECTORY UINT32_C(2)

typedef struct RinFileSeekV1 {
    uint32_t struct_size;
    uint32_t version;
    RinFile file;
    int64_t offset;
    int32_t whence;
    uint32_t flags;
    uint64_t reserved[2];
} RinFileSeekV1;

typedef struct RinFileTruncateV1 {
    uint32_t struct_size;
    uint32_t version;
    RinFile file;
    uint64_t size;
    uint64_t reserved[2];
} RinFileTruncateV1;

typedef struct RinFsPathRequestV1 {
    uint32_t struct_size;
    uint32_t version;
    /* A non-invalid directory is an SDK-owned capability handle.  The path
     * is resolved beneath it and must not escape with dot-dot components. */
    RinDirectory directory;
    /* Used only by PATH_RENAME; all other path operations require invalid. */
    RinDirectory secondary_directory;
    RinStringV1 path;
    RinStringV1 secondary_path;
    uint32_t flags;
    uint32_t mode;
    uint64_t reserved[2];
} RinFsPathRequestV1;

/* Opens a current-sandbox durable File Portal grant as an SDK-owned opaque
 * file or directory handle.  The grant id never becomes a path or a raw
 * descriptor in the public request. */
typedef struct RinFsPortalOpenV1 {
    uint32_t struct_size;
    uint32_t version;
    uint64_t grant_id;
    uint32_t object_type;
    uint32_t rights;
    uint32_t flags;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinFsPortalOpenV1;

typedef struct RinFileStatV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t mode;
    uint32_t flags;
    uint64_t size;
    uint64_t modified_ns;
    uint64_t created_ns;
    uint32_t link_count;
    uint32_t reserved0;
    uint64_t reserved[2];
} RinFileStatV1;

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
    /* Caller-owned output buffer.  The owner writes the entry name here and
     * returns the actual byte count in name.size; callers must provide at
     * least 256 bytes for the v1 directory owner. */
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
RIN_SDK_API RinResult rin_file_close_v1(RinFile file);
RIN_SDK_API RinResult rin_directory_close_v1(RinDirectory directory);
RIN_SDK_API RinResult rin_file_seek_v1(const RinFileSeekV1* request, uint64_t* position);
RIN_SDK_API RinResult rin_file_truncate_v1(const RinFileTruncateV1* request);
RIN_SDK_API RinResult rin_directory_sync_v1(RinDirectory directory);
RIN_SDK_API RinResult rin_path_stat_v1(const RinFsPathRequestV1* request, RinFileStatV1* stat);
RIN_SDK_API RinResult rin_path_mkdir_v1(const RinFsPathRequestV1* request);
RIN_SDK_API RinResult rin_path_unlink_v1(const RinFsPathRequestV1* request);
RIN_SDK_API RinResult rin_path_rename_v1(const RinFsPathRequestV1* request);
RIN_SDK_API RinResult rin_fs_portal_open_v1(const RinFsPortalOpenV1* request,
                                            uint64_t* handle);

#if defined(__cplusplus)
static_assert(sizeof(RinFileSeekV1) == 48u, "RinFileSeekV1 ABI drift");
static_assert(sizeof(RinFileTruncateV1) == 40u,
              "RinFileTruncateV1 ABI drift");
static_assert(sizeof(RinFsPathRequestV1) == 80u,
              "RinFsPathRequestV1 ABI drift");
static_assert(sizeof(RinFsPortalOpenV1) == 48u,
              "RinFsPortalOpenV1 ABI drift");
static_assert(sizeof(RinFileStatV1) == 64u, "RinFileStatV1 ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinFileSeekV1) == 48u, "RinFileSeekV1 ABI drift");
_Static_assert(sizeof(RinFileTruncateV1) == 40u,
               "RinFileTruncateV1 ABI drift");
_Static_assert(sizeof(RinFsPathRequestV1) == 80u,
               "RinFsPathRequestV1 ABI drift");
_Static_assert(sizeof(RinFsPortalOpenV1) == 48u,
               "RinFsPortalOpenV1 ABI drift");
_Static_assert(sizeof(RinFileStatV1) == 64u, "RinFileStatV1 ABI drift");
#endif

#ifdef __cplusplus
}
#endif
#endif
