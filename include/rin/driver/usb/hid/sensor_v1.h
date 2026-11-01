/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RINOS_PUBLIC_RIN_DRIVER_USB_HID_SENSOR_V1_H
#define RINOS_PUBLIC_RIN_DRIVER_USB_HID_SENSOR_V1_H

#include <stdint.h>

#define RIN_USB_HID_SENSOR_ABI_VERSION UINT32_C(1)
#define RIN_USB_HID_SENSOR_MAX_DESCRIPTOR_BYTES UINT32_C(1024)
#define RIN_USB_HID_SENSOR_MAX_REPORT_BYTES UINT32_C(64)
#define RIN_USB_HID_SENSOR_MAX_PROFILES UINT32_C(4)

#define RIN_USB_HID_SENSOR_ACCELEROMETER_3D UINT32_C(1)
#define RIN_USB_HID_SENSOR_GYROMETER_3D UINT32_C(2)
#define RIN_USB_HID_SENSOR_AMBIENT_LIGHT UINT32_C(3)
#define RIN_USB_HID_SENSOR_HUMAN_PROXIMITY UINT32_C(4)

#define RIN_USB_HID_SENSOR_SAMPLE_VALID UINT32_C(1)
#define RIN_USB_HID_SENSOR_FIELD_SIGNED UINT8_C(1)
#define RIN_USB_HID_SENSOR_PROFILE_UNIT_PRESENT UINT8_C(1)

#define RIN_USB_HID_SENSOR_POWER_UNDEFINED UINT32_C(0)
#define RIN_USB_HID_SENSOR_POWER_D0_FULL UINT32_C(1)
#define RIN_USB_HID_SENSOR_POWER_D1_LOW UINT32_C(2)
#define RIN_USB_HID_SENSOR_POWER_D2_STANDBY_WAKE UINT32_C(3)
#define RIN_USB_HID_SENSOR_POWER_D3_SLEEP_WAKE UINT32_C(4)
#define RIN_USB_HID_SENSOR_POWER_D4_OFF UINT32_C(5)

typedef enum RinUsbHidSensorResult {
    RIN_USB_HID_SENSOR_OK = 0,
    RIN_USB_HID_SENSOR_INVALID_ARGUMENT = -1,
    RIN_USB_HID_SENSOR_MALFORMED = -2,
    RIN_USB_HID_SENSOR_UNSUPPORTED = -3,
    RIN_USB_HID_SENSOR_LIMIT_EXCEEDED = -4
} RinUsbHidSensorResult;

typedef struct RinUsbHidSensorFieldV1 {
    uint16_t bit_offset;
    uint8_t bit_size;
    uint8_t flags;
    int32_t logical_minimum;
    int32_t logical_maximum;
} RinUsbHidSensorFieldV1;

typedef struct RinUsbHidSensorPowerProfileV1 {
    uint32_t report_size;
    uint32_t collection_id;
    uint16_t bit_offset;
    uint8_t report_id;
    uint8_t bit_size;
    int32_t logical_minimum;
    int32_t logical_maximum;
    int32_t state_values[6];
    uint32_t reserved[2];
} RinUsbHidSensorPowerProfileV1;

typedef struct RinUsbHidSensorProfileV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t sensor_type;
    uint32_t report_size;
    uint32_t report_bit_count;
    uint32_t unit;
    int8_t unit_exponent;
    uint8_t report_id;
    uint8_t value_count;
    uint8_t flags;
    uint8_t collection_id;
    uint8_t reserved0[3];
    RinUsbHidSensorFieldV1 fields[3];
    RinUsbHidSensorPowerProfileV1 power;
    uint32_t reserved[4];
} RinUsbHidSensorProfileV1;

typedef struct RinUsbHidSensorProfilesV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t descriptor_size;
    uint32_t profile_count;
    RinUsbHidSensorProfileV1 profiles[RIN_USB_HID_SENSOR_MAX_PROFILES];
    uint32_t reserved[4];
} RinUsbHidSensorProfilesV1;

/* Values retain the HID logical scale and unit. A zero HID Unit means the
 * standard default unit for sensor_type; unit_exponent is applied in base 10. */
typedef struct RinUsbHidSensorSampleV1 {
    uint32_t sensor_type;
    uint32_t flags;
    uint32_t unit;
    uint32_t status;
    int32_t values[3];
    uint8_t report_id;
    uint8_t value_count;
    int8_t unit_exponent;
    uint8_t reserved0;
} RinUsbHidSensorSampleV1;

typedef struct RinUsbHidSensorCalibrationV1 {
    uint32_t abi_version;
    uint32_t struct_size;
    uint32_t sensor_type;
    uint32_t value_count;
    int32_t offset[3];
    int32_t scale_q16[3];
    uint32_t reserved[2];
} RinUsbHidSensorCalibrationV1;

RinUsbHidSensorResult rin_usb_hid_sensor_admit_descriptor(
    const uint8_t* descriptor, uint32_t descriptor_size,
    RinUsbHidSensorProfilesV1* profiles_out);
RinUsbHidSensorResult rin_usb_hid_sensor_decode_report(
    const RinUsbHidSensorProfileV1* profile, const uint8_t* report,
    uint32_t report_size, RinUsbHidSensorSampleV1* sample_out);
RinUsbHidSensorResult rin_usb_hid_sensor_power_report_update(
    const RinUsbHidSensorProfileV1* profile, uint32_t power_state,
    uint8_t* feature_report, uint32_t report_size);
RinUsbHidSensorResult rin_usb_hid_sensor_calibration_default(
    uint32_t sensor_type, uint32_t value_count,
    RinUsbHidSensorCalibrationV1* calibration_out);
RinUsbHidSensorResult rin_usb_hid_sensor_calibration_validate(
    const RinUsbHidSensorCalibrationV1* calibration);
RinUsbHidSensorResult rin_usb_hid_sensor_calibration_apply(
    const RinUsbHidSensorCalibrationV1* calibration,
    const RinUsbHidSensorSampleV1* input,
    RinUsbHidSensorSampleV1* sample_out);

#if defined(__cplusplus)
static_assert(sizeof(RinUsbHidSensorSampleV1) == 32u,
              "USB HID sensor sample ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinUsbHidSensorSampleV1) == 32u,
               "USB HID sensor sample ABI drift");
#endif

#endif
