#pragma once

#include <coal/def.hpp>

namespace coal::mock {

template<typename TagT, typename SizeT = std::size_t>
struct basic_object
{
    static void reset_mock()
    {
        total_ctor_default_count = 0;
        total_ctor_value_count = 0;
        total_ctor_copy_count = 0;
        total_ctor_move_count = 0;
        total_otor_copy_count = 0;
        total_otor_move_count = 0;
        total_dtor_count = 0;
    }

    basic_object()
    {
        ctor_default_called = true;

        ++ctor_default_count;
        ++total_ctor_default_count;
    }

    basic_object(std::uint32_t val)
        : value(val)
    {
        ctor_value_called = true;

        ++ctor_value_count;
        ++total_ctor_value_count;
    }

    basic_object(const basic_object& other)
    {
        ctor_copy_called = true;

        value = other.value;

        ctor_default_count = other.ctor_default_count;
        ctor_value_count = other.ctor_value_count;
        ctor_copy_count = other.ctor_copy_count;
        ctor_move_count = other.ctor_move_count;
        otor_copy_count = other.otor_copy_count;
        otor_move_count = other.otor_move_count;

        ++ctor_copy_count;
        ++total_ctor_copy_count;
    }

    basic_object(basic_object&& other)
    {
        ctor_move_called = true;

        value = other.value;

        ctor_default_count = other.ctor_default_count;
        ctor_value_count = other.ctor_value_count;
        ctor_copy_count = other.ctor_copy_count;
        ctor_move_count = other.ctor_move_count;
        otor_copy_count = other.otor_copy_count;
        otor_move_count = other.otor_move_count;

        ++ctor_move_count;
        ++total_ctor_move_count;

        other.ctor_default_count = 0;
        other.ctor_copy_count = 0;
        other.ctor_move_count = 0;
        other.otor_copy_count = 0;
        other.otor_move_count = 0;
    }

    ~basic_object()
    {
        if (dtor_called != nullptr)
        {
            *dtor_called = true;
        }
        ++total_dtor_count;
    }

    basic_object& operator=(const basic_object& other)
    {
        otor_copy_called = true;

        value = other.value;

        ctor_default_count = other.ctor_default_count;
        ctor_value_count = other.ctor_value_count;
        ctor_copy_count = other.ctor_copy_count;
        ctor_move_count = other.ctor_move_count;
        otor_copy_count = other.otor_copy_count;
        otor_move_count = other.otor_move_count;

        ++otor_copy_count;
        ++total_otor_copy_count;

        return *this;
    }

    basic_object& operator=(basic_object&& other)
    {
        otor_move_called = true;

        value = other.value;

        ctor_default_count = other.ctor_default_count;
        ctor_value_count = other.ctor_value_count;
        ctor_copy_count = other.ctor_copy_count;
        ctor_move_count = other.ctor_move_count;
        otor_copy_count = other.otor_copy_count;
        otor_move_count = other.otor_move_count;

        ++otor_move_count;
        ++total_otor_move_count;

        other.ctor_default_count = 0;
        other.ctor_value_count = 0;
        other.ctor_copy_count = 0;
        other.ctor_move_count = 0;
        other.otor_copy_count = 0;
        other.otor_move_count = 0;

        return *this;
    }

    bool operator==(const basic_object& other) const
    {
        return value == other.value;
    }

    bool operator!=(const basic_object& other) const
    {
        return !(*this == other);
    }

    std::uint32_t value{0};

    SizeT ctor_default_count{0};
    SizeT ctor_value_count{0};
    SizeT ctor_copy_count{0};
    SizeT ctor_move_count{0};
    SizeT otor_copy_count{0};
    SizeT otor_move_count{0};

    bool ctor_default_called : 1 {false};
    bool ctor_value_called : 1 {false};
    bool ctor_copy_called : 1 {false};
    bool ctor_move_called : 1 {false};
    bool otor_copy_called : 1 {false};
    bool otor_move_called : 1 {false};

    bool* dtor_called{nullptr};

    inline static SizeT total_ctor_default_count = 0;
    inline static SizeT total_ctor_value_count = 0;
    inline static SizeT total_ctor_copy_count = 0;
    inline static SizeT total_ctor_move_count = 0;
    inline static SizeT total_otor_copy_count = 0;
    inline static SizeT total_otor_move_count = 0;
    inline static SizeT total_dtor_count = 0;
};

template<typename TagT, typename SizeT = std::size_t>
struct basic_copy_only_object : basic_object<TagT>
{
    using basic_object<TagT>::basic_object;

    constexpr basic_copy_only_object(basic_copy_only_object&&) = delete;
    constexpr basic_copy_only_object& operator=(basic_copy_only_object&&) = delete;

    constexpr basic_copy_only_object(const basic_copy_only_object&) = default;
    constexpr basic_copy_only_object& operator=(const basic_copy_only_object&) = default;
};

template<typename TagT, typename SizeT = std::size_t>
struct basic_move_only_object : basic_object<TagT>
{
    using basic_object<TagT>::basic_object;

    constexpr basic_move_only_object(const basic_move_only_object&) = delete;
    constexpr basic_move_only_object& operator=(const basic_move_only_object&) = delete;

    constexpr basic_move_only_object(basic_move_only_object&&) = default;
    constexpr basic_move_only_object& operator=(basic_move_only_object&&) = default;
};

template<typename MockObjecT>
struct basic_object_fixture
{
    basic_object_fixture()
    {
        MockObjecT::reset_mock();
    }
};

struct default_object_tag
{};

// 'smaller' object by using std::uint8_t for mock counts
using small_object = basic_object<default_object_tag, std::uint8_t>;
using copy_only_small_object = basic_copy_only_object<default_object_tag, std::uint8_t>;
using move_only_small_object = basic_move_only_object<default_object_tag, std::uint8_t>;

using object = basic_object<default_object_tag>;
using copy_only_object = basic_copy_only_object<default_object_tag>;
using move_only_object = basic_move_only_object<default_object_tag>;

using small_object_fixture = basic_object_fixture<small_object>;
using copy_only_small_object_fixture = basic_object_fixture<copy_only_small_object>;
using move_only_small_object_fixture = basic_object_fixture<move_only_small_object>;

using object_fixture = basic_object_fixture<object>;
using copy_only_object_fixture = basic_object_fixture<copy_only_object>;
using move_only_object_fixture = basic_object_fixture<move_only_object>;

} // namespace coal::mock
