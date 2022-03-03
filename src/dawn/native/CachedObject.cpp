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

#include "dawn/native/CachedObject.h"

#include "dawn/common/Assert.h"
#include "dawn/native/Device.h"

namespace dawn::native {

    bool CachedObject::IsCachedReference() const {
        return mIsCachedReference;
    }

    void CachedObject::SetIsCachedReference() {
        mIsCachedReference = true;
    }

    size_t CachedObject::HashFunc::operator()(const CachedObject* obj) const {
        return obj->GetContentHash();
    }

    size_t CachedObject::GetContentHash() const {
        ASSERT(mIsContentHashInitialized);
        return mContentHash;
    }

    void CachedObject::SetContentHash(size_t contentHash) {
        ASSERT(!mIsContentHashInitialized);
        mContentHash = contentHash;
        mIsContentHashInitialized = true;
    }

    const std::string& CachedObject::GetCacheKey() const {
        ASSERT(mIsCacheKeyBaseInitialized);
        return mCacheKeyBase;
    }

    std::string CachedObject::GetCacheKey(DeviceBase* device) const {
        ASSERT(mIsCacheKeyBaseInitialized);
        // TODO(dawn:549) Prepend/append with device/adapter information.
        return mCacheKeyBase;
    }

    void CachedObject::SetCacheKey(const std::string& cacheKey) {
        ASSERT(!mIsContentHashInitialized);
        mCacheKeyBase = cacheKey;
        mIsCacheKeyBaseInitialized = true;
    }

    std::string CachedObject::ComputeCacheKeyBase() const {
        // This implementation should never be called. Only overrides should be called.
        UNREACHABLE();
    }

}  // namespace dawn::native
