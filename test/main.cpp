#include "SmallVector.hpp"

#include <iostream>
#include <memory>
#include <string>

void addString(SmallVectorImpl<std::unique_ptr<std::string>>& v) {
    v.append(std::make_unique<std::string>("hello"));
}

int main() {
    SmallVector<std::unique_ptr<std::string>, 4> v;
    v.emplaceBack(new std::string("aaa"));
    v.emplaceBack(new std::string("bbb"));
    v.emplaceBack(new std::string("ccc"));
    v.emplaceBack(new std::string("ddd"));
    v.emplaceBack(new std::string("eee"));

    SmallVector<std::unique_ptr<std::string>, 8> v2;

    addString(v2);
    addString(v);

    std::cout << *(v2.back()) << '\n';
    std::cout << *(v.back()) << '\n';
    std::cout << *(v[0]) << '\n';
    std::cout << *(v.at(1)) << '\n';

    for (const auto& ptr : v) {
        std::cout << *ptr << ' ';
    }

    std::cout << '\n';

    SmallVector<int, 4> v3{0, 1, 2, 3};
    SmallVector<int, 8> v4(v3.begin(), v3.end());

    for (int i : v3) {
        std::cout << i << ' ';
    }

    for (int i : v4) {
        std::cout << i << ' ';
    }

    std::cout << '\n';

    return 0;
}
