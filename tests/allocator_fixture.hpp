#pragma once

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <coal/alignment.hpp>
#include <coal/allocator_traits.hpp>
#include <coal/def.hpp>
#include <coal/memory_block.hpp>

namespace coal {

template<typename AllocatorT>
struct scope_memory_block : memory_block
{
    constexpr scope_memory_block() = default;

    constexpr scope_memory_block(const memory_block& block, AllocatorT* allocator_ptr)
        : memory_block{block}
        , allocator_ptr{allocator_ptr}
    {}

    constexpr scope_memory_block(scope_memory_block&& other) noexcept
        : memory_block{other}
        , allocator_ptr{other.allocator_ptr}
    {
        other.ptr = nullptr;
        other.size = 0;
        other.allocator_ptr = nullptr;
    }

    constexpr scope_memory_block& operator=(scope_memory_block&& other) noexcept
    {
        if (this != &other)
        {
            ptr = other.ptr;
            size = other.size;
            allocator_ptr = other.allocator_ptr;

            other.ptr = nullptr;
            other.size = 0;
            other.allocator_ptr = nullptr;
        }

        return *this;
    }

    ~scope_memory_block()
    {
        if (ptr)
        {
            REQUIRE(allocator_ptr);
            allocator_ptr->deallocate(*this);
            CHECK_FALSE(*this);
        }
    }

    AllocatorT* allocator_ptr{nullptr};
};

template<typename AllocatorT>
struct allocator_fixture
{
    void deallocate_and_check_is_nullblk(memory_block& block)
    {
        allocator.deallocate(block);
        CHECK(block == nullblk);
    }

    AllocatorT allocator;
};

template<typename AllocatorT>
struct basic_allocator_fixture
{
    using scope_memory_block_t = scope_memory_block<AllocatorT>;

    scope_memory_block<AllocatorT> scope_allocate(std::size_t size)
    {
        return {allocator.allocate(size), &allocator};
    }

    void test_basic_allocate()
    {
        SECTION("allocate")
        {
            SECTION("allocate size 0 return nullblk")
            {
                scope_memory_block_t block = scope_allocate(0);

                CHECK(block == nullblk);
            }

            SECTION("allocate size smaller than alignment")
            {
                scope_memory_block_t block = scope_allocate(allocator.get_alignment() - 1);

                CHECK(block.ptr != nullptr);
                CHECK(block.size == allocator.get_alignment() - 1);
            }

            SECTION("allocate size of alignment")
            {
                scope_memory_block_t block = scope_allocate(allocator.get_alignment());

                CHECK(block.ptr != nullptr);
                CHECK(block.size == allocator.get_alignment());
            }

            SECTION("allocate size bigger than alignment")
            {
                scope_memory_block_t block = scope_allocate(allocator.get_alignment() + 1);

                CHECK(block.ptr != nullptr);
                CHECK(block.size == allocator.get_alignment() + 1);
            }
        }
    }

    void test_basic_expand()
    {
        if constexpr (allocator_traits::has_expand<AllocatorT>)
        {
            if (!small_expand && !large_expand)
            {
                return;
            }

            SECTION("expand")
            {
                if (small_expand)
                {
                    SECTION("expand size 0 does nothing")
                    {
                        scope_memory_block_t block = scope_allocate(aligned_size);

                        CHECK(allocator.expand(block, 0));
                        CHECK(block.ptr != nullptr);
                        CHECK(block.size == aligned_size);
                    }

                    SECTION("expand size 0 nullblk does nothing")
                    {
                        memory_block block = nullblk;

                        CHECK(allocator.expand(block, 0));
                        CHECK(block == nullblk);
                    }

                    SECTION("expand nullblk allocate")
                    {
                        memory_block block = nullblk;

                        CHECK(allocator.expand(block, aligned_size));
                        CHECK(block.ptr != nullptr);
                        CHECK(block.size == aligned_size);
                    }

                    SECTION("expand size smaller than alignment to alignment")
                    {
                        scope_memory_block_t block = scope_allocate(allocator.get_alignment() - 1);

                        CHECK(allocator.expand(block, 1));
                        CHECK(block.ptr != nullptr);
                        CHECK(block.size == allocator.get_alignment());
                    }

                    SECTION("expand size bigger than alignment to double alignment")
                    {
                        scope_memory_block_t block = scope_allocate(allocator.get_alignment() + 1);

                        CHECK(allocator.expand(block, allocator.get_alignment() - 1));
                        CHECK(block.ptr != nullptr);
                        CHECK(block.size == allocator.get_alignment() * 2);
                    }
                }

                if (large_expand)
                {
                    SECTION("expand")
                    {
                        memory_block block = nullblk;

                        for (std::size_t i = 0; i < 10; ++i)
                        {
                            const std::size_t size = i % 2 ? unaligned_size : align_up(block.size + unaligned_size, allocator.get_alignment()) - block.size;
                            const std::size_t expected_size = block.size + size;

                            INFO("expand #" << i << " size " << size);

                            CHECK(allocator.expand(block, size));
                            CHECK(block.ptr != nullptr);
                            CHECK(block.size == expected_size);
                        }
                    }
                }
            }
        }
    }

