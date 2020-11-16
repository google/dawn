// Copyright 2017 The Dawn Authors
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

#ifndef COMMON_REFCOUNTED_H_
#define COMMON_REFCOUNTED_H_

#include "common/RefBase.h"

#include <atomic>
#include <cstdint>

class RefCounted {
  public:
    RefCounted(uint64_t payload = 0);

    uint64_t GetRefCountForTesting() const;
    uint64_t GetRefCountPayload() const;

    // Dawn API
    void Reference();
    void Release();

  protected:
    virtual ~RefCounted() = default;
    // A Derived class may override this if they require a custom deleter.
    virtual void DeleteThis();

  private:
    std::atomic<uint64_t> mRefCount;
};

template <typename T>
struct RefCountedTraits {
    static constexpr T* kNullValue = nullptr;
    static void Reference(T* value) {
        value->Reference();
    }
    static void Release(T* value) {
        value->Release();
    }
};

template <typename T>
class Ref : public RefBase<T*, RefCountedTraits<T>> {
  public:
    using RefBase<T*, RefCountedTraits<T>>::RefBase;
};

template <typename T>
Ref<T> AcquireRef(T* pointee) {
    Ref<T> ref;
    ref.Acquire(pointee);
    return ref;
}

#endif  // COMMON_REFCOUNTED_H_
