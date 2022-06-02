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

// static
CachedBlob CachedBlob::Create(size_t size) {
    if (size > 0) {
        uint8_t* data = new uint8_t[size];
        return CachedBlob(data, size, [=]() { delete[] data; });
    } else {
        return CachedBlob();
    }
}

CachedBlob::CachedBlob() : mData(nullptr), mSize(0), mDeleter({}) {}

CachedBlob::CachedBlob(uint8_t* data, size_t size, std::function<void()> deleter)
    : mData(data), mSize(size), mDeleter(deleter) {}

CachedBlob::CachedBlob(CachedBlob&&) = default;

CachedBlob& CachedBlob::operator=(CachedBlob&&) = default;

CachedBlob::~CachedBlob() {
    if (mDeleter) {
        mDeleter();
    }
}

bool CachedBlob::Empty() const {
    return mSize == 0;
}

const uint8_t* CachedBlob::Data() const {
    return mData;
}

uint8_t* CachedBlob::Data() {
    return mData;
}

size_t CachedBlob::Size() const {
    return mSize;
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
    if (mCache == nullptr) {
        return CachedBlob();
    }
    const size_t expectedSize = mCache->LoadData(key.data(), key.size(), nullptr, 0);
    if (expectedSize > 0) {
        // Need to put this inside to trigger copy elision.
        CachedBlob result = CachedBlob::Create(expectedSize);
        const size_t actualSize =
            mCache->LoadData(key.data(), key.size(), result.Data(), expectedSize);
        ASSERT(expectedSize == actualSize);
        return result;
    }
    return CachedBlob();
}

void BlobCache::StoreInternal(const CacheKey& key, size_t valueSize, const void* value) {
    ASSERT(value != nullptr);
    ASSERT(valueSize > 0);
    if (mCache == nullptr) {
        return;
    }
    mCache->StoreData(key.data(), key.size(), value, valueSize);
}

}  // namespace dawn::native
