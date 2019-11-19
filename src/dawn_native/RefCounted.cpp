// Copyright 2017 The Dawn Authors
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

#include "dawn_native/RefCounted.h"

#include "common/Assert.h"

namespace dawn_native {

    static constexpr size_t kPayloadBits = 1;
    static constexpr uint64_t kPayloadMask = (uint64_t(1) << kPayloadBits) - 1;
    static constexpr uint64_t kRefCountIncrement = (uint64_t(1) << kPayloadBits);

    RefCounted::RefCounted(uint64_t payload) : mRefCount(kRefCountIncrement + payload) {
        ASSERT((payload & kPayloadMask) == payload);
    }

    RefCounted::~RefCounted() {
    }

    uint64_t RefCounted::GetRefCountForTesting() const {
        return mRefCount >> kPayloadBits;
    }

    uint64_t RefCounted::GetRefCountPayload() const {
        // We only care about the payload bits of the refcount. These never change after
        // initialization so we can use the relaxed memory order. The order doesn't guarantee
        // anything except the atomicity of the load, which is enough since any past values of the
        // atomic will have the correct payload bits.
        return kPayloadMask & mRefCount.load(std::memory_order_relaxed);
    }

    void RefCounted::Reference() {
        ASSERT((mRefCount & ~kPayloadMask) != 0);
        mRefCount += kRefCountIncrement;
    }

    void RefCounted::Release() {
        ASSERT((mRefCount & ~kPayloadMask) != 0);

        mRefCount -= kRefCountIncrement;
        if (mRefCount < kRefCountIncrement) {
            delete this;
        }
    }

}  // namespace dawn_native
