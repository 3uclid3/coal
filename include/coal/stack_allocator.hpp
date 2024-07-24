#pragma once

#include <coal/alignment.hpp>
#include <coal/details/allocator_reallocation.hpp>
#include <coal/memory_block.hpp>

namespace coal {

template<std::size_t SizeT, std::size_t AlignmentT = default_alignment>
class stack_allocator
{
public:
    static constexpr std::size_t alignment = AlignmentT;
    static constexpr std::size_t max_size = SizeT;

public:
    constexpr std::size_t get_alignment() const;

    template<typename Initializer>
    constexpr void init(Initializer& initializer);

    [[nodiscard]] constexpr memory_block allocate(std::size_t size);
    [[nodiscard]] constexpr bool owns(const memory_block& block) const;
    constexpr bool expand(memory_block& block, std::size_t delta);
    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);
    constexpr void deallocate_all();

private:
    constexpr bool is_last_allocated_unaligned_block(const memory_block& block) const;

private:
    alignas(AlignmentT) std::uint8_t _data[SizeT];
    std::uint8_t* _ptr{_data};
};

template<std::size_t SizeT, std::size_t AlignmentT>
constexpr std::size_t stack_allocator<SizeT, AlignmentT>::get_alignment() const
{
    return AlignmentT;
}

template<std::size_t SizeT, std::size_t AlignmentT>
template<typename Initializer>
constexpr void stack_allocator<SizeT, AlignmentT>::init(Initializer& initializer)
{
    initializer.init(*this);
}

template<std::size_t SizeT, std::size_t AlignmentT>
constexpr memory_block stack_allocator<SizeT, AlignmentT>::allocate(std::size_t size)
{
    if (size == 0)
    {
        return nullblk;
    }

    const std::size_t aligned_size = align_up(size, alignment);

    if (_ptr + aligned_size > _data + SizeT)
    {
        return nullblk;
    }

    memory_block block{_ptr, size};

    _ptr += aligned_size;

    return block;
}

template<std::size_t SizeT, std::size_t AlignmentT>
constexpr bool stack_allocator<SizeT, AlignmentT>::owns(const memory_block& block) const
{
    return block && block.ptr >= _data && block.ptr < _data + SizeT;
}

template<std::size_t SizeT, std::size_t AlignmentT>
constexpr void stack_allocator<SizeT, AlignmentT>::deallocate(memory_block& block)
{
    if (!block)
    {
        return;
    }

    if (is_last_allocated_unaligned_block(block))
    {
        _ptr = block.as<std::uint8_t>();
    }

    block = nullblk;
}

template<std::size_t SizeT, std::size_t AlignmentT>
constexpr bool stack_allocator<SizeT, AlignmentT>::reallocate(memory_block& block, std::size_t new_size)
{
    if (auto [success, reallocated] = details::try_default_reallocate(*this, block, new_size); success)
    {
        return reallocated;
    }

    const std::size_t aligned_new_size = align_up(new_size, AlignmentT);

    if (is_last_allocated_unaligned_block(block))
    {
        if (block.as<std::uint8_t>() + aligned_new_size <= _data + SizeT)
        {
            block.size = new_size;
            _ptr = block.as<std::uint8_t>() + aligned_new_size;
            return true;
        }
        return false; // oom
    }

    const std::size_t aligned_size = align_up(block.size, AlignmentT);

    if (aligned_size >= aligned_new_size)
    {
        block.size = new_size;
        return true;
    }

    if (memory_block new_block = allocate(new_size))
    {
        memcpy(new_block.ptr, block.ptr, block.size);
        block = new_block;
        return true;
    }

    return false;
}

template<std::size_t SizeT, std::size_t AlignmentT>
constexpr bool stack_allocator<SizeT, AlignmentT>::expand(memory_block& block, std::size_t delta)
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

    if (!is_last_allocated_unaligned_block(block))
    {
        return false;
    }

    const std::size_t aligned_new_size = align_up(block.size + delta, alignment);

    if (block.as<std::uint8_t>() + aligned_new_size > _data + SizeT)
    {
        return false;
    }

    _ptr = block.as<std::uint8_t>() + aligned_new_size;
    block.size += delta;

    return true;
}

template<std::size_t SizeT, std::size_t AlignmentT>
constexpr void stack_allocator<SizeT, AlignmentT>::deallocate_all()
{
    _ptr = _data;
}

template<std::size_t SizeT, std::size_t AlignmentT>
constexpr bool stack_allocator<SizeT, AlignmentT>::is_last_allocated_unaligned_block(const memory_block& block) const
{
    return _ptr == static_cast<const std::uint8_t*>(block.ptr) + align_up(block.size, AlignmentT);
}

} // namespace coal