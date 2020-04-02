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

#include "dawn_native/metal/CommandBufferMTL.h"

#include "dawn_native/BindGroupTracker.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/Commands.h"
#include "dawn_native/RenderBundle.h"
#include "dawn_native/metal/BindGroupMTL.h"
#include "dawn_native/metal/BufferMTL.h"
#include "dawn_native/metal/ComputePipelineMTL.h"
#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/PipelineLayoutMTL.h"
#include "dawn_native/metal/RenderPipelineMTL.h"
#include "dawn_native/metal/SamplerMTL.h"
#include "dawn_native/metal/TextureMTL.h"

namespace dawn_native { namespace metal {

    namespace {

        // Allows this file to use MTLStoreActionStoreAndMultismapleResolve because the logic is
        // first to compute what the "best" Metal render pass descriptor is, then fix it up if we
        // are not on macOS 10.12 (i.e. the EmulateStoreAndMSAAResolve toggle is on).
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability"
        constexpr MTLStoreAction kMTLStoreActionStoreAndMultisampleResolve =
            MTLStoreActionStoreAndMultisampleResolve;
#pragma clang diagnostic pop

        // Creates an autoreleased MTLRenderPassDescriptor matching desc
        MTLRenderPassDescriptor* CreateMTLRenderPassDescriptor(BeginRenderPassCmd* renderPass) {
            MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];

            for (uint32_t i :
                 IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
                auto& attachmentInfo = renderPass->colorAttachments[i];

                switch (attachmentInfo.loadOp) {
                    case wgpu::LoadOp::Clear:
                        descriptor.colorAttachments[i].loadAction = MTLLoadActionClear;
                        descriptor.colorAttachments[i].clearColor = MTLClearColorMake(
                            attachmentInfo.clearColor.r, attachmentInfo.clearColor.g,
                            attachmentInfo.clearColor.b, attachmentInfo.clearColor.a);
                        break;

                    case wgpu::LoadOp::Load:
                        descriptor.colorAttachments[i].loadAction = MTLLoadActionLoad;
                        break;

                    default:
                        UNREACHABLE();
                        break;
                }

                descriptor.colorAttachments[i].texture =
                    ToBackend(attachmentInfo.view->GetTexture())->GetMTLTexture();
                descriptor.colorAttachments[i].level = attachmentInfo.view->GetBaseMipLevel();
                descriptor.colorAttachments[i].slice = attachmentInfo.view->GetBaseArrayLayer();

                bool hasResolveTarget = attachmentInfo.resolveTarget.Get() != nullptr;

                switch (attachmentInfo.storeOp) {
                    case wgpu::StoreOp::Store:
                        if (hasResolveTarget) {
                            descriptor.colorAttachments[i].resolveTexture =
                                ToBackend(attachmentInfo.resolveTarget->GetTexture())
                                    ->GetMTLTexture();
                            descriptor.colorAttachments[i].resolveLevel =
                                attachmentInfo.resolveTarget->GetBaseMipLevel();
                            descriptor.colorAttachments[i].resolveSlice =
                                attachmentInfo.resolveTarget->GetBaseArrayLayer();
                            descriptor.colorAttachments[i].storeAction =
                                kMTLStoreActionStoreAndMultisampleResolve;
                        } else {
                            descriptor.colorAttachments[i].storeAction = MTLStoreActionStore;
                        }
                        break;

                    case wgpu::StoreOp::Clear:
                        descriptor.colorAttachments[i].storeAction = MTLStoreActionDontCare;
                        break;

                    default:
                        UNREACHABLE();
                        break;
                }
            }

            if (renderPass->attachmentState->HasDepthStencilAttachment()) {
                auto& attachmentInfo = renderPass->depthStencilAttachment;

                // TODO(jiawei.shao@intel.com): support rendering into a layer of a texture.
                id<MTLTexture> texture =
                    ToBackend(attachmentInfo.view->GetTexture())->GetMTLTexture();
                const Format& format = attachmentInfo.view->GetTexture()->GetFormat();

                if (format.HasDepth()) {
                    descriptor.depthAttachment.texture = texture;

                    switch (attachmentInfo.depthStoreOp) {
                        case wgpu::StoreOp::Store:
                            descriptor.depthAttachment.storeAction = MTLStoreActionStore;
                            break;

                        case wgpu::StoreOp::Clear:
                            descriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
                            break;

                        default:
                            UNREACHABLE();
                            break;
                    }

                    switch (attachmentInfo.depthLoadOp) {
                        case wgpu::LoadOp::Clear:
                            descriptor.depthAttachment.loadAction = MTLLoadActionClear;
                            descriptor.depthAttachment.clearDepth = attachmentInfo.clearDepth;
                            break;

                        case wgpu::LoadOp::Load:
                            descriptor.depthAttachment.loadAction = MTLLoadActionLoad;
                            break;

                        default:
                            UNREACHABLE();
                            break;
                    }
                }

                if (format.HasStencil()) {
                    descriptor.stencilAttachment.texture = texture;

                    switch (attachmentInfo.stencilStoreOp) {
                        case wgpu::StoreOp::Store:
                            descriptor.stencilAttachment.storeAction = MTLStoreActionStore;
                            break;

                        case wgpu::StoreOp::Clear:
                            descriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
                            break;

                        default:
                            UNREACHABLE();
                            break;
                    }

                    switch (attachmentInfo.stencilLoadOp) {
                        case wgpu::LoadOp::Clear:
                            descriptor.stencilAttachment.loadAction = MTLLoadActionClear;
                            descriptor.stencilAttachment.clearStencil = attachmentInfo.clearStencil;
                            break;

                        case wgpu::LoadOp::Load:
                            descriptor.stencilAttachment.loadAction = MTLLoadActionLoad;
                            break;

                        default:
                            UNREACHABLE();
                            break;
                    }
                }
            }

            return descriptor;
        }

        // Helper function for Toggle EmulateStoreAndMSAAResolve
        void ResolveInAnotherRenderPass(
            CommandRecordingContext* commandContext,
            const MTLRenderPassDescriptor* mtlRenderPass,
            const std::array<id<MTLTexture>, kMaxColorAttachments>& resolveTextures) {
            MTLRenderPassDescriptor* mtlRenderPassForResolve =
                [MTLRenderPassDescriptor renderPassDescriptor];
            for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
                if (resolveTextures[i] == nil) {
                    continue;
                }

                mtlRenderPassForResolve.colorAttachments[i].texture =
                    mtlRenderPass.colorAttachments[i].texture;
                mtlRenderPassForResolve.colorAttachments[i].loadAction = MTLLoadActionLoad;
                mtlRenderPassForResolve.colorAttachments[i].storeAction =
                    MTLStoreActionMultisampleResolve;
                mtlRenderPassForResolve.colorAttachments[i].resolveTexture = resolveTextures[i];
                mtlRenderPassForResolve.colorAttachments[i].resolveLevel =
                    mtlRenderPass.colorAttachments[i].resolveLevel;
                mtlRenderPassForResolve.colorAttachments[i].resolveSlice =
                    mtlRenderPass.colorAttachments[i].resolveSlice;
            }

