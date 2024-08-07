#pragma once

#include <coal/alignment.hpp>
#include <coal/allocator_traits.hpp>
#include <coal/details/allocator_reallocation.hpp>
#include <coal/memory_block.hpp>

namespace coal {

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
class segregator_allocator
{
public:
    using small = SmallAllocatorT;
    using large = LargeAllocatorT;

    static constexpr std::size_t alignment = small::alignment > large::alignment ? small::alignment : large::alignment;
    static constexpr std::size_t threshold = ThresholdT;

public:
    [[nodiscard]] constexpr std::size_t get_alignment() const;

    [[nodiscard]] constexpr const small& get_small_allocator() const;
    [[nodiscard]] constexpr small& get_small_allocator();

    [[nodiscard]] constexpr const large& get_large_allocator() const;
    [[nodiscard]] constexpr large& get_large_allocator();

    template<typename Initializer>
    constexpr void init(Initializer& initializer);

    [[nodiscard]] constexpr memory_block allocate(std::size_t size);

    template<typename U = SmallAllocatorT, typename V = LargeAllocatorT>
    requires(allocator_traits::has_owns<U> && allocator_traits::has_owns<V>)
    [[nodiscard]] constexpr bool owns(const memory_block& block) const;

    template<typename U = SmallAllocatorT, typename V = LargeAllocatorT>
    requires(allocator_traits::has_expand<U> || allocator_traits::has_expand<V>)
    constexpr bool expand(memory_block& block, std::size_t delta);
    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);

    template<typename U = SmallAllocatorT, typename V = LargeAllocatorT>
    requires(allocator_traits::has_deallocate_all<U> && allocator_traits::has_deallocate_all<V>)
    constexpr void deallocate_all();

private:
    constexpr bool is_small(std::size_t size) const;

    static_assert(ThresholdT > 0, "Threshold must be greater than 0");
    static_assert(align_up(ThresholdT, alignment) == ThresholdT, "Threshold must be a multiple of the alignment");

    small _small;
    large _large;
};

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
constexpr std::size_t segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::get_alignment() const
{
    return alignment;
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
constexpr const segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::small& segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::get_small_allocator() const
{
    return _small;
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
constexpr segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::small& segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::get_small_allocator()
{
    return _small;
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
constexpr const segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::large& segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::get_large_allocator() const
{
    return _large;
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
constexpr segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::large& segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::get_large_allocator()
{
    return _large;
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
template<typename Initializer>
constexpr void segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::init(Initializer& initializer)
{
    _small.init(initializer);
    _large.init(initializer);

    initializer.init(*this);
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
constexpr memory_block segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::allocate(std::size_t size)
{
    if (is_small(size))
    {
        return _small.allocate(size);
    }

    return _large.allocate(size);
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
template<typename U, typename V>
requires(allocator_traits::has_owns<U> && allocator_traits::has_owns<V>)
constexpr bool segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::owns(const memory_block& block) const
{
    if (is_small(block.size))
    {
        return _small.owns(block);
    }

    return _large.owns(block);
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
template<typename U, typename V>
requires(allocator_traits::has_expand<U> || allocator_traits::has_expand<V>)
constexpr bool segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::expand(memory_block& block, std::size_t delta)
{
    if (is_small(block.size))
    {
        if constexpr (allocator_traits::has_expand<SmallAllocatorT>)
        {
            return _small.expand(block, delta);
        }
    }
    else if constexpr (allocator_traits::has_expand<LargeAllocatorT>)
    {
        return _large.expand(block, delta);
    }

    return false;
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
constexpr bool segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::reallocate(memory_block& block, std::size_t new_size)
{
    if (auto [success, reallocated] = details::try_default_reallocate(*this, block, new_size); success)
    {
        return reallocated;
    }

    // from small block?
    if (is_small(block.size))
    {
        if (is_small(new_size))
        {
            return _small.reallocate(block, new_size);
        }

        return details::reallocate_with_new_allocator(_small, _large, block, new_size);
    }

    // from large block to small block?
    if (is_small(new_size))
    {
        return details::reallocate_with_new_allocator(_large, _small, block, new_size);
    }

    return _large.reallocate(block, new_size);
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
constexpr void segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::deallocate(memory_block& block)
{
    if (is_small(block.size))
    {
        _small.deallocate(block);
    }
    else
    {
        _large.deallocate(block);
    }
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
template<typename U, typename V>
requires(allocator_traits::has_deallocate_all<U> && allocator_traits::has_deallocate_all<V>)
constexpr void segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::deallocate_all()
{
    _small.deallocate_all();
    _large.deallocate_all();
}

template<typename SmallAllocatorT, typename LargeAllocatorT, std::size_t ThresholdT>
constexpr bool segregator_allocator<SmallAllocatorT, LargeAllocatorT, ThresholdT>::is_small(std::size_t size) const
{
    return size <= threshold;
}

} // namespace coal