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

#ifndef SRC_DAWN_NATIVE_CACHEKEY_H_
#define SRC_DAWN_NATIVE_CACHEKEY_H_

#include <algorithm>
#include <bitset>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dawn/common/TypedInteger.h"
#include "dawn/common/ityp_array.h"

namespace dawn::native {

// Forward declare classes because of co-dependency.
class CacheKey;
class CachedObject;

// Stream operator for CacheKey for debugging.
std::ostream& operator<<(std::ostream& os, const CacheKey& key);

// Overridable serializer struct that should be implemented for cache key serializable
// types/classes.
template <typename T, typename SFINAE = void>
class CacheKeySerializer {
  public:
    static void Serialize(CacheKey* key, const T& t);
};

class CacheKey : public std::vector<uint8_t> {
  public:
    using std::vector<uint8_t>::vector;

    enum class Type { ComputePipeline, RenderPipeline, Shader };

    template <typename T>
    class UnsafeUnkeyedValue {
      public:
        UnsafeUnkeyedValue() = default;
        // NOLINTNEXTLINE(runtime/explicit) allow implicit construction to decrease verbosity
        UnsafeUnkeyedValue(T&& value) : mValue(std::forward<T>(value)) {}

        const T& UnsafeGetValue() const { return mValue; }

      private:
        T mValue;
    };

    template <typename T>
    CacheKey& Record(const T& t) {
        CacheKeySerializer<T>::Serialize(this, t);
        return *this;
    }
    template <typename T, typename... Args>
    CacheKey& Record(const T& t, const Args&... args) {
        CacheKeySerializer<T>::Serialize(this, t);
        return Record(args...);
    }

    // Records iterables by prepending the number of elements. Some common iterables are have a
    // CacheKeySerializer implemented to avoid needing to split them out when recording, i.e.
    // strings and CacheKeys, but they fundamentally do the same as this function.
    template <typename IterableT>
    CacheKey& RecordIterable(const IterableT& iterable) {
        // Always record the size of generic iterables as a size_t for now.
        Record(static_cast<size_t>(iterable.size()));
        for (auto it = iterable.begin(); it != iterable.end(); ++it) {
            Record(*it);
        }
        return *this;
    }
    template <typename Index, typename Value, size_t Size>
    CacheKey& RecordIterable(const ityp::array<Index, Value, Size>& iterable) {
        Record(static_cast<Index>(iterable.size()));
        for (auto it = iterable.begin(); it != iterable.end(); ++it) {
            Record(*it);
        }
        return *this;
    }
    template <typename Ptr>
    CacheKey& RecordIterable(const Ptr* ptr, size_t n) {
        Record(n);
        for (size_t i = 0; i < n; ++i) {
            Record(ptr[i]);
        }
        return *this;
    }
};

template <typename T>
CacheKey::UnsafeUnkeyedValue<T> UnsafeUnkeyedValue(T&& value) {
    return CacheKey::UnsafeUnkeyedValue<T>(std::forward<T>(value));
}

}  // namespace dawn::native

// CacheKeySerializer implementation temporarily moved to stream/Stream.h to
// simplify the diff in the refactor to stream::Stream.
#include "dawn/native/stream/Stream.h"

#endif  // SRC_DAWN_NATIVE_CACHEKEY_H_
