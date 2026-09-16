/* SPDX-License-Identifier: MIT */
#ifndef RIN_SDK_ICU_SERVICE_ABI_H
#define RIN_SDK_ICU_SERVICE_ABI_H

#include <stddef.h>
#include <stdint.h>

/* RinICU wire protocol v2 is a little-endian, byte-addressed protocol.  All
 * integer fields below are serialized in little-endian order and floating
 * point value_bits fields contain IEEE-754 binary64 bits.  The current RinOS
 * targets are little-endian; a future big-endian port must add an explicit
 * encode/decode layer instead of sending these records verbatim. */
#define RIN_ICU_SERVICE_ID "rinicu"
#define RIN_ICU_MAGIC 0x52495631u /* "RIV1" */
#define RIN_ICU_VERSION 2u /* wire protocol version; never a library ABI */
#define RIN_ICU_MAX_INLINE_PAYLOAD 65536u
#define RIN_ICU_MAX_BULK_ITEMS 4096u
#define RIN_ICU_MSG_FLAGS_KNOWN 0u

#define RIN_ICU_LOCALE_ID_MAX 32u
#define RIN_ICU_CURRENCY_CODE_MAX 8u

enum {
    RIN_ICU_STATUS_OK = 0,
    RIN_ICU_STATUS_INVALID = -1,
    RIN_ICU_STATUS_UNSUPPORTED = -2,
    RIN_ICU_STATUS_IO_ERROR = -3,
    RIN_ICU_STATUS_VERSION_MISMATCH = -4,
    RIN_ICU_STATUS_TOO_LARGE = -5,
    RIN_ICU_STATUS_BAD_HANDLE = -6,
    RIN_ICU_STATUS_NO_SPACE = -7,
    RIN_ICU_STATUS_DATA_ERROR = -8
};

enum {
    RIN_ICU_CMD_LOCALE_CANONICALIZE_V1 = 1,
    RIN_ICU_CMD_LOCALE_RESOLVE_V1 = 2,
    RIN_ICU_CMD_NORMALIZE_V1 = 3,
    RIN_ICU_CMD_LOCALE_MAXIMIZE_V1 = 4,
    RIN_ICU_CMD_LOCALE_MINIMIZE_V1 = 5,
    RIN_ICU_CMD_LOCALE_AVAILABLE_V1 = 6,
    RIN_ICU_CMD_LOCALE_PREFERRED_V1 = 7,
    RIN_ICU_CMD_CASE_MAP_V1 = 8,
    RIN_ICU_CMD_LOCALE_INFO_V1 = 9,

    RIN_ICU_CMD_COLLATOR_CREATE_V1 = 16,
    RIN_ICU_CMD_COLLATOR_COMPARE_V1 = 17,
    RIN_ICU_CMD_COLLATOR_SORT_KEY_V1 = 18,
    RIN_ICU_CMD_COLLATOR_SORT_KEYS_V1 = 19,

    RIN_ICU_CMD_SEGMENTER_CREATE_V1 = 32,
    RIN_ICU_CMD_SEGMENTER_RESET_V1 = 33,
    RIN_ICU_CMD_SEGMENTER_NEXT_V1 = 34,

    RIN_ICU_CMD_NUMBER_FORMATTER_CREATE_V1 = 48,
    RIN_ICU_CMD_NUMBER_FORMAT_V1 = 49,

    RIN_ICU_CMD_DATETIME_FORMATTER_CREATE_V1 = 64,
    RIN_ICU_CMD_DATETIME_FORMAT_EPOCH_MS_V1 = 65,

    RIN_ICU_CMD_PLURAL_RULES_CREATE_V1 = 80,
    RIN_ICU_CMD_PLURAL_RULES_SELECT_V1 = 81,

    RIN_ICU_CMD_DISPLAY_NAME_V1 = 112,
    RIN_ICU_CMD_LIST_FORMAT_V1 = 113,
    RIN_ICU_CMD_RELATIVE_TIME_FORMAT_V1 = 114,

    RIN_ICU_CMD_TIME_ZONE_CURRENT_V1 = 128,
    RIN_ICU_CMD_TIME_ZONE_CANONICALIZE_V1 = 129,
    RIN_ICU_CMD_TIME_ZONE_AVAILABLE_V1 = 130,
    RIN_ICU_CMD_TIME_ZONE_OFFSET_V1 = 131,
    RIN_ICU_CMD_TIME_ZONE_RELOAD_V1 = 132,
    RIN_ICU_CMD_TIME_ZONE_AVAILABLE_REGION_V1 = 133,
    RIN_ICU_CMD_TIME_ZONE_TRANSITION_V1 = 134,

