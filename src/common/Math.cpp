// Copyright 2017 The NXT Authors
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

#include "common/Math.h"

#include "common/Assert.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <intrin.h>
#endif

uint32_t ScanForward(uint32_t bits) {
    ASSERT(bits != 0);
    #if defined(_WIN32) || defined(_WIN64)
        unsigned long firstBitIndex = 0ul;
        unsigned char ret = _BitScanForward(&firstBitIndex, bits);
        ASSERT(ret != 0);
        return firstBitIndex;
    #else
        return static_cast<unsigned long>(__builtin_ctz(bits));
    #endif
}

uint32_t Log2(uint32_t value) {
    ASSERT(value != 0);
    #if defined(_WIN32) || defined(_WIN64)
        unsigned long firstBitIndex = 0ul;
        unsigned char ret = _BitScanReverse(&firstBitIndex, value);
        ASSERT(ret != 0);
        return firstBitIndex;
    #else
        return 31 - __builtin_clz(value);
    #endif
}

bool IsPowerOfTwo(size_t n) {
    ASSERT(n != 0);
    return (n & (n - 1)) == 0;
}

bool IsAligned(const void* ptr, size_t alignment) {
    ASSERT(IsPowerOfTwo(alignment));
    ASSERT(alignment != 0);
    return (reinterpret_cast<intptr_t>(ptr) & (alignment - 1)) == 0;
}

void* AlignVoidPtr(void* ptr, size_t alignment) {
    ASSERT(alignment != 0);
    return reinterpret_cast<void*>((reinterpret_cast<intptr_t>(ptr) + (alignment - 1)) & ~(alignment - 1));
}
