// Copyright 2020 The Dawn Authors
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

#ifndef COMMON_ALLOC_H_
#define COMMON_ALLOC_H_

#include <cstddef>
#include <new>

template <typename T>
T* AllocNoThrow(size_t count) {
#if defined(ADDRESS_SANITIZER)
    if (count * sizeof(T) >= 0x70000000) {
        // std::nothrow isn't implemented on ASAN and it has a 2GB allocation limit.
        // Catch large allocations and error out so fuzzers make progress.
        return nullptr;
    }
#endif
    return new (std::nothrow) T[count];
}

#endif  // COMMON_ALLOC_H_