    RIN_ICU_CMD_DESTROY_HANDLE_V1 = 96
};

enum {
    RIN_ICU_HANDLE_NONE = 0,
    RIN_ICU_HANDLE_COLLATOR = 1,
    RIN_ICU_HANDLE_SEGMENTER = 2,
    RIN_ICU_HANDLE_NUMBER_FORMATTER = 3,
    RIN_ICU_HANDLE_DATETIME_FORMATTER = 4,
    RIN_ICU_HANDLE_PLURAL_RULES = 5
};

enum {
    RIN_ICU_NORMALIZE_NFD = 0,
    RIN_ICU_NORMALIZE_NFC = 1,
    RIN_ICU_NORMALIZE_NFKD = 2,
    RIN_ICU_NORMALIZE_NFKC = 3
};

enum {
    RIN_ICU_COLLATION_STRENGTH_PRIMARY = 1,
    RIN_ICU_COLLATION_STRENGTH_SECONDARY = 2,
    RIN_ICU_COLLATION_STRENGTH_TERTIARY = 3,
    RIN_ICU_COLLATION_STRENGTH_QUATERNARY = 4
};

enum {
    RIN_ICU_CASE_FIRST_DEFAULT = 0,
    RIN_ICU_CASE_FIRST_LOWER = 1,
    RIN_ICU_CASE_FIRST_UPPER = 2
};

/* The low byte of RinIcuCollatorOptions.case_first is the case-order value.
 * The upper bits are product collator feature flags, retained in this field
 * to keep the v2 request shape stable for existing clients. */
#define RIN_ICU_COLLATOR_CASE_FIRST_MASK UINT32_C(0xff)
#define RIN_ICU_COLLATOR_FLAG_IGNORE_KANA_TYPE (UINT32_C(1) << 8)
#define RIN_ICU_COLLATOR_FLAG_IGNORE_WIDTH (UINT32_C(1) << 9)
#define RIN_ICU_COLLATOR_CASE_FIRST_FLAGS \
    (RIN_ICU_COLLATOR_FLAG_IGNORE_KANA_TYPE | RIN_ICU_COLLATOR_FLAG_IGNORE_WIDTH)

enum {
    RIN_ICU_SEGMENTATION_GRAPHEME = 1,
    RIN_ICU_SEGMENTATION_WORD = 2,
    RIN_ICU_SEGMENTATION_SENTENCE = 3,
    RIN_ICU_SEGMENTATION_LINE = 4
};

enum {
    RIN_ICU_SEGMENT_FLAG_WORD_LIKE = 1u << 0,
    RIN_ICU_SEGMENT_FLAG_SOFT_BREAK = 1u << 1,
    RIN_ICU_SEGMENT_FLAG_HARD_BREAK = 1u << 2
};

enum {
    RIN_ICU_NUMBER_STYLE_DECIMAL = 1,
    RIN_ICU_NUMBER_STYLE_PERCENT = 2,
    RIN_ICU_NUMBER_STYLE_CURRENCY = 3
};

enum {
    RIN_ICU_DATETIME_STYLE_DATE = 1,
    RIN_ICU_DATETIME_STYLE_TIME = 2,
    RIN_ICU_DATETIME_STYLE_DATETIME = 3
};

enum {
    RIN_ICU_HOUR_CYCLE_DEFAULT = 0,
    RIN_ICU_HOUR_CYCLE_H12 = 1,
    RIN_ICU_HOUR_CYCLE_H24 = 2
};

enum {
    RIN_ICU_PLURAL_KIND_CARDINAL = 1,
    RIN_ICU_PLURAL_KIND_ORDINAL = 2
};

enum {
    RIN_ICU_DISPLAY_NAME_LANGUAGE = 1,
    RIN_ICU_DISPLAY_NAME_REGION = 2,
    RIN_ICU_DISPLAY_NAME_SCRIPT = 3,
    RIN_ICU_DISPLAY_NAME_CALENDAR = 4,
    RIN_ICU_DISPLAY_NAME_DATE_TIME_FIELD = 5,
    RIN_ICU_DISPLAY_NAME_TIME_ZONE = 6,
    RIN_ICU_DISPLAY_NAME_CURRENCY = 7,
    RIN_ICU_DISPLAY_NAME_CURRENCY_NUMERIC = 8
};

