/* SPDX-License-Identifier: MIT */
/* Opaque, peer-bound capability used for large RequestServer bodies. */
#ifndef RIN_SDK_WEB_REQUEST_BODY_CAPABILITY_H
#define RIN_SDK_WEB_REQUEST_BODY_CAPABILITY_H

#include <stddef.h>
#include <stdint.h>

#define RIN_REQUEST_BODY_CAPABILITY_VERSION UINT32_C(1)
#define RIN_REQUEST_BODY_CAPABILITY_TOKEN_BYTES UINT32_C(32)
#define RIN_REQUEST_BODY_CAPABILITY_MAX_BYTES (UINT64_C(256) * 1024u * 1024u)
#define RIN_REQUEST_BODY_CAPABILITY_MAX_OFFSET (UINT64_C(1) << 40)

enum {
    RIN_REQUEST_BODY_CAPABILITY_READ_BIT = UINT32_C(0x00000001),
    RIN_REQUEST_BODY_CAPABILITY_WRITE_BIT = UINT32_C(0x00000002),
    RIN_REQUEST_BODY_CAPABILITY_COMPLETE_BIT = UINT32_C(0x00000004)
};

#define RIN_REQUEST_BODY_CAPABILITY_KNOWN_FLAGS \
    (RIN_REQUEST_BODY_CAPABILITY_READ_BIT | \
     RIN_REQUEST_BODY_CAPABILITY_WRITE_BIT | \
     RIN_REQUEST_BODY_CAPABILITY_COMPLETE_BIT)

/* This is deliberately a token-and-range descriptor.  It contains no
 * pathname, shared-memory name, native handle, pointer, or caller-selected
 * kernel object.  The receiving service resolves token ownership against its
 * authenticated peer and connection generation. */
typedef struct RinRequestBodyCapabilityV1 {
    uint32_t struct_size;
    uint32_t version;
    uint32_t flags;
    uint32_t reserved;
    uint64_t request_id;
    uint64_t connection_generation;
    uint64_t capability_generation;
    uint64_t byte_offset;
    uint64_t byte_length;
    uint8_t token[RIN_REQUEST_BODY_CAPABILITY_TOKEN_BYTES];
    uint64_t reserved_tail;
} RinRequestBodyCapabilityV1;

#if defined(__cplusplus)
static_assert(sizeof(RinRequestBodyCapabilityV1) == 96u,
              "request body capability ABI drift");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(RinRequestBodyCapabilityV1) == 96u,
               "request body capability ABI drift");
#endif

static inline int rin_request_body_capability_token_nonzero(
    const uint8_t token[RIN_REQUEST_BODY_CAPABILITY_TOKEN_BYTES])
{
    uint32_t index;
    uint8_t aggregate = 0u;
    if (token == NULL)
        return 0;
    for (index = 0u;
         index < RIN_REQUEST_BODY_CAPABILITY_TOKEN_BYTES; ++index)
        aggregate |= token[index];
    return aggregate != 0u;
}

static inline int rin_request_body_capability_valid(
    const RinRequestBodyCapabilityV1* capability)
{
    uint64_t end;
    const uint32_t flags = RIN_REQUEST_BODY_CAPABILITY_KNOWN_FLAGS;
    if (capability == NULL ||
        capability->struct_size != sizeof(*capability) ||
        capability->version != RIN_REQUEST_BODY_CAPABILITY_VERSION ||
        capability->request_id == 0u ||
        capability->connection_generation == 0u ||
        capability->capability_generation == 0u ||
        capability->byte_length == 0u ||
        capability->byte_length > RIN_REQUEST_BODY_CAPABILITY_MAX_BYTES ||
        capability->byte_offset > RIN_REQUEST_BODY_CAPABILITY_MAX_OFFSET ||
        capability->byte_offset > UINT64_MAX - capability->byte_length ||
        (capability->flags & ~flags) != 0u ||
        (capability->flags & (RIN_REQUEST_BODY_CAPABILITY_READ_BIT |
                              RIN_REQUEST_BODY_CAPABILITY_WRITE_BIT)) == 0u ||
        capability->reserved != 0u || capability->reserved_tail != 0u ||
        !rin_request_body_capability_token_nonzero(capability->token))
        return 0;
    end = capability->byte_offset + capability->byte_length;
    return end <= RIN_REQUEST_BODY_CAPABILITY_MAX_OFFSET +
                      RIN_REQUEST_BODY_CAPABILITY_MAX_BYTES;
}

static inline int rin_request_body_capability_bound_to(
    const RinRequestBodyCapabilityV1* capability, uint64_t request_id,
    uint64_t connection_generation)
{
    return rin_request_body_capability_valid(capability) &&
           request_id != 0u && connection_generation != 0u &&
           capability->request_id == request_id &&
           capability->connection_generation == connection_generation;
}

#endif /* RIN_SDK_WEB_REQUEST_BODY_CAPABILITY_H */
