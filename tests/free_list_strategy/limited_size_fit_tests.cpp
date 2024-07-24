#include <catch2/catch_test_macros.hpp>

#include <coal/free_list_strategy/first_fit.hpp>
#include <coal/free_list_strategy/limited_size.hpp>
#include <coal/memory_block.hpp>

#include <free_list_mock.hpp>

namespace coal::free_list_strategy {

TEST_CASE("limited_size allocate", "[limited_size], [free_list_strategy]")
{
    limited_size<first_fit, 1> strategy;
    strategy.list_size = 1;

    mock::free_list list;
    list.add_node(10);

    free_list_node* node = strategy.allocate(list, 10);

    REQUIRE(node != nullptr);
    CHECK(node->size == 10);
    CHECK(node->next == nullptr);

    CHECK(strategy.allocate(list, 10) == nullptr);
}

TEST_CASE("limited_size deallocate", "[limited_size], [free_list_strategy]")
{
    limited_size<first_fit, 1> strategy;

    mock::free_list list;

    memory_block block = list.new_node_block(8);
    CHECK(strategy.deallocate(list, block));

    block = list.new_node_block(8);
    CHECK_FALSE(strategy.deallocate(list, block));
}

} // namespace coal::free_list_strategy