            commandContext->BeginRender(mtlRenderPassForResolve);
            commandContext->EndRender();
        }

        // Helper functions for Toggle AlwaysResolveIntoZeroLevelAndLayer
        id<MTLTexture> CreateResolveTextureForWorkaround(Device* device,
                                                         MTLPixelFormat mtlFormat,
                                                         uint32_t width,
                                                         uint32_t height) {
            MTLTextureDescriptor* mtlDesc = [MTLTextureDescriptor new];
            mtlDesc.textureType = MTLTextureType2D;
            mtlDesc.usage = MTLTextureUsageRenderTarget;
            mtlDesc.pixelFormat = mtlFormat;
            mtlDesc.width = width;
            mtlDesc.height = height;
            mtlDesc.depth = 1;
            mtlDesc.mipmapLevelCount = 1;
            mtlDesc.arrayLength = 1;
            mtlDesc.storageMode = MTLStorageModePrivate;
            mtlDesc.sampleCount = 1;
            id<MTLTexture> resolveTexture =
                [device->GetMTLDevice() newTextureWithDescriptor:mtlDesc];
            [mtlDesc release];
            return resolveTexture;
        }

        void CopyIntoTrueResolveTarget(CommandRecordingContext* commandContext,
                                       id<MTLTexture> mtlTrueResolveTexture,
                                       uint32_t trueResolveLevel,
                                       uint32_t trueResolveSlice,
                                       id<MTLTexture> temporaryResolveTexture,
                                       uint32_t width,
                                       uint32_t height) {
            [commandContext->EnsureBlit() copyFromTexture:temporaryResolveTexture
                                              sourceSlice:0
                                              sourceLevel:0
                                             sourceOrigin:MTLOriginMake(0, 0, 0)
                                               sourceSize:MTLSizeMake(width, height, 1)
                                                toTexture:mtlTrueResolveTexture
                                         destinationSlice:trueResolveSlice
                                         destinationLevel:trueResolveLevel
                                        destinationOrigin:MTLOriginMake(0, 0, 0)];
        }

        // Metal uses a physical addressing mode which means buffers in the shading language are
        // just pointers to the virtual address of their start. This means there is no way to know
        // the length of a buffer to compute the length() of unsized arrays at the end of storage
        // buffers. SPIRV-Cross implements the length() of unsized arrays by requiring an extra
        // buffer that contains the length of other buffers. This structure that keeps track of the
        // length of storage buffers and can apply them to the reserved "buffer length buffer" when
        // needed for a draw or a dispatch.
        struct StorageBufferLengthTracker {
            wgpu::ShaderStage dirtyStages = wgpu::ShaderStage::None;

            // The lengths of buffers are stored as 32bit integers because that is the width the
            // MSL code generated by SPIRV-Cross expects.
            PerStage<std::array<uint32_t, kGenericMetalBufferSlots>> data;

            void Apply(id<MTLRenderCommandEncoder> render, RenderPipeline* pipeline) {
                wgpu::ShaderStage stagesToApply =
                    dirtyStages & pipeline->GetStagesRequiringStorageBufferLength();

                if (stagesToApply == wgpu::ShaderStage::None) {
                    return;
                }

                if (stagesToApply & wgpu::ShaderStage::Vertex) {
                    uint32_t bufferCount = ToBackend(pipeline->GetLayout())
                                               ->GetBufferBindingCount(SingleShaderStage::Vertex);
                    [render setVertexBytes:data[SingleShaderStage::Vertex].data()
                                    length:sizeof(uint32_t) * bufferCount
                                   atIndex:kBufferLengthBufferSlot];
                }

                if (stagesToApply & wgpu::ShaderStage::Fragment) {
                    uint32_t bufferCount = ToBackend(pipeline->GetLayout())
                                               ->GetBufferBindingCount(SingleShaderStage::Fragment);
                    [render setFragmentBytes:data[SingleShaderStage::Fragment].data()
                                      length:sizeof(uint32_t) * bufferCount
                                     atIndex:kBufferLengthBufferSlot];
                }

                // Only mark clean stages that were actually applied.
                dirtyStages ^= stagesToApply;
            }

            void Apply(id<MTLComputeCommandEncoder> compute, ComputePipeline* pipeline) {
                if (!(dirtyStages & wgpu::ShaderStage::Compute)) {
                    return;
                }

                if (!pipeline->RequiresStorageBufferLength()) {
                    return;
                }

                uint32_t bufferCount = ToBackend(pipeline->GetLayout())
                                           ->GetBufferBindingCount(SingleShaderStage::Compute);
                [compute setBytes:data[SingleShaderStage::Compute].data()
                           length:sizeof(uint32_t) * bufferCount
                          atIndex:kBufferLengthBufferSlot];

                dirtyStages ^= wgpu::ShaderStage::Compute;
            }
        };

        struct TextureBufferCopySplit {
            static constexpr uint32_t kMaxTextureBufferCopyRegions = 3;

            struct CopyInfo {
                NSUInteger bufferOffset;
                NSUInteger bytesPerRow;
                NSUInteger bytesPerImage;
                MTLOrigin textureOrigin;
                MTLSize copyExtent;
            };

            uint32_t count = 0;
            std::array<CopyInfo, kMaxTextureBufferCopyRegions> copies;
        };

        MTLOrigin MakeMTLOrigin(Origin3D origin) {
            return MTLOriginMake(origin.x, origin.y, origin.z);
        }

        MTLSize MakeMTLSize(Extent3D extent) {
            return MTLSizeMake(extent.width, extent.height, extent.depth);
        }

