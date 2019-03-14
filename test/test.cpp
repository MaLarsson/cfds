#include <SmallVector.hpp>
#include <catch2/catch.hpp>
#include <type_traits>

TEST_CASE("construction of SmallVectorImpl",
          "[SmallVectorImpl, construction]") {
    using T = cfds::SmallVectorImpl<int>;

    CHECK_FALSE(std::is_default_constructible<T>::value);
    CHECK_FALSE(std::is_copy_constructible<T>::value);
    CHECK_FALSE(std::is_move_constructible<T>::value);
}

TEST_CASE("SmallVector assignment", "[SmallVector, assignment]") {
    cfds::SmallVector<int, 4> v1{1, 2, 3, 4};
    cfds::SmallVector<int, 4> v2{1, 1, 1, 1};
    v2 = v1;

    CHECK(v1.back() == 4);
    CHECK(v1.back() == v2.back());

    cfds::SmallVector<int, 4> v3;
    v3 = std::move(v1);

    CHECK(v3.back() == v2.back());
}


TEST_CASE("Check if SmallVector is small", "[SmallVector]") {
    cfds::SmallVector<int, 4> v1{1, 2, 3, 4};
    cfds::SmallVector<int, 4> v2{1, 2, 3, 4, 5};
    cfds::SmallVector<int, 0> v3{};

    CHECK(v1.isSmall());
    CHECK_FALSE(v2.isSmall());
    CHECK(v3.isSmall());

    v3.append(1);

    CHECK_FALSE(v3.isSmall());
}


////////////////////////////////////////////////////////////////////////////////
// Testing bitwise operators

#include <functional>
#include <string>
#include <iostream>

TEST_CASE("hash stuff", "") {
    std::string str{"string"};
    std::hash<std::string> string_hasher{};

    std::size_t hash = string_hasher(str);

    int size = 1024;
    std::size_t bitmask = static_cast<std::size_t>(size) - 1;

    std::size_t masked_hash = hash & bitmask;
    int slot = static_cast<int>(masked_hash);

    std::cout << hash << ' ' << slot << '\n';

    CHECK(slot < size);
}

////////////////////////////////////////////////////////////////////////////////
// DenseSet tests

#include <SmallDenseSet.hpp>

TEST_CASE("DenseSet", "") {
    cfds::SmallDenseSet<int, 0> set1{1, 2};
    cfds::SmallDenseSet<std::string, 0> set2{"hello world"};
}
