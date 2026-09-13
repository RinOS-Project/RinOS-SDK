#ifndef RIN_SDK_MEDIA_H
#define RIN_SDK_MEDIA_H
#include "abi.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct RinMediaOpenV1 {
    uint32_t struct_size;
    uint32_t version;
    RinStringV1 mime_type;
    RinSliceV1 source;
    uint32_t media_type;
    uint32_t flags;
    uint64_t reserved[4];
} RinMediaOpenV1;
RIN_SDK_API RinResult rin_media_open_v1(const RinMediaOpenV1* request, RinMediaObject* object);
RIN_SDK_API RinResult rin_image_decode_v1(RinMediaObject object, uint32_t format, RinSliceV1 output, uint64_t* required);
RIN_SDK_API RinResult rin_audio_decode_v1(RinMediaObject object, RinSliceV1 output, uint64_t* frames);
RIN_SDK_API RinResult rin_video_decode_v1(RinMediaObject object, RinSurface target, uint64_t* timestamp_ns);
#ifdef __cplusplus
}
#endif
#endif
