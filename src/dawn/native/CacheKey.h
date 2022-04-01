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

#ifndef DAWNNATIVE_CACHE_KEY_H_
#define DAWNNATIVE_CACHE_KEY_H_

#include <limits>
#include <string>
#include <type_traits>
#include <vector>

#include "dawn/common/Assert.h"

namespace dawn::native {

    // Forward declare CacheKey class because of co-dependency.
    class CacheKey;

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

    // Specialized overload for string literals. Note we drop the null-terminator.
    template <size_t N>
    class CacheKeySerializer<char[N]> {
      public:
        static void Serialize(CacheKey* key, const char (&t)[N]) {
            static_assert(N > 0);
            key->Record(static_cast<size_t>(N));
            key->insert(key->end(), t, t + N);
        }
    };

}  // namespace dawn::native

#endif  // DAWNNATIVE_CACHE_KEY_H_
