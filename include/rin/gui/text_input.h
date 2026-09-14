/* SPDX-License-Identifier: MIT */
/* Public text-input and composition records used by GUI protocols. */
#ifndef RIN_SDK_GUI_TEXT_INPUT_H
#define RIN_SDK_GUI_TEXT_INPUT_H

#include <stdint.h>

#define RIN_TEXT_INPUT_FLAG_ENABLED 1u
#define RIN_TEXT_INPUT_FLAG_MULTILINE 2u
#define RIN_TEXT_INPUT_CONTENT_TEXT 0u
#define RIN_TEXT_INPUT_CONTENT_PASSWORD 1u
#define RIN_TEXT_INPUT_CONTENT_URL 2u
#define RIN_TEXT_INPUT_CONTENT_NUMBER 3u

#ifndef RIN_SDK_TEXT_INPUT_ABI_DEFINED
#define RIN_SDK_TEXT_INPUT_ABI_DEFINED
typedef struct RinTextInputStateV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t content_type;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    uint32_t reserved;
} RinTextInputStateV1;

typedef struct RinTextCompositionV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint32_t text_bytes;
    uint32_t selection_start;
    uint32_t selection_end;
    uint32_t reserved0;
    uint8_t text[400];
    uint64_t reserved[2];
} RinTextCompositionV1;
#endif

#if defined(__cplusplus)
static_assert(sizeof(RinTextInputStateV1) == 32u,
              "RinTextInputStateV1 ABI drift");
static_assert(sizeof(RinTextCompositionV1) == 440u,
              "RinTextCompositionV1 ABI drift");
#else
_Static_assert(sizeof(RinTextInputStateV1) == 32u,
               "RinTextInputStateV1 ABI drift");
_Static_assert(sizeof(RinTextCompositionV1) == 440u,
               "RinTextCompositionV1 ABI drift");
#endif
#endif
