#pragma once

#include <coal/affix_allocator.hpp>
#include <coal/def.hpp>
#include <coal/details/allocator_reallocation.hpp>
#include <coal/memory_block.hpp>

namespace coal {

template<typename AllocatorT>
class prefixed_size_allocator : private affix_allocator<AllocatorT, std::size_t>
{
    using super = affix_allocator<AllocatorT, std::size_t>;

public:
    static constexpr std::size_t alignment = super::alignment;

public:
    HEDLEY_WARN_UNUSED_RESULT constexpr std::size_t get_alignment() const;

    template<typename Initializer>
    constexpr void init(Initializer& initializer);

    HEDLEY_WARN_UNUSED_RESULT constexpr memory_block allocate(std::size_t size);

    template<typename U = AllocatorT>
    requires(allocator_traits::has_owns<U>)
    HEDLEY_WARN_UNUSED_RESULT constexpr bool owns(const memory_block& block) const;

    template<typename U = AllocatorT>
    requires(allocator_traits::has_expand<U>)
    constexpr bool expand(memory_block& block, std::size_t delta);
    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);

    constexpr std::size_t get_prefixed_size(const memory_block& block) const;

private:
    constexpr void set_prefixed_size(const memory_block& block, std::size_t size);
};

template<typename AllocatorT>
constexpr std::size_t prefixed_size_allocator<AllocatorT>::get_alignment() const
{
    return alignment;
}

template<typename AllocatorT>
template<typename Initializer>
constexpr void prefixed_size_allocator<AllocatorT>::init(Initializer& initializer)
{
    initializer.init(*this);
}

template<typename AllocatorT>
constexpr memory_block prefixed_size_allocator<AllocatorT>::allocate(std::size_t size)
{
    if (memory_block block = super::allocate(size))
    {
        set_prefixed_size(block, size);
        return block;
    }
    return nullblk;
}

template<typename AllocatorT>
template<typename U>
requires(allocator_traits::has_owns<U>)
constexpr bool prefixed_size_allocator<AllocatorT>::owns(const memory_block& block) const
{
    return block && super::owns(memory_block(block.ptr, get_prefixed_size(block)));
}

template<typename AllocatorT>
template<typename U>
requires(allocator_traits::has_expand<U>)
constexpr bool prefixed_size_allocator<AllocatorT>::expand(memory_block& block, std::size_t delta)
{
    if (delta == 0)
    {
        return true;
    }

    if (!block)
    {
        block = allocate(delta);
        return block;
    }

    memory_block sized_block(block.ptr, get_prefixed_size(block));

    if (!super::expand(sized_block, delta))
    {
        return false;
    }

    set_prefixed_size(sized_block, sized_block.size);

    block.ptr = sized_block.ptr;
    block.size += delta;
    return true;
}

template<typename AllocatorT>
constexpr bool prefixed_size_allocator<AllocatorT>::reallocate(memory_block& block, std::size_t new_size)
{
    if (auto [success, reallocated] = details::try_default_reallocate(*this, block, new_size); success)
    {
        return reallocated;
    }

    memory_block sized_block(block.ptr, get_prefixed_size(block));

    if (!super::reallocate(sized_block, new_size))
    {
        return false;
    }

    set_prefixed_size(sized_block, sized_block.size);

    block.ptr = sized_block.ptr;
    block.size = new_size;
    return true;
}

template<typename AllocatorT>
constexpr void prefixed_size_allocator<AllocatorT>::deallocate(memory_block& block)
{
    if (block)
    {
        memory_block sized_block(block.ptr, get_prefixed_size(block));
        super::deallocate(sized_block);
        block = nullblk;
    }
}

template<typename AllocatorT>
constexpr std::size_t prefixed_size_allocator<AllocatorT>::get_prefixed_size(const memory_block& block) const
{
    const std::size_t* ptr = super::get_prefix(block);
    assert(ptr != nullptr);
    return *ptr;
}

template<typename AllocatorT>
constexpr void prefixed_size_allocator<AllocatorT>::set_prefixed_size(const memory_block& block, std::size_t size)
{
    std::size_t* ptr = super::get_prefix(block);
    assert(ptr != nullptr);
    *ptr = size;
}

} // namespace coal