        TextureBufferCopySplit ComputeTextureBufferCopySplit(Origin3D origin,
                                                             Extent3D copyExtent,
                                                             Format textureFormat,
                                                             Extent3D virtualSizeAtLevel,
                                                             uint64_t bufferSize,
                                                             uint64_t bufferOffset,
                                                             uint32_t rowPitch,
                                                             uint32_t imageHeight) {
            TextureBufferCopySplit copy;

            // When copying textures from/to an unpacked buffer, the Metal validation layer doesn't
            // compute the correct range when checking if the buffer is big enough to contain the
            // data for the whole copy. Instead of looking at the position of the last texel in the
            // buffer, it computes the volume of the 3D box with rowPitch * (imageHeight /
            // format.blockHeight) * copySize.depth. For example considering the pixel buffer below
            // where in memory, each row data (D) of the texture is followed by some padding data
            // (P):
            //     |DDDDDDD|PP|
            //     |DDDDDDD|PP|
            //     |DDDDDDD|PP|
            //     |DDDDDDD|PP|
            //     |DDDDDDA|PP|
            // The last pixel read will be A, but the driver will think it is the whole last padding
            // row, causing it to generate an error when the pixel buffer is just big enough.

            // We work around this limitation by detecting when Metal would complain and copy the
            // last image and row separately using tight sourceBytesPerRow or sourceBytesPerImage.
            uint32_t rowPitchCountPerImage = imageHeight / textureFormat.blockHeight;
            uint32_t bytesPerImage = rowPitch * rowPitchCountPerImage;

            // Metal validation layer requires that if the texture's pixel format is a compressed
            // format, the sourceSize must be a multiple of the pixel format's block size or be
            // clamped to the edge of the texture if the block extends outside the bounds of a
            // texture.
            uint32_t clampedCopyExtentWidth =
                (origin.x + copyExtent.width > virtualSizeAtLevel.width)
                    ? (virtualSizeAtLevel.width - origin.x)
                    : copyExtent.width;
            uint32_t clampedCopyExtentHeight =
                (origin.y + copyExtent.height > virtualSizeAtLevel.height)
                    ? (virtualSizeAtLevel.height - origin.y)
                    : copyExtent.height;

            // Check whether buffer size is big enough.
            bool needWorkaround = bufferSize - bufferOffset < bytesPerImage * copyExtent.depth;
            if (!needWorkaround) {
                copy.count = 1;
                copy.copies[0].bufferOffset = bufferOffset;
                copy.copies[0].bytesPerRow = rowPitch;
                copy.copies[0].bytesPerImage = bytesPerImage;
                copy.copies[0].textureOrigin = MakeMTLOrigin(origin);
                copy.copies[0].copyExtent =
                    MTLSizeMake(clampedCopyExtentWidth, clampedCopyExtentHeight, copyExtent.depth);
                return copy;
            }

            uint64_t currentOffset = bufferOffset;

            // Doing all the copy except the last image.
            if (copyExtent.depth > 1) {
                copy.copies[copy.count].bufferOffset = currentOffset;
                copy.copies[copy.count].bytesPerRow = rowPitch;
                copy.copies[copy.count].bytesPerImage = bytesPerImage;
                copy.copies[copy.count].textureOrigin = MakeMTLOrigin(origin);
                copy.copies[copy.count].copyExtent = MTLSizeMake(
                    clampedCopyExtentWidth, clampedCopyExtentHeight, copyExtent.depth - 1);

                ++copy.count;

                // Update offset to copy to the last image.
                currentOffset += (copyExtent.depth - 1) * bytesPerImage;
            }

            // Doing all the copy in last image except the last row.
            uint32_t copyBlockRowCount = copyExtent.height / textureFormat.blockHeight;
            if (copyBlockRowCount > 1) {
                copy.copies[copy.count].bufferOffset = currentOffset;
                copy.copies[copy.count].bytesPerRow = rowPitch;
                copy.copies[copy.count].bytesPerImage = rowPitch * (copyBlockRowCount - 1);
                copy.copies[copy.count].textureOrigin =
                    MTLOriginMake(origin.x, origin.y, origin.z + copyExtent.depth - 1);

                ASSERT(copyExtent.height - textureFormat.blockHeight < virtualSizeAtLevel.height);
                copy.copies[copy.count].copyExtent = MTLSizeMake(
                    clampedCopyExtentWidth, copyExtent.height - textureFormat.blockHeight, 1);

                ++copy.count;

                // Update offset to copy to the last row.
                currentOffset += (copyBlockRowCount - 1) * rowPitch;
            }

            // Doing the last row copy with the exact number of bytes in last row.
            // Workaround this issue in a way just like the copy to a 1D texture.
            uint32_t lastRowDataSize =
                (copyExtent.width / textureFormat.blockWidth) * textureFormat.blockByteSize;
            uint32_t lastRowCopyExtentHeight =
                textureFormat.blockHeight + clampedCopyExtentHeight - copyExtent.height;
            ASSERT(lastRowCopyExtentHeight <= textureFormat.blockHeight);

            copy.copies[copy.count].bufferOffset = currentOffset;
            copy.copies[copy.count].bytesPerRow = lastRowDataSize;
            copy.copies[copy.count].bytesPerImage = lastRowDataSize;
            copy.copies[copy.count].textureOrigin =
                MTLOriginMake(origin.x, origin.y + copyExtent.height - textureFormat.blockHeight,
                              origin.z + copyExtent.depth - 1);
            copy.copies[copy.count].copyExtent =
                MTLSizeMake(clampedCopyExtentWidth, lastRowCopyExtentHeight, 1);
            ++copy.count;

            return copy;
        }

        void EnsureSourceTextureInitialized(Texture* texture,
                                            const Extent3D& size,
                                            const TextureCopy& src) {
            // TODO(crbug.com/dawn/145): Specify multiple layers based on |size|
            texture->EnsureSubresourceContentInitialized(src.mipLevel, 1, src.arrayLayer, 1);
        }

        void EnsureDestinationTextureInitialized(Texture* texture,
                                                 const Extent3D& size,
                                                 const TextureCopy& dst) {
            // TODO(crbug.com/dawn/145): Specify multiple layers based on |size|
            if (IsCompleteSubresourceCopiedTo(texture, size, dst.mipLevel)) {
                texture->SetIsSubresourceContentInitialized(true, dst.mipLevel, 1, dst.arrayLayer,
                                                            1);
            } else {
                texture->EnsureSubresourceContentInitialized(dst.mipLevel, 1, dst.arrayLayer, 1);
            }
        }

        // Keeps track of the dirty bind groups so they can be lazily applied when we know the
        // pipeline state.
        // Bind groups may be inherited because bind groups are packed in the buffer /
        // texture tables in contiguous order.
        class BindGroupTracker : public BindGroupTrackerBase<true, uint64_t> {
          public:
            explicit BindGroupTracker(StorageBufferLengthTracker* lengthTracker)
                : BindGroupTrackerBase(), mLengthTracker(lengthTracker) {
            }

