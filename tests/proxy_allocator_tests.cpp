#include <catch2/catch_test_macros.hpp>

#include <coal/proxy_allocator.hpp>
#include <coal/stack_allocator.hpp>

#include <allocator_fixture.hpp>

namespace coal {

TEST_CASE_METHOD(basic_allocator_fixture<proxy_allocator<stack_allocator<0x1000>>>, "proxy_allocator basics", "[proxy_allocator], [allocator]")
{
    stack_allocator<0x1000> stack_allocator;
    allocator.set_allocator(&stack_allocator);

    this->test_basics();
}

} // namespace coal
