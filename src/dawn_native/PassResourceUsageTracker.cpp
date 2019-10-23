// Copyright 2019 The Dawn Authors
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

#include "dawn_native/PassResourceUsageTracker.h"

#include "dawn_native/Buffer.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    void PassResourceUsageTracker::BufferUsedAs(BufferBase* buffer, wgpu::BufferUsage usage) {
        // std::map's operator[] will create the key and return 0 if the key didn't exist
        // before.
        mBufferUsages[buffer] |= usage;
    }

    void PassResourceUsageTracker::TextureUsedAs(TextureBase* texture, wgpu::TextureUsage usage) {
        // std::map's operator[] will create the key and return 0 if the key didn't exist
        // before.
        mTextureUsages[texture] |= usage;
    }

    MaybeError PassResourceUsageTracker::ValidateComputePassUsages() const {
        return ValidateUsages();
    }

    MaybeError PassResourceUsageTracker::ValidateRenderPassUsages() const {
        return ValidateUsages();
    }

    // Performs the per-pass usage validation checks
    MaybeError PassResourceUsageTracker::ValidateUsages() const {
        // Buffers can only be used as single-write or multiple read.
        for (auto& it : mBufferUsages) {
            BufferBase* buffer = it.first;
            wgpu::BufferUsage usage = it.second;

            if (usage & ~buffer->GetUsage()) {
                return DAWN_VALIDATION_ERROR("Buffer missing usage for the pass");
            }

            bool readOnly = (usage & kReadOnlyBufferUsages) == usage;
            bool singleUse = wgpu::HasZeroOrOneBits(usage);

            if (!readOnly && !singleUse) {
                return DAWN_VALIDATION_ERROR(
                    "Buffer used as writable usage and another usage in pass");
            }
        }

        // Textures can only be used as single-write or multiple read.
        // TODO(cwallez@chromium.org): implement per-subresource tracking
        for (auto& it : mTextureUsages) {
            TextureBase* texture = it.first;
            wgpu::TextureUsage usage = it.second;

            if (usage & ~texture->GetUsage()) {
                return DAWN_VALIDATION_ERROR("Texture missing usage for the pass");
            }

            // For textures the only read-only usage in a pass is Sampled, so checking the
            // usage constraint simplifies to checking a single usage bit is set.
            if (!wgpu::HasZeroOrOneBits(it.second)) {
                return DAWN_VALIDATION_ERROR("Texture used with more than one usage in pass");
            }
        }

        return {};
    }

    // Returns the per-pass usage for use by backends for APIs with explicit barriers.
    PassResourceUsage PassResourceUsageTracker::AcquireResourceUsage() {
        PassResourceUsage result;
        result.buffers.reserve(mBufferUsages.size());
        result.bufferUsages.reserve(mBufferUsages.size());
        result.textures.reserve(mTextureUsages.size());
        result.textureUsages.reserve(mTextureUsages.size());

        for (auto& it : mBufferUsages) {
            result.buffers.push_back(it.first);
            result.bufferUsages.push_back(it.second);
        }

        for (auto& it : mTextureUsages) {
            result.textures.push_back(it.first);
            result.textureUsages.push_back(it.second);
        }

        return result;
    }

}  // namespace dawn_native
