#include <ca/memory_block.hpp>

#include <catch2/catch_test_macros.hpp>

namespace ca {

TEST_CASE("memory_block")
{
    memory_block block;

    CHECK(block == nullblk);
    CHECK(!block);

    block.ptr = &block;
    block.size = sizeof(block);

    CHECK(block != nullblk);
    CHECK(block);
}

} // namespace ca
