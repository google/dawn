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

#ifndef SRC_DAWN_NATIVE_METAL_BINDGROUPTRACKERMTL_H_
#define SRC_DAWN_NATIVE_METAL_BINDGROUPTRACKERMTL_H_

#include <utility>

#include "src/dawn/common/MatchVariant.h"
#include "src/dawn/native/BindGroupTracker.h"
#include "src/dawn/native/metal/BindGroupMTL.h"
#include "src/dawn/native/metal/BufferMTL.h"
#include "src/dawn/native/metal/SamplerMTL.h"
#include "src/dawn/native/metal/StorageBufferLengthTrackerMTL.h"
#include "src/dawn/native/metal/TextureMTL.h"

namespace dawn::native::metal {

// Keeps track of the dirty bind groups so they can be lazily applied when we know the
// pipeline state.
// Bind groups may be inherited because bind groups are packed in the buffer /
// texture tables in contiguous order.
class BindGroupTracker : public BindGroupTrackerBase<true> {
  public:
    BindGroupTracker(StorageBufferLengthTracker* lengthTracker, bool useArgumentBuffers)
        : BindGroupTrackerBase(),
          mLengthTracker(lengthTracker),
          mUseArgumentBuffers(useArgumentBuffers) {}

    template <typename Encoder>
    void Apply(Encoder encoder) {
        BeforeApply();

        for (BindGroupIndex index : mDirtyBindGroupsObjectChangedOrIsDynamic) {
            ApplyBindGroup(encoder, index, ToBackend(mBindGroups[index]), GetDynamicOffsets(index),
                           ToBackend(mPipeline->GetLayout()));
        }

        AfterApply();
    }

