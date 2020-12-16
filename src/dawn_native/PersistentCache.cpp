// Copyright 2020 The Dawn Authors
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

#include "dawn_native/PersistentCache.h"

#include "common/Assert.h"
#include "dawn_native/Device.h"
#include "dawn_platform/DawnPlatform.h"

namespace dawn_native {

    PersistentCache::PersistentCache(DeviceBase* device)
        : mDevice(device), mCache(GetPlatformCache()) {
    }

    ScopedCachedBlob PersistentCache::LoadData(const PersistentCacheKey& key) {
        ScopedCachedBlob blob = {};
        if (mCache == nullptr) {
            return blob;
        }
        blob.bufferSize = mCache->LoadData(reinterpret_cast<WGPUDevice>(mDevice), key.data(),
                                           key.size(), nullptr, 0);
        if (blob.bufferSize > 0) {
            blob.buffer.reset(new uint8_t[blob.bufferSize]);
            const size_t bufferSize =
                mCache->LoadData(reinterpret_cast<WGPUDevice>(mDevice), key.data(), key.size(),
                                 blob.buffer.get(), blob.bufferSize);
            ASSERT(bufferSize == blob.bufferSize);
            return blob;
        }
        return blob;
    }

    void PersistentCache::StoreData(const PersistentCacheKey& key, const void* value, size_t size) {
        if (mCache == nullptr) {
            return;
        }
        ASSERT(value != nullptr);
        ASSERT(size > 0);
        mCache->StoreData(reinterpret_cast<WGPUDevice>(mDevice), key.data(), key.size(), value,
                          size);
    }

    dawn_platform::CachingInterface* PersistentCache::GetPlatformCache() {
        // TODO(dawn:549): Create a fingerprint of concatenated version strings (ex. Tint commit
        // hash, Dawn commit hash). This will be used by the client so it may know when to discard
        // previously cached Dawn objects should this fingerprint change.
        dawn_platform::Platform* platform = mDevice->GetPlatform();
        if (platform != nullptr) {
            return platform->GetCachingInterface(/*fingerprint*/ nullptr, /*fingerprintSize*/ 0);
        }
        return nullptr;
    }
}  // namespace dawn_native
