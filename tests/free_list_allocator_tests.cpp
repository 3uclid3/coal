#include <catch2/catch_template_test_macros.hpp>

#include <ca/free_list_allocator.hpp>
#include <ca/free_list_strategy/first_fit.hpp>
#include <ca/memory_block.hpp>
#include <ca/stack_allocator.hpp>

#include <allocator_fixture.hpp>

namespace ca {

using free_list_basic_allocators = std::tuple<
    free_list_allocator<stack_allocator<0x1000, 4>, free_list_strategy::first_fit>,
    free_list_allocator<stack_allocator<0x1000, 8>, free_list_strategy::first_fit>,
    free_list_allocator<stack_allocator<0x1000, 16>, free_list_strategy::first_fit>>;

TEMPLATE_LIST_TEST_CASE_METHOD(basic_allocator_fixture, "free_list_allocator basics", "[free_list_allocator], [allocator]", free_list_basic_allocators)
{
    this->test_basics();
}

} // namespace ca