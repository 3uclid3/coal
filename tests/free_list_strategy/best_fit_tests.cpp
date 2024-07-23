#include <catch2/catch_test_macros.hpp>

#include <ca/free_list_strategy/best_fit.hpp>
#include <ca/memory_block.hpp>

#include <free_list_mock.hpp>

namespace ca::free_list_strategy {

TEST_CASE("best_fist allocate", "[best_fist], [free_list_strategy]")
{
    best_fit strategy;

    mock::free_list list;
    list.add_node(10);
    list.add_node(20);
    list.add_node(30);

    free_list_node* node = strategy.allocate(list, 15);

    REQUIRE(node != nullptr);
    CHECK(node->size == 20);
    CHECK(node->next == nullptr);

    node = strategy.allocate(list, 15);

    REQUIRE(node != nullptr);
    CHECK(node->size == 30);
    CHECK(node->next == nullptr);

    CHECK(strategy.allocate(list, 15) == nullptr);
}

TEST_CASE("best_fist deallocate", "[best_fist], [free_list_strategy]")
{
    best_fit strategy;
    mock::free_list list;

    free_list_node* previous_node = nullptr;
    for (size_t i = 0; i < 8; ++i)
    {
        memory_block block = list.new_node_block(8 + i);

        CHECK(strategy.deallocate(list, block));
        CHECK(list.first_node == block.as<free_list_node>());
        CHECK(list.first_node->size == block.size);
        CHECK(list.first_node->next == previous_node);

        previous_node = list.first_node;
    }
}

} // namespace ca::free_list_strategy
