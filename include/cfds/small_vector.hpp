// Contains the definitions and declarations of small_vector<T, N> and
// small_vector_header<T> which behaves in a similar fashion to std::vector
// except for the fact that the first N elements are stored on the stack to
// avoid early heap allocations.
//
// An instance of small_vector_header<T> cant be instantiated but it can be used
// to type erase the inline size template paramater N from small_vector<T, N>.
// i.e. void f(small_vector_header<T>& v) can take any small_vector<T, N> as
// long as the T template parameter matches.

#pragma once

#include "meta.hpp"
#include "utility.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace cfds {

template <typename T>
class small_vector_header {
 public:
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

    ~small_vector_header() {
        if (!is_small()) std::free(m_first);
    }

    iterator begin() { return m_first; }
    const_iterator begin() const { return m_first; }
    const_iterator cbegin() const { return m_first; }

    iterator end() { return m_head; }
    const_iterator end() const { return m_head; }
    const_iterator cend() const { return m_head; }

    reverse_iterator rbegin() { return reverse_iterator(m_head); }
    const_reverse_iterator rbegin() const { return reverse_iterator(m_head); }
    const_reverse_iterator crbegin() const { return reverse_iterator(m_head); }

    reverse_iterator rend() { return reverse_iterator(m_first); }
    const_reverse_iterator rend() const { return reverse_iterator(m_first); }
    const_reverse_iterator crend() const { return reverse_iterator(m_first); }

    template <typename... Args>
    value_type& emplace_back(Args&&... args) {
        if (m_head == m_last) resize(capacity() + 1);
        ::new (m_head++) value_type(std::forward<Args>(args)...);
        return *(m_head - 1);
    }

    void push_back(const value_type& value) {
        push_back_impl(value, std::is_trivially_copyable<T>::type);
    }

    void push_back(value_type&& value) { emplace_back(std::move(value)); }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
	int index = static_cast<int>(pos - m_first);

	if (m_head == m_last) {
            int size = static_cast<int>(detail::next_power_of_two(capacity()));
            void* dest = detail::safe_malloc(sizeof(value_type) * size);
            pointer new_first = static_cast<pointer>(dest);

	    uninitialized_relocate(m_first, pos, new_first);
            uninitialized_relocate(pos, m_head, new_first + index + 1);

            ::new (new_first + index) value_type(std::forward<Args>(args)...);

            m_first = new_first;
            m_head = new_first + (pos - m_first) + 1;
	    m_last = new_first + size;
        } else {
	    iterator iter = const_cast<iterator>(pos);

            shift_data(pos, m_head, iter + 1);
	    ++m_head;

            ::new (iter) value_type(std::forward<Args>(args)...);
	}

