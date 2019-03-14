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
    SmallDenseSetImpl(int n) noexcept
        : buckets_(reinterpret_cast<pointer>(getFirstSmallElement())),
          size_(n) {}

    SmallDenseSetImpl() = delete;
    SmallDenseSetImpl(const SmallDenseSetImpl&) = delete;
    SmallDenseSetImpl(SmallDenseSetImpl&&) = delete;

    static constexpr bool isPowerOfTwo(int number) {
        return (number & (number - 1)) == 0;
    }

 private:
    pointer buckets_ = nullptr;
    int size_ = 0;

    // Returns a pointer to the first element of the inline buffer by
    // calculating the offset from the this pointer and the buffer member.
    void* getFirstSmallElement() const {
        std::size_t offset = reinterpret_cast<std::size_t>(
            &(reinterpret_cast<detail::SmallDenseSetAlignment<T>*>(0)->buffer));

        return const_cast<void*>(reinterpret_cast<const void*>(
            reinterpret_cast<const char*>(this) + offset));
    }
};

namespace detail {

template <typename T, int N>
struct AlignedStorageBase {
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer[N];
};

// AlignedStorageBase<T, 0> has to be aligned as if it contained an internal
// buffer so that the pointer arithmetic in
// SmallDenseSetImpl<T>::getFirstSmallElement() will work.
template <typename T>
struct alignas(alignof(T)) AlignedStorageBase<T, 0> {};

} // namespace detail

template <typename T, int N, typename Traits = DenseSetTraits<T>>
class SmallDenseSet : public SmallDenseSetImpl<T, Traits>,
                      private detail::AlignedStorageBase<T, N> {
    static_assert(N >= 0 && isPowerOfTwo(N),
                  "SmallDenseSet<T, N, Traits> requires N to be a power of two "
                  "or 0.");

 public:
    SmallDenseSet() : SmallDenseSetImpl(N) {}

    SmallDenseSet(std::initializer_list<T> init) : SmallDenseSet() {
        // TODO ...
    }
};

} // namespace cfds
