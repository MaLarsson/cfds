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
