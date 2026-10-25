/* SPDX-License-Identifier: MIT */
#ifndef RIN_MEDIA_SDK_CLIENT_V1_H
#define RIN_MEDIA_SDK_CLIENT_V1_H

#include <rin/media.h>

RinResult rin_media_sdk_client_open_v1(RinObject object,
                                       const RinMediaOpenV1* request);
RinResult rin_media_sdk_client_audio_info_v1(RinObject object,
                                              RinAudioInfoV1* info);
RinResult rin_media_sdk_client_audio_decode_v1(RinObject object,
                                                RinSliceV1 output,
                                                uint64_t* frames_out);
void rin_media_sdk_client_object_close_hook_v1(RinObject object);

#endif /* RIN_MEDIA_SDK_CLIENT_V1_H */
