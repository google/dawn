// Copyright 2019 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_CACHEDOBJECT_H_
#define SRC_DAWN_NATIVE_CACHEDOBJECT_H_

#include <cstddef>
#include <string>

#include "dawn/native/CacheKey.h"
#include "dawn/native/Forward.h"

namespace dawn::native {

// Some objects are cached so that instead of creating new duplicate objects,
// we increase the refcount of an existing object.
// When an object is successfully created, the device should call
// SetIsCachedReference() and insert the object into the cache.
class CachedObject {
  public:
    bool IsCachedReference() const;

    // Functor necessary for the unordered_set<CachedObject*>-based cache.
    struct HashFunc {
        size_t operator()(const CachedObject* obj) const;
    };

    size_t GetContentHash() const;
    void SetContentHash(size_t contentHash);

    // Returns the cache key for the object only, i.e. without device/adapter information.
    const CacheKey& GetCacheKey() const;

  protected:
    // Cache key member is protected so that derived classes can modify it.
    CacheKey mCacheKey;

  private:
    friend class DeviceBase;
    void SetIsCachedReference();

    bool mIsCachedReference = false;

    // Called by ObjectContentHasher upon creation to record the object.
    virtual size_t ComputeContentHash() = 0;

    size_t mContentHash = 0;
    bool mIsContentHashInitialized = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CACHEDOBJECT_H_
