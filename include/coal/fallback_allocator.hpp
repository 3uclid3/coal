#pragma once

#include <coal/allocator_traits.hpp>
#include <coal/details/allocator_reallocation.hpp>
#include <coal/memory_block.hpp>

namespace coal {

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
class fallback_allocator
{
public:
    using primary = PrimaryAllocatorT;
    using fallback = FallbackAllocatorT;

    static constexpr std::size_t alignment = primary::alignment > fallback::alignment ? primary::alignment : fallback::alignment;

public:
    [[nodiscard]] constexpr std::size_t get_alignment() const;

    [[nodiscard]] constexpr const primary& get_primary_allocator() const;
    [[nodiscard]] constexpr primary& get_primary_allocator();

    [[nodiscard]] constexpr const fallback& get_fallback_allocator() const;
    [[nodiscard]] constexpr fallback& get_fallback_allocator();

    template<typename Initializer>
    constexpr void init(Initializer& initializer);

    [[nodiscard]] constexpr memory_block allocate(std::size_t size);

    template<typename U = PrimaryAllocatorT, typename V = FallbackAllocatorT>
    requires(allocator_traits::has_owns<U> && allocator_traits::has_owns<V>)
    [[nodiscard]] constexpr bool owns(const memory_block& block) const;

    template<typename U = PrimaryAllocatorT, typename V = FallbackAllocatorT>
    requires(allocator_traits::has_expand<U> || allocator_traits::has_expand<V>)
    constexpr bool expand(memory_block& block, std::size_t delta);
    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);

    template<typename U = PrimaryAllocatorT, typename V = FallbackAllocatorT>
    requires(allocator_traits::has_deallocate_all<U> && allocator_traits::has_deallocate_all<V>)
    constexpr void deallocate_all();

private:
    primary _primary;
    fallback _fallback;
};

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
constexpr std::size_t fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::get_alignment() const
{
    return alignment;
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
constexpr const fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::primary& fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::get_primary_allocator() const
{
    return _primary;
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
constexpr fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::primary& fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::get_primary_allocator()
{
    return _primary;
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
constexpr const fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::fallback& fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::get_fallback_allocator() const
{
    return _fallback;
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
constexpr fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::fallback& fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::get_fallback_allocator()
{
    return _fallback;
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
template<typename Initializer>
constexpr void fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::init(Initializer& initializer)
{
    _primary.init(initializer);
    _fallback.init(initializer);

    initializer.init(*this);
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
constexpr memory_block fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::allocate(std::size_t size)
{
    if (size == 0)
    {
        return nullblk;
    }

    if (memory_block block = _primary.allocate(size))
    {
        return block;
    }

    return _fallback.allocate(size);
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
template<typename U, typename V>
requires(allocator_traits::has_owns<U> && allocator_traits::has_owns<V>)
constexpr bool fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::owns(const memory_block& block) const
{
    return _primary.owns(block) || _fallback.owns(block);
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
template<typename U, typename V>
requires(allocator_traits::has_expand<U> || allocator_traits::has_expand<V>)
constexpr bool fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::expand(memory_block& block, std::size_t delta)
{
    if constexpr (allocator_traits::has_expand<primary>)
    {
        if (_primary.owns(block))
        {
            return _primary.expand(block, delta);
        }
    }

    if constexpr (allocator_traits::has_expand<fallback>)
    {
        return _fallback.expand(block, delta);
    }

    return false;
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
constexpr bool fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::reallocate(memory_block& block, std::size_t new_size)
{
    const bool is_primary_owner = _primary.owns(block);

    if (is_primary_owner)
    {
        if (auto [success, reallocated] = details::try_default_reallocate(_primary, block, new_size); success)
        {
            return reallocated;
        }
    }
    else if (auto [success, reallocated] = details::try_default_reallocate(_fallback, block, new_size); success)
    {
        return reallocated;
    }

    if (is_primary_owner)
    {
        if (_primary.reallocate(block, new_size))
        {
            return true;
        }

        return details::reallocate_with_new_allocator(_primary, _fallback, block, new_size);
    }

    return _fallback.reallocate(block, new_size);
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
constexpr void fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::deallocate(memory_block& block)
{
    if (!block)
    {
        return;
    }

    if (_primary.owns(block))
    {
        _primary.deallocate(block);
    }
    else
    {
        _fallback.deallocate(block);
    }
}

template<typename PrimaryAllocatorT, typename FallbackAllocatorT>
template<typename U, typename V>
requires(allocator_traits::has_deallocate_all<U> && allocator_traits::has_deallocate_all<V>)
constexpr void fallback_allocator<PrimaryAllocatorT, FallbackAllocatorT>::deallocate_all()
{
    _primary.deallocate_all();
    _fallback.deallocate_all();
}

} // namespace coal
