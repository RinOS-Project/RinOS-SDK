/* SPDX-License-Identifier: MIT */
#include <rin/cpp/sdk.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <span>
#include <string_view>
#include <utility>

int main()
{
    constexpr std::string_view service_name{"com.example.echo"};
    const auto abi_name = rin::string(service_name);
    const auto empty_bytes = rin::slice(std::span<std::byte>{});
    const auto timeout = rin::duration_nanoseconds(
        std::chrono::milliseconds{250});
    (void)abi_name;
    if (!empty_bytes || !timeout) return 1;

    auto pair = rin::channel_create();
    auto* endpoints = pair.try_value();
    if (!endpoints) return static_cast<int>(pair.code());

    rin::unique_handle<RinChannel> sender(endpoints->first);
    rin::unique_handle<RinChannel> receiver(endpoints->second);
    const auto replaced = sender.try_replace(std::move(receiver));
    return replaced && sender && !receiver ? 0 : 1;
}
