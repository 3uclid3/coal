#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <ca/proxy_interface_allocator.hpp>
#include <ca/stack_allocator.hpp>

#include <allocator_fixture.hpp>
#include <allocator_mock.hpp>

namespace ca {

TEST_CASE_METHOD(basic_allocator_fixture<proxy_interface_allocator>, "proxy_interface_allocator basics", "[proxy_interface_allocator], [allocator]")
{
    stack_allocator<0x1000> stack_allocator;
    allocator.set_allocator(stack_allocator);

    this->test_basics();
}

TEST_CASE_METHOD(allocator_fixture<mock::allocator>, "proxy_interface_allocator", "[proxy_interface_allocator], [allocator]")
{
    proxy_interface_allocator proxy_allocator;
    proxy_allocator.set_allocator(allocator);

    memory_block block{&proxy_allocator, sizeof(proxy_allocator)};
    memory_block other_block{&allocator, sizeof(allocator)};

    SECTION("allocate")
    {
        mock::allocator::will_allocate = true;
        mock::allocator::allocate_block = block;

        CHECK(proxy_allocator.allocate(sizeof(proxy_allocator)) == block);
        CHECK(mock::allocator::allocate_count == 1);
    }

    SECTION("owns")
    {
        mock::allocator::will_owns = true;

        CHECK(proxy_allocator.owns(block));
        CHECK(mock::allocator::owns_count == 1);
    }

    SECTION("expand")
    {
        mock::allocator::will_expand = true;
        mock::allocator::expand_block = other_block;

        CHECK(proxy_allocator.expand(block, 2));
        CHECK(block == other_block);
        CHECK(mock::allocator::expand_count == 1);
    }

    SECTION("reallocate")
    {
        mock::allocator::will_reallocate = true;
        mock::allocator::reallocate_block = other_block;

        CHECK(proxy_allocator.reallocate(block, 2));
        CHECK(block == other_block);
        CHECK(mock::allocator::reallocate_count == 1);
    }

    SECTION("deallocate")
    {
        mock::allocator::will_deallocate = true;

        proxy_allocator.deallocate(block);

        CHECK(block == nullblk);
        CHECK(mock::allocator::deallocate_count == 1);
    }

    SECTION("deallocate_all")
    {
        proxy_allocator.deallocate_all();

        CHECK(mock::allocator::deallocate_all_count == 1);
    }

    SECTION("unset_allocator")
    {
        CHECK(proxy_allocator.has_allocator());

        proxy_allocator.unset_allocator();

        CHECK_FALSE(proxy_allocator.has_allocator());
    }
}

TEST_CASE_METHOD(allocator_fixture<mock::minimal_allocator>, "proxy_interface_allocator with minimal allocator", "[proxy_interface_allocator], [allocator]")
{
    proxy_interface_allocator proxy_allocator(allocator);

    memory_block block{&proxy_allocator, sizeof(proxy_allocator)};
    memory_block other_block{&allocator, sizeof(allocator)};

    SECTION("allocate")
    {
        mock::minimal_allocator::will_allocate = true;
        mock::minimal_allocator::allocate_block = block;

        CHECK(proxy_allocator.allocate(sizeof(proxy_allocator)) == block);
        CHECK(mock::minimal_allocator::allocate_count == 1);
    }

    SECTION("owns")
    {
        CHECK_FALSE(proxy_allocator.owns(block));
    }

    SECTION("expand")
    {
        CHECK_FALSE(proxy_allocator.expand(block, 2));
    }

    SECTION("reallocate")
    {
        mock::minimal_allocator::will_reallocate = true;
        mock::minimal_allocator::reallocate_block = other_block;

        CHECK(proxy_allocator.reallocate(block, 2));
        CHECK(block == other_block);
        CHECK(mock::minimal_allocator::reallocate_count == 1);
    }

    SECTION("deallocate")
    {
        mock::minimal_allocator::will_deallocate = true;

        proxy_allocator.deallocate(block);

        CHECK(block == nullblk);
        CHECK(mock::minimal_allocator::deallocate_count == 1);
    }

    SECTION("deallocate_all")
    {
        proxy_allocator.deallocate_all();

        SUCCEED();
    }

    SECTION("unset_allocator")
    {
        CHECK(proxy_allocator.has_allocator());

        proxy_allocator.unset_allocator();

        CHECK_FALSE(proxy_allocator.has_allocator());
    }
}

} // namespace ca
