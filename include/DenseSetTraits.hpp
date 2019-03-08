#pragma once

#include <cstddef>
#include <functional>
#include <limits>
#include <string>
#include <type_traits>

namespace cfds {
namespace detail {

template <typename T>
struct DenseSetTraitsImpl {
    using value_type = typename T::value_type;

    // Get empty
    template <
        typename U = T,
        typename std::enable_if<!meta::HasGetEmpty<U>::value>::type* = nullptr>
    static value_type getEmpty() {
        return std::numeric_limits<value_type>::max();
    }

    template <typename U = T, typename std::enable_if<
                                  meta::HasGetEmpty<U>::value>::type* = nullptr>
    static value_type getEmpty() {
        return T::getEmpty();
    }

    // Get tombstone
    template <typename U = T,
              typename std::enable_if<!meta::HasGetTombstone<U>::value>::type* =
                  nullptr>
    static value_type getTombstone() {
        return std::numeric_limits<value_type>::min();
    }

    template <typename U = T,
              typename std::enable_if<meta::HasGetTombstone<U>::value>::type* =
                  nullptr>
    static value_type getTombstone() {
        return T::getTombstone();
    }

    // Get hash
    template <typename U = T, typename std::enable_if<
                                  !meta::HasGetHash<U>::value>::type* = nullptr>
    static std::size_t getHash(const value_type& value) {
        return std::hash<value_type>{}(value);
    }

    template <typename U = T, typename std::enable_if<
                                  meta::HasGetHash<U>::value>::type* = nullptr>
    static std::size_t getHash(const value_type& value) {
        return T::getHash(value);
    }

    // Compare
    template <typename U = T, typename std::enable_if<
                                  !meta::HasCompare<U>::value>::type* = nullptr>
    static bool compare(const value_type& lhs, const value_type& rhs) {
        return lhs == rhs;
    }

    template <typename U = T, typename std::enable_if<
                                  meta::HasCompare<U>::value>::type* = nullptr>
    static bool compare(const value_type& lhs, const value_type& rhs) {
        return T::compare(lhs, rhs);
    }
};

} // namespace detail

template <typename T>
struct DenseSetTraits {
    using value_type = T;
};

// Specialization of DenseSetTrats for std::string.
// Note that the strings "__e" and "__t" are reserved to represent the empty
// value and the tombstone value respectively.
template <>
struct DenseSetTraits<std::string> {
    using value_type = std::string;
    static value_type getEmpty() { return "__e"; }
    static value_type getTombstone() { return "__t"; }
};

} // namespace cfds
