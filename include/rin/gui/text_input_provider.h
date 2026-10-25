/* SPDX-License-Identifier: MIT */
/* Public, pointer-free Text Input / IME Provider wire ABI. */
#ifndef RIN_SDK_GUI_TEXT_INPUT_PROVIDER_H
#define RIN_SDK_GUI_TEXT_INPUT_PROVIDER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RIN_IME_WIRE_MAGIC UINT32_C(0x31454d49) /* "IME1" little-endian */
#define RIN_IME_WIRE_VERSION_MAJOR 1u
#define RIN_IME_WIRE_VERSION_MINOR 0u
#define RIN_IME_WIRE_MAX_MESSAGE_BYTES 4096u
#define RIN_IME_WIRE_MIN_TIMEOUT_MS 1u
#define RIN_IME_WIRE_MAX_TIMEOUT_MS 30000u
#define RIN_IME_WIRE_DEFAULT_TIMEOUT_MS 1000u
#define RIN_IME_MAX_TEXT_BYTES 1024u
#define RIN_IME_MAX_SURROUNDING_BYTES 2048u
#define RIN_IME_MAX_ATTRIBUTES 32u
#define RIN_IME_MAX_CANDIDATES 16u
#define RIN_IME_CANDIDATE_TEXT_BYTES 96u
#define RIN_IME_CANDIDATE_ANNOTATION_BYTES 32u
#define RIN_IME_NO_CANDIDATE UINT32_MAX

#define RIN_IME_FEATURE_PREEDIT UINT64_C(0x0000000000000001)
#define RIN_IME_FEATURE_CANDIDATES UINT64_C(0x0000000000000002)
#define RIN_IME_FEATURE_SURROUNDING_TEXT UINT64_C(0x0000000000000004)
#define RIN_IME_FEATURE_ASYNC_REQUESTS UINT64_C(0x0000000000000008)
#define RIN_IME_FEATURE_CANCEL_RESET UINT64_C(0x0000000000000010)
#define RIN_IME_FEATURE_CARET_RECT UINT64_C(0x0000000000000020)
#define RIN_IME_FEATURE_KNOWN_MASK UINT64_C(0x000000000000003f)

#define RIN_IME_CONTEXT_MULTILINE UINT32_C(0x00000001)
#define RIN_IME_CONTEXT_NO_SUGGESTIONS UINT32_C(0x00000002)
#define RIN_IME_CONTEXT_AUTOCORRECT UINT32_C(0x00000004)
#define RIN_IME_CONTEXT_KNOWN_FLAGS UINT32_C(0x00000007)

#define RIN_IME_MODIFIER_SHIFT UINT32_C(0x00000001)
#define RIN_IME_MODIFIER_CONTROL UINT32_C(0x00000002)
#define RIN_IME_MODIFIER_ALT UINT32_C(0x00000004)
#define RIN_IME_MODIFIER_SUPER UINT32_C(0x00000008)
#define RIN_IME_MODIFIER_KNOWN_MASK UINT32_C(0x0000000f)
#define RIN_IME_CANDIDATE_DISABLED UINT32_C(0x00000001)

enum RinImeWireMessageTypeV1 {
    RIN_IME_WIRE_HELLO_V1 = 1,
    RIN_IME_WIRE_HELLO_ACK_V1 = 2,
    RIN_IME_WIRE_CONTEXT_V1 = 3,
    RIN_IME_WIRE_KEY_EVENT_V1 = 4,
    RIN_IME_WIRE_PREEDIT_V1 = 5,
    RIN_IME_WIRE_COMMIT_V1 = 6,
    RIN_IME_WIRE_CANDIDATES_V1 = 7,
    RIN_IME_WIRE_CANDIDATE_SELECTION_V1 = 8,
    RIN_IME_WIRE_CARET_RECT_V1 = 9,
    RIN_IME_WIRE_SURROUNDING_TEXT_V1 = 10,
    RIN_IME_WIRE_CANCEL_V1 = 11,
    RIN_IME_WIRE_RESET_V1 = 12,
    RIN_IME_WIRE_REPLY_V1 = 13,
};

enum RinImeContentPurposeV1 {
    RIN_IME_CONTENT_TEXT = 0,
    RIN_IME_CONTENT_PASSWORD = 1,
    RIN_IME_CONTENT_URL = 2,
    RIN_IME_CONTENT_NUMBER = 3,
};

