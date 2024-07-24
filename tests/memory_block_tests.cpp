#include <catch2/catch_test_macros.hpp>

#include <coal/memory_block.hpp>

namespace coal {

TEST_CASE("memory_block", "[memory_block]")
{
    memory_block block;

    CHECK(block == nullblk);
    CHECK(!block);

    block.ptr = &block;
    block.size = sizeof(block);

    CHECK(block != nullblk);
    CHECK(block);
}

} // namespace coal
