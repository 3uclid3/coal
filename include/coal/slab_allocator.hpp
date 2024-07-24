#pragma once

#include <array>

#include <coal/allocator_traits.hpp>
#include <coal/def.hpp>
#include <coal/details/allocator_reallocation.hpp>
#include <coal/memory_block.hpp>

namespace coal {

struct slab
{
    constexpr void* allocate()
    {
        void* ptr = first_free;
        first_free = *(static_cast<void**>(ptr));
        return ptr;
    }

    constexpr void deallocate(memory_block& block)
    {
        deallocate_at(block, 0);
    }

    constexpr void deallocate_at(memory_block& block, uintptr_t block_offset)
    {
        void* object = static_cast<uint8_t*>(block.ptr) + block_offset;
        *(static_cast<void**>(object)) = first_free;
        first_free = object;
    }

    void* first_free{nullptr};
};

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
class slab_allocator : private AllocatorT
{
    static_assert(SlabCapacityT > 0, "Slab capacity must be greater than zero.");
    static_assert((SlabCapacityT % AllocatorT::alignment) == 0, "Slab capacity must be a multiple of alignment.");
    static_assert((((SlabSizesT % AllocatorT::alignment) == 0) && ...), "Each allocation size must be a multiple of alignment.");
    static_assert(((SlabSizesT >= sizeof(void*)) && ...), "Each allocation size must be at least the size of a pointer.");

public:
    using allocator = AllocatorT;

    static constexpr std::size_t index_for_size(std::size_t size);
    static constexpr std::size_t size_at_index(std::size_t index);

    static constexpr std::size_t alignment = AllocatorT::alignment;
    static constexpr std::size_t max_size = size_at_index(sizeof...(SlabSizesT) - 1);

public:
    [[nodiscard]] constexpr std::size_t get_alignment() const;

    [[nodiscard]] constexpr memory_block allocate(std::size_t size);

    template<typename U = AllocatorT>
    requires(allocator_traits::has_owns<U>)
    [[nodiscard]] constexpr bool owns(const memory_block& block) const;

    constexpr bool expand(memory_block& block, std::size_t delta);
    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);

    template<typename U = AllocatorT>
    requires(allocator_traits::has_deallocate_all<U>)
    constexpr void deallocate_all();

private:
    void allocate_for_slab(slab& slab, std::size_t index);

    [[nodiscard]] constexpr memory_block unsafe_allocate(std::size_t size);
    constexpr void unsafe_deallocate(memory_block& block);

    std::array<slab, sizeof...(SlabSizesT)> _slabs{};
};

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
constexpr std::size_t slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::index_for_size(std::size_t size)
{
    std::size_t index = 0;
    (..., (index += (SlabSizesT < size ? 1 : 0)));
    return index;
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
constexpr std::size_t slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::size_at_index(std::size_t index)
{
    std::size_t size = 0;
    (..., (size = index-- == 0 ? SlabSizesT : size));
    return size;
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
constexpr std::size_t slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::get_alignment() const
{
    return alignment;
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
constexpr memory_block slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::allocate(std::size_t size)
{
    if (size == 0)
    {
        return nullblk;
    }

    if (size > max_size)
    {
        return nullblk;
    }

    return unsafe_allocate(size);
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
template<typename U>
requires(allocator_traits::has_owns<U>)
constexpr bool slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::owns(const memory_block& block) const
{
    return block.size <= size_at_index(sizeof...(SlabSizesT) - 1) && allocator::owns(block);
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
constexpr bool slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::expand(memory_block& block, std::size_t delta)
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

    const std::size_t new_size = block.size + delta;

    if (new_size > max_size)
    {
        return false;
    }

    const std::size_t index = index_for_size(block.size);
    const std::size_t new_index = index_for_size(new_size);

    if (index != new_index)
    {
        return false;
    }

    block.size = new_index;
    return true;
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
constexpr bool slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::reallocate(memory_block& block, std::size_t new_size)
{
    if (auto [success, reallocated] = details::try_default_reallocate(*this, block, new_size); success)
    {
        return reallocated;
    }

    const std::size_t index = index_for_size(block.size);
    const std::size_t new_index = index_for_size(new_size);

    if (index == new_index)
    {
        block.size = new_size;
        return true;
    }

    if (memory_block new_block = unsafe_allocate(new_size))
    {
        unsafe_deallocate(block);
        block = new_block;
        return true;
    }

    return false;
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
constexpr void slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::deallocate(memory_block& block)
{
    if (!block)
    {
        return;
    }

    std::size_t index = index_for_size(block.size);

    if (index >= _slabs.size())
    {
        return;
    }

    _slabs[index].deallocate(block);

    block = nullblk;
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
template<typename U>
requires(allocator_traits::has_deallocate_all<U>)
constexpr void slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::deallocate_all()
{
    AllocatorT::deallocate_all();

    _slabs = {};
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
void slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::allocate_for_slab(slab& slab, std::size_t index)
{
    memory_block block = AllocatorT::allocate(SlabCapacityT);

    if (!block)
    {
        return;
    }

    const std::size_t object_size = size_at_index(index);
    const std::size_t objects_count = block.size / object_size;

    for (std::size_t i = 0; i < objects_count; ++i)
    {
        slab.deallocate_at(block, i * object_size);
    }
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
constexpr memory_block slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::unsafe_allocate(std::size_t size)
{
    std::size_t index = index_for_size(size);

    assert(index < _slabs.size());

    slab& slab = _slabs[index];

    if (slab.first_free == nullptr)
    {
        allocate_for_slab(slab, index);
    }

    return memory_block{slab.allocate(), size};
}

template<typename AllocatorT, std::size_t SlabCapacityT, std::size_t... SlabSizesT>
constexpr void slab_allocator<AllocatorT, SlabCapacityT, SlabSizesT...>::unsafe_deallocate(memory_block& block)
{
    std::size_t index = index_for_size(block.size);

    assert(index < _slabs.size());

    _slabs[index].deallocate(block);

    block = nullblk;
}

} // namespace coal
