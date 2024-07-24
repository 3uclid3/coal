#pragma once

#include <coal/free_list_allocator.hpp>

namespace coal::free_list_strategy {

struct first_fit
{
    constexpr free_list_node* allocate(free_list& list, std::size_t size);
    constexpr bool deallocate(free_list& list, memory_block& block);
};

constexpr free_list_node* first_fit::allocate(free_list& list, std::size_t size)
{
    free_list_node* prev_node = nullptr;
    free_list_node* node = list.first_node;

    while (node)
    {
        if (node->size >= size)
        {
            if (prev_node)
            {
                prev_node->next = node->next;
            }
            else
            {
                list.first_node = node->next;
            }

            node->next = nullptr;
            return node;
        }

        prev_node = node;
        node = node->next;
    }

    return nullptr;
}

constexpr bool first_fit::deallocate(free_list& list, memory_block& block)
{
    free_list_node* node = block.as<free_list_node>();

    node->size = block.size;
    node->next = list.first_node;

    list.first_node = node;

    return true;
}

} // namespace coal::free_list_strategy
