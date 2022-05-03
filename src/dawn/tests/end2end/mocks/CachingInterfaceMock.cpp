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

#include "dawn/tests/end2end/mocks/CachingInterfaceMock.h"

using ::testing::Invoke;

CachingInterfaceMock::CachingInterfaceMock() {
    ON_CALL(*this, LoadData).WillByDefault(Invoke([=](auto&&... args) {
        return LoadDataDefault(args...);
    }));
    ON_CALL(*this, StoreData).WillByDefault(Invoke([=](auto&&... args) {
        return StoreDataDefault(args...);
    }));
}

void CachingInterfaceMock::Enable() {
    mEnabled = true;
}

void CachingInterfaceMock::Disable() {
    mEnabled = false;
}

size_t CachingInterfaceMock::GetHitCount() const {
    return mHitCount;
}

size_t CachingInterfaceMock::GetNumEntries() const {
    return mCache.size();
}

size_t CachingInterfaceMock::LoadDataDefault(const void* key,
                                             size_t keySize,
                                             void* value,
                                             size_t valueSize) {
    if (!mEnabled) {
        return 0;
    }

    const std::string keyStr(reinterpret_cast<const char*>(key), keySize);
    auto entry = mCache.find(keyStr);
    if (entry == mCache.end()) {
        return 0;
    }
    if (valueSize >= entry->second.size()) {
        // Only consider a cache-hit on the memcpy, since peeks are implementation detail.
        memcpy(value, entry->second.data(), entry->second.size());
        mHitCount++;
    }
    return entry->second.size();
}

void CachingInterfaceMock::StoreDataDefault(const void* key,
                                            size_t keySize,
                                            const void* value,
                                            size_t valueSize) {
    if (!mEnabled) {
        return;
    }

    const std::string keyStr(reinterpret_cast<const char*>(key), keySize);
    const uint8_t* it = reinterpret_cast<const uint8_t*>(value);
    std::vector<uint8_t> entry(it, it + valueSize);
    mCache.insert_or_assign(keyStr, entry);
}

DawnCachingMockPlatform::DawnCachingMockPlatform(dawn::platform::CachingInterface* cachingInterface)
    : mCachingInterface(cachingInterface) {}

dawn::platform::CachingInterface* DawnCachingMockPlatform::GetCachingInterface(
    const void* fingerprint,
    size_t fingerprintSize) {
    return mCachingInterface;
}
