// SPDX-License-Identifier: MIT

#ifndef RIN_SDK_RESOLVED_NAMEINFO_POLICY_H
#define RIN_SDK_RESOLVED_NAMEINFO_POLICY_H

#include <stddef.h>
#include <stdint.h>

#define RIN_RESOLVED_LOCAL_DOMAIN_MAX 253u

static inline uint8_t rin_resolved_ascii_lower(uint8_t value) {
    if (value >= (uint8_t)'A' && value <= (uint8_t)'Z')
        return (uint8_t)(value + ((uint8_t)'a' - (uint8_t)'A'));
    return value;
}

static inline int rin_resolved_dns_name_valid(const char* name,
                                              size_t length) {
    size_t label_length = 0u;
    size_t index;
    if (!name || length == 0u || length > RIN_RESOLVED_LOCAL_DOMAIN_MAX)
        return 0;
    for (index = 0u; index < length; ++index) {
        uint8_t value = (uint8_t)name[index];
        if (value == (uint8_t)'.') {
            if (label_length == 0u || label_length > 63u ||
                name[index - 1u] == '-')
                return 0;
            label_length = 0u;
            continue;
        }
        if (!((value >= (uint8_t)'A' && value <= (uint8_t)'Z') ||
              (value >= (uint8_t)'a' && value <= (uint8_t)'z') ||
              (value >= (uint8_t)'0' && value <= (uint8_t)'9') ||
              value == (uint8_t)'-') ||
            (label_length == 0u && value == (uint8_t)'-'))
            return 0;
        ++label_length;
        if (label_length > 63u) return 0;
    }
    return label_length != 0u && name[length - 1u] != '-';
}

static inline int rin_resolved_local_domain_normalize(
    const char* input, size_t length,
    char output[RIN_RESOLVED_LOCAL_DOMAIN_MAX + 1u]) {
    size_t index;
    if (!output || !rin_resolved_dns_name_valid(input, length)) return -1;
    for (index = 0u; index < length; ++index)
        output[index] = (char)rin_resolved_ascii_lower((uint8_t)input[index]);
    output[length] = '\0';
    return 0;
}

/* Return the prefix length to publish for NI_NOFQDN. A hostname is shortened
 * only when a validated, non-empty local-domain snapshot is an exact
 * case-insensitive label suffix. */
static inline int rin_resolved_nofqdn_length(const char* hostname,
                                             size_t hostname_length,
                                             const char* local_domain,
                                             size_t local_domain_length,
                                             size_t* result_length) {
    size_t suffix_start;
    size_t index;
    if (result_length) *result_length = 0u;
    if (!hostname || !result_length ||
        !rin_resolved_dns_name_valid(hostname, hostname_length) ||
        local_domain_length > RIN_RESOLVED_LOCAL_DOMAIN_MAX ||
        (local_domain_length != 0u &&
         !rin_resolved_dns_name_valid(local_domain, local_domain_length)))
        return -1;
    *result_length = hostname_length;
    if (local_domain_length == 0u ||
        hostname_length <= local_domain_length + 1u)
        return 0;
    suffix_start = hostname_length - local_domain_length;
    if (hostname[suffix_start - 1u] != '.') return 0;
    for (index = 0u; index < local_domain_length; ++index) {
        if (rin_resolved_ascii_lower((uint8_t)hostname[suffix_start + index]) !=
            rin_resolved_ascii_lower((uint8_t)local_domain[index]))
            return 0;
    }
    *result_length = suffix_start - 1u;
    return *result_length != 0u ? 0 : -1;
}

#endif
