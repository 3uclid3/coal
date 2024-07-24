#pragma once

#include <coal/alignment.hpp>
#include <coal/allocator_traits.hpp>
#include <coal/def.hpp>
#include <coal/details/allocator_reallocation.hpp>
#include <coal/memory_block.hpp>

namespace coal {

struct free_list_node
{
    free_list_node* next{nullptr};
    std::size_t size{0};
};

struct free_list
{
    constexpr operator bool() const { return first_node; }

    free_list_node* first_node{nullptr};
};

template<typename AllocatorT, typename StrategyT>
class free_list_allocator
{
public:
    using allocator = AllocatorT;
    using strategy = StrategyT;

    static constexpr std::size_t alignment = allocator::alignment;

public:
    constexpr std::size_t get_alignment() const;

    template<typename Initializer>
    constexpr void init(Initializer& initializer);

    [[nodiscard]] constexpr memory_block allocate(std::size_t size);

    template<typename U = AllocatorT>
    requires(allocator_traits::has_owns<U>)
    [[nodiscard]] constexpr bool owns(const memory_block& block) const;

    template<typename U = AllocatorT>
    requires(allocator_traits::has_expand<U>)
    constexpr bool expand(memory_block& block, std::size_t delta);
    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);

    template<typename U = AllocatorT>
    requires(allocator_traits::has_deallocate_all<U>)
    constexpr void deallocate_all();

private:
    allocator _allocator;
    strategy _strategy;
    free_list _free_list;
};

template<typename AllocatorT, typename StrategyT>
constexpr std::size_t free_list_allocator<AllocatorT, StrategyT>::get_alignment() const
{
    return alignment;
}

template<typename AllocatorT, typename StrategyT>
template<typename Initializer>
constexpr void free_list_allocator<AllocatorT, StrategyT>::init(Initializer& initializer)
{
    _allocator.init(initializer);
    _strategy.init(initializer);

    initializer.init(*this);
}

template<typename AllocatorT, typename StrategyT>
constexpr memory_block free_list_allocator<AllocatorT, StrategyT>::allocate(std::size_t size)
{
    const std::size_t aligned_size = align_up(size, alignment);

    if (_free_list)
    {
        if (free_list_node* node = _strategy.allocate(_free_list, aligned_size))
        {
            return memory_block{node, size};
        }
    }

    if (memory_block block = _allocator.allocate(aligned_size))
    {
        block.size = size;

        return block;
    }

    return nullblk;
}

template<typename AllocatorT, typename StrategyT>
template<typename U>
requires(allocator_traits::has_owns<U>)
constexpr bool free_list_allocator<AllocatorT, StrategyT>::owns(const memory_block& block) const
{
    return _allocator.owns(block);
}

template<typename AllocatorT, typename StrategyT>
template<typename U>
requires(allocator_traits::has_expand<U>)
constexpr bool free_list_allocator<AllocatorT, StrategyT>::expand(memory_block& block, std::size_t delta)
{
    return _allocator.expand(block, delta);
}

template<typename AllocatorT, typename StrategyT>
constexpr bool free_list_allocator<AllocatorT, StrategyT>::reallocate(memory_block& block, std::size_t new_size)
{
    if (auto [success, reallocated] = details::try_default_reallocate(*this, block, new_size); success)
    {
        return reallocated;
    }

    const std::size_t aligned_size = align_up(new_size, alignment);
    const std::size_t aligned_block_size = align_up(block.size, alignment);

    if (aligned_size < aligned_block_size)
    {
        block.size = new_size;
        return true;
    }

    if (memory_block new_block = allocate(new_size))
    {
        memcpy(new_block.ptr, block.ptr, block.size);

        deallocate(block);

        block = new_block;
        return true;
    }

    return false;
}

template<typename AllocatorT, typename StrategyT>
constexpr void free_list_allocator<AllocatorT, StrategyT>::deallocate(memory_block& block)
{
    if (!block)
    {
        return;
    }

    memory_block aligned_block(block.ptr, align_up(block.size, alignment));

    if (!_strategy.deallocate(_free_list, aligned_block))
    {
        _allocator.deallocate(aligned_block);
    }

    block = nullblk;
}

template<typename AllocatorT, typename StrategyT>
template<typename U>
requires(allocator_traits::has_deallocate_all<U>)
constexpr void free_list_allocator<AllocatorT, StrategyT>::deallocate_all()
{
    _allocator.deallocate_all();

    _free_list = {};
}

} // namespace coal