        return &m_first[index];
    }

    value_type& back() { return *(m_head - 1); }
    const value_type& back() const { return *(m_head - 1); }

    value_type& front() { return *m_first; }
    const value_type& front() const { return *m_first; }

    value_type& operator[](int index) { return *(m_first + index); }
    const value_type& operator[](int index) const { return *(m_first + index); }

    value_type& at(int index) {
        if (index >= size()) throw std::out_of_range("");
        return (*this)[index];
    }

    const value_type& at(int index) const {
        if (index >= size()) throw std::out_of_range("");
        return (*this)[index];
    }

    void reserve(int size) {
        if (size > capacity()) resize(size);
    }

    pointer data() noexcept { return m_first; }
    const_pointer data() const noexcept { return m_first; }

    int size() const { return m_head - m_first; }
    int capacity() const { return m_last - m_first; }
    bool empty() const { return m_first == m_head; }

    void clear() {
        destroy_range(m_first, m_head);
        m_head = m_first;
    }

    // Returns whether the inlined buffer is currently in use to store the data.
    bool is_small() const {
        return m_first == detail::get_buffer_address(this);
    }

    small_vector_header& operator=(const small_vector_header& other) {
        if (this == &other) return *this;

        if (size() >= other.size()) {
            if (other.size() > 0) {
                pointer head = std::copy(other.begin(), other.end(), begin());
                destroy_range(head, end());
                m_head = head;
            } else {
                clear();
            }

            return *this;
        }

        if (capacity() < other.size()) {
            clear();
            resize(other.size());
        } else if (size() > 0) {
            std::copy(other.begin(), other.begin() + size(), begin());
        }

        std::uninitialized_copy(other.begin() + size(), other.end(),
                                begin() + size());

        m_head = m_first + other.size();

        return *this;
    }

    small_vector_header& operator=(small_vector_header&& other) {
        if (this == &other) return *this;

        if (!other.is_small()) {
            destroy_range(m_first, m_head);

            if (!is_small()) std::free(m_first);

            m_first = other.m_first;
            m_head = other.m_head;
            m_last = other.m_last;

            other.m_first = other.m_head = other.m_last =
                static_cast<pointer>(detail::get_buffer_address(&other));

            return *this;
        }

        if (size() >= other.size()) {
            if (other.size() > 0) {
                pointer head = std::move(other.begin(), other.end(), begin());
                destroy_range(head, end());
                m_head = head;
            } else {
                clear();
            }

            other.clear();

            return *this;
        }

        if (capacity() < other.size()) {
            clear();
            resize(other.size());
        } else if (size() > 0) {
            std::move(other.begin(), other.begin() + size(), begin());
        }

        uninitialized_move(other.begin() + size(), other.end(),
                           begin() + size());

        m_head = m_first + other.size();
        other.clear();

        return *this;
    }

 protected:
    small_vector_header(int n) noexcept
        : m_first(reinterpret_cast<pointer>(detail::get_buffer_address(this))),
          m_last(m_first + n), m_head(m_first) {}

    small_vector_header() = delete;
    small_vector_header(const small_vector_header&) = delete;
    small_vector_header(small_vector_header&&) = delete;

    // Turn destroy_range into a noop when T is trivially copyable.
    template <typename U = T>
    static typename std::enable_if<std::is_trivially_copyable<U>::value>::type
    destroy_range(pointer, pointer) {}

    template <typename U = T>
    static typename std::enable_if<!std::is_trivially_copyable<U>::value>::type
    destroy_range(pointer begin, pointer end) {
        for (; begin != end; ++begin) {
            begin->~value_type();
        }
    }

 private:
    pointer m_first = nullptr;
    pointer m_last = nullptr;
    pointer m_head = nullptr;

    template <typename InputIterator, typename ForwardIterator>
    static void uninitialized_move(InputIterator first, InputIterator last,
                                   ForwardIterator dest) {
        std::uninitialized_copy(std::make_move_iterator(first),
                                std::make_move_iterator(last), dest);
    }

    // Use memcpy instread of placement new when T is trivially copyable.
    void push_back_impl(const value_type& value, std::true_type) {
        if (m_head == m_last) resize(capacity() + 1);
        std::memcpy(m_head++, std::addressof(value), sizeof(value_type));
    }

    void push_back_impl(const value_type& value, std::false_type) {
        emplace_back(value);
    }

    void resize(int size_hint) {
        int next_pow = static_cast<int>(detail::next_power_of_two(capacity()));
        int new_size = std::max(size_hint, next_pow);

        void* dest = detail::safe_malloc(sizeof(value_type) * new_size);
        pointer new_first = static_cast<pointer>(dest);

        uninitialized_relocate(m_first, m_head, new_first);

        if (!is_small()) std::free(m_first);

        m_last = new_first + new_size;
        m_head = new_first + size();
        m_first = new_first;
    }

    // TODO: only use std::memmove if T is trivially relocatable, same rules as
    // uninitialized_relocate.
    template <typename U = T>
    void shift_data(const_iterator first, const_iterator last,
                    iterator dest) noexcept {
        std::memmove(dest, first, sizeof(value_type) * (last - first));
    }

    // Relocate using std::memcpy if T is trivially relocatable.
    template <typename U = T>
    typename std::enable_if<meta::is_trivially_relocatable<U>::value>::type
    uninitialized_relocate(const_iterator first, const_iterator last,
                           iterator dest) noexcept {
        std::memcpy(dest, first, sizeof(value_type) * (last - first));
    }

    // Relocate by calling constructor and destructor as a pair since there is
    // no risk of the constructor throwing.
    template <typename U = T>
    typename std::enable_if<!meta::is_trivially_relocatable<U>::value &&
                            std::is_nothrow_move_constructible<U>::value>::type
    uninitialized_relocate(const_iterator first, const_iterator last,
                           iterator dest) noexcept {
        for (auto begin = first; begin != last; ++begin, ++dest) {
            ::new (dest) value_type(std::move(*begin));
            begin->~value_type();
        }
    }

    // Relocate by calling the constructor and only after all elements have been
    // relocated can the destructor for the old shells be called since the
    // constructor might throw.
    template <typename U = T>
    typename std::enable_if<!meta::is_trivially_relocatable<U>::value &&
                            !std::is_nothrow_move_constructible<U>::value>::type
    uninitialized_relocate(const_iterator first, const_iterator last,
                           iterator dest) {
        try {
            for (auto begin = m_first; begin != m_head; ++begin, ++dest) {
                ::new (dest) value_type(std::move(*begin));
            }
        } catch (...) {
            std::free(dest);
            throw;
        }

        for (auto begin = m_first; begin != m_head; ++begin) {
            begin->~value_type();
        }
    }
};

