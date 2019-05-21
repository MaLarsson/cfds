#include <catch2/catch.hpp>
#include <cfds/small_vector.hpp>
#include <type_traits>

TEST_CASE("Construction of small_vector_header",
          "[small_vector_header, construction]") {
    using T = cfds::small_vector_header<int>;

    CHECK_FALSE(std::is_default_constructible<T>::value);
    CHECK_FALSE(std::is_copy_constructible<T>::value);
    CHECK_FALSE(std::is_move_constructible<T>::value);
}

TEST_CASE("small_vector assignment", "[small_vector, assignment]") {
    cfds::small_vector<int, 4> v1{1, 2, 3, 4};
    cfds::small_vector<int, 4> v2{1, 1, 1, 1};
    v2 = v1;

    CHECK(v1.back() == 4);
    CHECK(v1.back() == v2.back());

    cfds::small_vector<int, 4> v3;
    v3 = std::move(v1);

    CHECK(v3.back() == v2.back());
}

TEST_CASE("Check if small_vector is small", "[small_vector]") {
    cfds::small_vector<int, 4> v1{1, 2, 3, 4};
    cfds::small_vector<int, 4> v2{1, 2, 3, 4, 5};
    cfds::small_vector<int, 0> v3{};

    CHECK(v1.is_small());
    CHECK_FALSE(v2.is_small());
    CHECK(v3.is_small());

    v3.emplace_back(1);

    CHECK_FALSE(v3.is_small());
}