enum RinImeKeyStateV1 {
    RIN_IME_KEY_RELEASED = 0,
    RIN_IME_KEY_PRESSED = 1,
};

enum RinImePreeditAttributeKindV1 {
    RIN_IME_PREEDIT_UNDERLINE = 1,
    RIN_IME_PREEDIT_CONVERTED = 2,
    RIN_IME_PREEDIT_TARGET = 3,
    RIN_IME_PREEDIT_ERROR = 4,
};

enum RinImeCancelReasonV1 {
    RIN_IME_CANCEL_FOCUS_LOST = 1,
    RIN_IME_CANCEL_CONTEXT_CHANGED = 2,
    RIN_IME_CANCEL_PROVIDER_RESTART = 3,
    RIN_IME_CANCEL_USER_REQUEST = 4,
};

enum RinImeReplyStatusV1 {
    RIN_IME_REPLY_OK = 0,
    RIN_IME_REPLY_REJECTED = 1,
    RIN_IME_REPLY_UNSUPPORTED = 2,
    RIN_IME_REPLY_BUSY = 3,
    RIN_IME_REPLY_FAILED = 4,
};

/* Every record starts with this header. Integers are little-endian on the
 * wire. There are no pointers, native handles, or size_t fields. A request
 * ID is nonzero and is echoed by its asynchronous reply. Context generation
 * is zero only for HELLO/HELLO_ACK and otherwise binds data to one focus. */
typedef struct RinImeWireHeaderV1 {
    uint32_t magic;
    uint16_t version_major;
    uint16_t version_minor;
    uint32_t message_type;
    uint32_t header_bytes;
    uint32_t message_bytes;
    uint32_t flags;
    uint64_t request_id;
    uint64_t context_generation;
    uint32_t reserved[2];
} RinImeWireHeaderV1;

/* HELLO offers versions/features; HELLO_ACK returns the negotiated feature
 * intersection. required_features must be a subset of features. Version
 * values are packed as (major << 16) | minor. */
typedef struct RinImeProviderHelloV1 {
    RinImeWireHeaderV1 header;
    uint32_t min_version;
    uint32_t max_version;
    uint64_t features;
    uint64_t required_features;
    uint32_t max_message_bytes;
    uint32_t request_timeout_ms;
    uint32_t reserved[2];
} RinImeProviderHelloV1;

/* Secure/password contexts are deliberately not deliverable to providers.
 * The broker must cancel/reset the provider and retain key/text delivery on
 * the trusted path whenever the focused field has password purpose. */
typedef struct RinImeProviderContextV1 {
    RinImeWireHeaderV1 header;
    uint32_t content_purpose;
    uint32_t flags;
    uint32_t max_commit_bytes;
    uint32_t reserved[3];
} RinImeProviderContextV1;

typedef struct RinImeKeyEventV1 {
    RinImeWireHeaderV1 header;
    uint32_t key_code;
    uint32_t state;
    uint32_t modifiers;
    uint32_t unicode_scalar; /* zero when the key has no text scalar */
    uint32_t reserved[2];
} RinImeKeyEventV1;

typedef struct RinImePreeditAttributeV1 {
    uint32_t start_byte;
    uint32_t end_byte;
    uint16_t kind;
    uint16_t reserved;
} RinImePreeditAttributeV1;

typedef struct RinImePreeditV1 {
    RinImeWireHeaderV1 header;
    uint32_t text_bytes;
    uint32_t selection_start;
    uint32_t selection_end;
    uint32_t attribute_count;
    uint32_t flags;
    uint32_t reserved;
    uint8_t text[RIN_IME_MAX_TEXT_BYTES];
    RinImePreeditAttributeV1 attributes[RIN_IME_MAX_ATTRIBUTES];
} RinImePreeditV1;

typedef struct RinImeCommitV1 {
    RinImeWireHeaderV1 header;
    uint32_t text_bytes;
    uint32_t flags;
    uint8_t text[RIN_IME_MAX_TEXT_BYTES];
} RinImeCommitV1;

