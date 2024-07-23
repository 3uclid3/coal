#pragma once

#include <ca/memory_block.hpp>

#define _CA_TYPE_TRAIT_HAS_METHOD_IMPL(name, method_name, return_type, suffix, ...) \
    namespace impl { \
    template<typename T> \
    struct name \
    { \
        template<typename U, return_type (U::*)(__VA_ARGS__) suffix> \
        struct check; \
        template<typename U> \
        static constexpr bool test(check<U, &U::method_name>*) \
        { \
            return true; \
        } \
        template<typename U> \
        static constexpr bool test(...) \
        { \
            return false; \
        } \
        static constexpr bool value = test<T>(nullptr); \
    }; \
    } \
    template<typename T> \
    inline static constexpr bool name = impl::name<T>::value

#define CA_TYPE_TRAIT_HAS_METHOD(name, method_name, return_type, ...) \
    _CA_TYPE_TRAIT_HAS_METHOD_IMPL(name, method_name, return_type, , __VA_ARGS__)

#define CA_TYPE_TRAIT_HAS_METHOD_CONST(name, method_name, return_type, ...) \
    _CA_TYPE_TRAIT_HAS_METHOD_IMPL(name, method_name, return_type, const, __VA_ARGS__)

namespace ca {
struct memory_block;
}

namespace ca::allocator_traits {

CA_TYPE_TRAIT_HAS_METHOD_CONST(has_owns, owns, bool, const memory_block&);
CA_TYPE_TRAIT_HAS_METHOD(has_expand, expand, bool, memory_block&, size_t);
CA_TYPE_TRAIT_HAS_METHOD(has_deallocate_all, deallocate_all, void);

} // namespace ca::allocator_traits
