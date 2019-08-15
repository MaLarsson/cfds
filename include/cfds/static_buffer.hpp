// Contains the definition of static_buffer<T> which can be used as temporary
// storage. It's constructed from forward iterators, allocating space for the
// elements are copying them into the storage.

#pragma once

#include "meta.hpp"
#include "utility.hpp"

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace cfds {
namespace detail {

template <typename T>
struct static_buffer {
    using value_type = T;
    using size_type = int;
    using difference_type = std::ptrdiff_t;

    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;

    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    pointer m_begin = nullptr;
    pointer m_end = nullptr;

    template <typename ForwardIterator>
    static_buffer(typename std::enable_if<
                      meta::is_forward_iterator<ForwardIterator>::value,
                      ForwardIterator>::type first,
                  ForwardIterator last) {
        std::size_t count = std::distance(first, last);
        void* data = detail::safe_malloc(sizeof(value_type) * count);

        m_begin = static_cast<pointer>(data);
        m_end = m_begin;

        construct_range(first, last, count);
    }

    ~static_buffer() {
        clear();
        std::free(m_begin);
    }

    iterator begin() { return m_begin; }
    iterator end() { return m_end; }

    void swap(static_buffer& other) noexcept {
        using std::swap;
        swap(m_begin, other.m_begin);
        swap(m_end, other.m_end);
    }

 private:
    template <typename U = T>
    typename std::enable_if<std::is_trivially_copyable<U>::value>::type
    clear() {}

    template <typename U = T>
    typename std::enable_if<!std::is_trivially_copyable<U>::value>::type
    clear() {
        for (pointer p = m_begin; p != m_end; ++p) {
            p->~value_type();
        }
    }

    template <typename ForwardIterator, typename U = T>
    typename std::enable_if<std::is_trivially_copyable<U>::value>::type
    construct_range(ForwardIterator first, ForwardIterator last,
                    std::size_t count) {
        std::memcpy(m_end, first, sizeof(value_type) * count);
        m_end += count;
    }

    template <typename ForwardIterator, typename U = T>
    typename std::enable_if<!std::is_trivially_copyable<U>::value>::type
    construct_range(ForwardIterator first, ForwardIterator last, std::size_t) {
        for (; first != last; ++first) {
            ::new (m_end) value_type(*first);
            ++m_end;
        }
    }
};

} // namespace detail
} // namespace cfds