typedef struct RinImeCandidateV1 {
    uint32_t text_bytes;
    uint32_t annotation_bytes;
    uint32_t flags;
    uint32_t reserved;
    uint8_t text[RIN_IME_CANDIDATE_TEXT_BYTES];
    uint8_t annotation[RIN_IME_CANDIDATE_ANNOTATION_BYTES];
} RinImeCandidateV1;

typedef struct RinImeCandidatesV1 {
    RinImeWireHeaderV1 header;
    uint32_t candidate_count;
    uint32_t selected_index; /* RIN_IME_NO_CANDIDATE when none is selected */
    uint64_t candidate_generation; /* changes whenever the list is replaced */
    uint32_t reserved[2];
    RinImeCandidateV1 candidates[RIN_IME_MAX_CANDIDATES];
} RinImeCandidatesV1;

typedef struct RinImeCandidateSelectionV1 {
    RinImeWireHeaderV1 header;
    uint64_t candidate_generation;
    uint32_t candidate_index;
    uint32_t reserved[3];
} RinImeCandidateSelectionV1;

typedef struct RinImeCaretRectV1 {
    RinImeWireHeaderV1 header;
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    uint32_t reserved[2];
} RinImeCaretRectV1;

typedef struct RinImeSurroundingTextV1 {
    RinImeWireHeaderV1 header;
    uint32_t text_bytes;
    uint32_t selection_start;
    uint32_t selection_end;
    uint32_t flags;
    uint8_t text[RIN_IME_MAX_SURROUNDING_BYTES];
} RinImeSurroundingTextV1;

typedef struct RinImeControlV1 {
    RinImeWireHeaderV1 header;
    uint32_t reason;
    uint32_t reserved[3];
} RinImeControlV1;

typedef struct RinImeReplyV1 {
    RinImeWireHeaderV1 header;
    uint32_t status;
    uint32_t reserved[3];
} RinImeReplyV1;

static inline int rin_ime_wire_all_zero_v1(const uint8_t *bytes,
                                            uint32_t count) {
    uint32_t i;
    for (i = 0; i < count; ++i) {
        if (bytes[i] != 0u) return 0;
    }
    return 1;
}

/* Strict UTF-8: rejects NUL, overlong sequences, surrogates and values above
 * U+10FFFF. Text is length-delimited and must not rely on a trailing NUL. */
static inline int rin_ime_wire_utf8_v1(const uint8_t *s, uint32_t n) {
    uint32_t i = 0u;
    if (s == 0 && n != 0u) return 0;
    while (i < n) {
        uint8_t a = s[i++];
        if (a == 0u) return 0;
        if (a <= 0x7fu) continue;
        if (a >= 0xc2u && a <= 0xdfu) {
            if (i >= n || s[i] < 0x80u || s[i] > 0xbfu) return 0;
            ++i;
            continue;
        }
        if (a >= 0xe0u && a <= 0xefu) {
            uint8_t b, c;
            if (n - i < 2u) return 0;
            b = s[i++]; c = s[i++];
            if (c < 0x80u || c > 0xbfu) return 0;
            if (a == 0xe0u) { if (b < 0xa0u || b > 0xbfu) return 0; }
            else if (a == 0xedu) { if (b < 0x80u || b > 0x9fu) return 0; }
            else if (b < 0x80u || b > 0xbfu) return 0;
            continue;
        }
        if (a >= 0xf0u && a <= 0xf4u) {
            uint8_t b, c, d;
            if (n - i < 3u) return 0;
            b = s[i++]; c = s[i++]; d = s[i++];
            if (c < 0x80u || c > 0xbfu || d < 0x80u || d > 0xbfu)
                return 0;
            if (a == 0xf0u) { if (b < 0x90u || b > 0xbfu) return 0; }
            else if (a == 0xf4u) { if (b < 0x80u || b > 0x8fu) return 0; }
            else if (b < 0x80u || b > 0xbfu) return 0;
            continue;
        }
        return 0;
    }
    return 1;
}

static inline int rin_ime_wire_utf8_boundary_v1(const uint8_t *s,
                                                 uint32_t n,
                                                 uint32_t offset) {
    return offset <= n &&
        (offset == n || (s != 0 && (s[offset] & 0xc0u) != 0x80u));
}

