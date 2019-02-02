#include <cstring>
#include <iterator>
#include <type_traits>
#include <utility>

#include <iostream>

template <typename T>
class SmallVectorImpl;

namespace detail {

template <typename T>
struct SmallVectorAlignmentAndSize {
    SmallVectorImpl<T> impl;
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer;
};

}  // namespace detail

template <typename T>
class SmallVectorImpl {
   public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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

    void reserve(int size) {
        if (size > (last_ - first_)) resize(size);
    }

    void shrinkToFit() {
        if (head_ != last_) reserve(last_ - first_);
    }

   protected:
    SmallVectorImpl(pointer first, pointer last)
        : first_(first), last_(last), head_(first) {}

    // Only allow this object to be created from SmallVector<T, N>
    SmallVectorImpl() = delete;
    SmallVectorImpl(const SmallVectorImpl&) = delete;
    SmallVectorImpl(SmallVectorImpl&&) = delete;
    SmallVectorImpl& operator=(const SmallVectorImpl&) = delete;
    SmallVectorImpl& operator=(SmallVectorImpl&&) = delete;

   private:
    pointer first_ = nullptr;
    pointer last_ = nullptr;
    pointer head_ = nullptr;

    void* getFirstSmallElement() const {
        std::size_t offset = reinterpret_cast<std::size_t>(
            &((detail::SmallVectorAlignmentAndSize<T>*)0)->buffer);
        return const_cast<void*>(reinterpret_cast<const void*>(
            reinterpret_cast<const char*>(this) + offset));
    }

    bool isSmall() const { return first_ == getFirstSmallElement(); }

    // TODO: std::memcpy is used for all types but will be undefined behaviour
    // for type that are not trivially relocatable. This can be solved by SFINAE
    // on type trait is_trivially_relocatable. This will not add a runtime
    // overhead but will infer a small compile time increase.
    void resize(int newSize) {
        void* dest = malloc(sizeof(value_type) * newSize);
        pointer newFirst = static_cast<pointer>(dest);
        std::memcpy(newFirst, first_, sizeof(value_type) * (last_ - first_));

        if (!isSmall()) free(first_);

        last_ = newFirst + newSize;
        head_ = newFirst + (head_ - first_);
        first_ = newFirst;
    }
};

template <typename T, int N = 4>
class SmallVector : public SmallVectorImpl<T> {
    static_assert(N > 0, "SmallVector requires capacity greater than 0.");

   public:
    SmallVector()
        : SmallVectorImpl<T>(reinterpret_cast<T*>(buffer),
                             reinterpret_cast<T*>(buffer) + N) {}

   private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type buffer[N];
};
