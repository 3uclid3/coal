#pragma once

#include <array>
#include <cassert>
#include <functional>

#include <coal/alignment.hpp>
#include <coal/allocator_traits.hpp>
#include <coal/memory_block.hpp>
#include <coal/null_allocator.hpp>

namespace coal {

class proxy_interface_allocator
{
public:
    static constexpr std::size_t alignment = default_alignment;

    constexpr proxy_interface_allocator() = default;

    template<typename AllocatorT>
    constexpr explicit proxy_interface_allocator(AllocatorT& allocator);

    constexpr ~proxy_interface_allocator();

    [[nodiscard]] constexpr std::size_t get_alignment() const;

    template<typename Initializer>
    constexpr void init(Initializer& initializer);

    [[nodiscard]] constexpr memory_block allocate(std::size_t size);
    [[nodiscard]] constexpr bool owns(const memory_block& block) const;
    constexpr bool expand(memory_block& block, std::size_t delta);
    constexpr bool reallocate(memory_block& block, std::size_t new_size);
    constexpr void deallocate(memory_block& block);
    constexpr void deallocate_all();

    template<typename AllocatorT>
    constexpr void set_allocator(AllocatorT& allocator);
    constexpr void unset_allocator();
    constexpr bool has_allocator() const;

private:
    struct interface
    {
        using initializer_t = std::function<void(void*)>;

        virtual ~interface() = default;

        [[nodiscard]] virtual std::size_t get_alignment() const = 0;

        virtual void init(const initializer_t& initializer) = 0;

        [[nodiscard]] virtual memory_block allocate(std::size_t size) = 0;
        [[nodiscard]] virtual bool owns(const memory_block& block) const = 0;
        virtual bool expand(memory_block& block, std::size_t delta) = 0;
        virtual bool reallocate(memory_block& block, std::size_t new_size) = 0;
        virtual void deallocate(memory_block& block) = 0;
        virtual void deallocate_all() = 0;
    };

    template<typename AllocatorT>
    struct implementation : interface
    {
        constexpr explicit implementation(AllocatorT& allocator);

        [[nodiscard]] std::size_t get_alignment() const override;

        void init(const initializer_t& initializer) override;

        [[nodiscard]] memory_block allocate(std::size_t size) override;
        [[nodiscard]] bool owns(const memory_block& block) const override;
        bool expand(memory_block& block, std::size_t delta) override;
        bool reallocate(memory_block& block, std::size_t new_size) override;
        void deallocate(memory_block& block) override;
        void deallocate_all() override;

        std::reference_wrapper<AllocatorT> allocator;
    };

    // all implementations are the same alignment and size
    using null_implementation = implementation<null_allocator>;

    alignas(alignof(null_implementation)) std::array<std::uint8_t, sizeof(null_implementation)> _storage{};
    interface* _interface{nullptr};
};

template<typename AllocatorT>
constexpr proxy_interface_allocator::implementation<AllocatorT>::implementation(AllocatorT& allocator)
    : allocator{std::ref(allocator)}
{
}

template<typename AllocatorT>
std::size_t proxy_interface_allocator::implementation<AllocatorT>::get_alignment() const
{
    return allocator.get().get_alignment();
}
template<typename AllocatorT>
void proxy_interface_allocator::implementation<AllocatorT>::init(const initializer_t& initializer)
{
    initializer(&allocator.get());
}

template<typename AllocatorT>
memory_block proxy_interface_allocator::implementation<AllocatorT>::allocate(std::size_t size)
{
    return allocator.get().allocate(size);
}

template<typename AllocatorT>
bool proxy_interface_allocator::implementation<AllocatorT>::owns(const memory_block& block) const
{
    if constexpr (allocator_traits::has_owns<AllocatorT>)
    {
        return allocator.get().owns(block);
    }

    return false;
}

template<typename AllocatorT>
bool proxy_interface_allocator::implementation<AllocatorT>::expand(memory_block& block, std::size_t delta)
{
    if constexpr (allocator_traits::has_expand<AllocatorT>)
    {
        return allocator.get().expand(block, delta);
    }

    return false;
}

template<typename AllocatorT>
bool proxy_interface_allocator::implementation<AllocatorT>::reallocate(memory_block& block, std::size_t new_size)
{
    return allocator.get().reallocate(block, new_size);
}

template<typename AllocatorT>
void proxy_interface_allocator::implementation<AllocatorT>::deallocate(memory_block& block)
{
    allocator.get().deallocate(block);
}

template<typename AllocatorT>
void proxy_interface_allocator::implementation<AllocatorT>::deallocate_all()
{
    if constexpr (allocator_traits::has_deallocate_all<AllocatorT>)
    {
        return allocator.get().deallocate_all();
    }
}

template<typename AllocatorT>
constexpr proxy_interface_allocator::proxy_interface_allocator(AllocatorT& allocator)
{
    set_allocator(allocator);
}

constexpr proxy_interface_allocator::~proxy_interface_allocator()
{
    unset_allocator();
}

constexpr std::size_t proxy_interface_allocator::get_alignment() const
{
    assert(has_allocator());
    return _interface->get_alignment();
}

template<typename Initializer>
constexpr void proxy_interface_allocator::init(Initializer& initializer)
{
    // TODO init interface

    initializer.init(*this);
}

constexpr memory_block proxy_interface_allocator::allocate(std::size_t size)
{
    assert(has_allocator());
    return _interface->allocate(size);
}

constexpr bool proxy_interface_allocator::owns(const memory_block& block) const
{
    assert(has_allocator());
    return _interface->owns(block);
}

constexpr bool proxy_interface_allocator::expand(memory_block& block, std::size_t delta)
{
    assert(has_allocator());
    return _interface->expand(block, delta);
}

constexpr bool proxy_interface_allocator::reallocate(memory_block& block, std::size_t new_size)
{
    assert(has_allocator());
    return _interface->reallocate(block, new_size);
}

constexpr void proxy_interface_allocator::deallocate(memory_block& block)
{
    assert(has_allocator());
    _interface->deallocate(block);
}

constexpr void proxy_interface_allocator::deallocate_all()
{
    assert(has_allocator());
    _interface->deallocate_all();
}

template<typename AllocatorT>
constexpr void proxy_interface_allocator::set_allocator(AllocatorT& allocator)
{
    unset_allocator();

    _interface = new (_storage.data()) implementation<AllocatorT>{std::ref(allocator)};
}

constexpr void proxy_interface_allocator::unset_allocator()
{
    if (has_allocator())
    {
        _interface->~interface();
        _interface = nullptr;
    }
}

constexpr bool proxy_interface_allocator::has_allocator() const
{
    return _interface != nullptr;
}

} // namespace coal
