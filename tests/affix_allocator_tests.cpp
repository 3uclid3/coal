#include <tuple>

#include <catch2/catch_template_test_macros.hpp>

#include <coal/affix_allocator.hpp>
#include <coal/def.hpp>
#include <coal/stack_allocator.hpp>

#include <allocator_fixture.hpp>
#include <object_mock.hpp>

namespace coal {

struct prefix : mock::basic_object<prefix, std::uint8_t>
{
    static constexpr std::uint32_t pattern{0xbaadf00d};

    std::uint32_t value{pattern};
};

struct suffix : mock::basic_object<suffix, std::uint8_t>
{
    static constexpr std::uint32_t pattern{0xf00dbaad};

    std::uint32_t value{pattern};
};

using stack_allocator4_t = stack_allocator<0x1000, 4>;
using stack_allocator8_t = stack_allocator<0x1000, 8>;
using stack_allocator16_t = stack_allocator<0x1000, 16>;

using prefix_allocator = affix_allocator<stack_allocator4_t, prefix, no_memory_affix>;
using suffix_allocator = affix_allocator<stack_allocator4_t, no_memory_affix, suffix>;

using affix_allocator4 = affix_allocator<stack_allocator4_t, prefix, suffix>;
using affix_allocator8 = affix_allocator<stack_allocator8_t, prefix, suffix>;
using affix_allocator16 = affix_allocator<stack_allocator16_t, prefix, suffix>;

using affix_allocator_types = std::tuple<
    prefix_allocator,
    suffix_allocator,

