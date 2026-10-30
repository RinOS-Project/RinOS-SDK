/* SPDX-License-Identifier: MIT */
#ifndef RIN_API_RIN_LEGACY_MOUSE_EVENT_ABI_H
#define RIN_API_RIN_LEGACY_MOUSE_EVENT_ABI_H

#include <stdint.h>

/* PS/2マウスのイベントioctl。既存の6 byte形式は要求0x03で維持する。 */
#define RIN_LEGACY_MOUSE_IOCTL_GET_EVENT_V1 UINT32_C(0x04)
#define RIN_LEGACY_MOUSE_EVENT_VERSION_V1 UINT16_C(1)

#define RIN_LEGACY_MOUSE_EVENT_AVAILABLE UINT16_C(0x0001)
#define RIN_LEGACY_MOUSE_EVENT_RESYNC_REQUIRED UINT16_C(0x0002)
#define RIN_LEGACY_MOUSE_EVENT_MOTION_COALESCED UINT16_C(0x0004)
#define RIN_LEGACY_MOUSE_EVENT_MOTION_LOSS UINT16_C(0x0008)
#define RIN_LEGACY_MOUSE_EVENT_WHEEL_LOSS UINT16_C(0x0010)
#define RIN_LEGACY_MOUSE_EVENT_BUTTON_LOSS UINT16_C(0x0020)
#define RIN_LEGACY_MOUSE_EVENT_COUNTER_SATURATED UINT16_C(0x0040)
#define RIN_LEGACY_MOUSE_EVENT_FLAG_MASK UINT16_C(0x007f)

/* 読み取りはこのデバイス共通の破壊的queueで行う。複数open間でeventは
 * 分配され、それぞれ独立したqueueにはならない。入力はstruct_sizeとversion。
 * ioctl 0x04はeventがあれば本structのsize、なければ0を返す。
 * overflow時はRESYNC_REQUIREDを返し、sequenceは再同期境界になる。
 * dx/dyはその境界までにqueueから欠落した相対移動量の合計。
 * 合計がi32で表せない場合は0とMOTION_LOSSを返す。wheel_deltaは0で、
 * wheelとbuttonsは現在状態を示す。buttonsのbit 0..4は左、右、中、
 * サイド、追加ボタン。損失event数はデバイス稼働中の累積値で、
 * 各LOSS flagは前回の再同期以降にその種類の損失があったことを示す。
 * 旧ioctl 0x03はoverflow時にDRV_ERR_BUSYを一度返す。続けてreadし、
 * 絶対位置とbutton状態を再同期する。 */
typedef struct RinLegacyMouseEventV1 {
    uint32_t struct_size;
    uint16_t version;
    uint16_t flags;
    uint64_t sequence;
    uint64_t dropped_events;
    int32_t dx;
    int32_t dy;
    int32_t wheel_delta;
    int32_t x;
    int32_t y;
    int32_t wheel;
    uint8_t buttons;
    uint8_t reserved[3];
    uint32_t reserved0;
    uint64_t coalesced_motion_events;
    uint64_t wheel_loss_events;
    uint64_t button_transition_loss_events;
} RinLegacyMouseEventV1;

#if defined(__cplusplus)
static_assert(sizeof(RinLegacyMouseEventV1) == 80u,
              "legacy mouse event ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinLegacyMouseEventV1) == 80u,
               "legacy mouse event ABI drift");
#endif

#endif /* RIN_API_RIN_LEGACY_MOUSE_EVENT_ABI_H */
