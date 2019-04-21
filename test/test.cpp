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
