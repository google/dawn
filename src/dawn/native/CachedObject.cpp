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
    ASSERT(!mIsContentHashInitialized || contentHash == mContentHash);
    mContentHash = contentHash;
    mIsContentHashInitialized = true;
}

const CacheKey& CachedObject::GetCacheKey() const {
    return mCacheKey;
}

// static
template <>
void stream::Stream<CachedObject>::Write(stream::Sink* sink, const CachedObject& obj) {
    StreamIn(sink, obj.GetCacheKey());
}

}  // namespace dawn::native