  private:
    // Handles a call to SetBindGroup, directing the commands to the correct encoder.
    // There is a single function that takes both encoders to factor code. Other approaches
    // like templates wouldn't work because the name of methods are different between the
    // two encoder types.
    void ApplyBindGroupImpl(id<MTLRenderCommandEncoder> render,
                            id<MTLComputeCommandEncoder> compute,
                            BindGroupIndex index,
                            BindGroup* group,
                            const ityp::span<BindingIndex, uint32_t>& dynamicOffsets,
                            PipelineLayout* pipelineLayout) {
        // TODO(crbug.com/dawn/854): Maintain buffers and offsets arrays in BindGroup
        // so that we only have to do one setVertexBuffers and one setFragmentBuffers
        // call here.
        for (BindingIndex bindingIndex : Range(group->GetLayout()->GetBindingCount())) {
            const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);

            bool hasVertStage =
                bindingInfo.visibility & wgpu::ShaderStage::Vertex && render != nullptr;
            bool hasFragStage =
                bindingInfo.visibility & wgpu::ShaderStage::Fragment && render != nullptr;
            bool hasComputeStage =
                bindingInfo.visibility & wgpu::ShaderStage::Compute && compute != nullptr;

            uint32_t vertIndex = 0;
            uint32_t fragIndex = 0;
            uint32_t computeIndex = 0;

            if (hasVertStage) {
                vertIndex = pipelineLayout->GetBindingIndexInfo(
                    SingleShaderStage::Vertex)[index][bindingIndex];
            }
            if (hasFragStage) {
                fragIndex = pipelineLayout->GetBindingIndexInfo(
                    SingleShaderStage::Fragment)[index][bindingIndex];
            }
            if (hasComputeStage) {
                computeIndex = pipelineLayout->GetBindingIndexInfo(
                    SingleShaderStage::Compute)[index][bindingIndex];
            }

            auto HandleTextureBinding = [&]() {
                auto textureView = ToBackend(group->GetBindingAsTextureView(bindingIndex));
                id<MTLTexture> texture = textureView->GetMTLTexture();
                if (!mUseArgumentBuffers) {
                    if (hasVertStage &&
                        mBoundTextures[SingleShaderStage::Vertex][vertIndex] != texture) {
                        mBoundTextures[SingleShaderStage::Vertex][vertIndex] = texture;
                        [render setVertexTexture:texture atIndex:vertIndex];
                    }
                    if (hasFragStage &&
                        mBoundTextures[SingleShaderStage::Fragment][fragIndex] != texture) {
                        mBoundTextures[SingleShaderStage::Fragment][fragIndex] = texture;
                        [render setFragmentTexture:texture atIndex:fragIndex];
                    }
                    if (hasComputeStage &&
                        mBoundTextures[SingleShaderStage::Compute][computeIndex] != texture) {
                        mBoundTextures[SingleShaderStage::Compute][computeIndex] = texture;
                        [compute setTexture:texture atIndex:computeIndex];
                    }
                }
            };

            MatchVariant(
                bindingInfo.bindingLayout,
                [&](const BufferBindingInfo& layout) {
                    const BufferBinding& binding = group->GetBindingAsBufferBinding(bindingIndex);
                    ToBackend(binding.buffer)->TrackUsage();

                    if (hasVertStage) {
                        mLengthTracker->data[SingleShaderStage::Vertex][vertIndex] =
                            checked_cast<uint32_t>(binding.size);
                        mLengthTracker->dirtyStages |= wgpu::ShaderStage::Vertex;
                    }
                    if (hasFragStage) {
                        mLengthTracker->data[SingleShaderStage::Fragment][fragIndex] =
                            checked_cast<uint32_t>(binding.size);
                        mLengthTracker->dirtyStages |= wgpu::ShaderStage::Fragment;
                    }
                    if (hasComputeStage) {
                        mLengthTracker->data[SingleShaderStage::Compute][computeIndex] =
                            checked_cast<uint32_t>(binding.size);
                        mLengthTracker->dirtyStages |= wgpu::ShaderStage::Compute;
                    }

                    const id<MTLBuffer> buffer = ToBackend(binding.buffer)->GetMTLBuffer();
                    if (!mUseArgumentBuffers) {
                        NSUInteger offset = binding.offset;

                        // TODO(crbug.com/dawn/854): Record bound buffer status to use
                        // setBufferOffset to achieve better performance.

                        // TODO(crbug.com/363031535): The dynamic offsets should come from the
                        // immediate buffer.
                        if (layout.hasDynamicOffset) {
                            // Dynamic buffers are packed at the front of BindingIndices.
                            offset += dynamicOffsets[bindingIndex];
                        }

                        if (hasVertStage) {
                            [render setVertexBuffers:&buffer
                                             offsets:&offset
                                           withRange:NSMakeRange(vertIndex, 1)];
                        }
                        if (hasFragStage) {
                            [render setFragmentBuffers:&buffer
                                               offsets:&offset
                                             withRange:NSMakeRange(fragIndex, 1)];
                        }
                        if (hasComputeStage) {
                            [compute setBuffers:&buffer
                                        offsets:&offset
                                      withRange:NSMakeRange(computeIndex, 1)];
                        }
                    }
                },
                [&](const SamplerBindingInfo&) {
                    auto sampler = ToBackend(group->GetBindingAsSampler(bindingIndex));
                    id<MTLSamplerState> samplerState = sampler->GetMTLSamplerState();
                    if (!mUseArgumentBuffers) {
                        if (hasVertStage &&
                            mBoundSamplers[SingleShaderStage::Vertex][vertIndex] != samplerState) {
                            mBoundSamplers[SingleShaderStage::Vertex][vertIndex] = samplerState;
                            [render setVertexSamplerState:samplerState atIndex:vertIndex];
                        }
                        if (hasFragStage &&
                            mBoundSamplers[SingleShaderStage::Fragment][fragIndex] !=
                                samplerState) {
                            mBoundSamplers[SingleShaderStage::Fragment][fragIndex] = samplerState;
                            [render setFragmentSamplerState:samplerState atIndex:fragIndex];
                        }
                        if (hasComputeStage &&
                            mBoundSamplers[SingleShaderStage::Compute][computeIndex] !=
                                samplerState) {
                            mBoundSamplers[SingleShaderStage::Compute][computeIndex] = samplerState;
                            [compute setSamplerState:sampler->GetMTLSamplerState()
                                             atIndex:computeIndex];
                        }
                    }
                },
                [&](const StaticSamplerBindingInfo&) {
                    // Static samplers are handled in the frontend.
                    // TODO(crbug.com/dawn/2482): Implement static samplers in the
                    // Metal backend.
                    DAWN_UNREACHABLE();
                },
                [&](const TextureBindingInfo&) { HandleTextureBinding(); },
                [&](const StorageTextureBindingInfo& info) { HandleTextureBinding(); },
                [&](const TexelBufferBindingInfo&) {
                    // Metal does not support texel buffers.
                    // TODO(crbug/382544164): Prototype texel buffer feature
                    DAWN_UNREACHABLE();
                },
                [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); },
                [](const ExternalTextureBindingInfo&) { DAWN_UNREACHABLE(); });
        }