static inline int rin_ime_wire_header_v1(const RinImeWireHeaderV1 *h,
                                          uint32_t bytes,
                                          uint32_t expected_type) {
    if (h == 0 || bytes < (uint32_t)sizeof(*h) ||
        bytes > RIN_IME_WIRE_MAX_MESSAGE_BYTES) return 0;
    if (h->magic != RIN_IME_WIRE_MAGIC ||
        h->version_major != RIN_IME_WIRE_VERSION_MAJOR ||
        h->version_minor != RIN_IME_WIRE_VERSION_MINOR ||
        h->message_type != expected_type ||
        h->header_bytes != (uint32_t)sizeof(*h) ||
        h->message_bytes != bytes || h->flags != 0u ||
        h->request_id == 0u || h->reserved[0] != 0u ||
        h->reserved[1] != 0u) return 0;
    if (expected_type == RIN_IME_WIRE_HELLO_V1 ||
        expected_type == RIN_IME_WIRE_HELLO_ACK_V1)
        return h->context_generation == 0u;
    return h->context_generation != 0u;
}

static inline int rin_ime_wire_hello_v1(const RinImeProviderHelloV1 *m,
                                         uint32_t type) {
    uint32_t current = (RIN_IME_WIRE_VERSION_MAJOR << 16) |
                       RIN_IME_WIRE_VERSION_MINOR;
    if (!rin_ime_wire_header_v1(&m->header, (uint32_t)sizeof(*m), type) ||
        m->min_version == 0u || m->min_version > current ||
        m->max_version < current || m->min_version > m->max_version ||
        (m->features & ~RIN_IME_FEATURE_KNOWN_MASK) != 0u ||
        (m->required_features & ~m->features) != 0u ||
        m->max_message_bytes < (uint32_t)sizeof(RinImeWireHeaderV1) ||
        m->max_message_bytes > RIN_IME_WIRE_MAX_MESSAGE_BYTES ||
        m->request_timeout_ms < RIN_IME_WIRE_MIN_TIMEOUT_MS ||
        m->request_timeout_ms > RIN_IME_WIRE_MAX_TIMEOUT_MS) return 0;
    return rin_ime_wire_all_zero_v1((const uint8_t *)m->reserved,
                                    (uint32_t)sizeof(m->reserved));
}

