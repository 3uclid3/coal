#include <tuple>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <coal/prefixed_size_allocator.hpp>
#include <coal/stack_allocator.hpp>

#include <allocator_fixture.hpp>
#include <allocator_mock.hpp>

namespace coal {
 
using prefixed_size_basic_allocators = std::tuple<
    prefixed_size_allocator<stack_allocator<0x1000, 4>>,
    prefixed_size_allocator<stack_allocator<0x1000, 8>>,
    prefixed_size_allocator<stack_allocator<0x1000, 16>>>;

TEMPLATE_LIST_TEST_CASE_METHOD(basic_allocator_fixture, "prefixed_size_allocator basics", "[prefixed_size_allocator], [allocator]", prefixed_size_basic_allocators)
{
    this->test_basics();
}

using mock_prefixed_size_allocator = prefixed_size_allocator<mock::minimal_allocator>;

struct mock_prefixed_size_allocator_fixture : allocator_fixture<mock_prefixed_size_allocator>
{
    struct mock_initializer
    {
        void init([[maybe_unused]] mock_prefixed_size_allocator& root)
        {
            ++init_count;
        }

        void init([[maybe_unused]] mock::minimal_allocator& a)
        {
            CHECK(init_count == 0);
        }

        std::size_t init_count{0};
    };

    mock_prefixed_size_allocator_fixture()
    {
        mock::minimal_allocator::reset_mock();
    }
};

TEST_CASE_METHOD(mock_prefixed_size_allocator_fixture, "prefixed_size_allocator init", "[prefixed_size_allocator], [allocator]")
{
    mock_prefixed_size_allocator allocator;
    mock_initializer initializer;
    allocator.init(initializer);

    CHECK(mock::minimal_allocator::init_count == 1);
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

} // namespace coal
