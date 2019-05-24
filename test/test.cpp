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

TEST_CASE("Construct through iterators", "[small_vector]") {
    cfds::small_vector<int, 8> from{1, 2, 3, 4, 5, 6, 7, 8};
    cfds::small_vector<int, 4> to(std::begin(from), std::end(from));

    CHECK(to.at(0) == 1);
    CHECK(to.at(1) == 2);
    CHECK(to.at(2) == 3);
    CHECK(to.at(3) == 4);
    CHECK(to.at(4) == 5);
    CHECK(to.at(5) == 6);
    CHECK(to.at(6) == 7);
    CHECK(to.at(7) == 8);
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

    cfds::small_vector<int, 0> v4;
    v4 = v3;

    CHECK(v4.back() == v3.back());

    cfds::small_vector<int, 0> v5;
    v5 = std::move(v3);

    CHECK(v5.back() == v4.back());
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

TEST_CASE("Modify small_vector through small_vector_header&", "[small_vector]") {
    cfds::small_vector<int, 4> v{1};

    auto fn = [](cfds::small_vector_header<int>& ref, int number) {
        ref.emplace_back(number);
    };

    CHECK(v.back() == 1);

    fn(v, 2);

    CHECK(v.front() == 1);
    CHECK(v.at(0) == 1);
    CHECK(v.back() == 2);
}

TEST_CASE("Get first element with front", "[small_vector]") {
    cfds::small_vector<int, 0> v{1, 2, 3, 4};

    CHECK(v.front() == 1);
}

TEST_CASE("Modify through front and back", "[small_vector]") {
    cfds::small_vector<int, 0> v{1, 2, 3, 4};

    CHECK(v.front() == 1);
    CHECK(v.back() == 4);

    v.front() = 10;
    v.back() = 14;

    CHECK(v.front() == 10);
    CHECK(v.back() == 14);

    CHECK(v.at(0) == 10);
    CHECK(v.at(1) == 2);
    CHECK(v.at(2) == 3);
    CHECK(v.at(3) == 14);
}

TEST_CASE("Access elements through data", "[small_vector]") {
    cfds::small_vector<int> v{1, 2, 3, 4};
    int* data = v.data();

    CHECK(data[0] == 1);
    CHECK(data[1] == 2);
    CHECK(data[2] == 3);
    CHECK(data[3] == 4);
}

TEST_CASE("Emplace in middle of small_vector", "[small_vector]") {
    cfds::small_vector<double> v{1.0, 2.0, 3.0};

    auto iter = v.emplace(std::begin(v) + 1, 1.5);

    CHECK(*iter == 1.5);
    CHECK(iter == std::begin(v) + 1);

    CHECK(v[0] == 1.0);
    CHECK(v[1] == 1.5);
    CHECK(v[2] == 2.0);
    CHECK(v[3] == 3.0);

    auto iter2 = v.emplace(std::begin(v) + 3, 2.5);

    CHECK(*iter2 == 2.5);
    CHECK(iter2 == std::begin(v) + 3);

    CHECK(v[0] == 1.0);
    CHECK(v[1] == 1.5);
    CHECK(v[2] == 2.0);
    CHECK(v[3] == 2.5);
    CHECK(v[4] == 3.0);
}

TEST_CASE("Emplace with non-trivial type", "[small_vector]") {
    cfds::small_vector<std::string> v{"aa", "bb"};

    auto iter = v.emplace(std::begin(v) + 1, "ab");

    CHECK(*iter == "ab");
    CHECK(iter == std::begin(v) + 1);

    CHECK(v[0] == "aa");
    CHECK(v[1] == "ab");
    CHECK(v[2] == "bb");
}
