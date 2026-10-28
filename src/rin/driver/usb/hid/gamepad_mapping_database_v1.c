/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <rin/driver/cpu_relax.h>
#include <rin/driver/usb/hid/gamepad_mapping_database_v1.h>

static void db_zero(void* value, uint32_t size)
{
    volatile uint8_t* bytes = (volatile uint8_t*)value;
    if (!value) return;
    while (size-- != 0u) *bytes++ = 0u;
}

static void db_copy(void* destination, const void* source, uint32_t size)
{
    volatile uint8_t* out = (volatile uint8_t*)destination;
    const volatile uint8_t* in = (const volatile uint8_t*)source;
    if (!destination || !source) return;
    while (size-- != 0u) *out++ = *in++;
}

static int db_all_zero(const void* value, uint32_t size)
{
    const volatile uint8_t* bytes = (const volatile uint8_t*)value;
    uint8_t combined = 0u;
    if (!value) return 0;
    while (size-- != 0u) combined |= *bytes++;
    return combined == 0u;
}

static int db_record_valid(
    const RinUsbHidGamepadMappingDatabaseRecordV1* record)
{
    if (!record || record->abi_version !=
                       RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_V1 ||
        record->struct_size != sizeof(*record) || record->vendor_id == 0u ||
        record->product_id == 0u ||
        record->device_version_min > record->device_version_max ||
        record->flags != RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_FLAGS ||
        rin_usb_hid_gamepad_mapping_validate(&record->mapping) != RIN_RESULT_OK ||
        rin_usb_hid_gamepad_calibration_validate(&record->calibration) !=
            RIN_USB_HID_GAMEPAD_OK ||
        !db_all_zero(record->reserved, sizeof(record->reserved)))
        return 0;
    return 1;
}

static int db_same_key(
    const RinUsbHidGamepadMappingDatabaseRecordV1* first,
    const RinUsbHidGamepadMappingDatabaseRecordV1* second)
{
    return first && second && first->vendor_id == second->vendor_id &&
           first->product_id == second->product_id;
}

static int db_records_valid(
    const RinUsbHidGamepadMappingDatabaseRecordV1* records,
    uint32_t record_count)
{
    if (record_count > RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS)
        return 0;
    if (record_count != 0u && !records) return 0;
    for (uint32_t index = 0u; index < record_count; ++index) {
        if (!db_record_valid(&records[index])) return 0;
        for (uint32_t other = 0u; other < index; ++other)
            if (db_same_key(&records[index], &records[other])) return 0;
    }
    return 1;
}

static void db_lock(RinUsbHidGamepadMappingDatabaseV1* database)
{
    while (__atomic_exchange_n(&database->lock, 1u, __ATOMIC_ACQUIRE) != 0u) {
#if defined(__i386__) || defined(__x86_64__)
        rin_driver_cpu_relax();
#endif
    }
}

static void db_unlock(RinUsbHidGamepadMappingDatabaseV1* database)
{
    __atomic_store_n(&database->lock, 0u, __ATOMIC_RELEASE);
}

static int db_owner_valid(
    const RinUsbHidGamepadMappingDatabaseV1* database)
{
    return database && database->magic ==
                           RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAGIC &&
           database->generation != 0u &&
           database->record_count <=
               RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS &&
           database->load && database->store &&
           db_records_valid(database->records, database->record_count);
}

