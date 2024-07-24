#include <vector>

#include <catch2/catch_template_test_macros.hpp>

#include <coal/slab_allocator.hpp>
#include <coal/stack_allocator.hpp>

#include <allocator_fixture.hpp>
#include <allocator_mock.hpp>

namespace coal {

using slab_basic_allocators = std::tuple<
    slab_allocator<stack_allocator<0x1000 * 6, 4>, 0x1000, 32, 64, 128, 256, 512, 1024>,
    slab_allocator<stack_allocator<0x1000 * 6, 8>, 0x1000, 32, 64, 128, 256, 512, 1024>,
    slab_allocator<stack_allocator<0x1000 * 6, 16>, 0x1000, 32, 64, 128, 256, 512, 1024>>;

TEMPLATE_LIST_TEST_CASE_METHOD(basic_allocator_fixture, "slab_allocator basics", "[slab_allocator], [allocator]", slab_basic_allocators)
{
    this->small_expand = false;
    this->large_expand = false;

    this->test_basics();
}

using mock_slab_allocator = slab_allocator<mock::minimal_allocator, 0x1000, 32, 64, 128, 256, 512, 1024>;

struct slab_allocator_fixture : allocator_fixture<mock_slab_allocator>
{
    struct mock_initializer
    {
        void init([[maybe_unused]] mock_slab_allocator& root)
        {
            ++init_count;
        }

        void init([[maybe_unused]] mock::minimal_allocator& a)
        {
            CHECK(init_count == 0);
        }

        std::size_t init_count{0};
    };

    slab_allocator_fixture()
    {
        mock::minimal_allocator::reset_mock();
    }
};

TEST_CASE_METHOD(slab_allocator_fixture, "slab_allocator init", "[slab_allocator], [allocator]")
{
    mock_slab_allocator allocator;
    mock_initializer initializer;
    allocator.init(initializer);

    CHECK(mock::minimal_allocator::init_count == 1);
}

TEST_CASE("slab_allocator allocate and deallocate objects with sizes matching slab sizes", "[slab_allocator], [allocator]")
{
    slab_allocator<stack_allocator<0x1000 * 3>, 0x1000, 32, 64, 128> allocator;

    memory_block block1 = allocator.allocate(32);
    CHECK(block1.ptr != nullptr);
    CHECK(block1.size == 32);

    memory_block block2 = allocator.allocate(64);
    CHECK(block2.ptr != nullptr);
    CHECK(block2.size == 64);

    memory_block block3 = allocator.allocate(128);
    CHECK(block3.ptr != nullptr);
    CHECK(block3.size == 128);

    allocator.deallocate(block1);
    allocator.deallocate(block2);
    allocator.deallocate(block3);
}

TEST_CASE("slab_allocator allocate and deallocate objects with sizes less than slab sizes", "[slab_allocator], [allocator]")
{
    slab_allocator<stack_allocator<0x1000 * 3>, 0x1000, 32, 64, 128> allocator;

    memory_block block1 = allocator.allocate(24);
    CHECK(block1.ptr != nullptr);
    CHECK(block1.size == 24);

    memory_block block2 = allocator.allocate(48);
    CHECK(block2.ptr != nullptr);
    CHECK(block2.size == 48);

    memory_block block3 = allocator.allocate(96);
    CHECK(block3.ptr != nullptr);
    CHECK(block3.size == 96);

    allocator.deallocate(block1);
    allocator.deallocate(block2);
    allocator.deallocate(block3);
}

TEST_CASE("slab_allocator allocate nullblk with unsupported size", "[slab_allocator], [allocator]")
{
    slab_allocator<stack_allocator<0x1000 * 3>, 0x1000, 32, 64, 128> allocator;

    SECTION("zero")
    {
        CHECK(allocator.allocate(0) == nullblk);
    }

    SECTION("out of bounds")
    {
        CHECK(allocator.allocate(150) == nullblk);
        CHECK(allocator.allocate(256) == nullblk);
    }
}

TEST_CASE("slab_allocator deallocate nullblk", "[slab_allocator], [allocator]")
{
    slab_allocator<stack_allocator<0x1000 * 3>, 0x1000, 32, 64, 128> allocator;

    memory_block block = nullblk;
    allocator.deallocate(block);
    CHECK(block == nullblk);
}

TEST_CASE("slab_allocator fragmentation handling with 'random' deallocation", "[slab_allocator], [allocator]")
{
    constexpr std::size_t allocation_count = 10;

    slab_allocator<stack_allocator<0x1000 * 3>, 0x1000, 32, 64, 128> allocator;

    std::vector<std::size_t> indices = {3, 8, 2, 9, 4, 5, 0, 7, 1, 6};
    REQUIRE(indices.size() == allocation_count);

    // Allocate 10 blocks of 32 bytes
    std::vector<memory_block> blocks;
    blocks.reserve(allocation_count);
    for (std::size_t i = 0; i < allocation_count; ++i)
    {
        blocks.push_back(allocator.allocate(32));
        CHECK(blocks[i].ptr != nullptr);
    }

    // Deallocate blocks in 'random' order
    for (std::size_t i : indices)
    {
        allocator.deallocate(blocks[i]);
    }

    // Allocate again and ensure no issues
    for (std::size_t i = 0; i < allocation_count; ++i)
    {
        blocks[i] = allocator.allocate(32);
        CHECK(blocks[i].ptr != nullptr);
    }
}

TEST_CASE("slab_allocator stress test with delayed deallocation", "[slab_allocator], [allocator]")
{
    slab_allocator<stack_allocator<0x1000 * 32>, 0x1000, 32, 64> allocator;

    std::vector<memory_block> blocks;

    // Allocate 1000 blocks
    for (std::size_t i = 0; i < 1000; ++i)
    {
        blocks.push_back(allocator.allocate(i % 2 ? 32 : 64));
        CHECK(blocks[i].ptr != nullptr);
    }

    // Deallocate all blocks
    for (auto& block : blocks)
    {
        allocator.deallocate(block);
    }
}

} // namespace coal
