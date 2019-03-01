#pragma once

#include <functional>
#include <limits>
#include <string>
#include <utility>

namespace cfds {

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

namespace detail {

// TODO: should not always return std::false_type
template <typename T>
struct HasFunctionX : std::false_type {};

template <typename T>
struct DenseSetTraitsImpl {
    using value_type = typename T::value_type;

    // Get empty
    template <typename U = T,
              typename std::enable_if<!HasFunctionX<U>::value, int>::type = 0>
    static value_type getEmpty() {
        return std::numeric_limits<value_type>::max();
    }

    template <typename U = T,
              typename std::enable_if<HasFunctionX<U>::value, int>::type = 0>
    static value_type getEmpty() {
        return T::getEmpty();
    }

    // Get tombstone
    template <typename U = T,
              typename std::enable_if<!HasFunctionX<U>::value, int>::type = 0>
    static value_type getTombstone() {
        return std::numeric_limits<value_type>::min();
    }

    template <typename U = T,
              typename std::enable_if<HasFunctionX<U>::value, int>::type = 0>
    static value_type getTombstone() {
        return T::getTombstone();
    }

    // Get hash
    template <typename U = T,
              typename std::enable_if<!HasFunctionX<U>::value, int>::type = 0>
    static std::size_t getHash(const value_type& value) {
        return std::hash<value_type>{}(value);
    }

    template <typename U = T,
              typename std::enable_if<HasFunctionX<U>::value, int>::type = 0>
    static std::size_t getHash(const value_type& value) {
        return T::getHash(value);
    }

    // Compare
    template <typename U = T,
              typename std::enable_if<!HasFunctionX<U>::value, int>::type = 0>
    static bool compare(const value_type& lhs, const value_type& rhs) {
        return lhs == rhs;
    }

    template <typename U = T,
              typename std::enable_if<HasFunctionX<U>::value, int>::type = 0>
    static bool compare(const value_type& lhs, const value_type& rhs) {
        return T::compare(lhs, rhs);
    }
};

} // namespace detail

template <typename T, typename Traits = DenseSetTraits<T>>
class DenseSet {
    using Hasher = detail::DenseSetTraitsImpl<Traits>;

 public:
    using key_type = T;
    using value_type = T;
    using size_type = int;
    using difference_type = std::ptrdiff_t;

    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    using iterator = pointer;
    using const_iterator = const_pointer;

    DenseSet(const T& value) {
        Hasher::getEmpty();
        Hasher::getTombstone();
        Hasher::getHash(value);
        Hasher::compare(value, value);
    }

 private:
    // TODO ...
};

} // namespace cfds