            template <typename Encoder>
            void Apply(Encoder encoder) {
                for (uint32_t index : IterateBitSet(mDirtyBindGroupsObjectChangedOrIsDynamic)) {
                    ApplyBindGroup(encoder, index, ToBackend(mBindGroups[index]),
                                   mDynamicOffsetCounts[index], mDynamicOffsets[index].data(),
                                   ToBackend(mPipelineLayout));
                }
                DidApply();
            }

          private:
            // Handles a call to SetBindGroup, directing the commands to the correct encoder.
            // There is a single function that takes both encoders to factor code. Other approaches
            // like templates wouldn't work because the name of methods are different between the
            // two encoder types.
            void ApplyBindGroupImpl(id<MTLRenderCommandEncoder> render,
                                    id<MTLComputeCommandEncoder> compute,
                                    uint32_t index,
                                    BindGroup* group,
                                    uint32_t dynamicOffsetCount,
                                    uint64_t* dynamicOffsets,
                                    PipelineLayout* pipelineLayout) {
                uint32_t currentDynamicBufferIndex = 0;

                // TODO(kainino@chromium.org): Maintain buffers and offsets arrays in BindGroup
                // so that we only have to do one setVertexBuffers and one setFragmentBuffers
                // call here.
                for (BindingIndex bindingIndex = 0;
                     bindingIndex < group->GetLayout()->GetBindingCount(); ++bindingIndex) {
                    const BindingInfo& bindingInfo =
                        group->GetLayout()->GetBindingInfo(bindingIndex);

                    bool hasVertStage =
                        bindingInfo.visibility & wgpu::ShaderStage::Vertex && render != nil;
                    bool hasFragStage =
                        bindingInfo.visibility & wgpu::ShaderStage::Fragment && render != nil;
                    bool hasComputeStage =
                        bindingInfo.visibility & wgpu::ShaderStage::Compute && compute != nil;

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

                    switch (bindingInfo.type) {
                        case wgpu::BindingType::UniformBuffer:
                        case wgpu::BindingType::StorageBuffer:
                        case wgpu::BindingType::ReadonlyStorageBuffer: {
                            const BufferBinding& binding =
                                group->GetBindingAsBufferBinding(bindingIndex);
                            const id<MTLBuffer> buffer = ToBackend(binding.buffer)->GetMTLBuffer();
                            NSUInteger offset = binding.offset;

                            // TODO(shaobo.yan@intel.com): Record bound buffer status to use
                            // setBufferOffset to achieve better performance.
                            if (bindingInfo.hasDynamicOffset) {
                                offset += dynamicOffsets[currentDynamicBufferIndex];
                                currentDynamicBufferIndex++;
                            }

                            if (hasVertStage) {
                                mLengthTracker->data[SingleShaderStage::Vertex][vertIndex] =
                                    binding.size;
                                mLengthTracker->dirtyStages |= wgpu::ShaderStage::Vertex;
                                [render setVertexBuffers:&buffer
                                                 offsets:&offset
                                               withRange:NSMakeRange(vertIndex, 1)];
                            }
                            if (hasFragStage) {
                                mLengthTracker->data[SingleShaderStage::Fragment][fragIndex] =
                                    binding.size;
                                mLengthTracker->dirtyStages |= wgpu::ShaderStage::Fragment;
                                [render setFragmentBuffers:&buffer
                                                   offsets:&offset
                                                 withRange:NSMakeRange(fragIndex, 1)];
                            }
                            if (hasComputeStage) {
                                mLengthTracker->data[SingleShaderStage::Compute][computeIndex] =
                                    binding.size;
                                mLengthTracker->dirtyStages |= wgpu::ShaderStage::Compute;
                                [compute setBuffers:&buffer
                                            offsets:&offset
                                          withRange:NSMakeRange(computeIndex, 1)];
                            }

                            break;
                        }

                        case wgpu::BindingType::Sampler: {
                            auto sampler = ToBackend(group->GetBindingAsSampler(bindingIndex));
                            if (hasVertStage) {
                                [render setVertexSamplerState:sampler->GetMTLSamplerState()
                                                      atIndex:vertIndex];
                            }
                            if (hasFragStage) {
                                [render setFragmentSamplerState:sampler->GetMTLSamplerState()
                                                        atIndex:fragIndex];
                            }
                            if (hasComputeStage) {
                                [compute setSamplerState:sampler->GetMTLSamplerState()
                                                 atIndex:computeIndex];
                            }
                            break;
                        }

                        case wgpu::BindingType::SampledTexture: {
                            auto textureView =
                                ToBackend(group->GetBindingAsTextureView(bindingIndex));
                            if (hasVertStage) {
                                [render setVertexTexture:textureView->GetMTLTexture()
                                                 atIndex:vertIndex];
                            }
                            if (hasFragStage) {
                                [render setFragmentTexture:textureView->GetMTLTexture()
                                                   atIndex:fragIndex];
                            }
                            if (hasComputeStage) {
                                [compute setTexture:textureView->GetMTLTexture()
                                            atIndex:computeIndex];
                            }
                            break;
                        }

                        case wgpu::BindingType::StorageTexture:
                        case wgpu::BindingType::ReadonlyStorageTexture:
                        case wgpu::BindingType::WriteonlyStorageTexture:
                            UNREACHABLE();
                            break;
                    }
                }
            }

            template <typename... Args>
            void ApplyBindGroup(id<MTLRenderCommandEncoder> encoder, Args&&... args) {
                ApplyBindGroupImpl(encoder, nil, std::forward<Args&&>(args)...);
            }

            template <typename... Args>
            void ApplyBindGroup(id<MTLComputeCommandEncoder> encoder, Args&&... args) {
                ApplyBindGroupImpl(nil, encoder, std::forward<Args&&>(args)...);
            }

            StorageBufferLengthTracker* mLengthTracker;
        };

        // Keeps track of the dirty vertex buffer values so they can be lazily applied when we know
        // all the relevant state.
        class VertexBufferTracker {
          public:
            void OnSetVertexBuffer(uint32_t slot, Buffer* buffer, uint64_t offset) {
                mVertexBuffers[slot] = buffer->GetMTLBuffer();
                mVertexBufferOffsets[slot] = offset;

                // Use 64 bit masks and make sure there are no shift UB
                static_assert(kMaxVertexBuffers <= 8 * sizeof(unsigned long long) - 1, "");
                mDirtyVertexBuffers |= 1ull << slot;
            }

