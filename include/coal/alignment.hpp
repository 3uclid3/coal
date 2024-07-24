#pragma once

#include <concepts>
#include <type_traits>

#ifndef COAL_DEFAULT_ALIGNMENT
#define COAL_DEFAULT_ALIGNMENT 8
#endif

namespace coal {

inline static constexpr std::size_t default_alignment{COAL_DEFAULT_ALIGNMENT};

constexpr auto align_down(std::integral auto n, std::integral auto a)
{
    constexpr auto align_down_impl = [](auto un, auto ua) {
        return (un & ~(ua - 1));
    };
    return align_down_impl(std::make_unsigned_t<decltype(n)>(n), std::make_unsigned_t<decltype(a)>(a));
}

constexpr auto align_up(std::integral auto n, std::integral auto a)
{
    return align_down(n + a - 1, a);
}

} // namespace coal
