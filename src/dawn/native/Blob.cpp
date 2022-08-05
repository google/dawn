// Copyright 2022 The Dawn Authors
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

#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/native/Blob.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {

Blob CreateBlob(size_t size, size_t alignment) {
    ASSERT(IsPowerOfTwo(alignment));
    ASSERT(alignment != 0);
    if (size > 0) {
        // Allocate extra space so that there will be sufficient space for |size| even after
        // the |data| pointer is aligned.
        // TODO(crbug.com/dawn/824): Use aligned_alloc when possible. It should be available
        // with C++17 but on macOS it also requires macOS 10.15 to work.
        size_t allocatedSize = size + alignment - 1;
        uint8_t* data = new uint8_t[allocatedSize];
        uint8_t* ptr = AlignPtr(data, alignment);
        ASSERT(ptr + size <= data + allocatedSize);
        return Blob::UnsafeCreateWithDeleter(ptr, size, [=]() { delete[] data; });
    } else {
        return Blob();
    }
}

// static
Blob Blob::UnsafeCreateWithDeleter(uint8_t* data, size_t size, std::function<void()> deleter) {
    return Blob(data, size, deleter);
}

Blob::Blob() : mData(nullptr), mSize(0), mDeleter({}) {}

Blob::Blob(uint8_t* data, size_t size, std::function<void()> deleter)
    : mData(data), mSize(size), mDeleter(std::move(deleter)) {
    // It is invalid to make a blob that has null data unless its size is also zero.
    ASSERT(data != nullptr || size == 0);
}

Blob::Blob(Blob&& rhs) : mData(rhs.mData), mSize(rhs.mSize) {
    mDeleter = std::move(rhs.mDeleter);
    rhs.mDeleter = nullptr;
}

Blob& Blob::operator=(Blob&& rhs) {
    mData = rhs.mData;
    mSize = rhs.mSize;
    if (mDeleter) {
        mDeleter();
    }
    mDeleter = std::move(rhs.mDeleter);
    rhs.mDeleter = nullptr;
    return *this;
}

Blob::~Blob() {
    if (mDeleter) {
        mDeleter();
    }
}

bool Blob::Empty() const {
    return mSize == 0;
}

const uint8_t* Blob::Data() const {
    return mData;
}

uint8_t* Blob::Data() {
    return mData;
}

size_t Blob::Size() const {
    return mSize;
}

void Blob::AlignTo(size_t alignment) {
    if (IsPtrAligned(mData, alignment)) {
        return;
    }

    Blob blob = CreateBlob(mSize, alignment);
    memcpy(blob.Data(), mData, mSize);
    *this = std::move(blob);
}

template <>
void stream::Stream<Blob>::Write(stream::Sink* s, const Blob& b) {
    size_t size = b.Size();
    StreamIn(s, size);
    if (size > 0) {
        void* ptr = s->GetSpace(size);
        memcpy(ptr, b.Data(), size);
    }
}

template <>
MaybeError stream::Stream<Blob>::Read(stream::Source* s, Blob* b) {
    size_t size;
    DAWN_TRY(StreamOut(s, &size));
    if (size > 0) {
        const void* ptr;
        DAWN_TRY(s->Read(&ptr, size));
        *b = CreateBlob(size);
        memcpy(b->Data(), ptr, size);
    } else {
        *b = Blob();
    }
    return {};
}

}  // namespace dawn::native
