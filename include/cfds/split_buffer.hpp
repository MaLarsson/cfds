#pragma once

#include "meta.hpp"
#include "utility.hpp"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace cfds {
namespace detail {

template <typename T>
struct split_buffer {
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
    pointer m_end_cap = nullptr;

    split_buffer() = default;

    split_buffer(size_type n) {
        void* data = detail::safe_malloc(sizeof(value_type) * n);

        m_begin = static_cast<pointer>(data);
        m_end = m_begin;
        m_end_cap = m_begin + n;
    }

    ~split_buffer() {
        clear();
        std::free(m_begin);
    }

    iterator begin() { return m_begin; }
    iterator end() { return m_end; }

    void clear() noexcept {
        for (pointer p = m_begin; p != m_end; ++p) {
            p->~value_type();
        }
    }

    template <typename InputIterator>
    typename std::enable_if<
        meta::is_input_iterator<InputIterator>::value &&
        !meta::is_forward_iterator<InputIterator>::value>::type
    construct_at_end(InputIterator first, InputIterator last) {
        for (; first != last; ++first) {
            if (m_end == m_end_cap) {
                size_type old_cap = m_end_cap - m_begin;
                size_type new_cap = std::max<size_type>(2 * old_cap, 8);
                split_buffer new_buffer(new_cap);

                for (pointer p = m_begin; p != m_end; ++p, ++new_buffer.m_end) {
                    ::new (new_buffer.m_end) value_type(std::move(*p));
                }

                swap(new_buffer);
            }

            ::new (m_end) value_type(*first);
            ++m_end;
        }
    }

    template <typename ForwardIterator>
    typename std::enable_if<
        meta::is_forward_iterator<ForwardIterator>::value>::type
    construct_at_end(ForwardIterator first, ForwardIterator last) {
        // TODO ...
        for (; first != last; ++first) {
            if (m_end == m_end_cap) {
                size_type old_cap = m_end_cap - m_begin;
                size_type new_cap = std::max<size_type>(2 * old_cap, 8);
                split_buffer new_buffer(new_cap);

                for (pointer p = m_begin; p != m_end; ++p, ++new_buffer.m_end) {
                    ::new (new_buffer.m_end) value_type(std::move(*p));
                }

                swap(new_buffer);
            }

            ::new (m_end) value_type(*first);
            ++m_end;
        }
    }

    void swap(split_buffer& other) noexcept {
        using std::swap;
        swap(m_begin, other.m_begin);
        swap(m_end, other.m_end);
        swap(m_end_cap, other.m_end_cap);
    }
};

} // namespace detail
} // namespace cfds
