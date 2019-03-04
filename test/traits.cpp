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
};

} // namespace cfds

TEST_CASE("HasFunctionX meta function", "[meta]") {
    using Traits = cfds::DenseSetTraits<test::EmptyStruct>;

    CHECK(meta::HasGetEmpty<Traits>::value);
    CHECK_FALSE(meta::HasGetTombstone<Traits>::value);
    CHECK_FALSE(meta::HasGetHash<Traits>::value);
    CHECK_FALSE(meta::HasCompare<Traits>::value);
}
