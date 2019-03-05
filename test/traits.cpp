#include <SmallDenseSet.hpp>
#include <catch2/catch.hpp>

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
    using Traits = cfds::DenseSetTraits<test::EmptyStruct>;

    CHECK(meta::HasGetEmpty<Traits>::value);
    CHECK_FALSE(meta::HasGetTombstone<Traits>::value);
    CHECK_FALSE(meta::HasGetHash<Traits>::value);
    CHECK(meta::HasCompare<Traits>::value);
}
