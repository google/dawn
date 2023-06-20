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

#include "dawn/common/RefCounted.h"

#include <cstddef>
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
#include <sanitizer/tsan_interface.h>
#endif
#endif

#include "dawn/common/Assert.h"

namespace dawn {

static constexpr size_t kPayloadBits = 1;
static constexpr uint64_t kPayloadMask = (uint64_t(1) << kPayloadBits) - 1;
static constexpr uint64_t kRefCountIncrement = (uint64_t(1) << kPayloadBits);

RefCount::RefCount(uint64_t payload) : mRefCount(kRefCountIncrement + payload) {
    ASSERT((payload & kPayloadMask) == payload);
}

uint64_t RefCount::GetValueForTesting() const {
    return mRefCount >> kPayloadBits;
}

uint64_t RefCount::GetPayload() const {
    // We only care about the payload bits of the refcount. These never change after
    // initialization so we can use the relaxed memory order. The order doesn't guarantee
    // anything except the atomicity of the load, which is enough since any past values of the
    // atomic will have the correct payload bits.
    return kPayloadMask & mRefCount.load(std::memory_order_relaxed);
}

void RefCount::Increment() {
    ASSERT((mRefCount & ~kPayloadMask) != 0);

    // The relaxed ordering guarantees only the atomicity of the update, which is enough here
    // because the reference we are copying from still exists and makes sure other threads
    // don't delete `this`.
    // See the explanation in the Boost documentation:
    //     https://www.boost.org/doc/libs/1_55_0/doc/html/atomic/usage_examples.html
    mRefCount.fetch_add(kRefCountIncrement, std::memory_order_relaxed);
}

bool RefCount::TryIncrement() {
    uint64_t current = mRefCount.load(std::memory_order_relaxed);
    bool success = false;
    do {
        if ((current & ~kPayloadMask) == 0u) {
            return false;
        }
        // The relaxed ordering guarantees only the atomicity of the update. This is fine because:
        //   - If another thread's decrement happens before this increment, the increment should
        //     fail.
        //   - If another thread's decrement happens after this increment, the decrement shouldn't
        //     delete the object, because the ref count > 0.
        // See Boost library for reference:
        //   https://github.com/boostorg/smart_ptr/blob/develop/include/boost/smart_ptr/detail/sp_counted_base_std_atomic.hpp#L62
        success = mRefCount.compare_exchange_weak(current, current + kRefCountIncrement,
                                                  std::memory_order_relaxed);
    } while (!success);
    return true;
}

bool RefCount::Decrement() {
    ASSERT((mRefCount & ~kPayloadMask) != 0);

    // The release fence here is to make sure all accesses to the object on a thread A
    // happen-before the object is deleted on a thread B. The release memory order ensures that
    // all accesses on thread A happen-before the refcount is decreased and the atomic variable
    // makes sure the refcount decrease in A happens-before the refcount decrease in B. Finally
    // the acquire fence in the destruction case makes sure the refcount decrease in B
    // happens-before the `delete this`.
    //
    // See the explanation in the Boost documentation:
    //     https://www.boost.org/doc/libs/1_55_0/doc/html/atomic/usage_examples.html
    uint64_t previousRefCount = mRefCount.fetch_sub(kRefCountIncrement, std::memory_order_release);

    // Check that the previous reference count was strictly less than 2, ignoring payload bits.
    if (previousRefCount < 2 * kRefCountIncrement) {
        // Note that on ARM64 this will generate a `dmb ish` instruction which is a global
        // memory barrier, when an acquire load on mRefCount (using the `ldar` instruction)
        // should be enough and could end up being faster.

        // https://github.com/google/sanitizers/issues/1415 There is false positive bug in TSAN
        // when using standalone fence.
#if defined(__has_feature)
#if __has_feature(thread_sanitizer)
        __tsan_acquire(&mRefCount);
#endif
#endif
        std::atomic_thread_fence(std::memory_order_acquire);
        return true;
    }
    return false;
}

RefCounted::RefCounted(uint64_t payload) : mRefCount(payload) {}
RefCounted::~RefCounted() = default;

uint64_t RefCounted::GetRefCountForTesting() const {
    return mRefCount.GetValueForTesting();
}

uint64_t RefCounted::GetRefCountPayload() const {
    return mRefCount.GetPayload();
}

void RefCounted::Reference() {
    mRefCount.Increment();
}

void RefCounted::Release() {
    if (mRefCount.Decrement()) {
        DeleteThis();
    }
}

void RefCounted::ReleaseAndLockBeforeDestroy() {
    if (mRefCount.Decrement()) {
        LockAndDeleteThis();
    }
}

void RefCounted::DeleteThis() {
    delete this;
}

void RefCounted::LockAndDeleteThis() {
    DeleteThis();
}

}  // namespace dawn
