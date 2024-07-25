#pragma once

#include <array>
#include <cassert>

#include <coal/free_list_allocator.hpp>
#include <coal/memory_block.hpp>

namespace coal::mock {

struct free_list : coal::free_list
{
    constexpr memory_block new_node_block(size_t size)
    {
        return memory_block{new_node(size), size};
    }

    constexpr free_list_node* new_node(size_t size)
    {
        assert(used_size < nodes.size());
        free_list_node* node = &nodes[used_size++];
        node->size = size;
        node->next = nullptr;
        return node;
    }

    constexpr free_list_node* add_node(size_t size)
    {
        free_list_node* node = new_node(size);

        if (last_node == nullptr)
        {
            first_node = node;
            last_node = node;
        }
        else
        {
            assert(last_node->next == nullptr);
            last_node->next = node;
            last_node = node;
        }

        return node;
    }

    std::array<free_list_node, 32> nodes{};
    size_t used_size{0};
    free_list_node* last_node{nullptr};
};

} // namespace coal::mock
