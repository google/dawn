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

#ifndef DAWNNATIVE_CACHED_OBJECT_H_
#define DAWNNATIVE_CACHED_OBJECT_H_

#include "dawn/native/Forward.h"

#include <cstddef>
#include <string>

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

        // Two versions of GetCacheKey, when passed a device, prepends the stored cache
        // key base with device and adapter information. When called without passing a
        // device, returns the stored cache key base. This is useful when the instance
        // is a member to a parent class.
        const std::string& GetCacheKey() const;
        std::string GetCacheKey(DeviceBase* device) const;
        void SetCacheKey(const std::string& cacheKey);

      private:
        friend class DeviceBase;
        void SetIsCachedReference();

        bool mIsCachedReference = false;

        // Called by ObjectContentHasher upon creation to record the object.
        virtual size_t ComputeContentHash() = 0;

        // Not all classes implement cache key computation, so by default we assert.
        virtual std::string ComputeCacheKeyBase() const;

        size_t mContentHash = 0;
        bool mIsContentHashInitialized = false;
        std::string mCacheKeyBase = "";
        bool mIsCacheKeyBaseInitialized = false;
    };

}  // namespace dawn::native

#endif  // DAWNNATIVE_CACHED_OBJECT_H_
