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

#include <string>
#include <vector>

#include "dawn/common/Compiler.h"

namespace dawn::native {

    using CacheKey = std::vector<uint8_t>;

    // Overridable serializer struct that should be implemented for cache key serializable
    // types/classes.
    template <typename T, typename SFINAE = void>
    struct CacheKeySerializer {
        static void Serialize(CacheKey* key, const T& t);
    };

    // Specialized overload for integral types. Note that we are currently serializing as a string
    // to avoid handling null termiantors.
    template <typename Integer>
    struct CacheKeySerializer<Integer, std::enable_if_t<std::is_integral_v<Integer>>> {
        static void Serialize(CacheKey* key, const Integer i) {
            std::string str = std::to_string(i);
            key->insert(key->end(), str.begin(), str.end());
        }
    };

    // Specialized overload for floating point types. Note that we are currently serializing as a
    // string to avoid handling null termiantors.
    template <typename Float>
    struct CacheKeySerializer<Float, std::enable_if_t<std::is_floating_point_v<Float>>> {
        static void Serialize(CacheKey* key, const Float f) {
            std::string str = std::to_string(f);
            key->insert(key->end(), str.begin(), str.end());
        }
    };

    // Specialized overload for string literals. Note we drop the null-terminator.
    template <size_t N>
    struct CacheKeySerializer<char[N]> {
        static void Serialize(CacheKey* key, const char (&t)[N]) {
            std::string len = std::to_string(N - 1);
            key->insert(key->end(), len.begin(), len.end());
            key->push_back('"');
            key->insert(key->end(), t, t + N - 1);
            key->push_back('"');
        }
    };

    // Helper template function that defers to underlying static functions.
    template <typename T>
    void SerializeInto(CacheKey* key, const T& t) {
        CacheKeySerializer<T>::Serialize(key, t);
    }

    // Given list of arguments of types with a free implementation of SerializeIntoImpl in the
    // dawn::native namespace, serializes each argument and appends them to the CacheKey while
    // prepending member ids before each argument.
    template <typename... Ts>
    CacheKey GetCacheKey(const Ts&... inputs) {
        CacheKey key;
        key.push_back('{');
        int memberId = 0;
        auto Serialize = [&](const auto& input) {
            std::string memberIdStr = (memberId == 0 ? "" : ",") + std::to_string(memberId) + ":";
            key.insert(key.end(), memberIdStr.begin(), memberIdStr.end());
            SerializeInto(&key, input);
            memberId++;
        };
        (Serialize(inputs), ...);
        key.push_back('}');
        return key;
    }

}  // namespace dawn::native

#endif  // DAWNNATIVE_CACHE_KEY_H_