template <typename T, int N = 4>
class small_vector : public small_vector_header<T>,
                     private detail::aligned_storage_base<T, N> {
    static_assert(N >= 0,
                  "small_vector<T, N> requires N to be greater or equal to 0.");

 public:
    small_vector() noexcept : small_vector_header<T>(N) {}

    ~small_vector() { this->destroy_range(this->begin(), this->end()); }

    template <typename InputIterator>
    small_vector(typename std::enable_if<
                     !meta::is_forward_iterator<InputIterator>::value,
                     InputIterator>::type first,
                 InputIterator last)
        : small_vector() {
        for (; first != last; ++first) {
            this->emplace_back(*first);
        }
    }

    template <typename ForwardIterator>
    small_vector(typename std::enable_if<
                     meta::is_forward_iterator<ForwardIterator>::value,
                     ForwardIterator>::type first,
                 ForwardIterator last)
        : small_vector() {
        this->reserve(last - first);
        for (; first != last; ++first) {
            this->emplace_back(*first);
        }
    }

    small_vector(std::initializer_list<T> init) : small_vector() {
        this->reserve(init.size());
        for (auto& element : init) {
            this->emplace_back(element);
        }
    }

    small_vector(const small_vector& other) : small_vector() {
        if (!other.empty()) small_vector_header<T>::operator=(other);
    }

    template <int N2>
    small_vector(const small_vector<T, N2>& other) : small_vector() {
        if (!other.empty()) small_vector_header<T>::operator=(other);
    }

    small_vector(small_vector&& other) : small_vector() {
        if (!other.empty()) small_vector_header<T>::operator=(std::move(other));
    }

    template <int N2>
    small_vector(small_vector<T, N2>&& other) : small_vector() {
        if (!other.empty()) small_vector_header<T>::operator=(std::move(other));
    }

    small_vector& operator=(const small_vector& other) {
        small_vector_header<T>::operator=(other);
        return *this;
    }

    template <int N2>
    small_vector& operator=(const small_vector<T, N2>& other) {
        small_vector_header<T>::operator=(other);
        return *this;
    }

    small_vector& operator=(small_vector&& other) {
        small_vector_header<T>::operator=(std::move(other));
        return *this;
    }

    template <int N2>
    small_vector& operator=(small_vector<T, N2>&& other) {
        small_vector_header<T>::operator=(std::move(other));
        return *this;
    }
};

} // namespace cfds
