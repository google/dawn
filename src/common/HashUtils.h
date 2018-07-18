// Copyright 2018 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef COMMON_HASHUTILS_H_
#define COMMON_HASHUTILS_H_

#include "common/Platform.h"

#include <functional>

// Wrapper around std::hash to make it a templated function instead of a functor. It is marginally
// nicer, and avoids adding to the std namespace to add hashing of other types.
template <typename T>
size_t Hash(const T& value) {
    return std::hash<T>()(value);
}

// When hashing sparse structures we want to iteratively build a hash value with only parts of the
// data. HashCombine "hashes" together an existing hash and hashable values.
//
// Example usage to compute the hash of a mask and values corresponding to the mask:
//
//    size_t hash = Hash(mask):
//    for (uint32_t i : IterateBitSet(mask)) { HashCombine(&hash, hashables[i]); }
//    return hash;
template <typename T>
void HashCombine(size_t* hash, const T& value) {
#if defined(DAWN_PLATFORM_64_BIT)
    const size_t offset = 0x9e3779b97f4a7c16;
#elif defined(DAWN_PLATFORM_32_BIT)
    const size_t offset = 0x9e3779b9;
#else
#    error "Unsupported platform"
#endif
    *hash ^= Hash(value) + offset + (*hash << 6) + (*hash >> 2);
}

template <typename T, typename... Args>
void HashCombine(size_t* hash, const T& value, const Args&... args) {
    HashCombine(hash, value);
    HashCombine(hash, args...);
}

#endif  // COMMON_HASHUTILS_H_
