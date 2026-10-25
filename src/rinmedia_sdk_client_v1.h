/* SPDX-License-Identifier: MIT */
#ifndef RIN_MEDIA_SDK_CLIENT_V1_H
#define RIN_MEDIA_SDK_CLIENT_V1_H

#include <rin/media.h>

RinResult rin_media_sdk_client_open_v1(RinObject object,
                                       const RinMediaOpenV1* request);
RinResult rin_media_sdk_client_image_info_v1(RinObject object,
                                              RinImageInfoV1* info);
RinResult rin_media_sdk_client_image_decode_v1(RinObject object,
                                                uint32_t format,
                                                RinSliceV1 output,
                                                uint64_t* required);
RinResult rin_media_sdk_client_audio_info_v1(RinObject object,
                                              RinAudioInfoV1* info);
RinResult rin_media_sdk_client_audio_decode_v1(RinObject object,
                                                RinSliceV1 output,
                                                uint64_t* frames_out);
RinResult rin_media_sdk_client_video_info_v1(RinObject object,
                                              RinVideoInfoV1* info);
RinResult rin_media_sdk_client_video_decode_frame_v1(
    RinObject object, RinRenderTarget* target,
    RinVideoFrameInfoV1* frame_info);
void rin_media_sdk_client_object_close_hook_v1(RinObject object);

#endif /* RIN_MEDIA_SDK_CLIENT_V1_H */
