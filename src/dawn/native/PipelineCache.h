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

#ifndef SRC_DAWN_NATIVE_PIPELINECACHE_H_
#define SRC_DAWN_NATIVE_PIPELINECACHE_H_

#include "dawn/common/RefCounted.h"
#include "dawn/native/BlobCache.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/Error.h"

namespace dawn::native {

// Abstraction layer for backend dependent pipeline caching.
class PipelineCacheBase : public RefCounted {
  public:
    // Returns whether or not we got a cache hit when initializing.
    bool CacheHit() const;

    // Serializes and writes the current contents of the backend cache object into the backing
    // blob cache, potentially overwriting what is already there. Useful when we are working
    // with more monolithic-like caches where we expect overwriting sometimes.
    MaybeError Flush();

    // Serializes and writes the current contents of the backend cache object into the backing
    // blob cache iff the initial read from the backend cache did not result in a hit.
    MaybeError FlushIfNeeded();

  protected:
    PipelineCacheBase(BlobCache* cache, const CacheKey& key);

    // Initializes and returns the cached blob given the cache and keys. Used by backend
    // implementations to get the cache and set the cache hit state. Should only be called once.
    Blob Initialize();

  private:
    // Backend implementation of serialization of the cache into a blob.
    // Note: given that no local cached blob should be destructed and copy elision has strict
    // requirement cached blob is passed in as a pointer to be assigned.
    virtual MaybeError SerializeToBlobImpl(Blob* blob) = 0;

    // The blob cache is owned by the Adapter and pipeline caches are owned/created by devices
    // or adapters. Since the device owns a reference to the Instance which owns the Adapter,
    // the blob cache is guaranteed to be valid throughout the lifetime of the object.
    BlobCache* mCache;
    CacheKey mKey;
    bool mInitialized = false;
    bool mCacheHit = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PIPELINECACHE_H_
