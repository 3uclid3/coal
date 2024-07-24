#pragma once

#include <coal/def.hpp>

namespace coal {

struct free_list;
struct free_list_node;
struct memory_block;

} // namespace coal

namespace coal::free_list_strategy {

template<typename StrategyT, std::size_t MaxSizeT>
struct limited_size : private StrategyT
{
    using strategy = StrategyT;

    constexpr free_list_node* allocate(free_list& list, std::size_t size);
    constexpr bool deallocate(free_list& list, memory_block& block);

    std::size_t list_size{0};
};

template<typename StrategyT, std::size_t MaxSizeT>
constexpr free_list_node* limited_size<StrategyT, MaxSizeT>::allocate(free_list& list, std::size_t size)
{
    if (list_size == 0)
    {
        return nullptr;
    }

    free_list_node* node = strategy::allocate(list, size);

    if (node != nullptr)
    {
        --list_size;
    }

    return node;
}

template<typename StrategyT, std::size_t MaxSizeT>
constexpr bool limited_size<StrategyT, MaxSizeT>::deallocate(free_list& list, memory_block& block)
{
    if (list_size >= MaxSizeT)
    {
        return false;
    }

    if (!strategy::deallocate(list, block))
    {
        return false;
    }

    ++list_size;
    return true;
}

} // namespace coal::free_list_strategy
