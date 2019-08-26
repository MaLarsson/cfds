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

#include "detail/static_buffer.hpp"
#include "detail/utility.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <limits>
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
        if (!is_small()) std::free(m_begin);
    }

    iterator begin() { return m_begin; }
    const_iterator begin() const { return m_begin; }
    const_iterator cbegin() const { return m_begin; }

    iterator end() { return m_end; }
    const_iterator end() const { return m_end; }
    const_iterator cend() const { return m_end; }

    reverse_iterator rbegin() { return reverse_iterator(m_end); }
    const_reverse_iterator rbegin() const { return reverse_iterator(m_end); }
    const_reverse_iterator crbegin() const { return reverse_iterator(m_end); }

    reverse_iterator rend() { return reverse_iterator(m_begin); }
    const_reverse_iterator rend() const { return reverse_iterator(m_begin); }
    const_reverse_iterator crend() const { return reverse_iterator(m_begin); }

    void assign(size_type count, const value_type& value) {
        clear();
        reserve(count);

        for (size_type i = 0; i < count; ++i) {
            push_back(value);
        }
    }

    template <typename InputIterator>
    typename std::enable_if<
        meta::is_input_iterator<InputIterator>::value &&
        !meta::is_forward_iterator<InputIterator>::value>::type
    assign(InputIterator first, InputIterator last) {
        clear();

        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }

    template <typename ForwardIterator>
    typename std::enable_if<
        meta::is_forward_iterator<ForwardIterator>::value>::type
    assign(ForwardIterator first, ForwardIterator last) {
        clear();
        reserve(std::distance(first, last));

        for (; first != last; ++first) {
            emplace_back(*first);
        }
    }

    void assign(std::initializer_list<T> ilist) {
        clear();
        reserve(ilist.size());

        for (auto&& element : ilist) {
            emplace_back(element);
        }
    }

    template <typename... Args>
    value_type& emplace_back(Args&&... args) {
        if (m_end == m_end_cap) grow(capacity() + 1);
        ::new (m_end++) value_type(std::forward<Args>(args)...);
        return *(m_end - 1);
    }

    void push_back(const value_type& value) {
        push_back_impl(value, typename std::is_trivially_copyable<T>::type{});
    }

    void push_back(value_type&& value) { emplace_back(std::move(value)); }

    void pop_back() {
        destroy_range(m_end - 1, m_end);
        --m_end;
    }

    void resize(size_type count) {
        if (count < size()) {
            destroy_range(m_begin + count, m_end);
            m_end = m_begin + count;
            return;
        }

        if (count > size()) {
            reserve(count);

            for (int i = size(); i < count; ++i) {
                emplace_back(value_type{});
            }
        }
    }

    void resize(size_type count, const value_type& value) {
        if (count < size()) {
            destroy_range(m_begin + count, m_end);
            m_end = m_begin + count;
            return;
        }

        if (count > size()) {
            reserve(count);

            for (int i = size(); i < count; ++i) {
                push_back(value);
            }
        }
    }

    void swap(small_vector_header& other) {
        if (this == &other) return;

        if (!is_small() && !other.is_small()) {
            using std::swap;
            swap(m_begin, other.m_begin);
            swap(m_end_cap, other.m_end_cap);
            swap(m_end, other.m_end);
            return;
        }

        if (size() > other.size()) {
            slow_swap(*this, other);
            return;
        }

        slow_swap(other, *this);
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        iterator iter = make_space(pos, 1);
        ::new (iter) value_type(std::forward<Args>(args)...);
        return iter;
    }

    iterator insert(const_iterator pos, const value_type& value) {
        iterator iter = make_space(pos, 1);
        insert_at_pos(iter, value);

        return iter;
    }

    iterator insert(const_iterator pos, value_type&& value) {
        return emplace(pos, std::move(value));
    }

    iterator insert(const_iterator pos, size_type count,
                    const value_type& value) {
        iterator iter = make_space(pos, count);

        for (size_type i = 0; i < count; ++i) {
            insert_at_pos(iter + i, value);
        }

        return iter;
    }

    // Overload of insert with iterators that has the iterator_category
    // std::input_iterator_tag. Input iterators are single pass so we cant
    // calculate the distance between first and last before insertion.
    template <typename InputIterator>
    typename std::enable_if<
        meta::is_input_iterator<InputIterator>::value &&
            !meta::is_forward_iterator<InputIterator>::value,
        iterator>::type
    insert(const_iterator pos, InputIterator first, InputIterator last) {
        int index = static_cast<int>(pos - m_begin);
        detail::static_buffer<value_type> buffer(&m_begin[index], m_end);

        destroy_range(&m_begin[index], m_end);
        m_end = &m_begin[index];

        for (auto iter = first; iter != last; ++iter) {
            push_back(*iter);
        }

        insert(m_end, buffer.begin(), buffer.end());

        return &m_begin[index];
    }

    template <typename ForwardIterator>
    typename std::enable_if<meta::is_forward_iterator<ForwardIterator>::value,
                            iterator>::type
    insert(const_iterator pos, ForwardIterator first, ForwardIterator last) {
        int count = std::distance(first, last);
        iterator iter = make_space(pos, count);

        for (size_type i = 0; i < count; ++i, (void)++first) {
            insert_at_pos(iter + i, *first);
        }

        return iter;
    }

    iterator insert(const_iterator pos,
                    std::initializer_list<value_type> ilist) {
        return insert(pos, std::begin(ilist), std::end(ilist));
    }

    value_type& back() { return *(m_end - 1); }
    const value_type& back() const { return *(m_end - 1); }

    value_type& front() { return *m_begin; }
    const value_type& front() const { return *m_begin; }

    value_type& operator[](int index) { return *(m_begin + index); }
    const value_type& operator[](int index) const { return *(m_begin + index); }

    value_type& at(int index) {
        if (index >= size()) throw std::out_of_range("");
        return (*this)[index];
    }

    const value_type& at(int index) const {
        if (index >= size()) throw std::out_of_range("");
        return (*this)[index];
    }

    void reserve(int size) {
        if (size > capacity()) grow(size);
    }

    pointer data() noexcept { return m_begin; }
    const_pointer data() const noexcept { return m_begin; }

    size_type max_size() const noexcept {
        return std::min<size_type>(std::numeric_limits<size_type>::max(),
                                   std::numeric_limits<difference_type>::max());
    }

    size_type size() const noexcept { return m_end - m_begin; }
    size_type capacity() const noexcept { return m_end_cap - m_begin; }
    bool empty() const noexcept { return m_begin == m_end; }

    void shrink_to_fit() {
        if (is_small() || size() == capacity()) return;

        if (size() == 0) {
            std::free(m_begin);
            m_begin = m_end = m_end_cap = nullptr;
            return;
        }

        pointer new_begin = static_cast<pointer>(
            detail::safe_malloc(sizeof(value_type) * size()));

        try {
            uninitialized_relocate(m_begin, m_end, new_begin);
        } catch (...) {
            std::free(new_begin);
            throw;
        }

        std::free(m_begin);

        m_end_cap = new_begin + size();
        m_end = new_begin + size();
        m_begin = new_begin;
    }

    void clear() {
        destroy_range(m_begin, m_end);
        m_end = m_begin;
    }

    iterator erase(const_iterator pos) { return erase(pos, pos + 1); }

    iterator erase(const_iterator first, const_iterator last) {
        if (first == last) return const_cast<iterator>(first);

        destroy_range(first, last);

        if (last != m_end) {
            shift_data(last, m_end, const_cast<iterator>(first));
        }

        m_end -= last - first;

        return const_cast<iterator>(first);
    }

    // Returns whether the inlined buffer is currently in use to store the data.
    bool is_small() const {
        return m_begin == detail::get_buffer_address(this);
    }

    small_vector_header& operator=(const small_vector_header& other) {
        if (this == &other) return *this;

        if (size() >= other.size()) {
            if (other.size() > 0) {
                pointer head = std::copy(other.begin(), other.end(), begin());
                destroy_range(head, end());
                m_end = head;
            } else {
                clear();
            }

            return *this;
        }

        if (capacity() < other.size()) {
            clear();
            grow(other.size());
        } else if (size() > 0) {
            std::copy(other.begin(), other.begin() + size(), begin());
        }

        std::uninitialized_copy(other.begin() + size(), other.end(),
                                begin() + size());

        m_end = m_begin + other.size();

        return *this;
    }

    small_vector_header& operator=(small_vector_header&& other) {
        if (this == &other) return *this;

        if (!other.is_small()) {
            destroy_range(m_begin, m_end);

            if (!is_small()) std::free(m_begin);

            m_begin = other.m_begin;
            m_end = other.m_end;
            m_end_cap = other.m_end_cap;

            other.m_begin = other.m_end = other.m_end_cap =
                static_cast<pointer>(detail::get_buffer_address(&other));

            return *this;
        }

        if (size() >= other.size()) {
            if (other.size() > 0) {
                pointer head = std::move(other.begin(), other.end(), begin());
                destroy_range(head, end());
                m_end = head;
            } else {
                clear();
            }

            other.clear();

            return *this;
        }

        if (capacity() < other.size()) {
            clear();
            grow(other.size());
        } else if (size() > 0) {
            std::move(other.begin(), other.begin() + size(), begin());
        }

        uninitialized_move(other.begin() + size(), other.end(),
                           begin() + size());

        m_end = m_begin + other.size();
        other.clear();

        return *this;
    }

 protected:
    small_vector_header(int n) noexcept
        : m_begin(reinterpret_cast<pointer>(detail::get_buffer_address(this))),
          m_end(m_begin), m_end_cap(m_begin + n) {}

    small_vector_header() = delete;
    small_vector_header(const small_vector_header&) = delete;
    small_vector_header(small_vector_header&&) = delete;

    // Turn destroy_range into a noop when T is trivially copyable.
    template <typename U = T>
    static typename std::enable_if<std::is_trivially_copyable<U>::value>::type
    destroy_range(const_iterator, const_iterator) {}

    template <typename U = T>
    static typename std::enable_if<!std::is_trivially_copyable<U>::value>::type
    destroy_range(const_iterator begin, const_iterator end) {
        for (; begin != end; ++begin) {
            begin->~value_type();
        }
    }

 private:
    pointer m_begin = nullptr;
    pointer m_end = nullptr;
    pointer m_end_cap = nullptr;

    void slow_swap(small_vector_header& big, small_vector_header& small) {
        if (big.size() > small.capacity()) small.grow(big.size());

        size_type nr_shared = small.size();

        for (size_type i = 0; i < nr_shared; ++i) {
            using std::swap;
            swap(big[i], small[i]);
        }

        uninitialized_relocate(big.m_begin + nr_shared, big.m_end,
                               small.m_begin + nr_shared);

        small.m_end = small.m_begin + big.size();
        big.m_end = big.m_begin + nr_shared;
    }

    template <typename InputIterator, typename ForwardIterator>
    static void uninitialized_move(InputIterator first, InputIterator last,
                                   ForwardIterator dest) {
        std::uninitialized_copy(std::make_move_iterator(first),
                                std::make_move_iterator(last), dest);
    }

    // Use memcpy instread of placement new when T is trivially copyable.
    template <typename U = T>
    static typename std::enable_if<std::is_trivially_copyable<U>::value>::type
    insert_at_pos(pointer pos, const value_type& value) {
        std::memcpy(pos, std::addressof(value), sizeof(value_type));
    }

    template <typename U = T>
    static typename std::enable_if<!std::is_trivially_copyable<U>::value>::type
    insert_at_pos(pointer pos, const value_type& value) {
        ::new (pos) value_type(value);
    }

    // Use memcpy instread of placement new when T is trivially copyable.
    void push_back_impl(const value_type& value, std::true_type) {
        if (m_end == m_end_cap) grow(capacity() + 1);
        std::memcpy(m_end++, std::addressof(value), sizeof(value_type));
    }

    void push_back_impl(const value_type& value, std::false_type) {
        emplace_back(value);
    }

    // Shifts around data to be able to construct count elements into the
    // small_vector starting at pos. This function may reallocate so an iterator
    // to the position in which elements can be constructed is returned.
    iterator make_space(const_iterator pos, size_type count) {
        int index = static_cast<int>(pos - m_begin);
        int new_size = size() + count;

        if (new_size > capacity()) {
            int new_cap = capacity() + count;
            pointer new_begin = allocate(new_cap);

            try {
                uninitialized_relocate(m_begin, pos, new_begin);
                uninitialized_relocate(pos, m_end, new_begin + index + count);
            } catch (...) {
                std::free(new_begin);
                throw;
            }

            if (!is_small()) std::free(m_begin);

            m_begin = new_begin;
            m_end = new_begin + new_size;
            m_end_cap = new_begin + new_cap;
        } else {
            shift_data(pos, m_end, &m_begin[index] + count);
            ++m_end;
        }

        return &m_begin[index];
    }

    // Allocates a new chunk of memory based on the size_hint and returns a
    // pointer to the beginning of the newly allocated chunk. The size of the
    // newly allocated memory is put into the size_hint.
    pointer allocate(int& size_hint) {
        int next_pow = static_cast<int>(detail::next_power_of_two(capacity()));
        size_hint = std::max(size_hint, next_pow);

        return static_cast<pointer>(
            detail::safe_malloc(sizeof(value_type) * size_hint));
    }

    void grow(int size_hint) {
        pointer new_begin = allocate(size_hint);

        try {
            uninitialized_relocate(m_begin, m_end, new_begin);
        } catch (...) {
            std::free(new_begin);
            throw;
        }

        if (!is_small()) std::free(m_begin);

        m_end_cap = new_begin + size_hint;
        m_end = new_begin + size();
        m_begin = new_begin;
    }

    // Shift using std::memmove if T is trivially relocatable.
    template <typename U = T>
    typename std::enable_if<meta::is_trivially_relocatable<U>::value>::type
    shift_data(const_iterator first, const_iterator last,
               iterator dest) noexcept {
        std::memmove(dest, first, sizeof(value_type) * (last - first));
    }

    // Shift by calling constructor and destructor as a pair.
    template <typename U = T>
    typename std::enable_if<!meta::is_trivially_relocatable<U>::value>::type
    shift_data(const_iterator first, const_iterator last, iterator dest) {
        if (dest < first) {
            for (auto begin = first; begin != last; ++begin, (void)++dest) {
                ::new (dest) value_type(std::move(*begin));
                begin->~value_type();
            }
        } else {
            for (const_reverse_iterator rbegin(last);
                 rbegin != const_reverse_iterator(first);
                 ++rbegin, (void)++dest) {
                ::new (dest) value_type(std::move(*rbegin));
                rbegin->~value_type();
            }
        }
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
        for (auto begin = first; begin != last; ++begin, (void)++dest) {
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
        for (auto begin = first; begin != last; ++begin, (void)++dest) {
            ::new (dest) value_type(std::move(*begin));
        }

        for (auto begin = first; begin != last; ++begin) {
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
                     meta::is_input_iterator<InputIterator>::value &&
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
        this->reserve(std::distance(first, last));

        for (; first != last; ++first) {
            this->emplace_back(*first);
        }
    }

    small_vector(std::initializer_list<T> ilist) : small_vector() {
        this->reserve(ilist.size());

        for (auto&& element : ilist) {
            this->emplace_back(element);
        }
    }

    small_vector(const small_vector_header<T>& other) : small_vector() {
        if (!other.empty()) small_vector_header<T>::operator=(other);
    }

    small_vector(small_vector_header<T>&& other) : small_vector() {
        if (!other.empty()) small_vector_header<T>::operator=(std::move(other));
    }

    small_vector& operator=(const small_vector_header<T>& other) {
        small_vector_header<T>::operator=(other);
        return *this;
    }

    small_vector& operator=(small_vector_header<T>&& other) {
        small_vector_header<T>::operator=(std::move(other));
        return *this;
    }
};

template <typename T>
void swap(small_vector_header<T>& x, small_vector_header<T>& y) {
    x.swap(y);
}

} // namespace cfds
