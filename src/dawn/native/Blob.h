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

#ifndef SRC_DAWN_NATIVE_BLOB_H_
#define SRC_DAWN_NATIVE_BLOB_H_

#include <functional>
#include <memory>

#include "dawn/common/Platform.h"

#if defined(DAWN_PLATFORM_WINDOWS)
#include "dawn/native/d3d12/d3d12_platform.h"
#endif  // DAWN_PLATFORM_WINDOWS

namespace dawn::native {

// Blob represents a block of bytes. It may be constructed from
// various other container types and uses type erasure to take
// ownership of the container and release its memory on destruction.
class Blob {
  public:
    static Blob Create(size_t size);

#if defined(DAWN_PLATFORM_WINDOWS)
    static Blob Create(Microsoft::WRL::ComPtr<ID3DBlob> blob);
#endif  // DAWN_PLATFORM_WINDOWS

    Blob();
    ~Blob();

    Blob(const Blob&) = delete;
    Blob& operator=(const Blob&) = delete;

    Blob(Blob&&);
    Blob& operator=(Blob&&);

    bool Empty() const;
    const uint8_t* Data() const;
    uint8_t* Data();
    size_t Size() const;

  private:
    explicit Blob(uint8_t* data, size_t size, std::function<void()> deleter);

    uint8_t* mData;
    size_t mSize;
    std::function<void()> mDeleter;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLOB_H_
