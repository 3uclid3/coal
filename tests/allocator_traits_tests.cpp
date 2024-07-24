#include <catch2/catch_test_macros.hpp>

#include <coal/allocator_traits.hpp>
#include <coal/def.hpp>
#include <coal/memory_block.hpp>

namespace coal::allocator_traits {

struct empty_allocator
{
};

class non_empty_allocator
{
public:
    HEDLEY_WARN_UNUSED_RESULT constexpr memory_block allocate(std::size_t) { return nullblk; }
    HEDLEY_WARN_UNUSED_RESULT constexpr bool owns(const memory_block&) const { return false; }
    constexpr bool expand(memory_block&, std::size_t) { return false; }
    constexpr bool reallocate(memory_block&, std::size_t) { return false; }
    constexpr void deallocate(memory_block&) {}
    constexpr void deallocate_all() {}
};

TEST_CASE("allocator_traits", "[allocator_traits]")
{
    STATIC_CHECK(has_owns<non_empty_allocator>);
    STATIC_CHECK(has_expand<non_empty_allocator>);
    STATIC_CHECK(has_deallocate_all<non_empty_allocator>);

    STATIC_CHECK_FALSE(has_owns<empty_allocator>);
    STATIC_CHECK_FALSE(has_expand<empty_allocator>);
    STATIC_CHECK_FALSE(has_deallocate_all<empty_allocator>);
}

} // namespace coal::allocator_traits