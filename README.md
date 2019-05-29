# Cache Friendly Data Structures [![Build Status](https://travis-ci.org/MaLarsson/cfds.svg?branch=master)](https://travis-ci.org/MaLarsson/cfds)
Efficient and cache friendly data structures.

## Overview
```cpp
#include <cfds/small_vector.hpp>
#include <memory>

int main() {
    // Create vector of std::unique_ptr of int with stack buffer of 4 elements.
    cfds::small_vector<std::unique_ptr<int>, 4> v{1, 2, 3, 4};

    // Allocates space for 8 elements and relocates elements with std::memcpy.
    // cfds::meta::is_trivially_relocatable<T>::value will be used to determine
    // if std::memcpy or construct/destroy will be used for the relocation.
    v.push_back(std::make_unique<int>(5));
}
```

## Building
The library is a header only library so there is nothing to build in order to use the library in your own projects.
All that is needed is to copy the contents of the include folder into your projects include folder.

### Unit Tests
If you want to build and run the unit tests locally a recent version of cmake and a C++11 conforming compiler is needed. The test framework [Catch2](https://github.com/catchorg/Catch2) will be downloaded during the cmake configuration step.

When all the requirment are met the tests can be compiled and run:

``` cmake
git clone https://github.com/MaLarsson/cfds.git
cd cfds && mkdir build && cd build
cmake ..
cmake --build . --target check
```

## Licence
Please see [LICENCE.md](https://github.com/MaLarsson/cfds/blob/master/LICENCE.md).
