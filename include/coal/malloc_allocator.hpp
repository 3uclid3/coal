#pragma once

#include <cassert>
#include <cstdlib>

#include <coal/alignment.hpp>
#include <coal/memory_block.hpp>

namespace coal {

class malloc_allocator
{
public:
    static constexpr std::size_t alignment = default_alignment;

public:
    [[nodiscard]] constexpr std::size_t get_alignment() const;

    template<typename Initializer>
    constexpr void init(Initializer& initializer);

    [[nodiscard]] constexpr memory_block allocate(std::size_t size);

    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);
};

constexpr std::size_t malloc_allocator::get_alignment() const
{
    return alignment;
}

template<typename Initializer>
constexpr void malloc_allocator::init(Initializer& initializer)
{
    initializer.init(*this);
}

constexpr memory_block malloc_allocator::allocate([[maybe_unused]] std::size_t size)
{
    return memory_block{std::malloc(size), size};
}

constexpr bool malloc_allocator::reallocate([[maybe_unused]] memory_block& block, [[maybe_unused]] std::size_t new_size)
{
    if (void* ptr = std::realloc(block.ptr, new_size))
    {
        block.ptr = ptr;
        block.size = new_size;
        return true;
    }

    return false;
}

constexpr void malloc_allocator::deallocate([[maybe_unused]] memory_block& block)
{
    std::free(block.ptr);
}

} // namespace coal
