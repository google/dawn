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
#include <variant>

#include "dawn/common/ContentLessObjectCacheable.h"
#include "dawn/common/Ref.h"
#include "dawn/common/RefCounted.h"
#include "dawn/common/WeakRef.h"

namespace dawn {
namespace detail {

// Tagged-type to force special path for EqualityFunc when dealing with Erase. When erasing, we only
// care about pointer equality, not value equality. This is also particularly important because
// trying to promote on the Erase path can cause failures as the object's last ref could've been
// dropped already.
template <typename RefCountedT>
struct ForErase {
    explicit ForErase(RefCountedT* value) : mValue(value) {}
    RefCountedT* mValue;
};

// All cached WeakRefs must have an immutable hash value determined at insertion. This ensures that
// even if the last ref of the cached value is dropped, we still get the same hash in the set for
// erasing.
template <typename RefCountedT>
using WeakRefAndHash = std::pair<WeakRef<RefCountedT>, size_t>;

// The cache always holds WeakRefs internally, however, to enable lookups using pointers and special
// Erase equality, we use a variant type to branch.
template <typename RefCountedT>
using ContentLessObjectCacheKey =
    std::variant<RefCountedT*, WeakRefAndHash<RefCountedT>, ForErase<RefCountedT>>;

enum class KeyType : size_t { Pointer = 0, WeakRef = 1, ForErase = 2 };

template <typename RefCountedT>
struct ContentLessObjectCacheHashVisitor {
    using BaseHashFunc = typename RefCountedT::HashFunc;

    size_t operator()(const RefCountedT* ptr) const { return BaseHashFunc()(ptr); }
    size_t operator()(const WeakRefAndHash<RefCountedT>& weakref) const { return weakref.second; }
    size_t operator()(const ForErase<RefCountedT>& forErase) const {
        return BaseHashFunc()(forErase.mValue);
    }
};

template <typename RefCountedT>
struct ContentLessObjectCacheKeyFuncs {
    static_assert(
        std::is_same_v<RefCountedT*,
                       std::variant_alternative_t<static_cast<size_t>(KeyType::Pointer),
                                                  ContentLessObjectCacheKey<RefCountedT>>>);
    static_assert(
        std::is_same_v<WeakRefAndHash<RefCountedT>,
                       std::variant_alternative_t<static_cast<size_t>(KeyType::WeakRef),
                                                  ContentLessObjectCacheKey<RefCountedT>>>);
    static_assert(
        std::is_same_v<ForErase<RefCountedT>,
                       std::variant_alternative_t<static_cast<size_t>(KeyType::ForErase),
                                                  ContentLessObjectCacheKey<RefCountedT>>>);

    struct HashFunc {
        size_t operator()(const ContentLessObjectCacheKey<RefCountedT>& key) const {
            return std::visit(ContentLessObjectCacheHashVisitor<RefCountedT>(), key);
        }
    };

