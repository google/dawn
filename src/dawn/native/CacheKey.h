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

#include <utility>

#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {

class CacheKey : public stream::ByteVectorSink {
  public:
    using stream::ByteVectorSink::ByteVectorSink;

    enum class Type { ComputePipeline, RenderPipeline, Shader };

    template <typename T>
    class UnsafeUnkeyedValue {
      public:
        UnsafeUnkeyedValue() = default;
        // NOLINTNEXTLINE(runtime/explicit) allow implicit construction to decrease verbosity
        UnsafeUnkeyedValue(T&& value) : mValue(std::forward<T>(value)) {}

        const T& UnsafeGetValue() const { return mValue; }

        // Friend definition of StreamIn which can be found by ADL to override
        // stream::StreamIn<T>.
        friend constexpr void StreamIn(stream::Sink*, const UnsafeUnkeyedValue&) {}

      private:
        T mValue;
    };
};

template <typename T>
CacheKey::UnsafeUnkeyedValue<T> UnsafeUnkeyedValue(T&& value) {
    return CacheKey::UnsafeUnkeyedValue<T>(std::forward<T>(value));
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_CACHEKEY_H_
