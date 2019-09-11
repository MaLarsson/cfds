#include <algorithm>
#include <catch2/catch.hpp>
#include <cfds/small_vector.hpp>
#include <iterator>
#include <memory>
#include <sstream>
#include <type_traits>

TEST_CASE("Construction of small_vector_header",
          "[small_vector_header, construction]") {
    using T = cfds::small_vector_header<int>;

    CHECK_FALSE(std::is_default_constructible<T>::value);
    CHECK_FALSE(std::is_copy_constructible<T>::value);
    CHECK_FALSE(std::is_move_constructible<T>::value);
}

TEST_CASE("Construct through iterators", "[small_vector, constructor]") {
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

TEST_CASE("Assign small_vector with assign", "[small_vector, assign]") {
    cfds::small_vector<int, 4> v{1, 2, 3};

    SECTION("Assign smaller initializer_list") {
        v.assign({4, 5});

        CHECK(v.size() == 2);
        CHECK(v[0] == 4);
        CHECK(v[1] == 5);
    }

    SECTION("Assign bigger initializer_list") {
        v.assign({4, 5, 6, 7, 8, 9});

        CHECK(v.size() == 6);
        CHECK(v[0] == 4);
        CHECK(v[1] == 5);
        CHECK(v[2] == 6);
        CHECK(v[3] == 7);
        CHECK(v[4] == 8);
        CHECK(v[5] == 9);
    }

    SECTION("Assign smaller iterator pair") {
        cfds::small_vector<int, 4> from{4, 5};
        v.assign(from.begin(), from.end());

        CHECK(v.size() == 2);
        CHECK(v[0] == 4);
        CHECK(v[1] == 5);
    }

    SECTION("Assign bigger iterator pair") {
        cfds::small_vector<int, 6> from{4, 5, 6, 7, 8, 9};
        v.assign(from.begin(), from.end());

        CHECK(v.size() == 6);
        CHECK(v[0] == 4);
        CHECK(v[1] == 5);
        CHECK(v[2] == 6);
        CHECK(v[3] == 7);
        CHECK(v[4] == 8);
        CHECK(v[5] == 9);
    }
}

TEST_CASE("push_back value into vector", "[small_vector, push_back]") {
    cfds::small_vector<int, 2> v{1, 2};

    SECTION("push_back pr-value") {
        v.push_back(3);

        CHECK(v.size() == 3);
        CHECK(v[2] == 3);
    }

    SECTION("push_back x-value") {
        int i = 3;
        v.push_back(std::move(i));

        CHECK(v.size() == 3);
        CHECK(v[2] == 3);
    }

    SECTION("push_back l-value") {
        int i = 3;
        v.push_back(i);

        CHECK(v.size() == 3);
        CHECK(v[2] == 3);
    }
}

TEST_CASE("Check if small_vector is small", "[small_vector, is_small]") {
    cfds::small_vector<int, 4> v1{1, 2, 3, 4};
    cfds::small_vector<int, 4> v2{1, 2, 3, 4, 5};
    cfds::small_vector<int, 0> v3{};

    CHECK(v1.is_small());
    CHECK_FALSE(v2.is_small());
    CHECK(v3.is_small());

    v3.emplace_back(1);

    CHECK_FALSE(v3.is_small());
}

TEST_CASE("Modify small_vector through small_vector_header&",
          "[small_vector]") {
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

TEST_CASE("Get first element with front", "[small_vector, front]") {
    cfds::small_vector<int, 0> v{1, 2, 3, 4};

    CHECK(v.front() == 1);
}

TEST_CASE("Modify through front and back", "[small_vector, front, back]") {
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

TEST_CASE("Access elements through data", "[small_vector, data]") {
    cfds::small_vector<int> v{1, 2, 3, 4};
    int* data = v.data();

    CHECK(data[0] == 1);
    CHECK(data[1] == 2);
    CHECK(data[2] == 3);
    CHECK(data[3] == 4);
}

TEST_CASE("Emplace in middle of small_vector", "[small_vector, emplace]") {
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

TEST_CASE("Emplace overflow", "[small_vector, emplace]") {
    cfds::small_vector<double> v{1.0, 2.0, 3.0, 4.0};

    CHECK(v.size() == 4);
    CHECK(v[0] == 1.0);
    CHECK(v[1] == 2.0);
    CHECK(v[2] == 3.0);
    CHECK(v[3] == 4.0);

    v.emplace(std::begin(v) + 1, 1.5);

    CHECK(v.size() == 5);
    CHECK(v[0] == 1.0);
    CHECK(v[1] == 1.5);
    CHECK(v[2] == 2.0);
    CHECK(v[3] == 3.0);
    CHECK(v[4] == 4.0);
}

TEST_CASE("Emplace with non-trivial type", "[small_vector, emplace]") {
    cfds::small_vector<std::string> v{"aa", "bb"};

    CHECK(v.size() == 2);

    auto iter = v.emplace(std::begin(v) + 1, "ab");

    CHECK(v.size() == 3);

    CHECK(*iter == "ab");
    CHECK(iter == std::begin(v) + 1);

    CHECK(v[0] == "aa");
    CHECK(v[1] == "ab");
    CHECK(v[2] == "bb");
}

TEST_CASE("Erase with same iterator", "[small_vector, erase]") {
    cfds::small_vector<std::string> v{"aa", "bb", "cc", "dd"};

    CHECK(v.size() == 4);

    auto iter = v.erase(std::begin(v), std::begin(v));

    CHECK(iter == std::begin(v));
    CHECK(v.size() == 4);
    CHECK(v[0] == "aa");
    CHECK(v[1] == "bb");
    CHECK(v[2] == "cc");
    CHECK(v[3] == "dd");
}

TEST_CASE("Erase elements for small_vector", "[small_vector, erase]") {
    cfds::small_vector<std::string> v{"aa", "bb", "cc", "dd"};

    CHECK(v.size() == 4);

    auto iter = v.erase(std::find(std::begin(v), std::end(v), "cc"));

    CHECK(*iter == "dd");
    CHECK(v.size() == 3);
    CHECK(v[0] == "aa");
    CHECK(v[1] == "bb");
    CHECK(v[2] == "dd");
}

TEST_CASE("Remove last element with pop_back", "[small_vector, pop_back]") {
    std::shared_ptr<int> shared = std::make_shared<int>(1);
    cfds::small_vector<std::shared_ptr<int>> shared_ptr_v{shared, shared};
    cfds::small_vector<std::string> string_v{"aa", "bb"};
    cfds::small_vector<int> int_v{1, 2};

    CHECK(shared.use_count() == 3);
    CHECK(shared_ptr_v.size() == 2);
    CHECK(string_v.size() == 2);
    CHECK(int_v.size() == 2);

    shared_ptr_v.pop_back();
    string_v.pop_back();
    int_v.pop_back();

    CHECK(shared.use_count() == 2);
    CHECK(shared_ptr_v.size() == 1);
    CHECK(string_v.size() == 1);
    CHECK(int_v.size() == 1);
}

TEST_CASE("Insert elements into small_vector", "[small_vector, insert]") {
    cfds::small_vector<int> full_v{1, 2, 3, 4};
    cfds::small_vector<int> v{4};

    CHECK(full_v.size() == 4);
    CHECK(full_v[0] == 1);
    CHECK(full_v[1] == 2);
    CHECK(full_v[2] == 3);
    CHECK(full_v[3] == 4);

    SECTION("Insert lvalue into full small_vector") {
        int i = 0;
        full_v.insert(std::begin(full_v), i);

        CHECK(full_v.size() == 5);
        CHECK(full_v[0] == 0);
        CHECK(full_v[1] == 1);
        CHECK(full_v[2] == 2);
        CHECK(full_v[3] == 3);
        CHECK(full_v[4] == 4);
    }

    SECTION("Insert rvalue into full small_vector") {
        full_v.insert(std::begin(full_v), 0);

        CHECK(full_v.size() == 5);
        CHECK(full_v[0] == 0);
        CHECK(full_v[1] == 1);
        CHECK(full_v[2] == 2);
        CHECK(full_v[3] == 3);
        CHECK(full_v[4] == 4);
    }
}

TEST_CASE("Insert elements with iterators", "[small_vector, insert]") {
    cfds::small_vector<int> v{1, 5};
    cfds::small_vector<int> from{2, 3, 4};

    v.insert(std::begin(v) + 1, std::begin(from), std::end(from));

    CHECK(v.size() == 5);
    CHECK(v[0] == 1);
    CHECK(v[1] == 2);
    CHECK(v[2] == 3);
    CHECK(v[3] == 4);
    CHECK(v[4] == 5);
}

TEST_CASE("Insert elements with input iterators", "[small_vector, insert]") {
    cfds::small_vector<char> v{'a', 'd'};
    std::istringstream from("bc");

    v.insert(std::begin(v) + 1, std::istreambuf_iterator<char>(from),
             std::istreambuf_iterator<char>());

    CHECK(v.size() == 4);
    CHECK(v[0] == 'a');
    CHECK(v[1] == 'b');
    CHECK(v[2] == 'c');
    CHECK(v[3] == 'd');
}

TEST_CASE("Swap vectors", "[small_vector, swap]") {
    cfds::small_vector<int, 8> v1{1, 2, 3, 4};
    cfds::small_vector<int, 4> v2{5, 6};

    swap(v1, v2);

    CHECK(v1.size() == 2);
    CHECK(v2.size() == 4);

    CHECK(v1[0] == 5);
    CHECK(v1[1] == 6);

    CHECK(v2[0] == 1);
    CHECK(v2[1] == 2);
    CHECK(v2[2] == 3);
    CHECK(v2[3] == 4);
}

TEST_CASE("Resize vector", "[small_vector, resize]") {
    cfds::small_vector<int> v{1, 2, 3};

    SECTION("Resize to smaller than original size") {
        v.resize(2);

        CHECK(v.size() == 2);
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
    }

    SECTION("Resize to bigger than original size") {
        v.resize(4);

        CHECK(v.size() == 4);
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 3);
        CHECK(v[3] == 0);
    }

    SECTION("Resize to bigger than original size with value") {
        v.resize(5, 10);

        CHECK(v.size() == 5);
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 3);
        CHECK(v[3] == 10);
        CHECK(v[4] == 10);
    }

    SECTION("Resize to same as original size") {
        v.resize(3);

        CHECK(v.size() == 3);
        CHECK(v[0] == 1);
        CHECK(v[1] == 2);
        CHECK(v[2] == 3);
    }
}

TEST_CASE("Shrink small_vector for only fit the data",
          "[small_vector, shrink_to_fit]") {
    cfds::small_vector<int, 0> v{1};
    v.reserve(8);

    CHECK(v.capacity() == 8);

    v.shrink_to_fit();

    CHECK(v.capacity() == 1);

    v.clear();
    v.shrink_to_fit();

    CHECK(v.capacity() == 0);
}

TEST_CASE("Compare small_vector", "[small_vector, comparison]") {
    cfds::small_vector<int> v1{1, 2, 3};
    cfds::small_vector<int, 2> v2{4, 5};

    CHECK(v1 < v2);
    CHECK(v1 <= v2);
    CHECK_FALSE(v2 < v1);
    CHECK_FALSE(v2 <= v1);

    CHECK(v1 != v2);
    CHECK_FALSE(v1 == v2);
    CHECK(v1 == v1);
    CHECK(v2 == v2);

    CHECK(v2 > v1);
    CHECK(v2 >= v1);
    CHECK_FALSE(v1 > v2);
    CHECK_FALSE(v1 >= v2);

    CHECK(v1 >= v1);
    CHECK(v1 <= v1);
    CHECK(v2 >= v2);
    CHECK(v2 <= v2);
}
