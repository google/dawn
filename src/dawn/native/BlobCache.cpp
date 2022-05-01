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

#include "dawn/common/Assert.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/Instance.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::native {

CachedBlob::CachedBlob(size_t size) {
    if (size != 0) {
        Reset(size);
    }
}

bool CachedBlob::Empty() const {
    return mSize == 0;
}

const uint8_t* CachedBlob::Data() const {
    return mData.get();
}

uint8_t* CachedBlob::Data() {
    return mData.get();
}

size_t CachedBlob::Size() const {
    return mSize;
}

void CachedBlob::Reset(size_t size) {
    mSize = size;
    mData = std::make_unique<uint8_t[]>(size);
}

BlobCache::BlobCache(dawn::platform::CachingInterface* cachingInterface)
    : mCache(cachingInterface) {}

CachedBlob BlobCache::Load(const CacheKey& key) {
    std::lock_guard<std::mutex> lock(mMutex);
    return LoadInternal(key);
}

void BlobCache::Store(const CacheKey& key, size_t valueSize, const void* value) {
    std::lock_guard<std::mutex> lock(mMutex);
    StoreInternal(key, valueSize, value);
}

void BlobCache::Store(const CacheKey& key, const CachedBlob& value) {
    Store(key, value.Size(), value.Data());
}

CachedBlob BlobCache::LoadInternal(const CacheKey& key) {
    CachedBlob result;
    if (mCache == nullptr) {
        return result;
    }
    const size_t expectedSize = mCache->LoadData(nullptr, key.data(), key.size(), nullptr, 0);
    if (expectedSize > 0) {
        result.Reset(expectedSize);
        const size_t actualSize =
            mCache->LoadData(nullptr, key.data(), key.size(), result.Data(), expectedSize);
        ASSERT(expectedSize == actualSize);
    }
    return result;
}

void BlobCache::StoreInternal(const CacheKey& key, size_t valueSize, const void* value) {
    ASSERT(value != nullptr);
    ASSERT(valueSize > 0);
    if (mCache == nullptr) {
        return;
    }
    mCache->StoreData(nullptr, key.data(), key.size(), value, valueSize);
}

}  // namespace dawn::native