    affix_allocator4,
    affix_allocator8,
    affix_allocator16>;

#define CHECK_IF_HAS_PREFIX(condition) \
    if constexpr (TestType::has_prefix) CHECK(condition)
#define CHECK_IF_HAS_SUFFIX(condition) \
    if constexpr (TestType::has_suffix) CHECK(condition)

template<typename AllocatorT>
struct affix_allocator_fixture : allocator_fixture<AllocatorT>, mock::basic_object_fixture<prefix>, mock::basic_object_fixture<suffix>
{
    void check_affixes(const memory_block& inner_block)
    {
        const typename AllocatorT::prefix* prefix = this->allocator.get_prefix(inner_block);
        const typename AllocatorT::suffix* suffix = this->allocator.get_suffix(inner_block);

        if constexpr (AllocatorT::prefix_size > 0)
        {
            REQUIRE(prefix != nullptr);
            CHECK(prefix->value == AllocatorT::prefix::pattern);
        }
        else
        {
            CHECK(prefix == nullptr);
        }

        if constexpr (AllocatorT::suffix_size > 0)
        {
            REQUIRE(suffix != nullptr);
            CHECK(suffix->value == AllocatorT::suffix::pattern);
        }
        else
        {
            CHECK(suffix == nullptr);
        }
    }
};

TEMPLATE_LIST_TEST_CASE_METHOD(basic_allocator_fixture, "affix_allocator basics", "[affix_allocator], [allocator]", affix_allocator_types)
{
    this->test_basics();
}

TEMPLATE_LIST_TEST_CASE_METHOD(affix_allocator_fixture, "affix_allocator allocate returns nullblk when allocating 0 byte", "[affix_allocator], [allocator]", affix_allocator_types)
{
    memory_block block = this->allocator.allocate(0);

    CHECK(block == nullblk);
}

TEMPLATE_LIST_TEST_CASE_METHOD(affix_allocator_fixture, "affix_allocator allocate returns nullblk when no space for affixes", "[affix_allocator], [allocator]", affix_allocator_types)
{
    memory_block block = this->allocator.allocate(TestType::allocator::max_size);

    CHECK(block == nullblk);
}

TEMPLATE_LIST_TEST_CASE_METHOD(affix_allocator_fixture, "affix_allocator allocate returns the requested size and affix(es) are applied", "[affix_allocator], [allocator]", affix_allocator_types)
{
    memory_block block = this->allocator.allocate(8);

    REQUIRE(block.ptr != nullptr);
    CHECK(block.size == 8);

    this->check_affixes(block);

    memory_block block1 = this->allocator.allocate(8);

    REQUIRE(block1.ptr != nullptr);
    CHECK(block1.size == 8);

    this->check_affixes(block); // check affixes are still valid
    this->check_affixes(block1);

    this->deallocate_and_check_is_nullblk(block);
    this->deallocate_and_check_is_nullblk(block1);

    CHECK_IF_HAS_PREFIX(prefix::total_ctor_default_count == 2);
    CHECK_IF_HAS_PREFIX(prefix::total_dtor_count == 2);
    CHECK_IF_HAS_SUFFIX(suffix::total_ctor_default_count == 2);
    CHECK_IF_HAS_SUFFIX(suffix::total_dtor_count == 2);
}

TEMPLATE_LIST_TEST_CASE_METHOD(affix_allocator_fixture, "affix_allocator reallocate returns the requested size and affix(es) are applied", "[affix_allocator], [allocator]", affix_allocator_types)
{
    memory_block block = this->allocator.allocate(8);

    REQUIRE(block.ptr != nullptr);
    CHECK(block.size == 8);

    this->check_affixes(block);

    memory_block block1 = this->allocator.allocate(8);

    REQUIRE(block1.ptr != nullptr);
    CHECK(block1.size == 8);

    this->check_affixes(block); // check affixes are still valid
    this->check_affixes(block1);

    this->deallocate_and_check_is_nullblk(block);
    this->deallocate_and_check_is_nullblk(block1);

    CHECK_IF_HAS_PREFIX(prefix::total_ctor_default_count == 2);
    CHECK_IF_HAS_PREFIX(prefix::total_dtor_count == 2);
    CHECK_IF_HAS_SUFFIX(suffix::total_ctor_default_count == 2);
    CHECK_IF_HAS_SUFFIX(suffix::total_dtor_count == 2);
}

TEMPLATE_LIST_TEST_CASE_METHOD(affix_allocator_fixture, "affix_allocator reallocate decrease -> inscrease is same position and prefix", "[affix_allocator], [allocator]", affix_allocator_types)
{
    memory_block block = this->allocator.allocate(8);
    REQUIRE(block.ptr != nullptr);
    CHECK(block.size == 8);
    this->check_affixes(block);

    CHECK_IF_HAS_PREFIX(prefix::total_ctor_default_count == 1);
    CHECK_IF_HAS_SUFFIX(suffix::total_ctor_default_count == 1);

    memory_block old_block = block;
    REQUIRE(this->allocator.reallocate(block, 4));
    CHECK(block.ptr == old_block.ptr);
    CHECK(block.size == 4);
    this->check_affixes(block);

    CHECK_IF_HAS_PREFIX(prefix::total_ctor_default_count == 1);
    CHECK_IF_HAS_PREFIX(prefix::total_ctor_move_count == 2);
    CHECK_IF_HAS_PREFIX(prefix::total_dtor_count == 2);
    CHECK_IF_HAS_SUFFIX(suffix::total_ctor_default_count == 1);
    CHECK_IF_HAS_SUFFIX(suffix::total_ctor_move_count == 2);
    CHECK_IF_HAS_SUFFIX(suffix::total_dtor_count == 2);

    old_block = block;
    REQUIRE(this->allocator.reallocate(block, 8));
    CHECK(block.ptr == old_block.ptr);
    CHECK(block.size == 8);
    this->check_affixes(block);

    CHECK_IF_HAS_PREFIX(prefix::total_ctor_default_count == 1);
    CHECK_IF_HAS_PREFIX(prefix::total_ctor_move_count == 4);
    CHECK_IF_HAS_PREFIX(prefix::total_dtor_count == 4);
    CHECK_IF_HAS_SUFFIX(suffix::total_ctor_default_count == 1);
    CHECK_IF_HAS_SUFFIX(suffix::total_ctor_move_count == 4);
    CHECK_IF_HAS_SUFFIX(suffix::total_dtor_count == 4);

    this->deallocate_and_check_is_nullblk(block);

    CHECK_IF_HAS_PREFIX(prefix::total_dtor_count == 5);
    CHECK_IF_HAS_SUFFIX(suffix::total_dtor_count == 5);
}

TEMPLATE_LIST_TEST_CASE_METHOD(affix_allocator_fixture, "affix_allocator expand nullblk by 0 returns true", "[affix_allocator], [allocator]", affix_allocator_types)
{
    memory_block block = nullblk;
    CHECK(this->allocator.expand(block, 0));
    CHECK(block == nullblk);
}

TEMPLATE_LIST_TEST_CASE_METHOD(affix_allocator_fixture, "stack_allocator expand by 0 returns same block", "[affix_allocator], [allocator]", affix_allocator_types)
{
    memory_block block = this->allocator.allocate(8);
    memory_block pre_expand_block = block;

    CHECK(this->allocator.expand(block, 0));
    CHECK(block == pre_expand_block);

    this->check_affixes(block);
    this->deallocate_and_check_is_nullblk(block);
}

} // namespace coal
