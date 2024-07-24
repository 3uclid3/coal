#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <coal/null_allocator.hpp>

namespace coal {

TEST_CASE("null_allocator allocate nullblk", "[null_allocator], [allocator]")
{
    std::size_t size = GENERATE(4, 8, 12, 16, 32);

    null_allocator allocator;
    CHECK(allocator.allocate(size) == nullblk);
}

TEST_CASE("null_allocator owns nullblk", "[null_allocator], [allocator]")
{
    null_allocator allocator;
    memory_block block(&allocator, sizeof(null_allocator));

    CHECK(allocator.owns(nullblk));
    CHECK_FALSE(allocator.owns(block));
}

TEST_CASE("null_allocator cannot expand", "[null_allocator], [allocator]")
{
    memory_block block = nullblk;

    null_allocator allocator;
    CHECK_FALSE(allocator.expand(block, 1));
    CHECK(block == nullblk);
}

TEST_CASE("null_allocator cannot reallocate", "[null_allocator], [allocator]")
{
    memory_block block = nullblk;

    null_allocator allocator;
    CHECK_FALSE(allocator.reallocate(block, 1));
    CHECK(block == nullblk);
}

TEST_CASE("null_allocator cannot deallocate", "[null_allocator], [allocator]")
{
    memory_block block = nullblk;

    null_allocator allocator;
    allocator.deallocate(block);
    CHECK(block == nullblk);
}

} // namespace coal