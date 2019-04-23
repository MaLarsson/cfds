#include <catch2/catch.hpp>
#include <cfds/small_dense_set.hpp>
#include <type_traits>

namespace test {

struct empty_struct {};

} // namespace test

namespace cfds {

template <>
struct dense_set_traits<test::empty_struct> {
    using value_type = test::empty_struct;
    static value_type get_empty() { return value_type{}; }
    static bool compare(const value_type&, const value_type&) { return true; }
};

} // namespace cfds

TEST_CASE("HasFunctionX meta function", "[meta]") {
    using T = cfds::dense_set_traits<test::empty_struct>;

    CHECK(meta::has_get_empty<T>::value);
    CHECK_FALSE(meta::has_get_tombstone<T>::value);
    CHECK_FALSE(meta::has_get_hash<T>::value);
    CHECK(meta::has_compare<T>::value);

    CHECK(std::is_same<meta::has_get_empty<T>::type, std::true_type>::value);
    CHECK(std::is_same<meta::has_get_tombstone<T>::type, std::false_type>::value);
    CHECK(std::is_same<meta::has_get_hash<T>::type, std::false_type>::value);
    CHECK(std::is_same<meta::has_compare<T>::type, std::true_type>::value);
}
