#pragma once

#include <cstring>

#include <ca/memory_block.hpp>

namespace ca::details {

struct default_reallocate_result
{
    bool success;
    bool reallocated;
};

template<typename AllocatorT>
constexpr default_reallocate_result try_default_reallocate(AllocatorT& allocator, memory_block& block, std::size_t new_size)
{
    if (block.size == new_size)
    {
        return {true, true};
    }

    if (new_size == 0)
    {
        allocator.deallocate(block);
        return {true, true};
    }

    if (!block)
    {
        block = allocator.allocate(new_size);
        return {true, block != nullblk};
    }

    return {false, false};
}

template<typename OriginalAllocatorT, typename NewAllocatorT>
constexpr bool reallocate_with_new_allocator(OriginalAllocatorT& original_allocator, NewAllocatorT& new_allocator, memory_block& block, std::size_t new_size)
{
    if (memory_block new_block = new_allocator.allocate(new_size))
    {
        std::memcpy(new_block.ptr, block.ptr, block.size);
        original_allocator.deallocate(block);
        block = new_block;

        return true;
    }

    return false;
}

} // namespace ca::details
