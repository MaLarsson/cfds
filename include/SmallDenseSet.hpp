// Contains the definitions and declarations of SmallDenseSet<T, N, Traits> and
// SmallDenseSetImpl<T, Traits>.
//
// SmallDenseSet<T, N, Traits> is a hash set using open addressing and quadratic
// probing to handle collisions. It also has the first N buckets stored on the
// stack to avoid early heap allocations.
//
// An instance of SmallDenseSetImpl<T, Traits> cant be instantiated but it can
// be used to type erase the inline size template paramater N from
// SmallDenseSet<T, N, Traits>.
// i.e. void f(SmallDenseSetImpl<T, Traits>& v) can take any
// SmallDenseSet<T, N, Traits> as long as the T and Traits template parameter
// matches.

#pragma once

#include <functional>
#include <limits>
#include <string>
#include <utility>

#define CFDS_META_HAS_FUNCTION(name, fn)                                       \
    template <typename T, typename Ret, typename... Args>                      \
    struct name {                                                              \
        using type = std::false_type;                                          \
        static constexpr bool value = false;                                   \
    };                                                                         \
                                                                               \
    template <typename T, typename Ret, typename... Args>                      \
    struct name<T, Ret(Args...)> {                                             \
        template <typename T>                                                  \
        static constexpr auto check(T*) ->                                     \
            typename std::is_same<decltype(T::fn(std::declval<Args>()...)),    \
                                  Ret>::type;                                  \
                                                                               \
        template <typename>                                                    \
        static constexpr std::false_type check(...);                           \
                                                                               \
        using type = decltype(check<T>(nullptr));                              \
                                                                               \
        static constexpr bool value = type::value;                             \
    };

namespace meta {
namespace detail {

CFDS_META_HAS_FUNCTION(HasGetEmptyImpl, getEmpty);
CFDS_META_HAS_FUNCTION(HasGetTombstoneImpl, getTombstone);
CFDS_META_HAS_FUNCTION(HasGetHashImpl, getHash);
CFDS_META_HAS_FUNCTION(HasCompareImpl, compare);

} // namespace detail

template <typename T>
using HasGetEmpty = detail::HasGetEmptyImpl<T, typename T::value_type, void>;

template <typename T>
using HasGetTombstone =
    detail::HasGetTombstoneImpl<T, typename T::value_type, void>;

template <typename T>
using HasGetHash =
    detail::HasGetHashImpl<T, std::size_t, typename T::value_type>;

template <typename T>
using HasCompare =
    detail::HasCompareImpl<T, bool, const typename T::value_type&,
                           const typename T::value_type&>;

} // namespace meta

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
