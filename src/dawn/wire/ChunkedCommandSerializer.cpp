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

#include "dawn/wire/ChunkedCommandSerializer.h"

namespace dawn::wire {

ChunkedCommandSerializer::ChunkedCommandSerializer(CommandSerializer* serializer)
    : mSerializer(serializer), mMaxAllocationSize(serializer->GetMaximumAllocationSize()) {}

void ChunkedCommandSerializer::SerializeChunkedCommand(const char* allocatedBuffer,
                                                       size_t remainingSize) {
    while (remainingSize > 0) {
        size_t chunkSize = std::min(remainingSize, mMaxAllocationSize);
        void* dst = mSerializer->GetCmdSpace(chunkSize);
        if (dst == nullptr) {
            return;
        }
        memcpy(dst, allocatedBuffer, chunkSize);

        allocatedBuffer += chunkSize;
        remainingSize -= chunkSize;
    }
}

}  // namespace dawn::wire
