#include <tuple>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <ca/prefixed_size_allocator.hpp>
#include <ca/stack_allocator.hpp>

#include <allocator_fixture.hpp>

namespace ca {

using prefixed_size_basic_allocators = std::tuple<
    prefixed_size_allocator<stack_allocator<0x1000, 4>>,
    prefixed_size_allocator<stack_allocator<0x1000, 8>>,
    prefixed_size_allocator<stack_allocator<0x1000, 16>>>;

TEMPLATE_LIST_TEST_CASE_METHOD(basic_allocator_fixture, "prefixed_size_allocator basics", "[prefixed_size_allocator], [allocator]", prefixed_size_basic_allocators)
{
    this->test_basics();
}

using prefixed_size_allocator_fixture = allocator_fixture<prefixed_size_allocator<stack_allocator<512>>>;

TEST_CASE_METHOD(prefixed_size_allocator_fixture, "prefixed_size_allocator allocate", "[prefixed_size_allocator], [allocator]")
{
    std::size_t size = GENERATE(4, 8, 12, 16, 32);

    memory_block block = allocator.allocate(size);

    CHECK(allocator.get_prefixed_size(block) == size);

    deallocate_and_check_is_nullblk(block);
}

TEST_CASE_METHOD(prefixed_size_allocator_fixture, "prefixed_size_allocator expand", "[prefixed_size_allocator], [allocator]")
{
    std::size_t size = GENERATE(4, 8, 12, 16, 32);

    memory_block block = allocator.allocate(size);
    allocator.expand(block, size);

    CHECK(allocator.get_prefixed_size(block) == size * 2);

    deallocate_and_check_is_nullblk(block);
}

TEST_CASE_METHOD(prefixed_size_allocator_fixture, "prefixed_size_allocator reallocate", "[prefixed_size_allocator], [allocator]")
{
    std::size_t size = GENERATE(4, 8, 12, 16, 32);

    memory_block block = allocator.allocate(size);
    allocator.reallocate(block, size * 2);

    CHECK(allocator.get_prefixed_size(block) == size * 2);

    deallocate_and_check_is_nullblk(block);
}

} // namespace ca
