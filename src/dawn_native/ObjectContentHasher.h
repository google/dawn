// Copyright 2020 The Dawn Authors
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

#ifndef DAWNNATIVE_OBJECT_CONTENT_HASHER_H_
#define DAWNNATIVE_OBJECT_CONTENT_HASHER_H_

#include "common/HashUtils.h"

#include <string>
#include <vector>

namespace dawn_native {

    // ObjectContentHasher records a hash that can be used as a key to lookup a cached object in a
    // cache.
    class ObjectContentHasher {
      public:
        // Record calls the appropriate record function based on the type.
        template <typename T, typename... Args>
        void Record(const T& value, const Args&... args) {
            RecordImpl<T, Args...>::Call(this, value, args...);
        }

        size_t GetContentHash() const;

      private:
        template <typename T, typename... Args>
        struct RecordImpl {
            static constexpr void Call(ObjectContentHasher* recorder,
                                       const T& value,
                                       const Args&... args) {
                HashCombine(&recorder->mContentHash, value, args...);
            }
        };

        template <typename T>
        struct RecordImpl<T*> {
            static constexpr void Call(ObjectContentHasher* recorder, T* obj) {
                // Calling Record(objPtr) is not allowed. This check exists to only prevent such
                // mistakes.
                static_assert(obj == nullptr, "");
            }
        };

        template <typename T>
        struct RecordImpl<std::vector<T>> {
            static constexpr void Call(ObjectContentHasher* recorder, const std::vector<T>& vec) {
                recorder->RecordIterable<std::vector<T>>(vec);
            }
        };

        template <typename IteratorT>
        constexpr void RecordIterable(const IteratorT& iterable) {
            for (auto it = iterable.begin(); it != iterable.end(); ++it) {
                Record(*it);
            }
        }

        size_t mContentHash = 0;
    };

    template <>
    struct ObjectContentHasher::RecordImpl<std::string> {
        static constexpr void Call(ObjectContentHasher* recorder, const std::string& str) {
            recorder->RecordIterable<std::string>(str);
        }
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_OBJECT_CONTENT_HASHER_H_
