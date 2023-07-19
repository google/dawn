// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHE_H_
#define SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHE_H_

#include <mutex>
#include <tuple>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"

namespace dawn {

// The ContentLessObjectCache stores raw pointers to living Refs without adding to their refcounts.
// This means that any RefCountedT that is inserted into the cache needs to make sure that their
// DeleteThis function erases itself from the cache. Otherwise, the cache can grow indefinitely via
// leaked pointers to deleted Refs.
template <typename RefCountedT>
class ContentLessObjectCache {
  public:
    // The dtor asserts that the cache is empty to aid in finding pointer leaks that can be possible
    // if the RefCountedT doesn't correctly implement the DeleteThis function to erase itself from
    // the cache.
    ~ContentLessObjectCache() { ASSERT(Empty()); }

    // Inserts the object into the cache returning a pair where the first is a Ref to the inserted
    // or existing object, and the second is a bool that is true if we inserted `object` and false
    // otherwise.
    std::pair<Ref<RefCountedT>, bool> Insert(RefCountedT* object) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto [it, inserted] = mCache.insert(object);
        if (inserted) {
            return {object, inserted};
        } else {
            // We need to check that the found instance isn't about to be destroyed. If it is, we
            // actually want to remove the old instance from the cache and insert this one. This can
            // happen if the last ref of the current instance in the cache hit is already in the
            // process of being removed but hasn't completed yet.
            Ref<RefCountedT> ref = TryGetRef(static_cast<RefCountedT*>(*it));
            if (ref != nullptr) {
                return {ref, false};
            } else {
                mCache.erase(it);
                auto result = mCache.insert(object);
                ASSERT(result.second);
                return {object, true};
            }
        }
    }

    // Returns a valid Ref<T> if the underlying RefCounted object's refcount has not reached 0.
    // Otherwise, returns nullptr.
    Ref<RefCountedT> Find(RefCountedT* blueprint) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto it = mCache.find(blueprint);
        if (it != mCache.end()) {
            return TryGetRef(static_cast<RefCountedT*>(*it));
        }
        return nullptr;
    }

    // Erases the object from the cache if it exists and are pointer equal. Otherwise does not
    // modify the cache.
    void Erase(RefCountedT* object) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto it = mCache.find(object);
        if (*it == object) {
            mCache.erase(it);
        }
    }

    // Returns true iff the cache is empty.
    bool Empty() {
        std::lock_guard<std::mutex> lock(mMutex);
        return mCache.empty();
    }

  private:
    std::mutex mMutex;
    std::unordered_set<RefCountedT*,
                       typename RefCountedT::HashFunc,
                       typename RefCountedT::EqualityFunc>
        mCache;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHE_H_
