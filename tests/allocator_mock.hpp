#pragma once

#include <coal/alignment.hpp>
#include <coal/def.hpp>
#include <coal/memory_block.hpp>

namespace coal::mock {

template<typename TagT>
struct basic_minimal_allocator
{
    static constexpr std::size_t alignment = default_alignment;

    HEDLEY_WARN_UNUSED_RESULT std::size_t get_alignment()
    {
        return alignment;
    }

    HEDLEY_WARN_UNUSED_RESULT memory_block allocate(std::size_t size)
    {
        COAL_UNUSED(size);
        if (!will_allocate)
        {
            return nullblk;
        }

        ++allocate_count;
        return allocate_block;
    }

    bool reallocate(memory_block& block, std::size_t new_size)
    {
        COAL_UNUSED(new_size);
        if (will_reallocate)
        {
            block = reallocate_block;
        }
        ++reallocate_count;
        return will_reallocate;
    }

    void deallocate(memory_block& block)
    {
        if (will_deallocate)
        {
            block = nullblk;
        }
        ++deallocate_count;
    }

    static void reset_mock()
    {
        will_allocate = true;
        will_reallocate = true;
        will_deallocate = true;
        allocate_block = nullblk;
        reallocate_block = nullblk;
        allocate_count = 0;
        reallocate_count = 0;
        deallocate_count = 0;
    }

    inline static bool will_allocate{true};
    inline static bool will_reallocate{true};
    inline static bool will_deallocate{true};
    inline static memory_block allocate_block{nullblk};
    inline static memory_block reallocate_block{nullblk};
    inline static std::size_t allocate_count{0};
    inline static std::size_t reallocate_count{0};
    inline static std::size_t deallocate_count{0};
};

template<typename TagT>
struct basic_allocator : basic_minimal_allocator<basic_allocator<TagT>>
{
    using super = basic_minimal_allocator<basic_allocator<TagT>>;

    HEDLEY_WARN_UNUSED_RESULT bool owns(const memory_block& block) const
    {
        COAL_UNUSED(block);
        ++owns_count;
        return will_owns;
    }

    bool expand(memory_block& block, std::size_t delta)
    {
        COAL_UNUSED(delta);
        if (will_expand)
        {
            block = expand_block;
        }
        ++expand_count;
        return will_expand;
    }

    void deallocate_all()
    {
        ++deallocate_all_count;
    }

    static void reset_mock()
    {
        super::reset_mock();

        will_owns = false;
        will_expand = true;
        expand_block = nullblk;
        owns_count = 0;
        expand_count = 0;
        deallocate_all_count = 0;
    }

    inline static bool will_owns{false};
    inline static bool will_expand{true};
    inline static memory_block expand_block{nullblk};
    inline static std::size_t owns_count{0};
    inline static std::size_t expand_count{0};
    inline static std::size_t deallocate_all_count{0};
};

struct default_allocator_tag
{};

using minimal_allocator = basic_minimal_allocator<default_allocator_tag>;
using allocator = basic_allocator<default_allocator_tag>;

} // namespace coal::mock
