#include <cfds/small_vector.hpp>
#include <catch2/catch.hpp>

TEST_CASE("get_buffer_address return correct address", "[utility, address]") {
    SECTION("get_buffer_address with small_vector") {
        cfds::small_vector<int, 4> v{1, 2};
        cfds::small_vector_header<int>* vPtr = &v;

        int* addr = static_cast<int*>(cfds::detail::get_buffer_address(vPtr));

        CHECK(*addr == 1);
        CHECK(*(addr + 1) == 2);
    }
}
