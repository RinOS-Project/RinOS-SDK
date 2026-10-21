/* SPDX-License-Identifier: MIT */
#include <rin/cpp/sdk.hpp>

int main()
{
    auto pair = rin::channel_create();
    if (!pair) return static_cast<int>(pair.code());

    rin::unique_handle<RinChannel> sender(pair.value().first);
    rin::unique_handle<RinChannel> receiver(pair.value().second);
    return sender && receiver ? 0 : 1;
}
