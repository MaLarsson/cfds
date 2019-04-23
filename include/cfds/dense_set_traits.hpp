#pragma once

#include "meta.hpp"

#include <cstddef>
#include <functional>
#include <limits>
#include <string>
#include <type_traits>

namespace cfds {
namespace detail {

template <typename T>
struct dense_set_traits_impl {
    using value_type = typename T::value_type;

    // Get empty
    template <typename U = T,
              typename std::enable_if<!meta::has_get_empty<U>::value>::type* =
                  nullptr>
    static value_type get_empty() {
        return std::numeric_limits<value_type>::max();
    }

    template <
        typename U = T,
        typename std::enable_if<meta::has_get_empty<U>::value>::type* = nullptr>
    static value_type get_empty() {
        return T::get_empty();
    }

    // Get tombstone
    template <typename U = T, typename std::enable_if<!meta::has_get_tombstone<
                                  U>::value>::type* = nullptr>
    static value_type get_tombstone() {
        return std::numeric_limits<value_type>::min();
    }

    template <typename U = T, typename std::enable_if<meta::has_get_tombstone<
                                  U>::value>::type* = nullptr>
    static value_type get_tombstone() {
        return T::get_tombstone();
    }

    // Get hash
    template <
        typename U = T,
        typename std::enable_if<!meta::has_get_hash<U>::value>::type* = nullptr>
    static std::size_t get_hash(const value_type& value) {
        return std::hash<value_type>{}(value);
    }

    template <
        typename U = T,
        typename std::enable_if<meta::has_get_hash<U>::value>::type* = nullptr>
    static std::size_t get_hash(const value_type& value) {
        return T::get_hash(value);
    }

    // Compare
    template <
        typename U = T,
        typename std::enable_if<!meta::has_compare<U>::value>::type* = nullptr>
    static bool compare(const value_type& lhs, const value_type& rhs) {
        return lhs == rhs;
    }

    template <typename U = T, typename std::enable_if<
                                  meta::has_compare<U>::value>::type* = nullptr>
    static bool compare(const value_type& lhs, const value_type& rhs) {
        return T::compare(lhs, rhs);
    }
};

} // namespace detail

template <typename T>
struct dense_set_traits {
    using value_type = T;
};

// Specialization of dense_set_traits for std::string.
// Note that the strings "__e" and "__t" are reserved to represent the empty
// value and the tombstone value respectively.
template <>
struct dense_set_traits<std::string> {
    using value_type = std::string;
    static value_type get_empty() { return "__e"; }
    static value_type get_tombstone() { return "__t"; }
};

} // namespace cfds
