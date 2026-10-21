/* SPDX-License-Identifier: MIT */
#include <rin/cpp/sdk.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ratio>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

static_assert(sizeof(RinObject) == sizeof(uint64_t));
static_assert(sizeof(RinChannel) == sizeof(uint64_t));
static_assert(sizeof(RinStringV1) == 16u);
static_assert(offsetof(RinStringV1, address) == 0u);
static_assert(offsetof(RinStringV1, size) == 8u);
static_assert(sizeof(RinSliceV1) == 16u);
static_assert(offsetof(RinSliceV1, address) == 0u);
static_assert(offsetof(RinSliceV1, size) == 8u);
static_assert(std::is_same_v<
              decltype(rin::process_wait(
                  RinProcess{}, std::chrono::milliseconds{1})),
              rin::outcome<int32_t>>);
static_assert(std::is_same_v<
              decltype(rin::wait_many(
                  std::span<RinWaitItemV1>{}, std::chrono::milliseconds{1})),
              rin::outcome<uint32_t>>);
static_assert(std::is_same_v<
              decltype(rin::service_connect(std::string_view{})),
              rin::outcome<RinChannel>>);

template<typename Duration>
concept HasDurationNanoseconds = requires(Duration duration) {
    rin::duration_nanoseconds(duration);
};

static_assert(!HasDurationNanoseconds<std::chrono::duration<double>>);

namespace {

RinResult close_result = RIN_SUCCESS;
RinObject last_closed_handle = RIN_HANDLE_INVALID;
uint32_t close_call_count = 0u;

bool duration_contract() {
    const auto normal = rin::duration_nanoseconds(
        std::chrono::milliseconds{250});
    if (!normal || normal.value() != UINT64_C(250000000)) return false;

    const auto negative = rin::duration_nanoseconds(
        std::chrono::microseconds{-1});
    if (negative || negative.code() != RIN_ERROR_INVALID_ARGUMENT) return false;

    using HalfNanosecond = std::chrono::duration<
        int64_t, std::ratio<1, 2000000000>>;
    const auto sub_nanosecond =
        rin::duration_nanoseconds(HalfNanosecond{1});
    if (!sub_nanosecond || sub_nanosecond.value() != 0u) return false;
    const auto negative_sub_nanosecond =
        rin::duration_nanoseconds(HalfNanosecond{-1});
    if (negative_sub_nanosecond ||
        negative_sub_nanosecond.code() != RIN_ERROR_INVALID_ARGUMENT) {
        return false;
    }

    const auto infinite_sentinel = rin::duration_nanoseconds(
        std::chrono::duration<uint64_t, std::nano>{UINT64_MAX});
    if (infinite_sentinel ||
        infinite_sentinel.code() != RIN_ERROR_LIMIT_EXCEEDED) {
        return false;
    }

    const auto scaled_overflow = rin::duration_nanoseconds(
        std::chrono::duration<uint64_t, std::micro>{UINT64_MAX});
    if (scaled_overflow ||
        scaled_overflow.code() != RIN_ERROR_LIMIT_EXCEEDED) {
        return false;
    }

    const auto largest_signed_nanoseconds = rin::duration_nanoseconds(
        std::chrono::nanoseconds{std::numeric_limits<int64_t>::max()});
    return largest_signed_nanoseconds &&
           largest_signed_nanoseconds.value() ==
               static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
}

bool slice_and_string_contract() {
    const auto empty = rin::slice(std::span<std::byte>{});
    if (!empty || empty.value().size != 0u || empty.value().address != 0u)
        return false;

    const std::array<uint32_t, 2> words{0x11223344u, 0x55667788u};
    const auto word_slice = rin::slice(std::span<const uint32_t>{words});
    if (!word_slice || word_slice.value().size != sizeof(words) ||
        word_slice.value().address != static_cast<uint64_t>(
            reinterpret_cast<uintptr_t>(words.data()))) {
        return false;
    }

    const char embedded_nul[] = {'a', '\0', 'b'};
    const std::string_view view{embedded_nul, sizeof(embedded_nul)};
    const RinStringV1 abi_string = rin::string(view);
    return abi_string.address == static_cast<uint64_t>(
               reinterpret_cast<uintptr_t>(embedded_nul)) &&
           abi_string.size == sizeof(embedded_nul);
}

bool outcome_contract() {
    rin::outcome<uint32_t> success{RIN_SUCCESS, 42u};
    if (!success.try_value() || *success.try_value() != 42u) return false;

    rin::outcome<uint32_t> failure{RIN_ERROR_NOT_FOUND, 99u};
    return failure.try_value() == nullptr &&
           failure.code() == RIN_ERROR_NOT_FOUND;
}

bool replacement_contract() {
    close_result = RIN_ERROR_ACCESS_DENIED;
    close_call_count = 0u;
    last_closed_handle = RIN_HANDLE_INVALID;

    {
        rin::unique_handle<RinChannel> current{11u};
        rin::unique_handle<RinChannel> replacement{22u};

        const auto close_failed = current.close();
        if (close_failed || current.get() != 11u || close_call_count != 1u ||
            last_closed_handle != 11u) {
            return false;
        }

        const auto replace_failed = current.try_replace(std::move(replacement));
        if (replace_failed || current.get() != 11u || replacement.get() != 22u ||
            close_call_count != 2u || last_closed_handle != 11u) {
            return false;
        }

        current = std::move(replacement);
        if (current.get() != 11u || replacement.get() != 22u ||
            close_call_count != 3u || last_closed_handle != 11u) {
            return false;
        }

        close_result = RIN_SUCCESS;
        const auto replaced = current.try_replace(std::move(replacement));
        if (!replaced || current.get() != 22u || replacement.get() != 0u ||
            close_call_count != 4u || last_closed_handle != 11u) {
            return false;
        }

        const auto closed = current.close();
        if (!closed || current.get() != 0u || close_call_count != 5u ||
            last_closed_handle != 22u) {
            return false;
        }
    }

    return close_call_count == 5u;
}

} /* namespace */

extern "C" RinResult rin_object_close_v1(RinObject object) {
    last_closed_handle = object;
    ++close_call_count;
    return close_result;
}

int main() {
    if (!duration_contract()) return 1;
    if (!slice_and_string_contract()) return 2;
    if (!outcome_contract()) return 3;
    if (!replacement_contract()) return 4;
    return 0;
}
