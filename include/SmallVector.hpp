// Contains the definitions and declarations of SmallVector<T, N> and
// SmallVectorImpl<T> which behaves in a similar fashion to std::vector except
// for the fact that the first N elements are stored on the stack to avoid early
// heap allocations.
//
// An instance of SmallVectorImpl<T> cant be instantiated but it can be used to
// type erase the inline size template paramater N from SmallVector<T, N>.
// i.e. void f(SmallVectorImpl<T>& v) can take any SmallVector<T, N> as long as
// the T template parameter matches.

#pragma once

#include "Meta.hpp"
#include "Utility.hpp"

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
class SmallVectorImpl {
    using rvalue_reference = T&&;

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

    ~SmallVectorImpl() {
        if (!isSmall()) std::free(first_);
    }

    iterator begin() { return first_; }
    const_iterator begin() const { return first_; }
    const_iterator cbegin() const { return first_; }

    iterator end() { return head_; }
    const_iterator end() const { return head_; }
    const_iterator cend() const { return head_; }

    reverse_iterator rbegin() { return reverse_iterator(head_); }
    const_reverse_iterator rbegin() const { return reverse_iterator(head_); }
    const_reverse_iterator crbegin() const { return reverse_iterator(head_); }

    reverse_iterator rend() { return reverse_iterator(first_); }
    const_reverse_iterator rend() const { return reverse_iterator(first_); }
    const_reverse_iterator crend() const { return reverse_iterator(first_); }

    template <typename... Args>
    value_type& emplaceBack(Args&&... args) {
        if (head_ == last_) resize(capacity() + 1);
        ::new (head_++) value_type(std::forward<Args>(args)...);
        return *(head_ - 1);
    }

    void append(const value_type& value) {
        appendImpl(value, std::is_trivially_copyable<T>::type);
    }

    void append(value_type&& value) { emplaceBack(std::move(value)); }

    value_type& back() { return *(head_ - 1); }
    const value_type& back() const { return *(head_ - 1); }

    value_type& operator[](int index) { return *(first_ + index); }
    const value_type& operator[](int index) const { return *(first_ + index); }

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

    pointer data() noexcept { return first_; }
    const_pointer data() const noexcept { return first_; }

    int size() const { return head_ - first_; }
    int capacity() const { return last_ - first_; }
    bool isEmpty() const { return first_ == head_; }

    void clear() {
        destroyRange(first_, head_);
        head_ = first_;
    }

    // Returns whether the inlined buffer is currently in use to store the data.
    bool isSmall() const { return first_ == detail::getBufferAddress(this); }

