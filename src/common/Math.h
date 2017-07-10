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

#ifndef COMMON_MATH_H_
#define COMMON_MATH_H_

#include <cstddef>
#include <cstdint>

// The following are not valid for 0
uint32_t ScanForward(uint32_t bits);
uint32_t Log2(uint32_t value);
bool IsPowerOfTwo(size_t n);

bool IsAligned(const void* ptr, size_t alignment);
void* AlignVoidPtr(void* ptr, size_t alignment);

template<typename T>
T* Align(T* ptr, size_t alignment) {
    return reinterpret_cast<T*>(AlignVoidPtr(ptr, alignment));
}

template<typename T>
const T* Align(const T* ptr, size_t alignment) {
    return reinterpret_cast<const T*>(AlignVoidPtr(const_cast<T*>(ptr), alignment));
}

#endif // COMMON_MATH_H_
