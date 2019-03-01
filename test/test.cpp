#include <SmallVector.hpp>
#include <catch2/catch.hpp>
#include <type_traits>

TEST_CASE("construction of SmallVectorImpl",
          "[SmallVectorImpl, construction]") {
    using type = cfds::SmallVectorImpl<int>;

    CHECK_FALSE(std::is_default_constructible<type>::value);
    CHECK_FALSE(std::is_copy_constructible<type>::value);
    CHECK_FALSE(std::is_move_constructible<type>::value);
}

TEST_CASE("SmallVector assignment", "[SmallVector, assignment]") {
    cfds::SmallVector<int, 4> v{1, 2, 3, 4};
    cfds::SmallVector<int, 4> v2{1, 1, 1, 1};
    v2 = v;

    CHECK(v.back() == 4);
    CHECK(v.back() == v2.back());

    cfds::SmallVector<int, 4> v3;
    v3 = std::move(v);

    CHECK(v3.back() == v2.back());
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
    cfds::DenseSet<int> set1{1};
    cfds::DenseSet<std::string> set2{"hello world"};
}
