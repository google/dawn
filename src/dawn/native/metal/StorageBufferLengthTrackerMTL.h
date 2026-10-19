// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_METAL_STORAGEBUFFERLENGTHTRACKERMTL_H_
#define SRC_DAWN_NATIVE_METAL_STORAGEBUFFERLENGTHTRACKERMTL_H_

#include <limits>

#include "src/dawn/native/Device.h"
#include "src/dawn/native/RenderPipeline.h"
#include "src/dawn/native/metal/ComputePipelineMTL.h"
#include "src/dawn/native/metal/Forward.h"
#include "src/dawn/native/metal/PipelineLayoutMTL.h"
#include "src/dawn/native/metal/RenderPipelineMTL.h"

namespace dawn::native::metal {

// Metal buffer pointers do not expose their length, so Tint receives storage buffer lengths
// through immediate data for arrayLength().
struct StorageBufferLengthTracker {
    StorageBufferLengthTracker() = delete;
    explicit StorageBufferLengthTracker(DeviceBase* device) {
        // Lengths are stored as uint32_t. Make sure that's OK for the device.
        DAWN_ASSERT(device->GetLimits().v1.maxBufferSize <= std::numeric_limits<uint32_t>::max());
    }

    wgpu::ShaderStage dirtyStages = wgpu::ShaderStage::None;

    // Tint expects one uint32_t length per possible buffer binding.
    static constexpr size_t MaxBufferCount = kGenericMetalBufferSlots;
    PerStage<std::array<uint32_t, MaxBufferCount>> data;
    // Number of bytes to upload for each shader stage.
    PerStage<uint32_t> dataSize{0u};

    // TODO(crbug.com/366291600): Remove this logic when merging
    // StorageBufferLengthTracker in ImmediateTracker.
    void OnSetPipeline(RenderPipelineBase* pipeline) {
        size_t immediateCount = pipeline->GetImmediateMask().count();
        if (immediateCount != mLastImmediateCounts) {
            mLastImmediateCounts = immediateCount;
            dirtyStages |= (wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment);
        }
    }

    void OnSetPipeline(ComputePipelineBase* pipeline) {
        size_t immediateCount = pipeline->GetImmediateMask().count();
        if (immediateCount != mLastImmediateCounts) {
            mLastImmediateCounts = immediateCount;
            dirtyStages |= wgpu::ShaderStage::Compute;
        }
    }

    // TODO(crbug.com/366291600): Remove unused storage buffer size in immediate block
    // to avoid uploading unused data.
    void Apply(RenderPipeline* pipeline, bool enableVertexPulling) {
        if (dirtyStages & wgpu::ShaderStage::Vertex) {
            uint32_t bufferCount =
                ToBackend(pipeline->GetLayout())->GetBufferBindingCount(SingleShaderStage::Vertex);

            if (enableVertexPulling) {
                bufferCount += uint32_t{pipeline->GetVertexBufferCount()};
            }

            DAWN_ASSERT(bufferCount <= data[SingleShaderStage::Vertex].size());
            dataSize[SingleShaderStage::Vertex] = sizeof(uint32_t) * bufferCount;
        }

        if (dirtyStages & wgpu::ShaderStage::Fragment) {
            uint32_t bufferCount = ToBackend(pipeline->GetLayout())
                                       ->GetBufferBindingCount(SingleShaderStage::Fragment);
            DAWN_ASSERT(bufferCount <= data[SingleShaderStage::Fragment].size());
            dataSize[SingleShaderStage::Fragment] = sizeof(uint32_t) * bufferCount;
        }
    }

    // TODO(crbug.com/366291600): Remove unused storage buffer size in immediate block
    // to avoid uploading unused data.
    void Apply(ComputePipeline* pipeline) {
        if (!(dirtyStages & wgpu::ShaderStage::Compute)) {
            return;
        }

        uint32_t bufferCount =
            ToBackend(pipeline->GetLayout())->GetBufferBindingCount(SingleShaderStage::Compute);
        DAWN_ASSERT(bufferCount <= data[SingleShaderStage::Compute].size());
        dataSize[SingleShaderStage::Compute] = sizeof(uint32_t) * bufferCount;
    }

    size_t mLastImmediateCounts = 0;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_STORAGEBUFFERLENGTHTRACKERMTL_H_
