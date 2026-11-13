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

#ifndef SRC_DAWN_NATIVE_METAL_IMMEDIATESTRACKERMTL_H_
#define SRC_DAWN_NATIVE_METAL_IMMEDIATESTRACKERMTL_H_

#include "src/dawn/native/ImmediatesTracker.h"
#include "src/dawn/native/RenderPipeline.h"
#include "src/dawn/native/metal/ImmediatesLayoutMTL.h"
#include "src/dawn/native/metal/StorageBufferLengthTrackerMTL.h"

namespace dawn::native::metal {

class RenderImmediatesTracker
    : public UserImmediatesTrackerBase<RenderImmediates, RenderPipelineBase> {
  public:
    RenderImmediatesTracker() = default;

    void SetClampFragDepth(float minClampFragDepth, float maxClampFragDepth) {
        ClampFragDepthArgs fragDepthArgs;
        fragDepthArgs.minClampFragDepth = minClampFragDepth;
        fragDepthArgs.maxClampFragDepth = maxClampFragDepth;

        UpdateImmediates(offsetof(RenderImmediates, clampFragDepth), fragDepthArgs);
    }
};

using ComputeImmediatesTracker = UserImmediatesTrackerBase<ComputeImmediates, ComputePipelineBase>;

// Uploads immediates, both external and internal (including storage buffer lengths) through one
// Metal setBytes call per stage.
// TODO(crbug.com/366291600): Merge StorageBufferLength in ImmediateTracker
template <typename T, typename EncoderType>
class ImmediateTracker : public T {
  public:
    ImmediateTracker() = default;

    // Applies immediates and buffer length data to the Metal command encoder.
    // This method uploads both immediate values and storage buffer lengths
    // to a single buffer, with buffer lengths appended after immediates.
    // The data is uploaded using setVertexBytes/setFragmentBytes for render passes
    // or setBytes for compute passes.
    void Apply(EncoderType encoder, StorageBufferLengthTracker* lengthTracker) {
        DAWN_ASSERT(this->mLastPipeline != nullptr);

        // Update the stored immediates that have changed
        ImmediateMask pipelineMask = this->mLastPipeline->GetImmediateMask();
        ImmediateMask uploadBits = this->mDirty & pipelineMask;
        constexpr wgpu::ShaderStage stages =
            kIsRenderImmediates ? wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment
                                : wgpu::ShaderStage::Compute;
        for (auto [offset, size] : IterateRanges(uploadBits)) {
            uint32_t immediateContentStartOffset =
                static_cast<uint32_t>(offset) * kImmediateElementByteSize;
            size_t immediateRangeStartOffset =
                GetImmediateIndexInPipeline(static_cast<uint32_t>(offset), pipelineMask);
            WriteImmediateBlocks(stages, immediateRangeStartOffset * kImmediateElementByteSize,
                                 this->mContent.GetDataBytes(immediateContentStartOffset,
                                                             size * kImmediateElementByteSize));
        }

        uint32_t bufferSizeByteOffset = GetImmediateBufferSizesByteOffset(pipelineMask);

        // Update storage buffer length data that are needed and changed.
        for (auto stage : IterateStages(lengthTracker->dirtyStages)) {
            // Sizes must be > 0, otherwise we'll do min(index, bufferSize - 1) and underflow.
            WriteImmediateBlocks(
                StageBit(stage), bufferSizeByteOffset,
                ByteSpanFromRef(lengthTracker->data[stage]).first(lengthTracker->dataSize[stage]));
        }

        for (auto stage : IterateStages(stages)) {
            dirtySize[stage] =
                Align(bufferSizeByteOffset + lengthTracker->dataSize[stage], kSetBytesAlignment);
        }

        // Reset StorageBufferLengthTracker dirty stages
        lengthTracker->dirtyStages = wgpu::ShaderStage::None;

        UploadImmediates(encoder);

        // Reset all dirty bits after uploading.
        this->mDirty.reset();
    }

  private:
    static constexpr bool kIsRenderImmediates = std::is_same_v<T, RenderImmediatesTracker>;
    static constexpr bool kIsComputeImmediates = std::is_same_v<T, ComputeImmediatesTracker>;

    // Round up to 16 bytes so the setBytes length covers the size of the constant-buffer struct
    // argument, whose size is rounded up to its largest member alignment (at most 16 bytes, for
    // vec4-class members).
    static constexpr size_t kSetBytesAlignment = 16;

    static constexpr size_t MaxBufferCount = StorageBufferLengthTracker::MaxBufferCount;
    static constexpr size_t kImmediateStructU32 = kIsRenderImmediates
                                                      ? sizeof(RenderImmediates) / sizeof(uint32_t)
                                                      : sizeof(UserImmediates) / sizeof(uint32_t);
    static constexpr size_t kMaxImmediateBlockSize =
        Align(kImmediateStructU32 + MaxBufferCount, kSetBytesAlignment / kImmediateElementByteSize);

    // Writes data to the immediate block content for the specified shader stages.
    // This is used for both immediates and storage buffer length data.
    void WriteImmediateBlocks(wgpu::ShaderStage stages,
                              size_t byteOffset,
                              Span<const std::byte> data) {
        // Copy data to all affected shader stages.
        for (auto stage : IterateStages(stages)) {
            ByteSpanFromRef(mImmediateBlockContent[stage])
                .subspan(byteOffset, data.size())
                .CopyFrom(data);
        }
        dirtyStages |= stages;
    }

    // Uploads the immediate block content to the Metal render encoder.
    void UploadImmediates(id<MTLRenderCommandEncoder> renderEncoder) {
        if (dirtyStages & wgpu::ShaderStage::Vertex) {
            [renderEncoder setVertexBytes:mImmediateBlockContent[SingleShaderStage::Vertex].data()
                                   length:dirtySize[SingleShaderStage::Vertex]
                                  atIndex:kImmediateBlockBufferSlot];
        }

        if (dirtyStages & wgpu::ShaderStage::Fragment) {
            [renderEncoder
                setFragmentBytes:mImmediateBlockContent[SingleShaderStage::Fragment].data()
                          length:dirtySize[SingleShaderStage::Fragment]
                         atIndex:kImmediateBlockBufferSlot];
        }

        // Reset dirty stage.
        dirtyStages = wgpu::ShaderStage::None;
    }

    // Uploads the immediate block content to the Metal compute encoder.
    void UploadImmediates(id<MTLComputeCommandEncoder> computeEncoder) {
        if (dirtyStages == wgpu::ShaderStage::None) {
            return;
        }

        [computeEncoder setBytes:mImmediateBlockContent[SingleShaderStage::Compute].data()
                          length:dirtySize[SingleShaderStage::Compute]
                         atIndex:kImmediateBlockBufferSlot];

        // Reset dirty stage.
        dirtyStages = wgpu::ShaderStage::None;
    }

    // Per-stage storage for the immediate block content.
    // Each stage maintains its own copy of the data combining immediates
    // and buffer length information in a single contiguous block.
    PerStage<std::array<uint32_t, kMaxImmediateBlockSize>> mImmediateBlockContent;
    // Size of data to upload for each shader stage (in bytes)
    PerStage<size_t> dirtySize;
    // Tracks which shader stages have dirty data that needs to be uploaded
    wgpu::ShaderStage dirtyStages = wgpu::ShaderStage::None;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_IMMEDIATESTRACKERMTL_H_
