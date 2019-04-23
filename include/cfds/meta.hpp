#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace meta {

template <bool B>
using bool_constant = std::integral_constant<bool, B>;

template <int N>
struct priority_tag : priority_tag<N - 1> {};

template <>
struct priority_tag<0> {};

namespace detail {

template <typename T>
auto is_trivially_relocatable_impl(priority_tag<1>)
    -> bool_constant<T::is_trivially_relocatable::value>;

template <typename T>
auto is_trivially_relocatable_impl(priority_tag<0>)
    -> bool_constant<std::is_trivially_move_constructible<T>::value &&
                     std::is_trivially_destructible<T>::value>;

} // namespace detail

template <typename T>
struct is_trivially_relocatable
    : decltype(detail::is_trivially_relocatable_impl<T>(priority_tag<1>{})) {};

template <typename T>
struct is_trivially_relocatable<std::unique_ptr<T>> : std::true_type {};

template <typename T>
struct is_trivially_relocatable<std::shared_ptr<T>> : std::true_type {};

template <typename T>
struct is_trivially_relocatable<std::weak_ptr<T>> : std::true_type {};

template <typename Iterator>
struct is_forward_iterator
    : std::is_base_of<
          std::forward_iterator_tag,
          typename std::iterator_traits<Iterator>::iterator_category> {};

namespace detail {

#define CFDS_META_HAS_FUNCTION(name, fn)                                       \
    template <typename T, typename Ret, typename... Args>                      \
    struct name {                                                              \
        using type = std::false_type;                                          \
        static constexpr bool value = false;                                   \
    };                                                                         \
                                                                               \
    template <typename T, typename Ret, typename... Args>                      \
    struct name<T, Ret(Args...)> {                                             \
     private:                                                                  \
        template <typename U = T>                                              \
        static constexpr auto check(priority_tag<1>) ->                        \
            typename std::is_same<decltype(U::fn(std::declval<Args>()...)),    \
                                  Ret>::type;                                  \
                                                                               \
        template <typename>                                                    \
        static constexpr auto check(priority_tag<0>) -> std::false_type;       \
                                                                               \
     public:                                                                   \
        using type = decltype(check<T>(priority_tag<1>{}));                    \
        static constexpr bool value = type::value;                             \
    };

CFDS_META_HAS_FUNCTION(has_get_empty_impl, get_empty)
CFDS_META_HAS_FUNCTION(has_get_tombstone_impl, get_tombstone)
CFDS_META_HAS_FUNCTION(has_get_hash_impl, get_hash)
CFDS_META_HAS_FUNCTION(has_compare_impl, compare)

#undef CFDS_META_HAS_FUNCTION

} // namespace detail

template <typename T>
using has_get_empty =
    detail::has_get_empty_impl<T, typename T::value_type(void)>;

template <typename T>
using has_get_tombstone =
    detail::has_get_tombstone_impl<T, typename T::value_type(void)>;

template <typename T>
using has_get_hash =
    detail::has_get_hash_impl<T, std::size_t(const typename T::value_type&)>;

template <typename T>
using has_compare =
    detail::has_compare_impl<T, bool(const typename T::value_type&,
                                     const typename T::value_type&)>;

} // namespace meta
