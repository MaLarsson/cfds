#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace cfds {
namespace meta {

template <int N>
struct priority_tag : priority_tag<N - 1> {};

template <>
struct priority_tag<0> {};

namespace detail {

template <typename T>
auto is_trivially_relocatable_impl(priority_tag<1>)
    -> std::bool_constant<T::is_trivially_relocatable::value>;

template <typename T>
auto is_trivially_relocatable_impl(priority_tag<0>)
    -> std::bool_constant<std::is_trivially_move_constructible<T>::value &&
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

template <typename T>
concept trivially_relocatable = is_trivially_relocatable<T>::value;

namespace detail {

template <typename I>
using iterator_tag_t = typename std::iterator_traits<I>::iterator_category;

} // namespace detail

template <typename I>
concept iterator = std::weakly_incrementable<I> && requires(I i) {
    *i;
};

template <typename I>
concept input_iterator = iterator<I> &&
    std::derived_from<detail::iterator_tag_t<I>, std::input_iterator_tag>;

template <typename I>
concept forward_iterator = input_iterator<I> &&
    std::derived_from<detail::iterator_tag_t<I>, std::forward_iterator_tag>;

} // namespace meta
} // namespace cfds
