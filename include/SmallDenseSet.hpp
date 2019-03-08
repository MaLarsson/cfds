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

#include "DenseSetTraits.hpp"

#include <utility>
#include <initializer_list>
#include <cstddef>

namespace meta {
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

namespace cfds {

template <typename T, typename Traits = DenseSetTraits<T>>
class SmallDenseSetImpl;

namespace detail {

template <typename T>
struct SmallDenseSetAlignment {
    SmallDenseSetImpl<T> impl;
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer;
};

} // namespace detail

template <typename T, typename Traits>
class SmallDenseSetImpl {
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

 protected:
    SmallDenseSetImpl(int n) noexcept {}

    SmallDenseSetImpl() = delete;
    SmallDenseSetImpl(const SmallDenseSetImpl&) = delete;
    SmallDenseSetImpl(SmallDenseSetImpl&&) = delete;

 private:
    // TODO ...
};

template <typename T, int N>
struct SmallDenseSetStorage {
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer[N];
};

// SmallDenseSetStorage<T, 0> has to be aligned as if it contained an internal
// buffer so that the pointer arithmetic in
// SmallDenseSetImpl<T>::getFirstSmallElement() will work.
template <typename T>
struct alignas(alignof(T)) SmallDenseSetStorage<T, 0> {};

template <typename T, int N, typename Traits = DenseSetTraits<T>>
class SmallDenseSet : public SmallDenseSetImpl<T, Traits>,
                      SmallDenseSetStorage<T, N> {
    static_assert(N > -1, "SmallDenseSet requires N >= 0");

 public:
    SmallDenseSet() : SmallDenseSetImpl(N) {}

    SmallDenseSet(std::initializer_list<T> init) : SmallDenseSet() {
        // TODO ...
    }
};

} // namespace cfds
