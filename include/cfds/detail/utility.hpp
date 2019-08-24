#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace cfds {
namespace detail {

template <typename T, int N>
struct aligned_storage_base {
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer[N];
};

// aligned_storage_base<T, 0> has to be aligned as if it contained an internal
// buffer so that the pointer arithmetic in
// SmallDenseSetImpl<T>::getFirstSmallElement() will work.
template <typename T>
struct alignas(alignof(T)) aligned_storage_base<T, 0> {};

template <typename T>
struct aligned_storage_base_alignment {
    T padding;
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer;
};

// Returns a pointer to the first element of the inline buffer by
// calculating the offset from the this pointer and the buffer member.
template <typename T>
inline void* get_buffer_address(T* container) {
    std::size_t offset = reinterpret_cast<std::size_t>(
        &(reinterpret_cast<aligned_storage_base_alignment<T>*>(0)->buffer));

    return const_cast<void*>(reinterpret_cast<const void*>(
        reinterpret_cast<const char*>(container) + offset));
}

// Returns the next power of two starting from n, if n is a power of two the
// return value will still be the next power of two.
inline std::uint64_t next_power_of_two(std::uint64_t number) {
    number |= (number >> 1);
    number |= (number >> 2);
    number |= (number >> 4);
    number |= (number >> 8);
    number |= (number >> 16);
    number |= (number >> 32);
    return number + 1;
}

// Malloc which throws std::bad_alloc if allocation fails.
inline void* safe_malloc(std::size_t size) {
    void* data = std::malloc(size);
    if (data == nullptr) throw std::bad_alloc();
    return data;
}

} // namespace detail
} // namespace cfds
