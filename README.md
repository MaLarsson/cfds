# Cache Friendly Data Structures [![Build Status](https://travis-ci.org/MaLarsson/cfds.svg?branch=master)](https://travis-ci.org/MaLarsson/cfds)
Efficient and cache friendly data structures.

## Overview
```cpp
#include <SmallVector.hpp>
#include <memory>

int main() {
    // Create vector of std::unique_ptr of ints with stack buffer of 4 elements.
    cfds::SmallVector<std::unique_ptr<int>, 4> v{1, 2, 3, 4};
    
    // Allocates space for 8 elements and relocates elements with std::memcpy.
    // meta::IsTriviallyRelocatable<T>::value will be used to determine if
    // std::memcpy or construct/destroy will be used for the relocation.
    v.append(std::make_unique<int>(5));
}
```
