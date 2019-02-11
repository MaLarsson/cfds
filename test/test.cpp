#include <SmallVector.hpp>
#include <catch2/catch.hpp>

TEST_CASE("do stuff", "[dsa]") {
    SmallVector<int, 4> v1{1, 2, 3, 4};
    SmallVector<int, 0> v2;

    CHECK(v1.at(0) == 1);
    CHECK(v1.back() == 4);
    CHECK(v2.capacity() == 0);
}
