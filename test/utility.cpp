#include <SmallDenseSet.hpp>
#include <SmallVector.hpp>
#include <catch2/catch.hpp>

TEST_CASE("getBufferAddres return correct address", "[utility, address]") {
    SECTION("getBufferAddress with SmallVector") {
        cfds::SmallVector<int, 4> v{1, 2};
        cfds::SmallVectorImpl<int>* vPtr = &v;

        int* address = static_cast<int*>(cfds::detail::getBufferAddress(vPtr));

        CHECK(*address == 1);
        CHECK(*(address + 1) == 2);
    }

    SECTION("getBufferAddress With SmallDenseSet") {}
}
