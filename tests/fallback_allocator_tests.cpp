#include <catch2/catch_template_test_macros.hpp>

#include <coal/fallback_allocator.hpp>
#include <coal/memory_block.hpp>
#include <coal/stack_allocator.hpp>

#include <allocator_fixture.hpp>
#include <allocator_mock.hpp>

namespace coal {

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

    struct mock_initializer
    {
        void init([[maybe_unused]] mock_fallback_allocator& root)
        {
            ++init_count;
        }

        void init([[maybe_unused]] mock_primary& primary)
        {
            CHECK(init_count == 0);
        }

        void init([[maybe_unused]] mock_fallback& fallback)
        {
            CHECK(init_count == 0);
        }

        std::size_t init_count{0};
    };

    fallback_allocator_fixture()
    {
        mock_primary::reset_mock();
        mock_fallback::reset_mock();
    }
};

TEST_CASE_METHOD(fallback_allocator_fixture, "fallback_allocator init", "[fallback_allocator], [allocator]")
{
    mock_fallback_allocator allocator;
    mock_initializer initializer;
    allocator.init(initializer);

    CHECK(mock_fallback::init_count == 1);
    CHECK(mock_primary::init_count == 1);
}

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

} // namespace coal