static inline int rin_ime_wire_payload_v1(const void *message,
                                           uint32_t bytes) {
    const RinImeWireHeaderV1 *h = (const RinImeWireHeaderV1 *)message;
    if (message == 0 || bytes < (uint32_t)sizeof(*h) ||
        bytes > RIN_IME_WIRE_MAX_MESSAGE_BYTES) return 0;
    switch (h->message_type) {
    case RIN_IME_WIRE_HELLO_V1:
    case RIN_IME_WIRE_HELLO_ACK_V1:
        return bytes == (uint32_t)sizeof(RinImeProviderHelloV1) &&
            rin_ime_wire_hello_v1((const RinImeProviderHelloV1 *)message,
                                 h->message_type);
    case RIN_IME_WIRE_CONTEXT_V1: {
        const RinImeProviderContextV1 *m = (const RinImeProviderContextV1 *)message;
        return bytes == (uint32_t)sizeof(*m) &&
            rin_ime_wire_header_v1(&m->header, bytes, h->message_type) &&
            m->content_purpose <= RIN_IME_CONTENT_NUMBER &&
            m->content_purpose != RIN_IME_CONTENT_PASSWORD &&
            (m->flags & ~RIN_IME_CONTEXT_KNOWN_FLAGS) == 0u &&
            m->max_commit_bytes > 0u && m->max_commit_bytes <= RIN_IME_MAX_TEXT_BYTES &&
            rin_ime_wire_all_zero_v1((const uint8_t *)m->reserved,
                                     (uint32_t)sizeof(m->reserved));
    }
    case RIN_IME_WIRE_KEY_EVENT_V1: {
        const RinImeKeyEventV1 *m = (const RinImeKeyEventV1 *)message;
        uint32_t cp = m->unicode_scalar;
        return bytes == (uint32_t)sizeof(*m) &&
            rin_ime_wire_header_v1(&m->header, bytes, h->message_type) &&
            m->key_code != 0u && m->state <= RIN_IME_KEY_PRESSED &&
            (m->modifiers & ~RIN_IME_MODIFIER_KNOWN_MASK) == 0u &&
            (cp == 0u || (cp <= UINT32_C(0x10ffff) &&
                          !(cp >= UINT32_C(0xd800) && cp <= UINT32_C(0xdfff)))) &&
            rin_ime_wire_all_zero_v1((const uint8_t *)m->reserved,
                                     (uint32_t)sizeof(m->reserved));
    }
    case RIN_IME_WIRE_PREEDIT_V1: {
        const RinImePreeditV1 *m = (const RinImePreeditV1 *)message;
        uint32_t i;
        if (bytes != (uint32_t)sizeof(*m) ||
            !rin_ime_wire_header_v1(&m->header, bytes, h->message_type) ||
            m->text_bytes > RIN_IME_MAX_TEXT_BYTES ||
            m->selection_start > m->selection_end ||
            m->selection_end > m->text_bytes ||
            m->attribute_count > RIN_IME_MAX_ATTRIBUTES || m->flags != 0u ||
            m->reserved != 0u || !rin_ime_wire_utf8_v1(m->text, m->text_bytes) ||
            !rin_ime_wire_utf8_boundary_v1(m->text, m->text_bytes, m->selection_start) ||
            !rin_ime_wire_utf8_boundary_v1(m->text, m->text_bytes, m->selection_end) ||
            !rin_ime_wire_all_zero_v1(m->text + m->text_bytes,
                                      RIN_IME_MAX_TEXT_BYTES - m->text_bytes)) return 0;
        for (i = 0u; i < m->attribute_count; ++i) {
            const RinImePreeditAttributeV1 *a = &m->attributes[i];
            if (a->start_byte >= a->end_byte || a->end_byte > m->text_bytes ||
                a->kind < RIN_IME_PREEDIT_UNDERLINE ||
                a->kind > RIN_IME_PREEDIT_ERROR || a->reserved != 0u ||
                !rin_ime_wire_utf8_boundary_v1(m->text, m->text_bytes, a->start_byte) ||
                !rin_ime_wire_utf8_boundary_v1(m->text, m->text_bytes, a->end_byte)) return 0;
        }
        return rin_ime_wire_all_zero_v1((const uint8_t *)&m->attributes[m->attribute_count],
            (RIN_IME_MAX_ATTRIBUTES - m->attribute_count) *
                (uint32_t)sizeof(m->attributes[0]));
    }
    case RIN_IME_WIRE_COMMIT_V1: {
        const RinImeCommitV1 *m = (const RinImeCommitV1 *)message;
        return bytes == (uint32_t)sizeof(*m) &&
            rin_ime_wire_header_v1(&m->header, bytes, h->message_type) &&
            m->text_bytes > 0u && m->text_bytes <= RIN_IME_MAX_TEXT_BYTES &&
            m->flags == 0u && rin_ime_wire_utf8_v1(m->text, m->text_bytes) &&
            rin_ime_wire_all_zero_v1(m->text + m->text_bytes,
                                     RIN_IME_MAX_TEXT_BYTES - m->text_bytes);
    }
    case RIN_IME_WIRE_CANDIDATES_V1: {
        const RinImeCandidatesV1 *m = (const RinImeCandidatesV1 *)message;
        uint32_t i;
        if (bytes != (uint32_t)sizeof(*m) ||
            !rin_ime_wire_header_v1(&m->header, bytes, h->message_type) ||
            m->candidate_count > RIN_IME_MAX_CANDIDATES ||
            m->candidate_generation == 0u ||
            (m->candidate_count == 0u && m->selected_index != RIN_IME_NO_CANDIDATE) ||
            (m->candidate_count != 0u && m->selected_index != RIN_IME_NO_CANDIDATE &&
             m->selected_index >= m->candidate_count) ||
            !rin_ime_wire_all_zero_v1((const uint8_t *)m->reserved,
                                      (uint32_t)sizeof(m->reserved))) return 0;
        for (i = 0u; i < m->candidate_count; ++i) {
            const RinImeCandidateV1 *c = &m->candidates[i];
            if (c->text_bytes == 0u || c->text_bytes > RIN_IME_CANDIDATE_TEXT_BYTES ||
                c->annotation_bytes > RIN_IME_CANDIDATE_ANNOTATION_BYTES ||
                (c->flags & ~RIN_IME_CANDIDATE_DISABLED) != 0u || c->reserved != 0u ||
                !rin_ime_wire_utf8_v1(c->text, c->text_bytes) ||
                !rin_ime_wire_utf8_v1(c->annotation, c->annotation_bytes) ||
                !rin_ime_wire_all_zero_v1(c->text + c->text_bytes,
                    RIN_IME_CANDIDATE_TEXT_BYTES - c->text_bytes) ||
                !rin_ime_wire_all_zero_v1(c->annotation + c->annotation_bytes,
                    RIN_IME_CANDIDATE_ANNOTATION_BYTES - c->annotation_bytes)) return 0;
        }
        return rin_ime_wire_all_zero_v1((const uint8_t *)&m->candidates[m->candidate_count],
            (RIN_IME_MAX_CANDIDATES - m->candidate_count) *
                (uint32_t)sizeof(m->candidates[0]));
    }
    case RIN_IME_WIRE_CANDIDATE_SELECTION_V1: {
        const RinImeCandidateSelectionV1 *m = (const RinImeCandidateSelectionV1 *)message;
        return bytes == (uint32_t)sizeof(*m) &&
            rin_ime_wire_header_v1(&m->header, bytes, h->message_type) &&
            m->candidate_generation != 0u &&
            m->candidate_index < RIN_IME_MAX_CANDIDATES &&
            rin_ime_wire_all_zero_v1((const uint8_t *)m->reserved,
                                     (uint32_t)sizeof(m->reserved));
    }
    case RIN_IME_WIRE_CARET_RECT_V1: {
        const RinImeCaretRectV1 *m = (const RinImeCaretRectV1 *)message;
        return bytes == (uint32_t)sizeof(*m) &&
            rin_ime_wire_header_v1(&m->header, bytes, h->message_type) &&
            m->width >= 0 && m->height >= 0 &&
            rin_ime_wire_all_zero_v1((const uint8_t *)m->reserved,
                                     (uint32_t)sizeof(m->reserved));
    }
    case RIN_IME_WIRE_SURROUNDING_TEXT_V1: {
        const RinImeSurroundingTextV1 *m = (const RinImeSurroundingTextV1 *)message;
        return bytes == (uint32_t)sizeof(*m) &&
            rin_ime_wire_header_v1(&m->header, bytes, h->message_type) &&
            m->text_bytes <= RIN_IME_MAX_SURROUNDING_BYTES &&
            m->selection_start <= m->selection_end &&
            m->selection_end <= m->text_bytes && m->flags == 0u &&
            rin_ime_wire_utf8_v1(m->text, m->text_bytes) &&
            rin_ime_wire_utf8_boundary_v1(m->text, m->text_bytes, m->selection_start) &&
            rin_ime_wire_utf8_boundary_v1(m->text, m->text_bytes, m->selection_end) &&
            rin_ime_wire_all_zero_v1(m->text + m->text_bytes,
                RIN_IME_MAX_SURROUNDING_BYTES - m->text_bytes);
    }
    case RIN_IME_WIRE_CANCEL_V1:
    case RIN_IME_WIRE_RESET_V1: {
        const RinImeControlV1 *m = (const RinImeControlV1 *)message;
        return bytes == (uint32_t)sizeof(*m) &&
            rin_ime_wire_header_v1(&m->header, bytes, h->message_type) &&
            m->reason >= RIN_IME_CANCEL_FOCUS_LOST &&
            m->reason <= RIN_IME_CANCEL_USER_REQUEST &&
            rin_ime_wire_all_zero_v1((const uint8_t *)m->reserved,
                                     (uint32_t)sizeof(m->reserved));
    }
    case RIN_IME_WIRE_REPLY_V1: {
        const RinImeReplyV1 *m = (const RinImeReplyV1 *)message;
        return bytes == (uint32_t)sizeof(*m) &&
            rin_ime_wire_header_v1(&m->header, bytes, h->message_type) &&
            m->status <= RIN_IME_REPLY_FAILED &&
            rin_ime_wire_all_zero_v1((const uint8_t *)m->reserved,
                                     (uint32_t)sizeof(m->reserved));
    }
    default:
        return 0;
    }
}

