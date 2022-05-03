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

#ifndef SRC_DAWN_NATIVE_BLOBCACHE_H_
#define SRC_DAWN_NATIVE_BLOBCACHE_H_

#include <memory>
#include <mutex>

namespace dawn::platform {
class CachingInterface;
}

namespace dawn::native {

class BlobCache;
class CacheKey;
class InstanceBase;

class CachedBlob {
  public:
    explicit CachedBlob(size_t size = 0);

    bool Empty() const;
    const uint8_t* Data() const;
    uint8_t* Data();
    size_t Size() const;
    void Reset(size_t size);

  private:
    std::unique_ptr<uint8_t[]> mData = nullptr;
    size_t mSize = 0;
};

// This class should always be thread-safe because it may be called asynchronously. Its purpose
// is to wrap the CachingInterface provided via a platform.
class BlobCache {
  public:
    explicit BlobCache(dawn::platform::CachingInterface* cachingInterface = nullptr);

    // Returns empty blob if the key is not found in the cache.
    CachedBlob Load(const CacheKey& key);

    // Value to store must be non-empty/non-null.
    void Store(const CacheKey& key, size_t valueSize, const void* value);
    void Store(const CacheKey& key, const CachedBlob& value);

  private:
    // Non-thread safe internal implementations of load and store. Exposed callers that use
    // these helpers need to make sure that these are entered with `mMutex` held.
    CachedBlob LoadInternal(const CacheKey& key);
    void StoreInternal(const CacheKey& key, size_t valueSize, const void* value);

    // Protects thread safety of access to mCache.
    std::mutex mMutex;
    dawn::platform::CachingInterface* mCache;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BLOBCACHE_H_