    using BaseEqualityFunc = typename RefCountedT::EqualityFunc;
    struct EqualityFunc {
        bool operator()(const ContentLessObjectCacheKey<RefCountedT>& a,
                        const ContentLessObjectCacheKey<RefCountedT>& b) const {
            // First check if we are in the erasing scenario. We need to determine this early
            // because we handle the actual equality differently. Note that if either a or b is
            // a ForErase, it is safe to use UnsafeGet for both a and b because either:
            //   (1) a == b, in which case that means we are destroying the last copy and must be
            //       valid because cached objects must uncache themselves before being completely
            //       destroyed.
            //   (2) a != b, in which case the lock on the cache guarantees that the element in the
            //       cache has not been erased yet and hence cannot have been destroyed.
            bool erasing = std::holds_alternative<ForErase<RefCountedT>>(a) ||
                           std::holds_alternative<ForErase<RefCountedT>>(b);

            auto ExtractKey = [](bool erasing, const ContentLessObjectCacheKey<RefCountedT>& x)
                -> std::pair<RefCountedT*, Ref<RefCountedT>> {
                RefCountedT* xPtr = nullptr;
                Ref<RefCountedT> xRef;
                switch (static_cast<KeyType>(x.index())) {
                    case KeyType::Pointer:
                        xPtr = std::get<RefCountedT*>(x);
                        break;
                    case KeyType::WeakRef:
                        if (erasing) {
                            xPtr = std::get<WeakRefAndHash<RefCountedT>>(x).first.UnsafeGet();
                        } else {
                            xRef = std::get<WeakRefAndHash<RefCountedT>>(x).first.Promote();
                            xPtr = xRef.Get();
                        }
                        break;
                    case KeyType::ForErase:
                        xPtr = std::get<ForErase<RefCountedT>>(x).mValue;
                        break;
                    default:
                        UNREACHABLE();
                }
                return {xPtr, xRef};
            };

            auto [aPtr, aRef] = ExtractKey(erasing, a);
            auto [bPtr, bRef] = ExtractKey(erasing, b);
            if (aPtr == nullptr || bPtr == nullptr) {
                return false;
            }
            if (erasing) {
                return aPtr == bPtr;
            }
            return BaseEqualityFunc()(aPtr, bPtr);
        }
    };
};

}  // namespace detail

template <typename RefCountedT>
class ContentLessObjectCache {
    static_assert(std::is_base_of_v<detail::ContentLessObjectCacheableBase, RefCountedT>,
                  "Type must be cacheable to use with ContentLessObjectCache.");
    static_assert(std::is_base_of_v<RefCounted, RefCountedT>,
                  "Type must be refcounted to use with ContentLessObjectCache.");

  public:
    // The dtor asserts that the cache is empty to aid in finding pointer leaks that can be
    // possible if the RefCountedT doesn't correctly implement the DeleteThis function to Uncache.
    ~ContentLessObjectCache() { ASSERT(Empty()); }

    // Inserts the object into the cache returning a pair where the first is a Ref to the
    // inserted or existing object, and the second is a bool that is true if we inserted
    // `object` and false otherwise.
    std::pair<Ref<RefCountedT>, bool> Insert(RefCountedT* obj) {
        std::lock_guard<std::mutex> lock(mMutex);
        detail::WeakRefAndHash<RefCountedT> weakref =
            std::make_pair(GetWeakRef(obj), typename RefCountedT::HashFunc()(obj));
        auto [it, inserted] = mCache.insert(weakref);
        if (inserted) {
            obj->mCache = this;
            return {obj, inserted};
        } else {
            // Try to promote the found WeakRef to a Ref. If promotion fails, remove the old Key
            // and insert this one.
            Ref<RefCountedT> ref =
                std::get<detail::WeakRefAndHash<RefCountedT>>(*it).first.Promote();
            if (ref != nullptr) {
                return {ref, false};
            } else {
                mCache.erase(it);
                auto result = mCache.insert(weakref);
                ASSERT(result.second);
                obj->mCache = this;
                return {obj, true};
            }
        }
    }

    // Returns a valid Ref<T> if we can Promote the underlying WeakRef. Returns nullptr otherwise.
    Ref<RefCountedT> Find(RefCountedT* blueprint) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto it = mCache.find(blueprint);
        if (it != mCache.end()) {
            return std::get<detail::WeakRefAndHash<RefCountedT>>(*it).first.Promote();
        }
        return nullptr;
    }

    // Erases the object from the cache if it exists and are pointer equal. Otherwise does not
    // modify the cache.
    void Erase(RefCountedT* obj) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto it = mCache.find(detail::ForErase<RefCountedT>(obj));
        if (it == mCache.end()) {
            return;
        }
        obj->mCache = nullptr;
        mCache.erase(it);
    }

    // Returns true iff the cache is empty.
    bool Empty() {
        std::lock_guard<std::mutex> lock(mMutex);
        return mCache.empty();
    }

  private:
    std::mutex mMutex;
    std::unordered_set<detail::ContentLessObjectCacheKey<RefCountedT>,
                       typename detail::ContentLessObjectCacheKeyFuncs<RefCountedT>::HashFunc,
                       typename detail::ContentLessObjectCacheKeyFuncs<RefCountedT>::EqualityFunc>
        mCache;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHE_H_
