#pragma once

#include <array>
#include <functional>

#include <coal/allocator_traits.hpp>
#include <coal/memory_block.hpp>
#include <coal/null_allocator.hpp>

namespace coal {

template<typename Allocator>
class proxy_allocator
{
public:
    using allocator = Allocator;

    constexpr proxy_allocator() = default;

    [[nodiscard]] constexpr std::size_t get_alignment() const;

    [[nodiscard]] constexpr const allocator* get_allocator() const;
    [[nodiscard]] constexpr allocator* get_allocator();
    constexpr void set_allocator(allocator* allocator);

    template<typename Initializer>
    constexpr void init(Initializer& initializer);

    [[nodiscard]] constexpr memory_block allocate(std::size_t size);

    template<typename U = Allocator>
    requires(allocator_traits::has_owns<U>)
    [[nodiscard]] constexpr bool owns(const memory_block& block) const;

    template<typename U = Allocator>
    requires(allocator_traits::has_expand<U>)
    constexpr bool expand(memory_block& block, std::size_t delta);
    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);

    template<typename U = Allocator>
    requires(allocator_traits::has_deallocate_all<U>)
    constexpr void deallocate_all();

private:
    allocator* _allocator{nullptr};
};

template<typename Allocator>
constexpr std::size_t proxy_allocator<Allocator>::get_alignment() const
{
    assert(_allocator);
    return _allocator->get_alignment();
}

template<typename Allocator>
constexpr const proxy_allocator<Allocator>::allocator* proxy_allocator<Allocator>::get_allocator() const
{
    return _allocator;
}

template<typename Allocator>
constexpr proxy_allocator<Allocator>::allocator* proxy_allocator<Allocator>::get_allocator()
{
    return _allocator;
}

template<typename Allocator>
constexpr void proxy_allocator<Allocator>::set_allocator(allocator* allocator)
{
    _allocator = allocator;
}

template<typename Allocator>
template<typename Initializer>
constexpr void proxy_allocator<Allocator>::init(Initializer& initializer)
{
    // proxy goes first to allow the initializer to set the allocator
    initializer.init(*this);

    _allocator->init(initializer);
}

template<typename Allocator>
constexpr memory_block proxy_allocator<Allocator>::allocate(std::size_t size)
{
    assert(_allocator);
    return _allocator->allocate(size);
}

template<typename Allocator>
template<typename U>
requires(allocator_traits::has_owns<U>)
constexpr bool proxy_allocator<Allocator>::owns(const memory_block& block) const
{
    assert(_allocator);
    return _allocator->owns(block);
}

template<typename Allocator>
template<typename U>
requires(allocator_traits::has_expand<U>)
constexpr bool proxy_allocator<Allocator>::expand(memory_block& block, std::size_t delta)
{
    assert(_allocator);
    return _allocator->expand(block, delta);
}

template<typename Allocator>
constexpr bool proxy_allocator<Allocator>::reallocate(memory_block& block, std::size_t new_size)
{
    assert(_allocator);
    return _allocator->reallocate(block, new_size);
}

template<typename Allocator>
constexpr void proxy_allocator<Allocator>::deallocate(memory_block& block)
{
    assert(_allocator);
    _allocator->deallocate(block);
}

template<typename Allocator>
template<typename U>
requires(allocator_traits::has_deallocate_all<U>)
constexpr void proxy_allocator<Allocator>::deallocate_all()
{
    assert(_allocator);

    _allocator->deallocate_all();
}

} // namespace coal