#if defined(__cplusplus)
static_assert(sizeof(RinImeWireHeaderV1) == 48u, "IME wire header ABI drift");
static_assert(sizeof(RinImeProviderHelloV1) == 88u, "IME hello ABI drift");
static_assert(sizeof(RinImeProviderContextV1) == 72u, "IME context ABI drift");
static_assert(sizeof(RinImeKeyEventV1) == 72u, "IME key event ABI drift");
static_assert(sizeof(RinImePreeditAttributeV1) == 12u, "IME attribute ABI drift");
static_assert(sizeof(RinImePreeditV1) == 1480u, "IME preedit ABI drift");
static_assert(sizeof(RinImeCommitV1) == 1080u, "IME commit ABI drift");
static_assert(sizeof(RinImeCandidateV1) == 144u, "IME candidate ABI drift");
static_assert(sizeof(RinImeCandidatesV1) == 2376u, "IME candidates ABI drift");
static_assert(sizeof(RinImeCandidateSelectionV1) == 72u, "IME selection ABI drift");
static_assert(sizeof(RinImeCaretRectV1) == 72u, "IME caret ABI drift");
static_assert(sizeof(RinImeSurroundingTextV1) == 2112u, "IME surrounding ABI drift");
static_assert(sizeof(RinImeControlV1) == 64u, "IME control ABI drift");
static_assert(sizeof(RinImeReplyV1) == 64u, "IME reply ABI drift");
static_assert(offsetof(RinImeWireHeaderV1, request_id) == 24u, "IME request ID offset drift");
static_assert(offsetof(RinImeWireHeaderV1, context_generation) == 32u, "IME context offset drift");
static_assert(offsetof(RinImeProviderHelloV1, features) == 56u, "IME feature offset drift");
static_assert(offsetof(RinImePreeditV1, text) == 72u, "IME preedit text offset drift");
static_assert(offsetof(RinImePreeditV1, attributes) == 1096u, "IME attribute offset drift");
static_assert(offsetof(RinImeCandidatesV1, candidates) == 72u, "IME candidate offset drift");
static_assert(offsetof(RinImeSurroundingTextV1, text) == 64u, "IME context text offset drift");
#else
_Static_assert(sizeof(RinImeWireHeaderV1) == 48u, "IME wire header ABI drift");
_Static_assert(sizeof(RinImeProviderHelloV1) == 88u, "IME hello ABI drift");
_Static_assert(sizeof(RinImeProviderContextV1) == 72u, "IME context ABI drift");
_Static_assert(sizeof(RinImeKeyEventV1) == 72u, "IME key event ABI drift");
_Static_assert(sizeof(RinImePreeditAttributeV1) == 12u, "IME attribute ABI drift");
_Static_assert(sizeof(RinImePreeditV1) == 1480u, "IME preedit ABI drift");
_Static_assert(sizeof(RinImeCommitV1) == 1080u, "IME commit ABI drift");
_Static_assert(sizeof(RinImeCandidateV1) == 144u, "IME candidate ABI drift");
_Static_assert(sizeof(RinImeCandidatesV1) == 2376u, "IME candidates ABI drift");
_Static_assert(sizeof(RinImeCandidateSelectionV1) == 72u, "IME selection ABI drift");
_Static_assert(sizeof(RinImeCaretRectV1) == 72u, "IME caret ABI drift");
_Static_assert(sizeof(RinImeSurroundingTextV1) == 2112u, "IME surrounding ABI drift");
_Static_assert(sizeof(RinImeControlV1) == 64u, "IME control ABI drift");
_Static_assert(sizeof(RinImeReplyV1) == 64u, "IME reply ABI drift");
_Static_assert(offsetof(RinImeWireHeaderV1, request_id) == 24u, "IME request ID offset drift");
_Static_assert(offsetof(RinImeWireHeaderV1, context_generation) == 32u, "IME context offset drift");
_Static_assert(offsetof(RinImeProviderHelloV1, features) == 56u, "IME feature offset drift");
_Static_assert(offsetof(RinImePreeditV1, text) == 72u, "IME preedit text offset drift");
_Static_assert(offsetof(RinImePreeditV1, attributes) == 1096u, "IME attribute offset drift");
_Static_assert(offsetof(RinImeCandidatesV1, candidates) == 72u, "IME candidate offset drift");
_Static_assert(offsetof(RinImeSurroundingTextV1, text) == 64u, "IME context text offset drift");
#endif

#ifdef __cplusplus
}
#endif

#endif
