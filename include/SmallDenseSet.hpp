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
#include "Utility.hpp"

#include <cstddef>
#include <initializer_list>
#include <utility>

namespace cfds {

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
        : buckets_(reinterpret_cast<pointer>(detail::getBufferAddress(this))),
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
};

template <typename T, int N, typename Traits = DenseSetTraits<T>>
class SmallDenseSet : public SmallDenseSetImpl<T, Traits>,
                      private detail::AlignedStorageBase<T, N> {
    static_assert(N >= 0 && SmallDenseSetImpl<T, Traits>::isPowerOfTwo(N),
                  "SmallDenseSet<T, N, Traits> requires N to be a power of two "
                  "or 0.");

 public:
    SmallDenseSet() : SmallDenseSetImpl<T, Traits>(N) {}

    SmallDenseSet(std::initializer_list<T> init) : SmallDenseSet() {
        (void)init;
        // TODO ...
    }
};

} // namespace cfds