        if (mUseArgumentBuffers) {
            const uint32_t dynamicOffsetsCount = uint32_t(dynamicOffsets.size());

            // TODO(crbug.com/363031535): The dynamic offsets should all be in a single grouping
            // which is in the immediates buffer.
            uint32_t argumentBufferIdx, dynamicOffsetsBufferIdx;
            std::tie(argumentBufferIdx, dynamicOffsetsBufferIdx) =
                BindGroupSlotToArgumentBufferID(index);

            if (render) {
                [render setVertexBuffer:*(group->GetArgumentBuffer())
                                 offset:0
                                atIndex:argumentBufferIdx];

                [render setFragmentBuffer:*(group->GetArgumentBuffer())
                                   offset:0
                                  atIndex:argumentBufferIdx];

                if (dynamicOffsetsCount > 0) {
                    [render setVertexBytes:dynamicOffsets.data()
                                    length:dynamicOffsetsCount * sizeof(uint32_t)
                                   atIndex:dynamicOffsetsBufferIdx];

                    [render setFragmentBytes:dynamicOffsets.data()
                                      length:dynamicOffsetsCount * sizeof(uint32_t)
                                     atIndex:dynamicOffsetsBufferIdx];
                }
            } else {
                DAWN_ASSERT(compute != nullptr);

                [compute setBuffer:*(group->GetArgumentBuffer())
                            offset:0
                           atIndex:argumentBufferIdx];

                if (dynamicOffsetsCount > 0) {
                    [compute setBytes:dynamicOffsets.data()
                               length:dynamicOffsetsCount * sizeof(uint32_t)
                              atIndex:dynamicOffsetsBufferIdx];
                }
            }
        }
    }

    template <typename... Args>
    void ApplyBindGroup(id<MTLRenderCommandEncoder> encoder, Args&&... args) {
        ApplyBindGroupImpl(encoder, nullptr, std::forward<Args&&>(args)...);
    }

    template <typename... Args>
    void ApplyBindGroup(id<MTLComputeCommandEncoder> encoder, Args&&... args) {
        ApplyBindGroupImpl(nullptr, encoder, std::forward<Args&&>(args)...);
    }

    raw_ptr<StorageBufferLengthTracker> mLengthTracker;

    // Keep track of current texture and sampler bindings to minimize state changes even when bind
    // groups are dirtied. It's safe to keep raw pointers here since if an entry is set here, the
    // texture/sampler is bound in Metal and the Metal runtime will keep them alive.
    PerStage<absl::flat_hash_map<uint32_t, id<MTLTexture>>> mBoundTextures;
    PerStage<absl::flat_hash_map<uint32_t, id<MTLSamplerState>>> mBoundSamplers;

    bool mUseArgumentBuffers = false;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_BINDGROUPTRACKERMTL_H_
