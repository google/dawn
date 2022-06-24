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

#include "dawn/native/BlobCache.h"

#include <algorithm>

#include "dawn/common/Assert.h"
#include "dawn/common/Version_autogen.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/Instance.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::native {

BlobCache::BlobCache(dawn::platform::CachingInterface* cachingInterface)
    : mCache(cachingInterface) {}

Blob BlobCache::Load(const CacheKey& key) {
    std::lock_guard<std::mutex> lock(mMutex);
    return LoadInternal(key);
}

void BlobCache::Store(const CacheKey& key, size_t valueSize, const void* value) {
    std::lock_guard<std::mutex> lock(mMutex);
    StoreInternal(key, valueSize, value);
}

void BlobCache::Store(const CacheKey& key, const Blob& value) {
    Store(key, value.Size(), value.Data());
}

Blob BlobCache::LoadInternal(const CacheKey& key) {
    ASSERT(ValidateCacheKey(key));
    if (mCache == nullptr) {
        return Blob();
    }
    const size_t expectedSize = mCache->LoadData(key.data(), key.size(), nullptr, 0);
    if (expectedSize > 0) {
        // Need to put this inside to trigger copy elision.
        Blob result = CreateBlob(expectedSize);
        const size_t actualSize =
            mCache->LoadData(key.data(), key.size(), result.Data(), expectedSize);
        ASSERT(expectedSize == actualSize);
        return result;
    }
    return Blob();
}

void BlobCache::StoreInternal(const CacheKey& key, size_t valueSize, const void* value) {
    ASSERT(ValidateCacheKey(key));
    ASSERT(value != nullptr);
    ASSERT(valueSize > 0);
    if (mCache == nullptr) {
        return;
    }
    mCache->StoreData(key.data(), key.size(), value, valueSize);
}

bool BlobCache::ValidateCacheKey(const CacheKey& key) {
    return std::search(key.begin(), key.end(), kDawnVersion.begin(), kDawnVersion.end()) !=
           key.end();
}

}  // namespace dawn::native
