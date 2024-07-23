#include <catch2/catch_template_test_macros.hpp>

#include <ca/fallback_allocator.hpp>
#include <ca/memory_block.hpp>
#include <ca/stack_allocator.hpp>

#include <allocator_fixture.hpp>
#include <allocator_mock.hpp>

namespace ca {

using fallback_basic_allocators = std::tuple<
    fallback_allocator<stack_allocator<0x100, 4>, stack_allocator<0x1000, 4>>,
    fallback_allocator<stack_allocator<0x100, 8>, stack_allocator<0x1000, 8>>,
    fallback_allocator<stack_allocator<0x100, 16>, stack_allocator<0x1000, 16>>>;

TEMPLATE_LIST_TEST_CASE_METHOD(basic_allocator_fixture, "fallback_allocator basics", "[fallback_allocator], [allocator]", fallback_basic_allocators)
{
    this->test_basics();
}

struct primary_tag
{};
struct fallback_tag
{};

using mock_fallback_allocator = fallback_allocator<mock::basic_allocator<primary_tag>, mock::basic_allocator<fallback_tag>>;

struct fallback_allocator_fixture : allocator_fixture<mock_fallback_allocator>
{
    using mock_primary = mock::basic_allocator<primary_tag>;
    using mock_fallback = mock::basic_allocator<fallback_tag>;

    fallback_allocator_fixture()
    {
        mock_primary::reset_mock();
        mock_fallback::reset_mock();
    }
};

TEST_CASE_METHOD(fallback_allocator_fixture, "fallback_allocator allocate zero returns nullblk", "[fallback_allocator], [allocator]")
{
    mock_fallback_allocator allocator;
    memory_block block = allocator.allocate(0);

    CHECK(block == nullblk);
}

TEST_CASE_METHOD(fallback_allocator_fixture, "fallback_allocator allocate with primary allocator", "[fallback_allocator], [allocator]")
{
    mock_fallback_allocator allocator;
    mock_primary::allocate_block = memory_block{&allocator, sizeof(mock_fallback_allocator)};

    memory_block allocated_block = allocator.allocate(12);

    CHECK(mock_primary::allocate_block == allocated_block);
    CHECK(mock_primary::allocate_count == 1);
    CHECK(mock_fallback::allocate_count == 0);
}

TEST_CASE_METHOD(fallback_allocator_fixture, "fallback_allocator allocate with fallback allocator", "[fallback_allocator], [allocator]")
{
    mock_fallback_allocator allocator;
    mock_fallback::allocate_block = memory_block{&allocator, sizeof(mock_fallback_allocator)};

    memory_block allocated_block = allocator.allocate(12);

    CHECK(mock_fallback::allocate_block == allocated_block);
    CHECK(mock_fallback::allocate_count == 1);
    CHECK(mock_primary::allocate_count == 1);
}

TEST_CASE_METHOD(fallback_allocator_fixture, "fallback_allocator deallocate nullblk does nothing", "[fallback_allocator], [allocator]")
{
    mock_fallback_allocator allocator;
    memory_block block = nullblk;
    allocator.deallocate(block);

    CHECK(block == nullblk);
}

TEST_CASE_METHOD(fallback_allocator_fixture, "fallback_allocator deallocate owned by primary allocator", "[fallback_allocator], [allocator]")
{
    mock_fallback_allocator allocator;
    mock_primary::will_owns = true;
    mock_primary::allocate_block = memory_block{&allocator, sizeof(mock_fallback_allocator)};

    memory_block allocated_block = allocator.allocate(12);
    allocator.deallocate(allocated_block);

    CHECK(mock_primary::deallocate_count == 1);
    CHECK(mock_fallback::deallocate_count == 0);
}

TEST_CASE_METHOD(fallback_allocator_fixture, "fallback_allocator deallocate owned by fallback allocator", "[fallback_allocator], [allocator]")
{
    mock_fallback_allocator allocator;
    mock_fallback::will_owns = true;
    mock_fallback::allocate_block = memory_block{&allocator, sizeof(mock_fallback_allocator)};

    memory_block allocated_block = allocator.allocate(12);
    allocator.deallocate(allocated_block);

    CHECK(mock_fallback::deallocate_count == 1);
    CHECK(mock_primary::deallocate_count == 0);
}

} // namespace ca
