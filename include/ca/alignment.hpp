#pragma once

#include <concepts>
#include <type_traits>

namespace ca {

inline static constexpr std::size_t default_alignment{8};

constexpr std::size_t round_to_alignment(std::size_t size, std::size_t align)
{
    return size + ((size % align) == 0 ? 0 : align - (size % align));
}

constexpr auto align_down(std::integral auto n, std::integral auto a)
{
    constexpr auto align_down_impl = [](auto un, auto ua) {
        return (un & ~(ua - 1));
    };
    return align_down_impl(std::make_unsigned<decltype(n)>(n), std::make_unsigned<decltype(a)>(a));
}

constexpr auto align_up(std::integral auto n, std::integral auto a)
{
    return align_down(n + a - 1, a);
}

} // namespace ca
