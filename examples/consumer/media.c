/* SPDX-License-Identifier: MIT */
#include <stdint.h>
#include <stdio.h>

#include <rin/base.h>
#include <rin/media.h>

/* Complete 1x1 RGBA PNG and two-frame PCM WAV payloads keep the sample
 * self-contained and exercise the public slices without filesystem access. */
static const uint8_t sample_png[] = {
    0x89u, 0x50u, 0x4eu, 0x47u, 0x0du, 0x0au, 0x1au, 0x0au,
    0x00u, 0x00u, 0x00u, 0x0du, 0x49u, 0x48u, 0x44u, 0x52u,
    0x00u, 0x00u, 0x00u, 0x01u, 0x00u, 0x00u, 0x00u, 0x01u,
    0x08u, 0x06u, 0x00u, 0x00u, 0x00u, 0x1fu, 0x15u, 0xc4u,
    0x89u, 0x00u, 0x00u, 0x00u, 0x0du, 0x49u, 0x44u, 0x41u,
    0x54u, 0x78u, 0x9cu, 0x63u, 0xf8u, 0xcfu, 0xc0u, 0xf0u,
    0x1fu, 0x00u, 0x05u, 0x00u, 0x01u, 0xffu, 0x89u, 0x99u,
    0x3du, 0x1du, 0x00u, 0x00u, 0x00u, 0x00u, 0x49u, 0x45u,
    0x4eu, 0x44u, 0xaeu, 0x42u, 0x60u, 0x82u,
};

static const uint8_t sample_wav[] = {
    'R', 'I', 'F', 'F', 0x28u, 0x00u, 0x00u, 0x00u,
    'W', 'A', 'V', 'E', 'f', 'm', 't', ' ',
    0x10u, 0x00u, 0x00u, 0x00u, 0x01u, 0x00u, 0x01u, 0x00u,
    0x40u, 0x1fu, 0x00u, 0x00u, 0x80u, 0x3eu, 0x00u, 0x00u,
    0x02u, 0x00u, 0x10u, 0x00u, 'd', 'a', 't', 'a',
    0x04u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x10u,
};

static int close_media(RinMediaObject object, int result)
{
    RinResult close_result = rin_object_close_v1((RinObject)object);
    if (close_result != RIN_SUCCESS) {
        printf("media object close failed: %d\n", (int)close_result);
        return 1;
    }
    return result;
}

static int open_media(uint32_t media_type, const char* mime_type,
                      uint64_t mime_type_size,
                      const uint8_t* source, uint64_t source_size,
                      RinMediaObject* object)
{
    RinMediaOpenV1 request = {0};
    RinResult result;

    *object = RIN_HANDLE_INVALID;
    request.struct_size = sizeof(request);
    request.version = RIN_SDK_STRUCT_VERSION_1;
    request.mime_type.address = (uint64_t)(uintptr_t)mime_type;
    request.mime_type.size = mime_type_size;
    request.source.address = (uint64_t)(uintptr_t)source;
    request.source.size = source_size;
    request.media_type = media_type;

    result = rin_media_open_v1(&request, object);
    if (result == RIN_ERROR_NOT_SUPPORTED) {
        puts("media pipeline is not available on this target");
        return 3;
    }
    if (result != RIN_SUCCESS || *object == RIN_HANDLE_INVALID) {
        printf("media open failed: %d\n", (int)result);
        return 1;
    }
    return 0;
}

static int open_png(void)
{
    static const char mime_type[] = "image/png";
    RinMediaObject object = RIN_HANDLE_INVALID;
    int result = open_media(RIN_MEDIA_TYPE_IMAGE, mime_type,
                            sizeof(mime_type) - 1u, sample_png,
                            sizeof(sample_png), &object);
    if (result != 0) return result;
    puts("typed PNG media object opened and closed");
    return close_media(object, 0);
}

static int decode_wav(void)
{
    static const char mime_type[] = "audio/wav";
    uint8_t decoded_samples[4] = {0u, 0u, 0u, 0u};
    RinAudioInfoV1 info = {0};
    RinMediaObject object = RIN_HANDLE_INVALID;
    RinSliceV1 output = {0};
    uint64_t decoded_frames = 0u;
    RinResult result;
    int sample_status;

    sample_status = open_media(RIN_MEDIA_TYPE_AUDIO, mime_type,
                               sizeof(mime_type) - 1u, sample_wav,
                               sizeof(sample_wav), &object);
    if (sample_status != 0) return sample_status;

    info.struct_size = sizeof(info);
    info.version = RIN_SDK_STRUCT_VERSION_1;
    result = rin_audio_get_info_v1(object, &info);
    if (result != RIN_SUCCESS) {
        printf("audio metadata query failed: %d\n", (int)result);
        return close_media(object, 1);
    }
    if (info.sample_rate_hz != 8000u || info.channels != 1u ||
        info.sample_format != RIN_MEDIA_AUDIO_SAMPLE_FORMAT_S16LE ||
        info.bytes_per_frame != 2u ||
        (info.flags & RIN_AUDIO_INFO_FRAME_COUNT_KNOWN) == 0u ||
        info.frame_count != 2u) {
        puts("audio metadata did not match the self-contained PCM fixture");
        return close_media(object, 1);
    }

    output.address = (uint64_t)(uintptr_t)decoded_samples;
    output.size = sizeof(decoded_samples);
    result = rin_audio_decode_v1(object, output, &decoded_frames);
    if (result != RIN_SUCCESS || decoded_frames != 2u) {
        printf("audio decode failed: %d (frames=%llu)\n",
               (int)result, (unsigned long long)decoded_frames);
        return close_media(object, 1);
    }
    puts("typed PCM WAV metadata queried and two frames decoded");
    return close_media(object, 0);
}

int main(void)
{
    int result = open_png();
    if (result != 0) return result;
    return decode_wav();
}