            void OnSetPipeline(RenderPipeline* lastPipeline, RenderPipeline* pipeline) {
                // When a new pipeline is bound we must set all the vertex buffers again because
                // they might have been offset by the pipeline layout, and they might be packed
                // differently from the previous pipeline.
                mDirtyVertexBuffers |= pipeline->GetVertexBufferSlotsUsed();
            }

            void Apply(id<MTLRenderCommandEncoder> encoder, RenderPipeline* pipeline) {
                std::bitset<kMaxVertexBuffers> vertexBuffersToApply =
                    mDirtyVertexBuffers & pipeline->GetVertexBufferSlotsUsed();

                for (uint32_t dawnIndex : IterateBitSet(vertexBuffersToApply)) {
                    uint32_t metalIndex = pipeline->GetMtlVertexBufferIndex(dawnIndex);

                    [encoder setVertexBuffers:&mVertexBuffers[dawnIndex]
                                      offsets:&mVertexBufferOffsets[dawnIndex]
                                    withRange:NSMakeRange(metalIndex, 1)];
                }

                mDirtyVertexBuffers.reset();
            }

          private:
            // All the indices in these arrays are Dawn vertex buffer indices
            std::bitset<kMaxVertexBuffers> mDirtyVertexBuffers;
            std::array<id<MTLBuffer>, kMaxVertexBuffers> mVertexBuffers;
            std::array<NSUInteger, kMaxVertexBuffers> mVertexBufferOffsets;
        };

    }  // anonymous namespace

    CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
        : CommandBufferBase(encoder, descriptor), mCommands(encoder->AcquireCommands()) {
    }

    CommandBuffer::~CommandBuffer() {
        FreeCommands(&mCommands);
    }

    void CommandBuffer::FillCommands(CommandRecordingContext* commandContext) {
        const std::vector<PassResourceUsage>& passResourceUsages = GetResourceUsages().perPass;
        size_t nextPassNumber = 0;

        auto LazyClearForPass = [](const PassResourceUsage& usages) {
            for (size_t i = 0; i < usages.textures.size(); ++i) {
                Texture* texture = ToBackend(usages.textures[i]);
                // Clear textures that are not output attachments. Output attachments will be
                // cleared in CreateMTLRenderPassDescriptor by setting the loadop to clear when the
                // texture subresource has not been initialized before the render pass.
                if (!(usages.textureUsages[i] & wgpu::TextureUsage::OutputAttachment)) {
                    texture->EnsureSubresourceContentInitialized(0, texture->GetNumMipLevels(), 0,
                                                                 texture->GetArrayLayers());
                }
            }
        };

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    mCommands.NextCommand<BeginComputePassCmd>();

                    LazyClearForPass(passResourceUsages[nextPassNumber]);
                    commandContext->EndBlit();

                    EncodeComputePass(commandContext);

                    nextPassNumber++;
                    break;
                }

                case Command::BeginRenderPass: {
                    BeginRenderPassCmd* cmd = mCommands.NextCommand<BeginRenderPassCmd>();

                    LazyClearForPass(passResourceUsages[nextPassNumber]);
                    commandContext->EndBlit();

                    LazyClearRenderPassAttachments(cmd);
                    MTLRenderPassDescriptor* descriptor = CreateMTLRenderPassDescriptor(cmd);
                    EncodeRenderPass(commandContext, descriptor, cmd->width, cmd->height);

                    nextPassNumber++;
                    break;
                }

                case Command::CopyBufferToBuffer: {
                    CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();

                    [commandContext->EnsureBlit()
                           copyFromBuffer:ToBackend(copy->source)->GetMTLBuffer()
                             sourceOffset:copy->sourceOffset
                                 toBuffer:ToBackend(copy->destination)->GetMTLBuffer()
                        destinationOffset:copy->destinationOffset
                                     size:copy->size];
                    break;
                }

                case Command::CopyBufferToTexture: {
                    CopyBufferToTextureCmd* copy = mCommands.NextCommand<CopyBufferToTextureCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    auto& copySize = copy->copySize;
                    Buffer* buffer = ToBackend(src.buffer.Get());
                    Texture* texture = ToBackend(dst.texture.Get());

                    EnsureDestinationTextureInitialized(texture, copy->copySize, copy->destination);

                    Extent3D virtualSizeAtLevel = texture->GetMipLevelVirtualSize(dst.mipLevel);
                    TextureBufferCopySplit splittedCopies = ComputeTextureBufferCopySplit(
                        dst.origin, copySize, texture->GetFormat(), virtualSizeAtLevel,
                        buffer->GetSize(), src.offset, src.rowPitch, src.imageHeight);

                    for (uint32_t i = 0; i < splittedCopies.count; ++i) {
                        const TextureBufferCopySplit::CopyInfo& copyInfo = splittedCopies.copies[i];
                        [commandContext->EnsureBlit() copyFromBuffer:buffer->GetMTLBuffer()
                                                        sourceOffset:copyInfo.bufferOffset
                                                   sourceBytesPerRow:copyInfo.bytesPerRow
                                                 sourceBytesPerImage:copyInfo.bytesPerImage
                                                          sourceSize:copyInfo.copyExtent
                                                           toTexture:texture->GetMTLTexture()
                                                    destinationSlice:dst.arrayLayer
                                                    destinationLevel:dst.mipLevel
                                                   destinationOrigin:copyInfo.textureOrigin];
                    }
                    break;
                }

                case Command::CopyTextureToBuffer: {
                    CopyTextureToBufferCmd* copy = mCommands.NextCommand<CopyTextureToBufferCmd>();
                    auto& src = copy->source;
                    auto& dst = copy->destination;
                    auto& copySize = copy->copySize;
                    Texture* texture = ToBackend(src.texture.Get());
                    Buffer* buffer = ToBackend(dst.buffer.Get());

                    EnsureSourceTextureInitialized(texture, copy->copySize, copy->source);

                    Extent3D virtualSizeAtLevel = texture->GetMipLevelVirtualSize(src.mipLevel);
                    TextureBufferCopySplit splittedCopies = ComputeTextureBufferCopySplit(
                        src.origin, copySize, texture->GetFormat(), virtualSizeAtLevel,
                        buffer->GetSize(), dst.offset, dst.rowPitch, dst.imageHeight);

                    for (uint32_t i = 0; i < splittedCopies.count; ++i) {
                        const TextureBufferCopySplit::CopyInfo& copyInfo = splittedCopies.copies[i];
                        [commandContext->EnsureBlit() copyFromTexture:texture->GetMTLTexture()
                                                          sourceSlice:src.arrayLayer
                                                          sourceLevel:src.mipLevel
                                                         sourceOrigin:copyInfo.textureOrigin
                                                           sourceSize:copyInfo.copyExtent
                                                             toBuffer:buffer->GetMTLBuffer()
                                                    destinationOffset:copyInfo.bufferOffset
                                               destinationBytesPerRow:copyInfo.bytesPerRow
                                             destinationBytesPerImage:copyInfo.bytesPerImage];
                    }
                    break;
                }

                case Command::CopyTextureToTexture: {
                    CopyTextureToTextureCmd* copy =
                        mCommands.NextCommand<CopyTextureToTextureCmd>();
                    Texture* srcTexture = ToBackend(copy->source.texture.Get());
                    Texture* dstTexture = ToBackend(copy->destination.texture.Get());

                    EnsureSourceTextureInitialized(srcTexture, copy->copySize, copy->source);
                    EnsureDestinationTextureInitialized(dstTexture, copy->copySize,
                                                        copy->destination);

                    [commandContext->EnsureBlit()
                          copyFromTexture:srcTexture->GetMTLTexture()
                              sourceSlice:copy->source.arrayLayer
                              sourceLevel:copy->source.mipLevel
                             sourceOrigin:MakeMTLOrigin(copy->source.origin)
                               sourceSize:MakeMTLSize(copy->copySize)
                                toTexture:dstTexture->GetMTLTexture()
                         destinationSlice:copy->destination.arrayLayer
                         destinationLevel:copy->destination.mipLevel
                        destinationOrigin:MakeMTLOrigin(copy->destination.origin)];
                    break;
                }

                default: {
                    UNREACHABLE();
                    break;
                }
            }
        }

        commandContext->EndBlit();
    }

    void CommandBuffer::EncodeComputePass(CommandRecordingContext* commandContext) {
        ComputePipeline* lastPipeline = nullptr;
        StorageBufferLengthTracker storageBufferLengths = {};
        BindGroupTracker bindGroups(&storageBufferLengths);

        id<MTLComputeCommandEncoder> encoder = commandContext->BeginCompute();

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndComputePass: {
                    mCommands.NextCommand<EndComputePassCmd>();
                    commandContext->EndCompute();
                    return;
                }

                case Command::Dispatch: {
                    DispatchCmd* dispatch = mCommands.NextCommand<DispatchCmd>();

                    bindGroups.Apply(encoder);
                    storageBufferLengths.Apply(encoder, lastPipeline);

                    [encoder dispatchThreadgroups:MTLSizeMake(dispatch->x, dispatch->y, dispatch->z)
                            threadsPerThreadgroup:lastPipeline->GetLocalWorkGroupSize()];
                    break;
                }

                case Command::DispatchIndirect: {
                    DispatchIndirectCmd* dispatch = mCommands.NextCommand<DispatchIndirectCmd>();

                    bindGroups.Apply(encoder);
                    storageBufferLengths.Apply(encoder, lastPipeline);

                    Buffer* buffer = ToBackend(dispatch->indirectBuffer.Get());
                    id<MTLBuffer> indirectBuffer = buffer->GetMTLBuffer();
                    [encoder dispatchThreadgroupsWithIndirectBuffer:indirectBuffer
                                               indirectBufferOffset:dispatch->indirectOffset
                                              threadsPerThreadgroup:lastPipeline
                                                                        ->GetLocalWorkGroupSize()];
                    break;
                }

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = mCommands.NextCommand<SetComputePipelineCmd>();
                    lastPipeline = ToBackend(cmd->pipeline).Get();

                    bindGroups.OnSetPipeline(lastPipeline);

                    lastPipeline->Encode(encoder);
                    break;
                }

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = mCommands.NextCommand<SetBindGroupCmd>();
                    uint32_t* dynamicOffsets = nullptr;
                    if (cmd->dynamicOffsetCount > 0) {
                        dynamicOffsets = mCommands.NextData<uint32_t>(cmd->dynamicOffsetCount);
                    }

                    bindGroups.OnSetBindGroup(cmd->index, ToBackend(cmd->group.Get()),
                                              cmd->dynamicOffsetCount, dynamicOffsets);
                    break;
                }

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = mCommands.NextCommand<InsertDebugMarkerCmd>();
                    char* label = mCommands.NextData<char>(cmd->length + 1);
                    NSString* mtlLabel = [[NSString alloc] initWithUTF8String:label];

                    [encoder insertDebugSignpost:mtlLabel];
                    [mtlLabel release];
                    break;
                }

                case Command::PopDebugGroup: {
                    mCommands.NextCommand<PopDebugGroupCmd>();

                    [encoder popDebugGroup];
                    break;
                }

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = mCommands.NextCommand<PushDebugGroupCmd>();
                    char* label = mCommands.NextData<char>(cmd->length + 1);
                    NSString* mtlLabel = [[NSString alloc] initWithUTF8String:label];

                    [encoder pushDebugGroup:mtlLabel];
                    [mtlLabel release];
                    break;
                }

                default: {
                    UNREACHABLE();
                    break;
                }
            }
        }

        // EndComputePass should have been called
        UNREACHABLE();
    }

    void CommandBuffer::EncodeRenderPass(CommandRecordingContext* commandContext,
                                         MTLRenderPassDescriptor* mtlRenderPass,
                                         uint32_t width,
                                         uint32_t height) {
        ASSERT(mtlRenderPass);

        Device* device = ToBackend(GetDevice());

        // Handle Toggle AlwaysResolveIntoZeroLevelAndLayer. We must handle this before applying
        // the store + MSAA resolve workaround, otherwise this toggle will never be handled because
        // the resolve texture is removed when applying the store + MSAA resolve workaround.
        if (device->IsToggleEnabled(Toggle::AlwaysResolveIntoZeroLevelAndLayer)) {
            std::array<id<MTLTexture>, kMaxColorAttachments> trueResolveTextures = {};
            std::array<uint32_t, kMaxColorAttachments> trueResolveLevels = {};
            std::array<uint32_t, kMaxColorAttachments> trueResolveSlices = {};

            // Use temporary resolve texture on the resolve targets with non-zero resolveLevel or
            // resolveSlice.
            bool useTemporaryResolveTexture = false;
            std::array<id<MTLTexture>, kMaxColorAttachments> temporaryResolveTextures = {};
            for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
                if (mtlRenderPass.colorAttachments[i].resolveTexture == nil) {
                    continue;
                }

                if (mtlRenderPass.colorAttachments[i].resolveLevel == 0 &&
                    mtlRenderPass.colorAttachments[i].resolveSlice == 0) {
                    continue;
                }

                trueResolveTextures[i] = mtlRenderPass.colorAttachments[i].resolveTexture;
                trueResolveLevels[i] = mtlRenderPass.colorAttachments[i].resolveLevel;
                trueResolveSlices[i] = mtlRenderPass.colorAttachments[i].resolveSlice;

                const MTLPixelFormat mtlFormat = trueResolveTextures[i].pixelFormat;
                temporaryResolveTextures[i] =
                    CreateResolveTextureForWorkaround(device, mtlFormat, width, height);

                mtlRenderPass.colorAttachments[i].resolveTexture = temporaryResolveTextures[i];
                mtlRenderPass.colorAttachments[i].resolveLevel = 0;
                mtlRenderPass.colorAttachments[i].resolveSlice = 0;
                useTemporaryResolveTexture = true;
            }

            // If we need to use a temporary resolve texture we need to copy the result of MSAA
            // resolve back to the true resolve targets.
            if (useTemporaryResolveTexture) {
                EncodeRenderPass(commandContext, mtlRenderPass, width, height);
                for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
                    if (trueResolveTextures[i] == nil) {
                        continue;
                    }

                    ASSERT(temporaryResolveTextures[i] != nil);
                    CopyIntoTrueResolveTarget(commandContext, trueResolveTextures[i],
                                              trueResolveLevels[i], trueResolveSlices[i],
                                              temporaryResolveTextures[i], width, height);
                }
                return;
            }
        }

        // Handle Store + MSAA resolve workaround (Toggle EmulateStoreAndMSAAResolve).
        if (device->IsToggleEnabled(Toggle::EmulateStoreAndMSAAResolve)) {
            bool hasStoreAndMSAAResolve = false;

            // Remove any store + MSAA resolve and remember them.
            std::array<id<MTLTexture>, kMaxColorAttachments> resolveTextures = {};
            for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
                if (mtlRenderPass.colorAttachments[i].storeAction ==
                    kMTLStoreActionStoreAndMultisampleResolve) {
                    hasStoreAndMSAAResolve = true;
                    resolveTextures[i] = mtlRenderPass.colorAttachments[i].resolveTexture;

                    mtlRenderPass.colorAttachments[i].storeAction = MTLStoreActionStore;
                    mtlRenderPass.colorAttachments[i].resolveTexture = nil;
                }
            }

            // If we found a store + MSAA resolve we need to resolve in a different render pass.
            if (hasStoreAndMSAAResolve) {
                EncodeRenderPass(commandContext, mtlRenderPass, width, height);
                ResolveInAnotherRenderPass(commandContext, mtlRenderPass, resolveTextures);
                return;
            }
        }

        EncodeRenderPassInternal(commandContext, mtlRenderPass, width, height);
    }

    void CommandBuffer::EncodeRenderPassInternal(CommandRecordingContext* commandContext,
                                                 MTLRenderPassDescriptor* mtlRenderPass,
                                                 uint32_t width,
                                                 uint32_t height) {
        RenderPipeline* lastPipeline = nullptr;
        id<MTLBuffer> indexBuffer = nil;
        uint32_t indexBufferBaseOffset = 0;
        VertexBufferTracker vertexBuffers;
        StorageBufferLengthTracker storageBufferLengths = {};
        BindGroupTracker bindGroups(&storageBufferLengths);

        id<MTLRenderCommandEncoder> encoder = commandContext->BeginRender(mtlRenderPass);

        auto EncodeRenderBundleCommand = [&](CommandIterator* iter, Command type) {
            switch (type) {
                case Command::Draw: {
                    DrawCmd* draw = iter->NextCommand<DrawCmd>();

                    vertexBuffers.Apply(encoder, lastPipeline);
                    bindGroups.Apply(encoder);
                    storageBufferLengths.Apply(encoder, lastPipeline);

                    // The instance count must be non-zero, otherwise no-op
                    if (draw->instanceCount != 0) {
                        // MTLFeatureSet_iOS_GPUFamily3_v1 does not support baseInstance
                        if (draw->firstInstance == 0) {
                            [encoder drawPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                        vertexStart:draw->firstVertex
                                        vertexCount:draw->vertexCount
                                      instanceCount:draw->instanceCount];
                        } else {
                            [encoder drawPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                        vertexStart:draw->firstVertex
                                        vertexCount:draw->vertexCount
                                      instanceCount:draw->instanceCount
                                       baseInstance:draw->firstInstance];
                        }
                    }
                    break;
                }

                case Command::DrawIndexed: {
                    DrawIndexedCmd* draw = iter->NextCommand<DrawIndexedCmd>();
                    size_t formatSize =
                        IndexFormatSize(lastPipeline->GetVertexStateDescriptor()->indexFormat);

                    vertexBuffers.Apply(encoder, lastPipeline);
                    bindGroups.Apply(encoder);
                    storageBufferLengths.Apply(encoder, lastPipeline);

                    // The index and instance count must be non-zero, otherwise no-op
                    if (draw->indexCount != 0 && draw->instanceCount != 0) {
                        // MTLFeatureSet_iOS_GPUFamily3_v1 does not support baseInstance and
                        // baseVertex.
                        if (draw->baseVertex == 0 && draw->firstInstance == 0) {
                            [encoder drawIndexedPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                                indexCount:draw->indexCount
                                                 indexType:lastPipeline->GetMTLIndexType()
                                               indexBuffer:indexBuffer
                                         indexBufferOffset:indexBufferBaseOffset +
                                                           draw->firstIndex * formatSize
                                             instanceCount:draw->instanceCount];
                        } else {
                            [encoder drawIndexedPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                                indexCount:draw->indexCount
                                                 indexType:lastPipeline->GetMTLIndexType()
                                               indexBuffer:indexBuffer
                                         indexBufferOffset:indexBufferBaseOffset +
                                                           draw->firstIndex * formatSize
                                             instanceCount:draw->instanceCount
                                                baseVertex:draw->baseVertex
                                              baseInstance:draw->firstInstance];
                        }
                    }
                    break;
                }

                case Command::DrawIndirect: {
                    DrawIndirectCmd* draw = iter->NextCommand<DrawIndirectCmd>();

                    vertexBuffers.Apply(encoder, lastPipeline);
                    bindGroups.Apply(encoder);
                    storageBufferLengths.Apply(encoder, lastPipeline);

                    Buffer* buffer = ToBackend(draw->indirectBuffer.Get());
                    id<MTLBuffer> indirectBuffer = buffer->GetMTLBuffer();
                    [encoder drawPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                              indirectBuffer:indirectBuffer
                        indirectBufferOffset:draw->indirectOffset];
                    break;
                }

                case Command::DrawIndexedIndirect: {
                    DrawIndirectCmd* draw = iter->NextCommand<DrawIndirectCmd>();

                    vertexBuffers.Apply(encoder, lastPipeline);
                    bindGroups.Apply(encoder);
                    storageBufferLengths.Apply(encoder, lastPipeline);

                    Buffer* buffer = ToBackend(draw->indirectBuffer.Get());
                    id<MTLBuffer> indirectBuffer = buffer->GetMTLBuffer();
                    [encoder drawIndexedPrimitives:lastPipeline->GetMTLPrimitiveTopology()
                                         indexType:lastPipeline->GetMTLIndexType()
                                       indexBuffer:indexBuffer
                                 indexBufferOffset:indexBufferBaseOffset
                                    indirectBuffer:indirectBuffer
                              indirectBufferOffset:draw->indirectOffset];
                    break;
                }

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = iter->NextCommand<InsertDebugMarkerCmd>();
                    char* label = iter->NextData<char>(cmd->length + 1);
                    NSString* mtlLabel = [[NSString alloc] initWithUTF8String:label];

                    [encoder insertDebugSignpost:mtlLabel];
                    [mtlLabel release];
                    break;
                }

                case Command::PopDebugGroup: {
                    iter->NextCommand<PopDebugGroupCmd>();

                    [encoder popDebugGroup];
                    break;
                }

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = iter->NextCommand<PushDebugGroupCmd>();
                    char* label = iter->NextData<char>(cmd->length + 1);
                    NSString* mtlLabel = [[NSString alloc] initWithUTF8String:label];

                    [encoder pushDebugGroup:mtlLabel];
                    [mtlLabel release];
                    break;
                }

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = iter->NextCommand<SetRenderPipelineCmd>();
                    RenderPipeline* newPipeline = ToBackend(cmd->pipeline).Get();

                    vertexBuffers.OnSetPipeline(lastPipeline, newPipeline);
                    bindGroups.OnSetPipeline(newPipeline);

                    [encoder setDepthStencilState:newPipeline->GetMTLDepthStencilState()];
                    [encoder setFrontFacingWinding:newPipeline->GetMTLFrontFace()];
                    [encoder setCullMode:newPipeline->GetMTLCullMode()];
                    newPipeline->Encode(encoder);

                    lastPipeline = newPipeline;
                    break;
                }

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = iter->NextCommand<SetBindGroupCmd>();
                    uint32_t* dynamicOffsets = nullptr;
                    if (cmd->dynamicOffsetCount > 0) {
                        dynamicOffsets = iter->NextData<uint32_t>(cmd->dynamicOffsetCount);
                    }

                    bindGroups.OnSetBindGroup(cmd->index, ToBackend(cmd->group.Get()),
                                              cmd->dynamicOffsetCount, dynamicOffsets);
                    break;
                }

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = iter->NextCommand<SetIndexBufferCmd>();
                    auto b = ToBackend(cmd->buffer.Get());
                    indexBuffer = b->GetMTLBuffer();
                    indexBufferBaseOffset = cmd->offset;
                    break;
                }

                case Command::SetVertexBuffer: {
                    SetVertexBufferCmd* cmd = iter->NextCommand<SetVertexBufferCmd>();

                    vertexBuffers.OnSetVertexBuffer(cmd->slot, ToBackend(cmd->buffer.Get()),
                                                    cmd->offset);
                    break;
                }

                default:
                    UNREACHABLE();
                    break;
            }
        };

        Command type;
        while (mCommands.NextCommandId(&type)) {
            switch (type) {
                case Command::EndRenderPass: {
                    mCommands.NextCommand<EndRenderPassCmd>();
                    commandContext->EndRender();
                    return;
                }

                case Command::SetStencilReference: {
                    SetStencilReferenceCmd* cmd = mCommands.NextCommand<SetStencilReferenceCmd>();
                    [encoder setStencilReferenceValue:cmd->reference];
                    break;
                }

                case Command::SetViewport: {
                    SetViewportCmd* cmd = mCommands.NextCommand<SetViewportCmd>();
                    MTLViewport viewport;
                    viewport.originX = cmd->x;
                    viewport.originY = cmd->y;
                    viewport.width = cmd->width;
                    viewport.height = cmd->height;
                    viewport.znear = cmd->minDepth;
                    viewport.zfar = cmd->maxDepth;

                    [encoder setViewport:viewport];
                    break;
                }

                case Command::SetScissorRect: {
                    SetScissorRectCmd* cmd = mCommands.NextCommand<SetScissorRectCmd>();
                    MTLScissorRect rect;
                    rect.x = cmd->x;
                    rect.y = cmd->y;
                    rect.width = cmd->width;
                    rect.height = cmd->height;

                    // The scissor rect x + width must be <= render pass width
                    if ((rect.x + rect.width) > width) {
                        rect.width = width - rect.x;
                    }
                    // The scissor rect y + height must be <= render pass height
                    if ((rect.y + rect.height > height)) {
                        rect.height = height - rect.y;
                    }

                    [encoder setScissorRect:rect];
                    break;
                }

                case Command::SetBlendColor: {
                    SetBlendColorCmd* cmd = mCommands.NextCommand<SetBlendColorCmd>();
                    [encoder setBlendColorRed:cmd->color.r
                                        green:cmd->color.g
                                         blue:cmd->color.b
                                        alpha:cmd->color.a];
                    break;
                }

                case Command::ExecuteBundles: {
                    ExecuteBundlesCmd* cmd = mCommands.NextCommand<ExecuteBundlesCmd>();
                    auto bundles = mCommands.NextData<Ref<RenderBundleBase>>(cmd->count);

                    for (uint32_t i = 0; i < cmd->count; ++i) {
                        CommandIterator* iter = bundles[i]->GetCommands();
                        iter->Reset();
                        while (iter->NextCommandId(&type)) {
                            EncodeRenderBundleCommand(iter, type);
                        }
                    }
                    break;
                }

                default: {
                    EncodeRenderBundleCommand(&mCommands, type);
                    break;
                }
            }
        }

        // EndRenderPass should have been called
        UNREACHABLE();
    }

}}  // namespace dawn_native::metal
