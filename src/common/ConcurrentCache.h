// Copyright 2021 The Dawn Authors
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

#ifndef COMMON_CONCURRENT_CACHE_H_
#define COMMON_CONCURRENT_CACHE_H_

#include "common/NonCopyable.h"

#include <mutex>
#include <unordered_set>
#include <utility>

template <typename T>
class ConcurrentCache : public NonMovable {
  public:
    ConcurrentCache() = default;

    T* Find(T* object) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto iter = mCache.find(object);
        if (iter == mCache.end()) {
            return nullptr;
        }
        return *iter;
    }

    std::pair<T*, bool> Insert(T* object) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto insertion = mCache.insert(object);
        return std::make_pair(*(insertion.first), insertion.second);
    }

    size_t Erase(T* object) {
        std::lock_guard<std::mutex> lock(mMutex);
        return mCache.erase(object);
    }

  private:
    std::mutex mMutex;
    std::unordered_set<T*, typename T::HashFunc, typename T::EqualityFunc> mCache;
};

#endif