    SmallVectorImpl& operator=(const SmallVectorImpl& other) {
        if (this == &other) return *this;

        if (size() >= other.size()) {
            if (other.size() > 0) {
                pointer newEnd = std::copy(other.begin(), other.end(), begin());
                destroyRange(newEnd, end());
                head_ = newEnd;
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

        head_ = first_ + other.size();

        return *this;
    }

    SmallVectorImpl& operator=(SmallVectorImpl&& other) {
        if (this == &other) return *this;

        if (!other.isSmall()) {
            destroyRange(first_, head_);

            if (!isSmall()) std::free(first_);

            first_ = other.first_;
            head_ = other.head_;
            last_ = other.last_;

            other.first_ = other.head_ = other.last_ =
                static_cast<pointer>(detail::getBufferAddress(&other));

            return *this;
        }

        if (size() >= other.size()) {
            if (other.size() > 0) {
                pointer newEnd = std::move(other.begin(), other.end(), begin());
                destroyRange(newEnd, end());
                head_ = newEnd;
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

        uninitializedMove(other.begin() + size(), other.end(),
                          begin() + size());

        head_ = first_ + other.size();
        other.clear();

        return *this;
    }

 protected:
    SmallVectorImpl(int n) noexcept
        : first_(reinterpret_cast<pointer>(detail::getBufferAddress(this))),
          last_(first_ + n), head_(first_) {}

    SmallVectorImpl() = delete;
    SmallVectorImpl(const SmallVectorImpl&) = delete;
    SmallVectorImpl(SmallVectorImpl&&) = delete;

    // Turn destroyRange into a noop when T is trivially copyable.
    template <typename U = T>
    static typename std::enable_if<std::is_trivially_copyable<U>::value>::type
    destroyRange(pointer, pointer) {}

    template <typename U = T>
    static typename std::enable_if<!std::is_trivially_copyable<U>::value>::type
    destroyRange(pointer begin, pointer end) {
        for (; begin != end; ++begin) {
            begin->~value_type();
        }
    }

 private:
    pointer first_ = nullptr;
    pointer last_ = nullptr;
    pointer head_ = nullptr;

    template <typename InputIterator, typename ForwardIterator>
    static void uninitializedMove(InputIterator first, InputIterator last,
                                  ForwardIterator dest) {
        std::uninitialized_copy(std::make_move_iterator(first),
                                std::make_move_iterator(last), dest);
    }

    // Use memcpy instread of placement new when T is trivially copyable.
    void appendImpl(const value_type& value, std::true_type) {
        if (head_ == last_) resize(capacity() + 1);
        std::memcpy(head_++, std::addressof(value), sizeof(value_type));
    }

    void appendImpl(const value_type& value, std::false_type) {
        emplaceBack(value);
    }

    void resize(int sizeHint) {
        int powerOfTwo = static_cast<int>(detail::nextPowerOfTwo(capacity()));
        int newSize = std::max(sizeHint, powerOfTwo);

        void* dest = detail::safeMalloc(sizeof(value_type) * newSize);
        pointer newFirst = static_cast<pointer>(dest);

        uninitializedRelocate(newFirst);

        if (!isSmall()) std::free(first_);

        last_ = newFirst + newSize;
        head_ = newFirst + size();
        first_ = newFirst;
    }

    // Relocate using std::memcpy if T is trivially relocatable.
    template <typename U = T, typename ForwardIterator>
    typename std::enable_if<meta::IsTriviallyRelocatable<U>::value>::type
    uninitializedRelocate(ForwardIterator dest) noexcept {
        std::memcpy(dest, first_, sizeof(value_type) * size());
    }

    // Relocate by calling constructor and destructor as a pair since there is
    // no risk of the constructor throwing.
    template <typename U = T, typename ForwardIterator>
    typename std::enable_if<!meta::IsTriviallyRelocatable<U>::value &&
                            std::is_nothrow_move_constructible<U>::value>::type
    uninitializedRelocate(ForwardIterator dest) noexcept {
        for (auto begin = first_; begin != head_; ++begin, ++dest) {
            ::new (dest) value_type(std::move(*begin));
            begin->~value_type();
        }
    }

    // Relocate by calling the constructor and only after all elements have been
    // relocated can the destructor for the old shells be called since the
    // constructor might throw.
    template <typename U = T, typename ForwardIterator>
    typename std::enable_if<!meta::IsTriviallyRelocatable<U>::value &&
                            !std::is_nothrow_move_constructible<U>::value>::type
    uninitializedRelocate(ForwardIterator dest) {
        try {
            for (auto begin = first_; begin != head_; ++begin, ++dest) {
                ::new (dest) value_type(std::move(*begin));
            }
        } catch (...) {
            std::free(dest);
            throw;
        }

        for (auto begin = first_; begin != head_; ++begin) {
            begin->~value_type();
        }
    }
};

template <typename T, int N = 4>
class SmallVector : public SmallVectorImpl<T>,
                    private detail::AlignedStorageBase<T, N> {
    static_assert(N >= 0,
                  "SmallVector<T, N> requires N to be greater or equal to 0.");

 public:
    SmallVector() noexcept : SmallVectorImpl<T>(N) {}

    ~SmallVector() { this->destroyRange(this->begin(), this->end()); }

    template <typename InputIterator>
    SmallVector(
        typename std::enable_if<!meta::IsForwardIterator<InputIterator>::value,
                                InputIterator>::type first,
        InputIterator last)
        : SmallVector() {
        for (; first != last; ++first) {
            this->emplaceBack(*first);
        }
    }

    template <typename ForwardIterator>
    SmallVector(
        typename std::enable_if<meta::IsForwardIterator<ForwardIterator>::value,
                                ForwardIterator>::type first,
        ForwardIterator last)
        : SmallVector() {
        this->reserve(last - first);
        for (; first != last; ++first) {
            this->emplaceBack(*first);
        }
    }

    SmallVector(std::initializer_list<T> init) : SmallVector() {
        this->reserve(init.size());
        for (auto& element : init) {
            this->emplaceBack(element);
        }
    }

    SmallVector(const SmallVector& other) : SmallVector() {
        if (!other.isEmpty()) SmallVectorImpl<T>::operator=(other);
    }

    SmallVector(SmallVector&& other) : SmallVector() {
        if (!other.isEmpty()) SmallVectorImpl<T>::operator=(std::move(other));
    }

    SmallVector& operator=(const SmallVector& other) {
        SmallVectorImpl<T>::operator=(other);
        return *this;
    }

    SmallVector& operator=(SmallVector&& other) {
        SmallVectorImpl<T>::operator=(std::move(other));
        return *this;
    }
};

} // namespace cfds
