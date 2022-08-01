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

#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

namespace dawn::native {

// Blob represents a block of bytes. It may be constructed from
// various other container types and uses type erasure to take
// ownership of the container and release its memory on destruction.
class Blob {
  public:
    // This function is used to create Blob with actual data.
    // Make sure the creation and deleter handles the data ownership and lifetime correctly.
    static Blob UnsafeCreateWithDeleter(uint8_t* data, size_t size, std::function<void()> deleter);

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

    // If the blob data is not aligned to |alignment|, copy it into a new backing store which
    // is aligned.
    void AlignTo(size_t alignment);

  private:
    // The constructor should be responsible to take ownership of |data| and releases ownership by
    // calling |deleter|. The deleter function is called at ~Blob() and during std::move.
    explicit Blob(uint8_t* data, size_t size, std::function<void()> deleter);

    uint8_t* mData;
    size_t mSize;
    std::function<void()> mDeleter;
};

Blob CreateBlob(size_t size, size_t alignment = 1);

template <typename T, typename = std::enable_if_t<std::is_fundamental_v<T>>>
Blob CreateBlob(std::vector<T> vec) {
    uint8_t* data = reinterpret_cast<uint8_t*>(vec.data());
    size_t size = vec.size() * sizeof(T);
    // Move the vector into a new allocation so we can destruct it in the deleter.
    auto* wrapped_vec = new std::vector<T>(std::move(vec));
    return Blob::UnsafeCreateWithDeleter(data, size, [wrapped_vec]() { delete wrapped_vec; });
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLOB_H_
