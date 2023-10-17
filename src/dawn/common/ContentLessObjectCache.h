// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
#include "dawn/common/StackContainer.h"
#include "dawn/common/WeakRef.h"

namespace dawn {

template <typename RefCountedT>
class ContentLessObjectCache;

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

    struct EqualityFunc {
        explicit EqualityFunc(ContentLessObjectCache<RefCountedT>* cache) : mCache(cache) {}

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
                        DAWN_UNREACHABLE();
                }
                return {xPtr, xRef};
            };
            auto [aPtr, aRef] = ExtractKey(erasing, a);
            auto [bPtr, bRef] = ExtractKey(erasing, b);

            bool result = false;
            if (aPtr == nullptr || bPtr == nullptr) {
                result = false;
            } else if (erasing) {
                result = aPtr == bPtr;
            } else {
                result = typename RefCountedT::EqualityFunc()(aPtr, bPtr);
            }

            if (aRef != nullptr) {
                mCache->TrackTemporaryRef(std::move(aRef));
            }
            if (bRef != nullptr) {
                mCache->TrackTemporaryRef(std::move(bRef));
            }
            return result;
        }

        ContentLessObjectCache<RefCountedT>* mCache = nullptr;
    };
};

}  // namespace detail

template <typename RefCountedT>
class ContentLessObjectCache {
    static_assert(std::is_base_of_v<detail::ContentLessObjectCacheableBase, RefCountedT>,
                  "Type must be cacheable to use with ContentLessObjectCache.");
    static_assert(std::is_base_of_v<RefCounted, RefCountedT>,
                  "Type must be refcounted to use with ContentLessObjectCache.");

    using CacheKeyFuncs = detail::ContentLessObjectCacheKeyFuncs<RefCountedT>;

  public:
    // Constructor needs to pass in 'this' to the EqualityFunc. Since the default bucket_count on
    // sets is implementation defined, creating a temporary unused set to get the value. The actual
    // type of the temporary does not matter.
    ContentLessObjectCache()
        : mCache(std::unordered_set<int>().bucket_count(),
                 typename CacheKeyFuncs::HashFunc(),
                 typename CacheKeyFuncs::EqualityFunc(this)) {}

    // The dtor asserts that the cache is empty to aid in finding pointer leaks that can be
    // possible if the RefCountedT doesn't correctly implement the DeleteThis function to Uncache.
    ~ContentLessObjectCache() { DAWN_ASSERT(Empty()); }

    // Inserts the object into the cache returning a pair where the first is a Ref to the
    // inserted or existing object, and the second is a bool that is true if we inserted
    // `object` and false otherwise.
    std::pair<Ref<RefCountedT>, bool> Insert(RefCountedT* obj) {
        return WithLockAndCleanup([&]() -> std::pair<Ref<RefCountedT>, bool> {
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
                    DAWN_ASSERT(result.second);
                    obj->mCache = this;
                    return {obj, true};
                }
            }
        });
    }

    // Returns a valid Ref<T> if we can Promote the underlying WeakRef. Returns nullptr otherwise.
    Ref<RefCountedT> Find(RefCountedT* blueprint) {
        return WithLockAndCleanup([&]() -> Ref<RefCountedT> {
            auto it = mCache.find(blueprint);
            if (it != mCache.end()) {
                return std::get<detail::WeakRefAndHash<RefCountedT>>(*it).first.Promote();
            }
            return nullptr;
        });
    }

    // Erases the object from the cache if it exists and are pointer equal. Otherwise does not
    // modify the cache. Since Erase never Promotes any WeakRefs, it does not need to be wrapped by
    // a WithLockAndCleanup, and a simple lock is enough.
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
    friend struct CacheKeyFuncs::EqualityFunc;

    void TrackTemporaryRef(Ref<RefCountedT> ref) { (*mTemporaryRefs)->push_back(std::move(ref)); }
    template <typename F>
    auto WithLockAndCleanup(F func) {
        using RetType = decltype(func());
        RetType result;

        // Creates and owns a temporary StackVector that we point to internally to track Refs.
        StackVector<Ref<RefCountedT>, 4> temps;
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mTemporaryRefs = &temps;
            result = func();
            mTemporaryRefs = nullptr;
        }
        return result;
    }

    std::mutex mMutex;
    std::unordered_set<detail::ContentLessObjectCacheKey<RefCountedT>,
                       typename CacheKeyFuncs::HashFunc,
                       typename CacheKeyFuncs::EqualityFunc>
        mCache;

    // The cache has a pointer to a StackVector of temporary Refs that are by-products of Promotes
    // inside the EqualityFunc. These Refs need to outlive the EqualityFunc calls because otherwise,
    // they could be the last living Ref of the object resulting in a re-entrant Erase call that
    // deadlocks on the mutex. Since the default max_load_factor of most std::unordered_set
    // implementations should be 1.0 (roughly 1 element per bucket), a StackVector of length 4
    // should be enough space in most cases. See dawn:1993 for more details.
    StackVector<Ref<RefCountedT>, 4>* mTemporaryRefs = nullptr;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHE_H_
