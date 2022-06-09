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
#include "dawn/native/Blob.h"

namespace dawn::native {

Blob CreateBlob(size_t size) {
    if (size > 0) {
        uint8_t* data = new uint8_t[size];
        return Blob::UnsafeCreateWithDeleter(data, size, [=]() { delete[] data; });
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
}

Blob& Blob::operator=(Blob&& rhs) {
    mData = rhs.mData;
    mSize = rhs.mSize;
    if (mDeleter) {
        mDeleter();
    }
    mDeleter = std::move(rhs.mDeleter);
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

}  // namespace dawn::native
