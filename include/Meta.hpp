#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace meta {

template <bool B>
using BoolConstant = std::integral_constant<bool, B>;

template <int N>
struct PriorityTag : PriorityTag<N - 1> {};

template <>
struct PriorityTag<0> {};

namespace detail {

template <typename T>
auto IsTriviallyRelocatableImpl(PriorityTag<1>)
    -> BoolConstant<T::IsTriviallyRelocatable::value>;

template <typename T>
auto IsTriviallyRelocatableImpl(PriorityTag<0>)
    -> BoolConstant<std::is_trivially_move_constructible<T>::value &&
                    std::is_trivially_destructible<T>::value>{};

} // namespace detail

template <typename T>
struct IsTriviallyRelocatable
    : decltype(detail::IsTriviallyRelocatableImpl<T>(PriorityTag<1>{})) {};

template <typename T>
struct IsTriviallyRelocatable<std::unique_ptr<T>> : std::true_type {};

template <typename T>
struct IsTriviallyRelocatable<std::shared_ptr<T>> : std::true_type {};

template <typename T>
struct IsTriviallyRelocatable<std::weak_ptr<T>> : std::true_type {};

template <typename Iterator>
struct IsForwardIterator
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
        template <typename U>                                                  \
        static constexpr auto check(U*) ->                                     \
            typename std::is_same<decltype(U::fn(std::declval<Args>()...)),    \
                                  Ret>::type;                                  \
                                                                               \
        template <typename>                                                    \
        static constexpr std::false_type check(...);                           \
                                                                               \
        using type = decltype(check<T>(nullptr));                              \
                                                                               \
        static constexpr bool value = type::value;                             \
    };

CFDS_META_HAS_FUNCTION(HasGetEmptyImpl, getEmpty);
CFDS_META_HAS_FUNCTION(HasGetTombstoneImpl, getTombstone);
CFDS_META_HAS_FUNCTION(HasGetHashImpl, getHash);
CFDS_META_HAS_FUNCTION(HasCompareImpl, compare);

#undef CFDS_META_HAS_FUNCTION

} // namespace detail

template <typename T>
using HasGetEmpty = detail::HasGetEmptyImpl<T, typename T::value_type(void)>;

template <typename T>
using HasGetTombstone =
    detail::HasGetTombstoneImpl<T, typename T::value_type(void)>;

template <typename T>
using HasGetHash =
    detail::HasGetHashImpl<T, std::size_t(const typename T::value_type&)>;

template <typename T>
using HasCompare =
    detail::HasCompareImpl<T, bool(const typename T::value_type&,
                                   const typename T::value_type&)>;

} // namespace meta
