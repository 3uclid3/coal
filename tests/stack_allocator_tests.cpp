#include <catch2/catch_template_test_macros.hpp>

#include <tuple>

#include <allocator_fixture.hpp>
#include <coal/stack_allocator.hpp>

namespace coal {

using stack_allocator_types = std::tuple<
    stack_allocator<0x1000, 4>,
    stack_allocator<0x1000, 8>,
    stack_allocator<0x1000, 16>>;

TEMPLATE_LIST_TEST_CASE_METHOD(basic_allocator_fixture, "stack_allocator basics", "[stack_allocator], [allocator]", stack_allocator_types)
{
    this->test_basics();
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator allocate returns nullblk when allocating 0 byte", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(0);

    CHECK(block == nullblk);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator allocate returns size", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block0 = this->allocator.allocate(1);
    memory_block block1 = this->allocator.allocate(TestType::alignment + 1);

    CHECK(block0.ptr != nullptr);
    CHECK(block0.size == 1);

    CHECK(block1.ptr != nullptr);
    CHECK(block1.size == TestType::alignment + 1);

    this->deallocate_and_check_is_nullblk(block0);
    this->deallocate_and_check_is_nullblk(block1);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator allocate can allocate all stack", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(TestType::max_size);

    CHECK(block.ptr != nullptr);
    CHECK(block.size == TestType::max_size);

    this->deallocate_and_check_is_nullblk(block);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator allocate returns nullblk when request size is > available size", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(TestType::max_size + 1);
    CHECK(block == nullblk);

    block = this->allocator.allocate(1);
    REQUIRE(block);

    memory_block next_block = this->allocator.allocate(TestType::max_size);
    CHECK(next_block == nullblk);

    this->deallocate_and_check_is_nullblk(block);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator allocate returns nullblk when out of memory", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(TestType::max_size);
    REQUIRE(block != nullblk);

    memory_block next_block = this->allocator.allocate(1);
    CHECK(next_block == nullblk);

    this->deallocate_and_check_is_nullblk(block);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator allocate reuse last deallocated if it was the most recent allocation", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block0 = this->allocator.allocate(8);
    memory_block block1 = this->allocator.allocate(8);

    auto ptr1 = block1.ptr;
    this->deallocate_and_check_is_nullblk(block1);

    memory_block block2 = this->allocator.allocate(8);
    CHECK(block2.ptr == ptr1);

    this->deallocate_and_check_is_nullblk(block0);
    this->deallocate_and_check_is_nullblk(block2);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator allocate dot no reuse last deallocated if it was not the most recent allocation", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block0 = this->allocator.allocate(8);
    memory_block block1 = this->allocator.allocate(8);

    auto ptr0 = block0.ptr;
    this->deallocate_and_check_is_nullblk(block0);

    memory_block block2 = this->allocator.allocate(8);
    CHECK(block2.ptr != ptr0);

    this->deallocate_and_check_is_nullblk(block1);
    this->deallocate_and_check_is_nullblk(block2);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator reallocate increase current allocation if its the most recent allocation", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(4);
    *static_cast<std::uint32_t*>(block.ptr) = 42;

    memory_block pre_realloc_block = block;
    CHECK(this->allocator.reallocate(block, 8));

    CHECK(block.ptr == pre_realloc_block.ptr);
    CHECK(block.size == 8);
    CHECK(*static_cast<std::uint32_t*>(block.ptr) == 42);

    this->deallocate_and_check_is_nullblk(block);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator reallocate allocate a new block if its not the most recent allocation", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(TestType::alignment);
    *static_cast<std::uint32_t*>(block.ptr) = 42;

    memory_block second_block = this->allocator.allocate(TestType::alignment);
    *static_cast<std::uint32_t*>(second_block.ptr) = 69;

    memory_block pre_realloc_block = block;
    CHECK(this->allocator.reallocate(block, TestType::alignment * 2));

    CHECK(block.ptr != pre_realloc_block.ptr);
    CHECK(block.size == TestType::alignment * 2);
    CHECK(*static_cast<std::uint32_t*>(block.ptr) == 42);
    CHECK(*static_cast<std::uint32_t*>(second_block.ptr) == 69);

    this->deallocate_and_check_is_nullblk(block);
    this->deallocate_and_check_is_nullblk(second_block);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator reallocate fail when out of memory", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(TestType::max_size);

    memory_block pre_realloc_block = block;
    CHECK_FALSE(this->allocator.reallocate(block, TestType::max_size + 1));

    CHECK(block == pre_realloc_block);

    this->deallocate_and_check_is_nullblk(block);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator reallocate decrease current allocation if its the most recent allocation", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(8);
    *static_cast<std::uint32_t*>(block.ptr) = 42;

    memory_block pre_realloc_block = block;
    CHECK(this->allocator.reallocate(block, 4));

    CHECK(block.ptr == pre_realloc_block.ptr);
    CHECK(block.size == 4);
    CHECK(*static_cast<std::uint32_t*>(block.ptr) == 42);

    this->deallocate_and_check_is_nullblk(block);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator reallocate decrease current allocation if its not the most recent allocation", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(8);
    *static_cast<std::uint32_t*>(block.ptr) = 42;

    memory_block second_block = this->allocator.allocate(4);
    *static_cast<std::uint32_t*>(second_block.ptr) = 69;

    memory_block pre_realloc_block = block;
    CHECK(this->allocator.reallocate(block, 4));

    CHECK(block.ptr == pre_realloc_block.ptr);
    CHECK(block.size == 4);
    CHECK(*static_cast<std::uint32_t*>(block.ptr) == 42);
    CHECK(*static_cast<std::uint32_t*>(second_block.ptr) == 69);

    this->deallocate_and_check_is_nullblk(block);
    this->deallocate_and_check_is_nullblk(second_block);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator expand nullblk by 0 returns true", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = nullblk;
    CHECK(this->allocator.expand(block, 0));
    CHECK(block == nullblk);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator expand by 0 returns same block", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = this->allocator.allocate(8);
    memory_block pre_expand_block = block;

    CHECK(this->allocator.expand(block, 0));
    CHECK(block == pre_expand_block);

    this->deallocate_and_check_is_nullblk(block);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator expand nullblk by non-zero allocate a new block", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = nullblk;
    CHECK(this->allocator.expand(block, 4));
    CHECK(block.ptr != nullptr);
    CHECK(block.size == 4);
}

TEMPLATE_LIST_TEST_CASE_METHOD(allocator_fixture, "stack_allocator expand over capacity returns false", "[stack_allocator], [allocator]", stack_allocator_types)
{
    memory_block block = nullblk;
    CHECK_FALSE(this->allocator.expand(block, TestType::max_size + 1));
    CHECK(block == nullblk);

    block = this->allocator.allocate(4);

    memory_block pre_expand_block = block;
    CHECK_FALSE(this->allocator.expand(block, TestType::max_size + 1));
    CHECK(block == pre_expand_block);

    this->deallocate_and_check_is_nullblk(block);
}

} // namespace coal
