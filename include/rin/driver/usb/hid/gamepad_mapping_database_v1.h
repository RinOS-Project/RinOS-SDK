/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Bounded, generation-checked persistent USB HID gamepad profiles. */

#ifndef RIN_DRIVER_USB_HID_GAMEPAD_MAPPING_DATABASE_V1_H
#define RIN_DRIVER_USB_HID_GAMEPAD_MAPPING_DATABASE_V1_H

#include <stdint.h>

#include <rin/driver/usb/hid/gamepad_calibration_v1.h>
#include <rin/driver/usb/hid/gamepad_mapping_v1.h>
#include <rin/contract_abi.h>
#define RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_V1 UINT32_C(0x00010000)
#define RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAGIC UINT32_C(0x31424447)
#define RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS UINT32_C(16)
#define RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_FLAG_MAPPING UINT32_C(0x00000001)
#define RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_FLAG_CALIBRATION UINT32_C(0x00000002)
#define RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_FLAGS \
    (RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_FLAG_MAPPING | \
     RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_FLAG_CALIBRATION)

typedef enum RinUsbHidGamepadMappingDatabaseResult {
    RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_OK = 0,
    RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_INVALID_ARGUMENT = -1,
    RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MALFORMED = -2,
    RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_NOT_FOUND = -3,
    RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_TABLE_FULL = -4,
    RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_BUSY = -5,
    RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_IO = -6,
    RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_COMMIT_UNKNOWN = -7,
    RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_RECOVERY_REQUIRED = -8
} RinUsbHidGamepadMappingDatabaseResult;

typedef struct RinUsbHidGamepadMappingDatabaseRecordV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_version_min;
    uint16_t device_version_max;
    uint32_t flags;
    RinUsbHidGamepadMappingV1 mapping;
    RinUsbHidGamepadCalibrationV1 calibration;
    uint32_t reserved[4];
} RinUsbHidGamepadMappingDatabaseRecordV1;

typedef int (*RinUsbHidGamepadMappingDatabaseLoadV1)(
    void* context,
    RinUsbHidGamepadMappingDatabaseRecordV1* records,
    uint32_t record_capacity, uint32_t* record_count,
    uint64_t* generation);

/* Returns 1 after the complete candidate has been durably committed, 0 when
 * it was rejected without mutation, and -1 when the result is unknown. */
typedef int (*RinUsbHidGamepadMappingDatabaseStoreV1)(
    void* context,
    const RinUsbHidGamepadMappingDatabaseRecordV1* records,
    uint32_t record_count, uint64_t expected_generation,
    uint64_t* new_generation);

typedef struct RinUsbHidGamepadMappingDatabaseConfigV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    RinUsbHidGamepadMappingDatabaseLoadV1 load;
    RinUsbHidGamepadMappingDatabaseStoreV1 store;
    void* storage_context;
    uint32_t reserved[4];
} RinUsbHidGamepadMappingDatabaseConfigV1;

typedef struct RinUsbHidGamepadMappingDatabaseV1 {
    uint32_t magic;
    volatile uint32_t lock;
    uint32_t callback_active;
    uint32_t recovery_required;
    uint64_t generation;
    uint32_t record_count;
    uint32_t reserved0;
    RinUsbHidGamepadMappingDatabaseLoadV1 load;
    RinUsbHidGamepadMappingDatabaseStoreV1 store;
    void* storage_context;
    RinUsbHidGamepadMappingDatabaseRecordV1
        records[RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS];
} RinUsbHidGamepadMappingDatabaseV1;

RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_init(
    RinUsbHidGamepadMappingDatabaseV1* database,
    const RinUsbHidGamepadMappingDatabaseConfigV1* config);

RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_lookup(
    RinUsbHidGamepadMappingDatabaseV1* database, uint16_t vendor_id,
    uint16_t product_id, uint16_t device_version,
    RinUsbHidGamepadMappingDatabaseRecordV1* record_out);

RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_set(
    RinUsbHidGamepadMappingDatabaseV1* database,
    const RinUsbHidGamepadMappingDatabaseRecordV1* record);

RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_remove(
    RinUsbHidGamepadMappingDatabaseV1* database, uint16_t vendor_id,
    uint16_t product_id);

/* Re-reads storage after a rejected/unknown commit. The database remains
 * recovery_required until this exact readback succeeds. */
RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_recover(
    RinUsbHidGamepadMappingDatabaseV1* database);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidGamepadMappingDatabaseRecordV1) == 228u,
              "USB HID gamepad mapping database ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidGamepadMappingDatabaseRecordV1) == 228u,
               "USB HID gamepad mapping database ABI drift");
#endif

#endif /* RIN_DRIVER_USB_HID_GAMEPAD_MAPPING_DATABASE_V1_H */
