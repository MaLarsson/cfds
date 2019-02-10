// Contains the definitions and declarations of SmallVector<T, N> and
// SmallVectorImpl<T> which behaves in a similar fashion to std::vector except
// for the fact that the first N elements are stored on the stack and not
// requiring any heap allocations.
//
// An instance of SmallVectorImpl<T> cant be instantiated but it can be used to
// type erase the inline size template paramater N from SmallVector<T, N>.
// i.e. void f(SmallVectorImpl<T>& v) can take any SmallVector<T, N> as long as
// the T template parameter matches.

#pragma once

#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace meta {

template <bool B>
using BoolConstant = std::integral_constant<bool, B>;

template <std::size_t N>
struct PriorityTag : PriorityTag<N - 1> {};

template <>
struct PriorityTag<0> {};

namespace detail {

template <typename T>
auto IsTriviallyRelocatableImpl(PriorityTag<1>)
    -> BoolConstant<T::IsTriviallyRelocatable::value>;

template <typename T>
auto IsTriviallyRelocatableImpl(PriorityTag<0>)
    -> BoolConstant<std::is_trivially_move_constructible<T>::value &&
                    std::is_trivially_destructible<T>::value>{};

} // namespace detail

template <typename T>
struct IsTriviallyRelocatable
    : decltype(detail::IsTriviallyRelocatableImpl<T>(PriorityTag<1>{})) {};

template <typename T>
struct IsTriviallyRelocatable<std::unique_ptr<T>> : std::true_type {};

template <typename T>
struct IsTriviallyRelocatable<std::shared_ptr<T>> : std::true_type {};

template <typename T>
struct IsTriviallyRelocatable<std::weak_ptr<T>> : std::true_type {};

template <typename Iterator>
struct IsForwardIterator
    : std::is_base_of<
          std::forward_iterator_tag,
          typename std::iterator_traits<Iterator>::iterator_category> {};

} // namespace meta

template <typename T>
class SmallVectorImpl;

namespace detail {

template <typename T>
struct SmallVectorAlignment {
    SmallVectorImpl<T> impl;
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer;
};

} // namespace detail

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
        if (!isSmall()) free(first_);
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
        if (head_ == last_) resize((last_ - first_) * 2);
        ::new (head_++) value_type(std::forward<Args>(args)...);
        return *(head_ - 1);
    }

    void append(const value_type& value) { emplaceBack(value); }
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
        if (size > (last_ - first_)) resize(size);
    }

    void shrinkToFit() {
        if (head_ != last_) resize(last_ - first_);
    }

    int size() const { return head_ - first_; }
    int capacity() const { return last_ - first_; }
    bool isEmpty() const { return first_ == head_; }

    void clear() {
        destroyRange(first_, head_);
        head_ = first_;
    }

 protected:
    SmallVectorImpl(pointer first, pointer last)
        : first_(first), last_(last), head_(first) {}

    SmallVectorImpl() = delete;
    SmallVectorImpl(const SmallVectorImpl&) = delete;
    SmallVectorImpl(SmallVectorImpl&&) = delete;
    SmallVectorImpl& operator=(const SmallVectorImpl&) = delete;
    SmallVectorImpl& operator=(SmallVectorImpl&&) = delete;

    static void destroyRange(pointer begin, pointer end) {
        for (; begin != end; ++begin) {
            begin->~value_type();
        }
    }

 private:
    pointer first_ = nullptr;
    pointer last_ = nullptr;
    pointer head_ = nullptr;

    // Returns a pointer to the first element of the inline buffer by
    // calculating the offset from the this pointer and the buffer member.
    void* getFirstSmallElement() const {
        std::size_t offset = reinterpret_cast<std::size_t>(
            &(reinterpret_cast<detail::SmallVectorAlignment<T>*>(0)->buffer));

        return const_cast<void*>(reinterpret_cast<const void*>(
            reinterpret_cast<const char*>(this) + offset));
    }

    // Returns whether the inlined buffer is currently in use to store the data.
    bool isSmall() const { return first_ == getFirstSmallElement(); }

    void resize(int newSize) {
        void* dest = malloc(sizeof(value_type) * newSize);
        pointer newFirst = static_cast<pointer>(dest);
        uninitializedRelocate(newFirst);

        if (!isSmall()) free(first_);

        last_ = newFirst + newSize;
        head_ = newFirst + (head_ - first_);
        first_ = newFirst;
    }

    // Relocate using std::memcpy if T is trivially relocatable.
    template <typename U = T, typename ForwardIterator>
    typename std::enable_if<meta::IsTriviallyRelocatable<U>::value>::type
    uninitializedRelocate(ForwardIterator dest) noexcept {
        std::memcpy(dest, first_, sizeof(value_type) * (head_ - first_));
    }

    // Relocate by calling constructor and destructor as a pair since there is
    // no risk of the constructor throwing.
    template <typename U = T, typename ForwardIterator>
    typename std::enable_if<!meta::IsTriviallyRelocatable<U>::value &&
                            std::is_nothrow_move_constructible<U>::value>::type
    uninitializedRelocate(ForwardIterator dest) noexcept {
        for (auto begin = first_; begin != last_; ++begin, ++dest) {
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
            for (auto begin = first_; begin != last_; ++begin, ++dest) {
                ::new (dest) value_type(std::move(*begin));
            }
        } catch (...) {
            free(dest);
            throw;
        }

        for (auto begin = first_; begin != last_; ++begin) {
            begin->~value_type();
        }
    }
};

template <typename T, int N = 4>
class SmallVector : public SmallVectorImpl<T> {
    static_assert(N > 0, "SmallVector requires capacity greater than 0.");

 public:
    SmallVector() noexcept
        : SmallVectorImpl<T>(reinterpret_cast<T*>(buffer),
                             reinterpret_cast<T*>(buffer) + N) {}

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
        this->reserve(other.size());
        for (auto& element : other) {
            this->emplaceBack(element);
        }
    }

    SmallVector(SmallVector&& other) {
        (void)other;
        // TODO
    }

 private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer[N];
};
