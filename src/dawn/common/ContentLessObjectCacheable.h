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

#ifndef SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHEABLE_H_
#define SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHEABLE_H_

#include "dawn/common/WeakRefSupport.h"

namespace dawn {

template <typename RefCountedT>
class ContentLessObjectCache;

namespace detail {

// Placeholding base class for cacheable types to enable easier compile-time verifications.
class ContentLessObjectCacheableBase {};

}  // namespace detail

// Classes need to extend this type if they want to be cacheable via the ContentLessObjectCache. It
// is also assumed that the type already extends RefCounted in some way. Otherwise, this helper
// class does not work.
template <typename RefCountedT>
class ContentLessObjectCacheable : public detail::ContentLessObjectCacheableBase,
                                   public WeakRefSupport<RefCountedT> {
  public:
    // Currently, any cacheables should call Uncache in their DeleteThis override. This is important
    // because otherwise, the objects may be leaked in the internal set.
    void Uncache() {
        if (mCache != nullptr) {
            // Note that Erase sets mCache to nullptr. We do it in Erase instead of doing it here in
            // case users call Erase somewhere else before the Uncache call.
            mCache->Erase(static_cast<RefCountedT*>(this));
        }
    }

  protected:
    // The dtor asserts that the cache isn't set to ensure that we were Uncache-d or never cached.
    ~ContentLessObjectCacheable() override { ASSERT(mCache == nullptr); }

  private:
    friend class ContentLessObjectCache<RefCountedT>;

    // Pointer to the owning cache if we were inserted at any point. This is set via the
    // Insert/Erase functions on the cache.
    ContentLessObjectCache<RefCountedT>* mCache = nullptr;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_CONTENTLESSOBJECTCACHEABLE_H_
