// Contains the definitions and declarations of small_dense_set<T, N, Traits>
// and small_dense_set_impl<T, Traits>.
//
// small_dense_set<T, N, Traits> is a hash set using open addressing and
// quadratic probing to handle collisions. It also has the first N buckets
// stored on the stack to avoid early heap allocations.
//
// An instance of small_dense_set_impl<T, Traits> cant be instantiated but it
// can be used to type erase the inline size template paramater N from
// small_dense_set<T, N, Traits>.
// i.e. void f(small_dense_set_impl<T, Traits>& v) can take any
// small_dense_set<T, N, Traits> as long as the T and Traits template parameter
// matches.

#pragma once

#include "dense_set_traits.hpp"
#include "utility.hpp"

#include <cstddef>
#include <initializer_list>
#include <utility>

namespace cfds {

template <typename T, typename Traits>
class small_dense_set_impl {
    using hasher = detail::dense_set_traits_impl<Traits>;

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
    small_dense_set_impl(int n) noexcept
        : m_buckets(reinterpret_cast<pointer>(detail::get_buffer_address(this))),
          size_(n) {}

    small_dense_set_impl() = delete;
    small_dense_set_impl(const small_dense_set_impl&) = delete;
    small_dense_set_impl(small_dense_set_impl&&) = delete;

    static constexpr bool is_power_of_two(int number) {
        return (number & (number - 1)) == 0;
    }

 private:
    pointer m_buckets = nullptr;
    int m_size = 0;
};

template <typename T, int N, typename Traits = DenseSetTraits<T>>
class small_dense_set : public small_dense_set_impl<T, Traits>,
                        private detail::aligned_storage_base<T, N> {
    static_assert(N >= 0 && small_dense_set_impl<T, Traits>::is_power_of_two(N),
                  "small_dense_set<T, N, Traits> requires N to be a power of "
                  "two or 0.");

 public:
    small_dense_set() : small_dense_set_impl<T, Traits>(N) {}

    small_dense_set(std::initializer_list<T> init) : small_dense_set() {
        (void)init;
        // TODO ...
    }
};

} // namespace cfds
