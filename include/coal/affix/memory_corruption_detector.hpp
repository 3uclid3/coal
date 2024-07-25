#pragma once

#include <array>
#include <cassert>
#include <concepts>

namespace coal {

struct assert_memory_corruption_reporter
{
    template<std::integral T>
    static void report([[maybe_unused]] const void* corrupted_ptr, [[maybe_unused]] T expected, [[maybe_unused]] T actual)
    {
        assert(expected == actual);
    }
};

template<std::integral T, T PatternT, std::size_t SizeT = 32, typename ReporterT = assert_memory_corruption_reporter>
struct memory_corruption_detector
{
    static constexpr T pattern = PatternT;

    constexpr memory_corruption_detector();
    constexpr ~memory_corruption_detector();

    std::array<T, SizeT> guard;
};

template<std::integral T, T PatternT, std::size_t SizeT, typename ReporterT>
constexpr memory_corruption_detector<T, PatternT, SizeT, ReporterT>::memory_corruption_detector()
{
    guard.fill(pattern);
}

template<std::integral T, T PatternT, std::size_t SizeT, typename ReporterT>
constexpr memory_corruption_detector<T, PatternT, SizeT, ReporterT>::~memory_corruption_detector()
{
    for (const T& value : guard)
    {
        if (value != pattern)
        {
            ReporterT::report(&value, pattern, value);
        }
    }
}

} // namespace coal
