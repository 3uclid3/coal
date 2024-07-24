#include <catch2/catch_template_test_macros.hpp>

#include <coal/free_list_allocator.hpp>
#include <coal/free_list_strategy/first_fit.hpp>
#include <coal/memory_block.hpp>
#include <coal/stack_allocator.hpp>

#include <allocator_fixture.hpp>
#include <allocator_mock.hpp>

namespace coal {

using mock_free_list_allocator = free_list_allocator<mock::minimal_allocator, free_list_strategy::first_fit>;

struct free_list_allocator_fixture : allocator_fixture<mock_free_list_allocator>
{
    struct mock_initializer
    {
        void init([[maybe_unused]] mock_free_list_allocator& root)
        {
            ++init_count;
        }

        void init([[maybe_unused]] mock::minimal_allocator& a)
        {
            CHECK(init_count == 0);
        }

        std::size_t init_count{0};
    };

    free_list_allocator_fixture()
    {
        mock::minimal_allocator::reset_mock();
    }
};

TEST_CASE_METHOD(free_list_allocator_fixture, "free_list_allocator init", "[free_list_allocator], [allocator]")
{
    mock_free_list_allocator allocator;
    mock_initializer initializer;
    allocator.init(initializer);

    CHECK(mock::minimal_allocator::init_count == 1);
}

using free_list_basic_allocators = std::tuple<
    free_list_allocator<stack_allocator<0x1000, 4>, free_list_strategy::first_fit>,
    free_list_allocator<stack_allocator<0x1000, 8>, free_list_strategy::first_fit>,
    free_list_allocator<stack_allocator<0x1000, 16>, free_list_strategy::first_fit>>;

TEMPLATE_LIST_TEST_CASE_METHOD(basic_allocator_fixture, "free_list_allocator basics", "[free_list_allocator], [allocator]", free_list_basic_allocators)
{
    this->test_basics();
}

} // namespace coal