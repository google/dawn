// Copyright 2021 The Dawn Authors
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

#ifndef SRC_DAWN_WIRE_BUFFERCONSUMER_IMPL_H_
#define SRC_DAWN_WIRE_BUFFERCONSUMER_IMPL_H_

#include <limits>
#include <type_traits>

#include "dawn/wire/BufferConsumer.h"

namespace dawn::wire {

template <typename BufferT>
template <typename T>
WireResult BufferConsumer<BufferT>::Peek(T** data) {
    if (sizeof(T) > mSize) {
        return WireResult::FatalError;
    }

    *data = reinterpret_cast<T*>(mBuffer);
    return WireResult::Success;
}

template <typename BufferT>
template <typename T>
WireResult BufferConsumer<BufferT>::Next(T** data) {
    constexpr size_t kSize = WireAlignSizeof<T>();
    if (kSize > mSize) {
        return WireResult::FatalError;
    }

    *data = reinterpret_cast<T*>(mBuffer);
    mBuffer += kSize;
    mSize -= kSize;
    return WireResult::Success;
}

template <typename BufferT>
template <typename T, typename N>
WireResult BufferConsumer<BufferT>::NextN(N count, T** data) {
    static_assert(std::is_unsigned<N>::value, "|count| argument of NextN must be unsigned.");

    // If size is zero then it indicates an overflow.
    auto size = WireAlignSizeofN<T>(count);
    if (!size || *size > mSize) {
        return WireResult::FatalError;
    }

    *data = reinterpret_cast<T*>(mBuffer);
    mBuffer += *size;
    mSize -= *size;
    return WireResult::Success;
}

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_BUFFERCONSUMER_IMPL_H_
