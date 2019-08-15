#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace cfds {
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
struct is_input_iterator
    : std::is_base_of<
          std::input_iterator_tag,
          typename std::iterator_traits<Iterator>::iterator_category> {};

template <typename Iterator>
struct is_forward_iterator
    : std::is_base_of<
          std::forward_iterator_tag,
          typename std::iterator_traits<Iterator>::iterator_category> {};

} // namespace meta
} // namespace cfds
