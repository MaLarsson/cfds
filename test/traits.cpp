#include <SmallDenseSet.hpp>
#include <catch2/catch.hpp>
#include <type_traits>

namespace test {

struct EmptyStruct {};

} // namespace test

namespace cfds {

template <>
struct DenseSetTraits<test::EmptyStruct> {
    using value_type = test::EmptyStruct;
    static value_type getEmpty() { return value_type{}; }
    static bool compare(const value_type&, const value_type&) { return true; }
};

} // namespace cfds

TEST_CASE("HasFunctionX meta function", "[meta]") {
    using T = cfds::DenseSetTraits<test::EmptyStruct>;

    CHECK(meta::HasGetEmpty<T>::value);
    CHECK_FALSE(meta::HasGetTombstone<T>::value);
    CHECK_FALSE(meta::HasGetHash<T>::value);
    CHECK(meta::HasCompare<T>::value);

    CHECK(std::is_same<meta::HasGetEmpty<T>::type, std::true_type>::value);
    CHECK(std::is_same<meta::HasGetTombstone<T>::type, std::false_type>::value);
    CHECK(std::is_same<meta::HasGetHash<T>::type, std::false_type>::value);
    CHECK(std::is_same<meta::HasCompare<T>::type, std::true_type>::value);
}
