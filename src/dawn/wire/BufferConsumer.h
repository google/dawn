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

#ifndef SRC_DAWN_WIRE_BUFFERCONSUMER_H_
#define SRC_DAWN_WIRE_BUFFERCONSUMER_H_

#include <cstddef>

#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/wire/WireResult.h"

namespace dawn::wire {

// Wire specific alignment helpers.
template <typename T>
constexpr size_t WireAlignSizeof() {
    return AlignSizeof<T, kWireBufferAlignment>();
}
template <typename T>
std::optional<size_t> WireAlignSizeofN(size_t n) {
    return AlignSizeofN<T, kWireBufferAlignment>(n);
}

// BufferConsumer is a utility class that allows reading bytes from a buffer
// while simultaneously decrementing the amount of remaining space by exactly
// the amount read. It helps prevent bugs where incrementing a pointer and
// decrementing a size value are not kept in sync.
// BufferConsumer also contains bounds checks to prevent reading out-of-bounds.
template <typename BufferT>
class BufferConsumer {
    static_assert(sizeof(BufferT) == 1,
                  "BufferT must be 1-byte, but may have const/volatile qualifiers.");

  public:
    BufferConsumer(BufferT* buffer, size_t size) : mBuffer(buffer), mSize(size) {}

    BufferT* Buffer() const { return mBuffer; }
    size_t AvailableSize() const { return mSize; }

  protected:
    template <typename T, typename N>
    WireResult NextN(N count, T** data);

    template <typename T>
    WireResult Next(T** data);

    template <typename T>
    WireResult Peek(T** data);

  private:
    BufferT* mBuffer;
    size_t mSize;
};

class SerializeBuffer : public BufferConsumer<char> {
  public:
    using BufferConsumer::BufferConsumer;
    using BufferConsumer::Next;
    using BufferConsumer::NextN;
};

class DeserializeBuffer : public BufferConsumer<const volatile char> {
  public:
    using BufferConsumer::BufferConsumer;
    using BufferConsumer::Peek;

    template <typename T, typename N>
    WireResult ReadN(N count, const volatile T** data) {
        return NextN(count, data);
    }

    template <typename T>
    WireResult Read(const volatile T** data) {
        return Next(data);
    }
};

}  // namespace dawn::wire

#endif  // SRC_DAWN_WIRE_BUFFERCONSUMER_H_
