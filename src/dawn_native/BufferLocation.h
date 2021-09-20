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

#ifndef DAWNNATIVE_BUFFERLOCATION_H_
#define DAWNNATIVE_BUFFERLOCATION_H_

#include "common/RefCounted.h"
#include "dawn_native/Buffer.h"

#include <cstdint>

namespace dawn_native {

    // A ref-counted wrapper around a Buffer ref and an offset into the buffer.
    class BufferLocation : public RefCounted {
      public:
        BufferLocation();
        BufferLocation(BufferBase* buffer, uint64_t offset = 0);
        ~BufferLocation();

        static Ref<BufferLocation> New();
        static Ref<BufferLocation> New(BufferBase* buffer, uint64_t offset = 0);

        bool IsNull() const;

        BufferBase* GetBuffer() const;
        uint64_t GetOffset() const;

        void Set(BufferBase* buffer, uint64_t offset);

      private:
        Ref<BufferBase> mBuffer;
        uint64_t mOffset = 0;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_BUFFERLOCATION_H_
