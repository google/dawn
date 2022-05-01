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

#include <bitset>
#include <iostream>
#include <limits>
#include <string>
#include <type_traits>
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

// Specialized overload for fundamental types.
template <typename T>
class CacheKeySerializer<T, std::enable_if_t<std::is_fundamental_v<T>>> {
  public:
    static void Serialize(CacheKey* key, const T t) {
        const char* it = reinterpret_cast<const char*>(&t);
        key->insert(key->end(), it, (it + sizeof(T)));
    }
};

// Specialized overload for bitsets that are smaller than 64.
template <size_t N>
class CacheKeySerializer<std::bitset<N>, std::enable_if_t<(N <= 64)>> {
  public:
    static void Serialize(CacheKey* key, const std::bitset<N>& t) { key->Record(t.to_ullong()); }
};

// Specialized overload for bitsets since using the built-in to_ullong have a size limit.
template <size_t N>
class CacheKeySerializer<std::bitset<N>, std::enable_if_t<(N > 64)>> {
  public:
    static void Serialize(CacheKey* key, const std::bitset<N>& t) {
        // Serializes the bitset into series of uint8_t, along with recording the size.
        static_assert(N > 0);
        key->Record(static_cast<size_t>(N));
        uint8_t value = 0;
        for (size_t i = 0; i < N; i++) {
            value <<= 1;
            // Explicitly convert to numeric since MSVC doesn't like mixing of bools.
            value |= t[i] ? 1 : 0;
            if (i % 8 == 7) {
                // Whenever we fill an 8 bit value, record it and zero it out.
                key->Record(value);
                value = 0;
            }
        }
        // Serialize the last value if we are not a multiple of 8.
        if (N % 8 != 0) {
            key->Record(value);
        }
    }
};

// Specialized overload for enums.
template <typename T>
class CacheKeySerializer<T, std::enable_if_t<std::is_enum_v<T>>> {
  public:
    static void Serialize(CacheKey* key, const T t) {
        CacheKeySerializer<std::underlying_type_t<T>>::Serialize(
            key, static_cast<std::underlying_type_t<T>>(t));
    }
};

// Specialized overload for TypedInteger.
template <typename Tag, typename Integer>
class CacheKeySerializer<::detail::TypedIntegerImpl<Tag, Integer>> {
  public:
    static void Serialize(CacheKey* key, const ::detail::TypedIntegerImpl<Tag, Integer> t) {
        CacheKeySerializer<Integer>::Serialize(key, static_cast<Integer>(t));
    }
};

// Specialized overload for pointers. Since we are serializing for a cache key, we always
// serialize via value, not by pointer. To handle nullptr scenarios, we always serialize whether
// the pointer was nullptr followed by the contents if applicable.
template <typename T>
class CacheKeySerializer<T, std::enable_if_t<std::is_pointer_v<T>>> {
  public:
    static void Serialize(CacheKey* key, const T t) {
        key->Record(t == nullptr);
        if (t != nullptr) {
            CacheKeySerializer<std::remove_cv_t<std::remove_pointer_t<T>>>::Serialize(key, *t);
        }
    }
};

// Specialized overload for fixed arrays of primitives.
template <typename T, size_t N>
class CacheKeySerializer<T[N], std::enable_if_t<std::is_fundamental_v<T>>> {
  public:
    static void Serialize(CacheKey* key, const T (&t)[N]) {
        static_assert(N > 0);
        key->Record(static_cast<size_t>(N));
        const char* it = reinterpret_cast<const char*>(t);
        key->insert(key->end(), it, it + sizeof(t));
    }
};

// Specialized overload for fixed arrays of non-primitives.
template <typename T, size_t N>
class CacheKeySerializer<T[N], std::enable_if_t<!std::is_fundamental_v<T>>> {
  public:
    static void Serialize(CacheKey* key, const T (&t)[N]) {
        static_assert(N > 0);
        key->Record(static_cast<size_t>(N));
        for (size_t i = 0; i < N; i++) {
            key->Record(t[i]);
        }
    }
};

// Specialized overload for CachedObjects.
template <typename T>
class CacheKeySerializer<T, std::enable_if_t<std::is_base_of_v<CachedObject, T>>> {
  public:
    static void Serialize(CacheKey* key, const T& t) { key->Record(t.GetCacheKey()); }
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CACHEKEY_H_