enum {
    RIN_ICU_LANGUAGE_DISPLAY_STANDARD = 1,
    RIN_ICU_LANGUAGE_DISPLAY_DIALECT = 2
};

enum {
    RIN_ICU_LIST_FORMAT_CONJUNCTION = 1,
    RIN_ICU_LIST_FORMAT_DISJUNCTION = 2,
    RIN_ICU_LIST_FORMAT_UNIT = 3
};

enum {
    RIN_ICU_STYLE_LONG = 1,
    RIN_ICU_STYLE_SHORT = 2,
    RIN_ICU_STYLE_NARROW = 3
};

enum {
    RIN_ICU_NUMERIC_DISPLAY_ALWAYS = 1,
    RIN_ICU_NUMERIC_DISPLAY_AUTO = 2
};

enum {
    RIN_ICU_TIME_UNIT_SECOND = 1,
    RIN_ICU_TIME_UNIT_MINUTE = 2,
    RIN_ICU_TIME_UNIT_HOUR = 3,
    RIN_ICU_TIME_UNIT_DAY = 4,
    RIN_ICU_TIME_UNIT_WEEK = 5,
    RIN_ICU_TIME_UNIT_MONTH = 6,
    RIN_ICU_TIME_UNIT_QUARTER = 7,
    RIN_ICU_TIME_UNIT_YEAR = 8
};

enum {
    RIN_ICU_TIME_ZONE_DIRECTION_PREVIOUS = 0,
    RIN_ICU_TIME_ZONE_DIRECTION_NEXT = 1,
    RIN_ICU_TIME_ZONE_INCLUDE_GIVEN_NO = 0,
    RIN_ICU_TIME_ZONE_INCLUDE_GIVEN_YES = 1,
    RIN_ICU_TIME_ZONE_TRANSITION_ANY = 0,
    RIN_ICU_TIME_ZONE_TRANSITION_OFFSET_CHANGE = 1
};

#if defined(_MSC_VER)
#define RIN_ICU_ABI_PACKED
#pragma pack(push, 1)
#else
#define RIN_ICU_ABI_PACKED __attribute__((packed))
#endif

typedef struct RIN_ICU_ABI_PACKED RinIcuMsgHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t command;
    int32_t status;
    uint32_t request_id;
    uint32_t handle_id;
    uint32_t payload_len;
    uint32_t flags;
} RinIcuMsgHeader;

typedef struct RIN_ICU_ABI_PACKED RinIcuCollatorOptions {
    uint32_t strength;
    uint32_t case_first;
    uint32_t numeric;
    uint32_t ignore_punctuation;
} RinIcuCollatorOptions;

typedef struct RIN_ICU_ABI_PACKED RinIcuSegmenterOptions {
    uint32_t kind;
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t reserved2;
} RinIcuSegmenterOptions;

typedef struct RIN_ICU_ABI_PACKED RinIcuNumberFormatterOptions {
    uint32_t style;
    uint32_t use_grouping;
    int32_t min_fraction_digits;
    int32_t max_fraction_digits;
    char currency_code[RIN_ICU_CURRENCY_CODE_MAX];
} RinIcuNumberFormatterOptions;

typedef struct RIN_ICU_ABI_PACKED RinIcuDateTimeFormatterOptions {
    uint32_t style;
    int32_t tz_offset_minutes;
    uint32_t hour_cycle;
    uint32_t reserved0;
} RinIcuDateTimeFormatterOptions;

typedef struct RIN_ICU_ABI_PACKED RinIcuPluralRulesOptions {
    uint32_t kind;
    uint32_t reserved0;
} RinIcuPluralRulesOptions;

typedef struct RIN_ICU_ABI_PACKED RinIcuTextRequest {
    uint32_t text_len;
} RinIcuTextRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuNormalizeRequest {
    uint32_t form;
    uint32_t text_len;
} RinIcuNormalizeRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuCaseMapRequest {
    uint32_t locale_len;
    uint32_t text_len;
    uint32_t to_upper;
    uint32_t reserved0;
} RinIcuCaseMapRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuCreateWithLocaleRequest {
    uint32_t locale_len;
} RinIcuCreateWithLocaleRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuCollatorCreateRequest {
    uint32_t locale_len;
    RinIcuCollatorOptions options;
} RinIcuCollatorCreateRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuSegmenterCreateRequest {
    uint32_t locale_len;
    RinIcuSegmenterOptions options;
} RinIcuSegmenterCreateRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuNumberFormatterCreateRequest {
    uint32_t locale_len;
    RinIcuNumberFormatterOptions options;
} RinIcuNumberFormatterCreateRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuDateTimeFormatterCreateRequest {
    uint32_t locale_len;
    RinIcuDateTimeFormatterOptions options;
} RinIcuDateTimeFormatterCreateRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuPluralRulesCreateRequest {
    uint32_t locale_len;
    RinIcuPluralRulesOptions options;
} RinIcuPluralRulesCreateRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuCompareRequest {
    uint32_t lhs_len;
    uint32_t rhs_len;
} RinIcuCompareRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuBulkTextRequest {
    uint32_t item_count;
    uint32_t reserved0;
} RinIcuBulkTextRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuBytesResponse {
    uint32_t data_len;
} RinIcuBytesResponse;