/* Called with database->lock held. It leaves the lock held. */
static RinUsbHidGamepadMappingDatabaseResult db_store_locked(
    RinUsbHidGamepadMappingDatabaseV1* database,
    const RinUsbHidGamepadMappingDatabaseRecordV1* records,
    uint32_t record_count)
{
    RinUsbHidGamepadMappingDatabaseStoreV1 store = database->store;
    void* context = database->storage_context;
    uint64_t expected_generation = database->generation;
    uint64_t new_generation = 0u;
    int result;
    if (!db_records_valid(records, record_count))
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MALFORMED;
    if (database->callback_active)
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_BUSY;
    database->callback_active = 1u;
    db_unlock(database);
    result = store(context, records, record_count, expected_generation,
                   &new_generation);
    db_lock(database);
    database->callback_active = 0u;
    if (!db_owner_valid(database) || database->generation != expected_generation) {
        database->recovery_required = 1u;
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_RECOVERY_REQUIRED;
    }
    if (result == 0) return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_IO;
    if (result != 1 || new_generation == 0u ||
        new_generation <= expected_generation) {
        database->recovery_required = 1u;
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_COMMIT_UNKNOWN;
    }
    db_copy(database->records, records,
            record_count * sizeof(database->records[0]));
    if (record_count < RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS)
        db_zero(&database->records[record_count],
                (RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS -
                 record_count) * sizeof(database->records[0]));
    database->record_count = record_count;
    database->generation = new_generation;
    return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_OK;
}

RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_init(
    RinUsbHidGamepadMappingDatabaseV1* database,
    const RinUsbHidGamepadMappingDatabaseConfigV1* config)
{
    RinUsbHidGamepadMappingDatabaseRecordV1 records[
        RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS];
    uint32_t record_count = 0u;
    uint64_t generation = 0u;
    int result;
    if (!database || !config || config->abi_version !=
                           RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_V1 ||
        config->struct_size != sizeof(*config) || !config->load ||
        !config->store || !db_all_zero(config->reserved,
                                       sizeof(config->reserved)))
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_INVALID_ARGUMENT;
    db_zero(records, sizeof(records));
    result = config->load(config->storage_context, records,
                          RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS,
                          &record_count, &generation);
    if (result != 1 || !db_records_valid(records, record_count) ||
        generation == 0u) {
        db_zero(records, sizeof(records));
        return result == 1
            ? RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MALFORMED
            : RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_IO;
    }
    db_zero(database, sizeof(*database));
    database->magic = RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAGIC;
    database->generation = generation;
    database->record_count = record_count;
    database->load = config->load;
    database->store = config->store;
    database->storage_context = config->storage_context;
    db_copy(database->records, records, sizeof(records));
    db_zero(records, sizeof(records));
    return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_OK;
}

RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_lookup(
    RinUsbHidGamepadMappingDatabaseV1* database, uint16_t vendor_id,
    uint16_t product_id, uint16_t device_version,
    RinUsbHidGamepadMappingDatabaseRecordV1* record_out)
{
    int found = 0;
    if (record_out) db_zero(record_out, sizeof(*record_out));
    if (!database || vendor_id == 0u || product_id == 0u || !record_out)
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_INVALID_ARGUMENT;
    db_lock(database);
    if (!db_owner_valid(database)) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MALFORMED;
    }
    if (database->callback_active) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_BUSY;
    }
    if (database->recovery_required) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_RECOVERY_REQUIRED;
    }
    for (uint32_t index = 0u; index < database->record_count; ++index) {
        const RinUsbHidGamepadMappingDatabaseRecordV1* record =
            &database->records[index];
        if (record->vendor_id == vendor_id && record->product_id == product_id) {
            if (device_version < record->device_version_min ||
                device_version > record->device_version_max) {
                db_unlock(database);
                return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_NOT_FOUND;
            }
            *record_out = *record;
            found = 1;
            break;
        }
    }
    db_unlock(database);
    return found ? RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_OK
                 : RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_NOT_FOUND;
}

RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_set(
    RinUsbHidGamepadMappingDatabaseV1* database,
    const RinUsbHidGamepadMappingDatabaseRecordV1* record)
{
    RinUsbHidGamepadMappingDatabaseRecordV1 candidate[
        RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS];
    int existing = -1;
    uint32_t count;
    if (!database || !db_record_valid(record))
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_INVALID_ARGUMENT;
    db_lock(database);
    if (!db_owner_valid(database)) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MALFORMED;
    }
    if (database->callback_active) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_BUSY;
    }
    if (database->recovery_required) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_RECOVERY_REQUIRED;
    }
    db_copy(candidate, database->records, sizeof(candidate));
    for (uint32_t index = 0u; index < database->record_count; ++index)
        if (db_same_key(record, &candidate[index])) {
            existing = (int)index;
            break;
        }
    if (existing < 0) {
        if (database->record_count ==
            RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS) {
            db_zero(candidate, sizeof(candidate));
            db_unlock(database);
            return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_TABLE_FULL;
        }
        existing = (int)database->record_count;
    }
    candidate[existing] = *record;
    count = database->record_count + (existing == (int)database->record_count);
    RinUsbHidGamepadMappingDatabaseResult result =
        db_store_locked(database, candidate, count);
    db_zero(candidate, sizeof(candidate));
    db_unlock(database);
    return result;
}

RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_remove(
    RinUsbHidGamepadMappingDatabaseV1* database, uint16_t vendor_id,
    uint16_t product_id)
{
    RinUsbHidGamepadMappingDatabaseRecordV1 candidate[
        RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS];
    uint32_t removed = 0u;
    uint32_t count;
    if (!database || vendor_id == 0u || product_id == 0u)
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_INVALID_ARGUMENT;
    db_lock(database);
    if (!db_owner_valid(database)) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MALFORMED;
    }
    if (database->callback_active) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_BUSY;
    }
    if (database->recovery_required) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_RECOVERY_REQUIRED;
    }
    db_copy(candidate, database->records, sizeof(candidate));
    count = database->record_count - 1u;
    for (uint32_t index = 0u; index < database->record_count; ++index) {
        if (candidate[index].vendor_id == vendor_id &&
            candidate[index].product_id == product_id) {
            for (uint32_t move = index + 1u;
                 move < database->record_count; ++move)
                candidate[move - 1u] = candidate[move];
            db_zero(&candidate[count],
                    sizeof(candidate[count]));
            removed = 1u;
            break;
        }
    }
    if (!removed) {
        db_zero(candidate, sizeof(candidate));
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_NOT_FOUND;
    }
    RinUsbHidGamepadMappingDatabaseResult result =
        db_store_locked(database, candidate, count);
    db_zero(candidate, sizeof(candidate));
    db_unlock(database);
    return result;
}

RinUsbHidGamepadMappingDatabaseResult
rin_usb_hid_gamepad_mapping_database_recover(
    RinUsbHidGamepadMappingDatabaseV1* database)
{
    RinUsbHidGamepadMappingDatabaseRecordV1 records[
        RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS];
    RinUsbHidGamepadMappingDatabaseLoadV1 load;
    void* context;
    uint32_t record_count = 0u;
    uint64_t generation = 0u;
    int result;
    if (!database) return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_INVALID_ARGUMENT;
    db_lock(database);
    if (!db_owner_valid(database)) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MALFORMED;
    }
    if (!database->recovery_required) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_OK;
    }
    if (database->callback_active) {
        db_unlock(database);
        return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_BUSY;
    }
    load = database->load;
    context = database->storage_context;
    database->callback_active = 1u;
    db_unlock(database);
    db_zero(records, sizeof(records));
    result = load(context, records,
                  RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MAX_RECORDS,
                  &record_count, &generation);
    db_lock(database);
    database->callback_active = 0u;
    if (result != 1 || !db_records_valid(records, record_count) ||
        generation == 0u) {
        db_zero(records, sizeof(records));
        db_unlock(database);
        return result == 1
            ? RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_MALFORMED
            : RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_IO;
    }
    db_copy(database->records, records, sizeof(records));
    database->record_count = record_count;
    database->generation = generation;
    database->recovery_required = 0u;
    db_zero(records, sizeof(records));
    db_unlock(database);
    return RIN_USB_HID_GAMEPAD_MAPPING_DATABASE_OK;
}
