#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <coal/malloc_allocator.hpp>

namespace coal {

TEST_CASE("malloc_allocator allocate", "[malloc_allocator], [allocator]")
{
    std::size_t size = GENERATE(4, 8, 12, 16, 32);

    malloc_allocator allocator;
    auto blk = allocator.allocate(size);

    CHECK(blk.ptr != nullptr);
    CHECK(blk.size == size);

    allocator.deallocate(blk);
}

TEST_CASE("malloc_allocator reallocate", "[malloc_allocator], [allocator]")
{
    std::size_t size = GENERATE(4, 8, 12, 16, 32);

    malloc_allocator allocator;
    auto blk = allocator.allocate(size);

    REQUIRE(blk);

    const std::size_t new_size = size * 2;

    CHECK(allocator.reallocate(blk, new_size));

    CHECK(blk.ptr != nullptr);
    CHECK(blk.size == new_size);

    allocator.deallocate(blk);
}

} // namespace coal