typedef struct RIN_ICU_ABI_PACKED RinIcuBulkBytesResponse {
    uint32_t item_count;
    uint32_t reserved0;
} RinIcuBulkBytesResponse;

typedef struct RIN_ICU_ABI_PACKED RinIcuCompareResponse {
    int32_t result;
    uint32_t reserved0;
} RinIcuCompareResponse;

typedef struct RIN_ICU_ABI_PACKED RinIcuSegmentResetRequest {
    uint32_t text_len;
} RinIcuSegmentResetRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuSegmentNextResponse {
    uint32_t has_value;
    uint32_t start;
    uint32_t end;
    uint32_t flags;
} RinIcuSegmentNextResponse;

typedef struct RIN_ICU_ABI_PACKED RinIcuNumberFormatRequest {
    uint64_t value_bits;
} RinIcuNumberFormatRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuDateTimeFormatRequest {
    int64_t epoch_ms;
} RinIcuDateTimeFormatRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuPluralSelectRequest {
    uint64_t value_bits;
} RinIcuPluralSelectRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuDisplayNameRequest {
    uint32_t locale_len;
    uint32_t code_len;
    uint32_t type;
    uint32_t style;
    uint32_t language_display;
    uint32_t reserved0;
} RinIcuDisplayNameRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuListFormatRequest {
    uint32_t locale_len;
    uint32_t item_count;
    uint32_t type;
    uint32_t style;
} RinIcuListFormatRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuRelativeTimeRequest {
    uint32_t locale_len;
    uint32_t style;
    uint32_t numeric_display;
    uint32_t unit;
    uint64_t value_bits;
} RinIcuRelativeTimeRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuTimeZoneOffsetRequest {
    uint32_t time_zone_len;
    uint32_t reserved0;
    int64_t epoch_ms;
} RinIcuTimeZoneOffsetRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuTimeZoneOffsetResponse {
    int32_t offset_minutes;
    uint32_t in_dst;
} RinIcuTimeZoneOffsetResponse;

typedef struct RIN_ICU_ABI_PACKED RinIcuTimeZoneTransitionRequest {
    uint32_t time_zone_len;
    uint32_t reserved0;
    int64_t epoch_ms;
    uint32_t direction;
    uint32_t include_given_time;
    uint32_t transition_rule;
} RinIcuTimeZoneTransitionRequest;

typedef struct RIN_ICU_ABI_PACKED RinIcuTimeZoneTransitionResponse {
    int64_t transition_epoch_ms;
} RinIcuTimeZoneTransitionResponse;

static inline int rin_icu_status_known_v2(int32_t status)
{
    switch (status) {
    case RIN_ICU_STATUS_OK:
    case RIN_ICU_STATUS_INVALID:
    case RIN_ICU_STATUS_UNSUPPORTED:
    case RIN_ICU_STATUS_IO_ERROR:
    case RIN_ICU_STATUS_VERSION_MISMATCH:
    case RIN_ICU_STATUS_TOO_LARGE:
    case RIN_ICU_STATUS_BAD_HANDLE:
    case RIN_ICU_STATUS_NO_SPACE:
    case RIN_ICU_STATUS_DATA_ERROR:
        return 1;
    default:
        return 0;
    }
}

