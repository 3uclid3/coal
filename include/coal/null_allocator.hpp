#pragma once

#include <cassert>

#include <coal/def.hpp>
#include <coal/memory_block.hpp>

namespace coal {

class null_allocator
{
public:
    static constexpr std::size_t alignment = 64 * 1024;

public:
    [[nodiscard]] constexpr std::size_t get_alignment() const;

    template<typename Initializer>
    constexpr void init(Initializer& initializer);

    [[nodiscard]] constexpr memory_block allocate(std::size_t size);
    [[nodiscard]] constexpr bool owns(const memory_block& block) const;

    constexpr bool expand(memory_block& block, std::size_t delta);
    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);
    constexpr void deallocate_all();
};

constexpr std::size_t null_allocator::get_alignment() const
{
    return alignment;
}

template<typename Initializer>
constexpr void null_allocator::init(Initializer& initializer)
{
    initializer.init(*this);
}

constexpr memory_block null_allocator::allocate(std::size_t size)
{
    COAL_UNUSED(size);

    return nullblk;
}

constexpr bool null_allocator::owns(const memory_block& block) const
{
    return !block;
}

constexpr bool null_allocator::expand(memory_block& block, std::size_t delta)
{
    COAL_UNUSED(block);
    COAL_UNUSED(delta);

    assert(!block);
    return false;
}

constexpr bool null_allocator::reallocate(memory_block& block, std::size_t new_size)
{
    COAL_UNUSED(block);
    COAL_UNUSED(new_size);

    assert(!block);
    return false;
}

constexpr void null_allocator::deallocate(memory_block& block)
{
    COAL_UNUSED(block);

    assert(!block);
}

constexpr void null_allocator::deallocate_all()
{
}

} // namespace coal