    void test_basic_reallocate()
    {
        SECTION("reallocate")
        {
            SECTION("reallocate size 0 deallocate")
            {
                scope_memory_block_t block = scope_allocate(aligned_size);

                CHECK(allocator.reallocate(block, 0));
                CHECK(block == nullblk);
            }

            SECTION("reallocate size 0 nullblk does nothing")
            {
                memory_block block = nullblk;

                CHECK(allocator.reallocate(block, 0));
                CHECK(block == nullblk);
            }

            SECTION("reallocate nullblk allocate")
            {
                memory_block block = nullblk;

                CHECK(allocator.reallocate(block, aligned_size));
                CHECK(block.ptr != nullptr);
                CHECK(block.size == aligned_size);
            }

            SECTION("reallocate size smaller than alignment to alignment")
            {
                scope_memory_block_t block = scope_allocate(allocator.get_alignment() - 1);

                CHECK(allocator.reallocate(block, allocator.get_alignment()));
                CHECK(block.ptr != nullptr);
                CHECK(block.size == allocator.get_alignment());
            }

            SECTION("reallocate size bigger than alignment to double alignment")
            {
                scope_memory_block_t block = scope_allocate(allocator.get_alignment() + 1);

                CHECK(allocator.reallocate(block, allocator.get_alignment() * 2));
                CHECK(block.ptr != nullptr);
                CHECK(block.size == allocator.get_alignment() * 2);
            }

            SECTION("reallocate")
            {
                memory_block block = nullblk;

                std::size_t size = 0;
                for (std::size_t i = 0; i < 10; ++i)
                {
                    size += unaligned_size;

                    if (i % 2)
                    {
                        size = align_up(size, allocator.get_alignment());
                    }

                    CHECK(allocator.reallocate(block, size));
                    CHECK(block.ptr != nullptr);
                    CHECK(block.size == size);
                }
            }
        }
    }

    void test_basic_deallocate()
    {
        SECTION("deallocate")
        {
            SECTION("deallocate nullblk does nothing")
            {
                memory_block block = nullblk;

                deallocate_and_check(block);
            }

            SECTION("deallocate set to nullblk")
            {
                memory_block block = allocator.allocate(aligned_size);

                deallocate_and_check(block);
            }
        }
    }

    void test_basic_deallocate_all()
    {
        if constexpr (allocator_traits::has_deallocate_all<AllocatorT>)
        {
            SECTION("deallocate_all")
            {
                for (std::size_t i = 0; i < 10; ++i)
                {
                    memory_block block = allocator.allocate(aligned_size);
                    COAL_UNUSED(block);
                }

                allocator.deallocate_all();
                SUCCEED();
            }
        }
    }

    void test_basic_stress()
    {
        SECTION("stress")
        {
            std::vector<scope_memory_block_t> allocated_blocks;
            allocated_blocks.reserve(70);

            INFO("allocate 50 blocks");
            for (std::size_t i = 0; i < 50; ++i)
            {
                const std::size_t size = i % 2 ? unaligned_size : aligned_size;
                allocated_blocks.emplace_back(allocator.allocate(size));

                CHECK(allocated_blocks.back().ptr != nullptr);
                CHECK(allocated_blocks.back().size == size);
            }

            INFO("deallocate 5 blocks");
            for (std::size_t i = 45; i < 50; ++i)
            {
                deallocate_and_check_is_nullblk(allocated_blocks[i]);
            }
            allocated_blocks.resize(45);

            INFO("allocate and expand 10 blocks");
            for (std::size_t i = 0; i < 10; ++i)
            {
                const std::size_t size = i % 2 ? unaligned_size : aligned_size;
                allocated_blocks.emplace_back(allocator.allocate(size));

                CHECK(allocated_blocks.back().ptr != nullptr);
                CHECK(allocated_blocks.back().size == size);

                CHECK(allocator.expand(allocated_blocks.back(), size));

                CHECK(allocated_blocks.back().ptr != nullptr);
                CHECK(allocated_blocks.back().size == size * 2);
            }

            INFO("allocate and reallocate 10 blocks");
            for (std::size_t i = 0; i < 10; ++i)
            {
                const std::size_t size = i % 2 ? unaligned_size : aligned_size;
                allocated_blocks.emplace_back(allocator.allocate(size));

                CHECK(allocated_blocks.back().ptr != nullptr);
                CHECK(allocated_blocks.back().size == size);

                CHECK(allocator.reallocate(allocated_blocks.back(), size * 2));

                CHECK(allocated_blocks.back().ptr != nullptr);
                CHECK(allocated_blocks.back().size == size * 2);
            }

            INFO("deallocate 10 blocks in reverse order");
            for (std::size_t i = 0; i < 10; ++i)
            {
                deallocate_and_check_is_nullblk(allocated_blocks.back());
                allocated_blocks.pop_back();
            }

            INFO("deallocate all blocks");
            for (auto& block : allocated_blocks)
            {
                deallocate_and_check_is_nullblk(block);
            }
            allocated_blocks.clear();
        }
    }

    void test_basics()
    {
        aligned_size = align_up(unaligned_size, allocator.get_alignment());

        if (aligned_size == unaligned_size)
        {
            unaligned_size -= 1;
        }

        test_basic_allocate();
        test_basic_expand();
        test_basic_reallocate();
        test_basic_deallocate();
        test_basic_deallocate_all();
    }

    void deallocate_and_check(memory_block& block)
    {
        allocator.deallocate(block);
        CHECK_FALSE(block);
    }

    AllocatorT allocator;

    // "random" size for testing
    std::size_t aligned_size = 42;
    std::size_t unaligned_size = 42;

    bool small_expand = true;
    bool large_expand = true;
};

} // namespace coal