static inline int rin_icu_command_known_v2(uint32_t command)
{
    switch (command) {
    case RIN_ICU_CMD_LOCALE_CANONICALIZE_V1:
    case RIN_ICU_CMD_LOCALE_RESOLVE_V1:
    case RIN_ICU_CMD_NORMALIZE_V1:
    case RIN_ICU_CMD_LOCALE_MAXIMIZE_V1:
    case RIN_ICU_CMD_LOCALE_MINIMIZE_V1:
    case RIN_ICU_CMD_LOCALE_AVAILABLE_V1:
    case RIN_ICU_CMD_LOCALE_PREFERRED_V1:
    case RIN_ICU_CMD_CASE_MAP_V1:
    case RIN_ICU_CMD_LOCALE_INFO_V1:
    case RIN_ICU_CMD_COLLATOR_CREATE_V1:
    case RIN_ICU_CMD_COLLATOR_COMPARE_V1:
    case RIN_ICU_CMD_COLLATOR_SORT_KEY_V1:
    case RIN_ICU_CMD_COLLATOR_SORT_KEYS_V1:
    case RIN_ICU_CMD_SEGMENTER_CREATE_V1:
    case RIN_ICU_CMD_SEGMENTER_RESET_V1:
    case RIN_ICU_CMD_SEGMENTER_NEXT_V1:
    case RIN_ICU_CMD_NUMBER_FORMATTER_CREATE_V1:
    case RIN_ICU_CMD_NUMBER_FORMAT_V1:
    case RIN_ICU_CMD_DATETIME_FORMATTER_CREATE_V1:
    case RIN_ICU_CMD_DATETIME_FORMAT_EPOCH_MS_V1:
    case RIN_ICU_CMD_PLURAL_RULES_CREATE_V1:
    case RIN_ICU_CMD_PLURAL_RULES_SELECT_V1:
    case RIN_ICU_CMD_DISPLAY_NAME_V1:
    case RIN_ICU_CMD_LIST_FORMAT_V1:
    case RIN_ICU_CMD_RELATIVE_TIME_FORMAT_V1:
    case RIN_ICU_CMD_TIME_ZONE_CURRENT_V1:
    case RIN_ICU_CMD_TIME_ZONE_CANONICALIZE_V1:
    case RIN_ICU_CMD_TIME_ZONE_AVAILABLE_V1:
    case RIN_ICU_CMD_TIME_ZONE_OFFSET_V1:
    case RIN_ICU_CMD_TIME_ZONE_RELOAD_V1:
    case RIN_ICU_CMD_TIME_ZONE_AVAILABLE_REGION_V1:
    case RIN_ICU_CMD_TIME_ZONE_TRANSITION_V1:
    case RIN_ICU_CMD_DESTROY_HANDLE_V1:
        return 1;
    default:
        return 0;
    }
}

static inline int rin_icu_header_valid_v2(const RinIcuMsgHeader* header)
{
    return header != NULL && header->magic == RIN_ICU_MAGIC &&
           header->version == RIN_ICU_VERSION && header->request_id != 0u &&
           header->payload_len <= RIN_ICU_MAX_INLINE_PAYLOAD &&
           rin_icu_command_known_v2(header->command) &&
           (header->flags & ~RIN_ICU_MSG_FLAGS_KNOWN) == 0u;
}

#if defined(_MSC_VER)
#pragma pack(pop)
#endif

#undef RIN_ICU_ABI_PACKED

#define RIN_ICU_STATIC_ASSERT(name, expression) \
    typedef char name[(expression) ? 1 : -1]

RIN_ICU_STATIC_ASSERT(rin_icu_header_size_is_stable,
                      sizeof(RinIcuMsgHeader) == 32u);
RIN_ICU_STATIC_ASSERT(rin_icu_collator_options_size_is_stable,
                      sizeof(RinIcuCollatorOptions) == 16u);
RIN_ICU_STATIC_ASSERT(rin_icu_segmenter_options_size_is_stable,
                      sizeof(RinIcuSegmenterOptions) == 16u);
RIN_ICU_STATIC_ASSERT(rin_icu_number_options_size_is_stable,
                      sizeof(RinIcuNumberFormatterOptions) == 24u);
RIN_ICU_STATIC_ASSERT(rin_icu_datetime_options_size_is_stable,
                      sizeof(RinIcuDateTimeFormatterOptions) == 16u);
RIN_ICU_STATIC_ASSERT(rin_icu_plural_options_size_is_stable,
                      sizeof(RinIcuPluralRulesOptions) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_text_request_size_is_stable,
                      sizeof(RinIcuTextRequest) == 4u);
RIN_ICU_STATIC_ASSERT(rin_icu_normalize_request_size_is_stable,
                      sizeof(RinIcuNormalizeRequest) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_case_map_request_size_is_stable,
                      sizeof(RinIcuCaseMapRequest) == 16u);
RIN_ICU_STATIC_ASSERT(rin_icu_create_locale_request_size_is_stable,
                      sizeof(RinIcuCreateWithLocaleRequest) == 4u);
