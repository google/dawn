// Copyright 2017 The Dawn Authors
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

#include "dawn/utils/TerribleCommandBuffer.h"

#include "dawn/common/Assert.h"

namespace utils {

TerribleCommandBuffer::TerribleCommandBuffer() {}

TerribleCommandBuffer::TerribleCommandBuffer(dawn::wire::CommandHandler* handler)
    : mHandler(handler) {}

void TerribleCommandBuffer::SetHandler(dawn::wire::CommandHandler* handler) {
    mHandler = handler;
}

size_t TerribleCommandBuffer::GetMaximumAllocationSize() const {
    return sizeof(mBuffer);
}

void* TerribleCommandBuffer::GetCmdSpace(size_t size) {
    // Note: This returns non-null even if size is zero.
    if (size > sizeof(mBuffer)) {
        return nullptr;
    }
    char* result = &mBuffer[mOffset];
    if (sizeof(mBuffer) - size < mOffset) {
        if (!Flush()) {
            return nullptr;
        }
        return GetCmdSpace(size);
    }

    mOffset += size;
    return result;
}

bool TerribleCommandBuffer::Flush() {
    bool success = mHandler->HandleCommands(mBuffer, mOffset) != nullptr;
    mOffset = 0;
    return success;
}

}  // namespace utils
