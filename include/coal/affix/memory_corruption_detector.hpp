#pragma once

#include <array>
#include <concepts>

#include <coal/def.hpp>

namespace coal {

struct assert_memory_corruption_reporter
{
    template<std::integral T>
    static void report(const void* corrupted_ptr, T expected, T actual)
    {
        assert(expected == actual);
        COAL_UNUSED(corrupted_ptr);
        COAL_UNUSED(expected);
        COAL_UNUSED(actual);
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