RIN_ICU_STATIC_ASSERT(rin_icu_collator_create_size_is_stable,
                      sizeof(RinIcuCollatorCreateRequest) == 20u);
RIN_ICU_STATIC_ASSERT(rin_icu_segmenter_create_size_is_stable,
                      sizeof(RinIcuSegmenterCreateRequest) == 20u);
RIN_ICU_STATIC_ASSERT(rin_icu_number_create_size_is_stable,
                      sizeof(RinIcuNumberFormatterCreateRequest) == 28u);
RIN_ICU_STATIC_ASSERT(rin_icu_datetime_create_size_is_stable,
                      sizeof(RinIcuDateTimeFormatterCreateRequest) == 20u);
RIN_ICU_STATIC_ASSERT(rin_icu_plural_create_size_is_stable,
                      sizeof(RinIcuPluralRulesCreateRequest) == 12u);
RIN_ICU_STATIC_ASSERT(rin_icu_compare_request_size_is_stable,
                      sizeof(RinIcuCompareRequest) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_bulk_request_size_is_stable,
                      sizeof(RinIcuBulkTextRequest) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_bytes_response_size_is_stable,
                      sizeof(RinIcuBytesResponse) == 4u);
RIN_ICU_STATIC_ASSERT(rin_icu_bulk_response_size_is_stable,
                      sizeof(RinIcuBulkBytesResponse) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_compare_response_size_is_stable,
                      sizeof(RinIcuCompareResponse) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_segment_reset_size_is_stable,
                      sizeof(RinIcuSegmentResetRequest) == 4u);
RIN_ICU_STATIC_ASSERT(rin_icu_segment_next_size_is_stable,
                      sizeof(RinIcuSegmentNextResponse) == 16u);
RIN_ICU_STATIC_ASSERT(rin_icu_number_format_size_is_stable,
                      sizeof(RinIcuNumberFormatRequest) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_datetime_format_size_is_stable,
                      sizeof(RinIcuDateTimeFormatRequest) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_plural_select_size_is_stable,
                      sizeof(RinIcuPluralSelectRequest) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_display_name_size_is_stable,
                      sizeof(RinIcuDisplayNameRequest) == 24u);
RIN_ICU_STATIC_ASSERT(rin_icu_list_format_size_is_stable,
                      sizeof(RinIcuListFormatRequest) == 16u);
RIN_ICU_STATIC_ASSERT(rin_icu_relative_time_size_is_stable,
                      sizeof(RinIcuRelativeTimeRequest) == 24u);
RIN_ICU_STATIC_ASSERT(rin_icu_timezone_request_size_is_stable,
                      sizeof(RinIcuTimeZoneOffsetRequest) == 16u);
RIN_ICU_STATIC_ASSERT(rin_icu_timezone_response_size_is_stable,
                      sizeof(RinIcuTimeZoneOffsetResponse) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_timezone_transition_request_size_is_stable,
                      sizeof(RinIcuTimeZoneTransitionRequest) == 28u);
RIN_ICU_STATIC_ASSERT(rin_icu_timezone_transition_response_size_is_stable,
                      sizeof(RinIcuTimeZoneTransitionResponse) == 8u);

RIN_ICU_STATIC_ASSERT(rin_icu_header_magic_offset_is_stable,
                      offsetof(RinIcuMsgHeader, magic) == 0u);
RIN_ICU_STATIC_ASSERT(rin_icu_header_command_offset_is_stable,
                      offsetof(RinIcuMsgHeader, command) == 8u);
RIN_ICU_STATIC_ASSERT(rin_icu_header_request_offset_is_stable,
                      offsetof(RinIcuMsgHeader, request_id) == 16u);
RIN_ICU_STATIC_ASSERT(rin_icu_header_payload_offset_is_stable,
                      offsetof(RinIcuMsgHeader, payload_len) == 24u);
RIN_ICU_STATIC_ASSERT(rin_icu_case_map_reserved_offset_is_stable,
                      offsetof(RinIcuCaseMapRequest, reserved0) == 12u);
RIN_ICU_STATIC_ASSERT(rin_icu_timezone_reserved_offset_is_stable,
                      offsetof(RinIcuTimeZoneOffsetRequest, reserved0) == 4u);
RIN_ICU_STATIC_ASSERT(rin_icu_timezone_transition_reserved_offset_is_stable,
                      offsetof(RinIcuTimeZoneTransitionRequest, reserved0) == 4u);

#undef RIN_ICU_STATIC_ASSERT

#endif
