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

#include "dawn_native/CommandValidation.h"

#include "common/BitSetIterator.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBufferStateTracker.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/PassResourceUsage.h"
#include "dawn_native/QuerySet.h"
#include "dawn_native/RenderBundle.h"
#include "dawn_native/RenderPipeline.h"

namespace dawn_native {

    namespace {

        inline MaybeError ValidateRenderBundleCommand(CommandIterator* commands,
                                                      Command type,
                                                      CommandBufferStateTracker* commandBufferState,
                                                      const char* disallowedMessage) {
            switch (type) {
                case Command::Draw: {
                    commands->NextCommand<DrawCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDraw());
                    break;
                }

                case Command::DrawIndexed: {
                    commands->NextCommand<DrawIndexedCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDrawIndexed());
                    break;
                }

                case Command::DrawIndirect: {
                    commands->NextCommand<DrawIndirectCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDraw());
                    break;
                }

                case Command::DrawIndexedIndirect: {
                    commands->NextCommand<DrawIndexedIndirectCmd>();
                    DAWN_TRY(commandBufferState->ValidateCanDrawIndexed());
                    break;
                }

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = commands->NextCommand<InsertDebugMarkerCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    break;
                }

                case Command::PopDebugGroup: {
                    commands->NextCommand<PopDebugGroupCmd>();
                    break;
                }

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    break;
                }

                case Command::SetRenderPipeline: {
                    SetRenderPipelineCmd* cmd = commands->NextCommand<SetRenderPipelineCmd>();
                    RenderPipelineBase* pipeline = cmd->pipeline.Get();
                    commandBufferState->SetRenderPipeline(pipeline);
                    break;
                }

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                    if (cmd->dynamicOffsetCount > 0) {
                        commands->NextData<uint32_t>(cmd->dynamicOffsetCount);
                    }

                    commandBufferState->SetBindGroup(cmd->index, cmd->group.Get());
                    break;
                }

                case Command::SetIndexBuffer: {
                    SetIndexBufferCmd* cmd = commands->NextCommand<SetIndexBufferCmd>();
                    commandBufferState->SetIndexBuffer(cmd->format);
                    break;
                }

                case Command::SetVertexBuffer: {
                    SetVertexBufferCmd* cmd = commands->NextCommand<SetVertexBufferCmd>();
                    commandBufferState->SetVertexBuffer(cmd->slot);
                    break;
                }

                default:
                    return DAWN_VALIDATION_ERROR(disallowedMessage);
            }

            return {};
        }

    }  // namespace

    MaybeError ValidateRenderBundle(CommandIterator* commands) {
        CommandBufferStateTracker commandBufferState;
        Command type;
        while (commands->NextCommandId(&type)) {
            DAWN_TRY(ValidateRenderBundleCommand(commands, type, &commandBufferState,
                                                 "Command disallowed inside a render bundle"));
        }

        return {};
    }

    MaybeError ValidateRenderPass(CommandIterator* commands, const BeginRenderPassCmd* renderPass) {
        CommandBufferStateTracker commandBufferState;
        Command type;
        while (commands->NextCommandId(&type)) {
            switch (type) {
                case Command::BeginOcclusionQuery: {
                    commands->NextCommand<BeginOcclusionQueryCmd>();
                    break;
                }

                case Command::EndOcclusionQuery: {
                    commands->NextCommand<EndOcclusionQueryCmd>();
                    break;
                }

                case Command::EndRenderPass: {
                    commands->NextCommand<EndRenderPassCmd>();
                    return {};
                }

                case Command::ExecuteBundles: {
                    ExecuteBundlesCmd* cmd = commands->NextCommand<ExecuteBundlesCmd>();
                    commands->NextData<Ref<RenderBundleBase>>(cmd->count);

                    if (cmd->count > 0) {
                        // Reset state. It is invalidated after render bundle execution.
                        commandBufferState = CommandBufferStateTracker{};
                    }

                    break;
                }

                case Command::SetStencilReference: {
                    commands->NextCommand<SetStencilReferenceCmd>();
                    break;
                }

                case Command::SetBlendColor: {
                    commands->NextCommand<SetBlendColorCmd>();
                    break;
                }

                case Command::SetViewport: {
                    commands->NextCommand<SetViewportCmd>();
                    break;
                }

                case Command::SetScissorRect: {
                    commands->NextCommand<SetScissorRectCmd>();
                    break;
                }

                case Command::WriteTimestamp: {
                    commands->NextCommand<WriteTimestampCmd>();
                    break;
                }

                default:
                    DAWN_TRY(
                        ValidateRenderBundleCommand(commands, type, &commandBufferState,
                                                    "Command disallowed inside a render pass"));
            }
        }

        UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Unfinished render pass");
    }

    MaybeError ValidateComputePass(CommandIterator* commands) {
        CommandBufferStateTracker commandBufferState;
        Command type;
        while (commands->NextCommandId(&type)) {
            switch (type) {
                case Command::EndComputePass: {
                    commands->NextCommand<EndComputePassCmd>();
                    return {};
                }

                case Command::Dispatch: {
                    commands->NextCommand<DispatchCmd>();
                    DAWN_TRY(commandBufferState.ValidateCanDispatch());
                    break;
                }

                case Command::DispatchIndirect: {
                    commands->NextCommand<DispatchIndirectCmd>();
                    DAWN_TRY(commandBufferState.ValidateCanDispatch());
                    break;
                }

                case Command::InsertDebugMarker: {
                    InsertDebugMarkerCmd* cmd = commands->NextCommand<InsertDebugMarkerCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    break;
                }

                case Command::PopDebugGroup: {
                    commands->NextCommand<PopDebugGroupCmd>();
                    break;
                }

                case Command::PushDebugGroup: {
                    PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    break;
                }

                case Command::SetComputePipeline: {
                    SetComputePipelineCmd* cmd = commands->NextCommand<SetComputePipelineCmd>();
                    ComputePipelineBase* pipeline = cmd->pipeline.Get();
                    commandBufferState.SetComputePipeline(pipeline);
                    break;
                }

                case Command::SetBindGroup: {
                    SetBindGroupCmd* cmd = commands->NextCommand<SetBindGroupCmd>();
                    if (cmd->dynamicOffsetCount > 0) {
                        commands->NextData<uint32_t>(cmd->dynamicOffsetCount);
                    }
                    commandBufferState.SetBindGroup(cmd->index, cmd->group.Get());
                    break;
                }

                case Command::WriteTimestamp: {
                    commands->NextCommand<WriteTimestampCmd>();
                    break;
                }

                default:
                    return DAWN_VALIDATION_ERROR("Command disallowed inside a compute pass");
            }
        }

        UNREACHABLE();
        return DAWN_VALIDATION_ERROR("Unfinished compute pass");
    }

    // Performs the per-pass usage validation checks
    // This will eventually need to differentiate between render and compute passes.
    // It will be valid to use a buffer both as uniform and storage in the same compute pass.
    // TODO(yunchao.he@intel.com): add read/write usage tracking for compute
    MaybeError ValidatePassResourceUsage(const PassResourceUsage& pass) {
        // TODO(cwallez@chromium.org): Remove this special casing once the PassResourceUsage is a
        // SyncScopeResourceUsage.
        if (pass.passType != PassType::Render) {
            return {};
        }

        // Buffers can only be used as single-write or multiple read.
        for (size_t i = 0; i < pass.buffers.size(); ++i) {
            wgpu::BufferUsage usage = pass.bufferUsages[i];
            bool readOnly = IsSubset(usage, kReadOnlyBufferUsages);
            bool singleUse = wgpu::HasZeroOrOneBits(usage);

            if (!readOnly && !singleUse) {
                return DAWN_VALIDATION_ERROR(
                    "Buffer used as writable usage and another usage in pass");
            }
        }

        // Check that every single subresource is used as either a single-write usage or a
        // combination of readonly usages.
        for (const PassTextureUsage& textureUsage : pass.textureUsages) {
            MaybeError error = {};
            textureUsage.Iterate([&](const SubresourceRange&, const wgpu::TextureUsage& usage) {
                bool readOnly = IsSubset(usage, kReadOnlyTextureUsages);
                bool singleUse = wgpu::HasZeroOrOneBits(usage);
                if (!readOnly && !singleUse && !error.IsError()) {
                    error = DAWN_VALIDATION_ERROR(
                        "Texture used as writable usage and another usage in render pass");
                }
            });
            DAWN_TRY(std::move(error));
        }
        return {};
    }

    MaybeError ValidateTimestampQuery(QuerySetBase* querySet, uint32_t queryIndex) {
        if (querySet->GetQueryType() != wgpu::QueryType::Timestamp) {
            return DAWN_VALIDATION_ERROR("The type of query set must be Timestamp");
        }

        if (queryIndex >= querySet->GetQueryCount()) {
            return DAWN_VALIDATION_ERROR("Query index exceeds the number of queries in query set");
        }

        return {};
    }

    bool IsRangeOverlapped(uint32_t startA, uint32_t startB, uint32_t length) {
        uint32_t maxStart = std::max(startA, startB);
        uint32_t minStart = std::min(startA, startB);
        return static_cast<uint64_t>(minStart) + static_cast<uint64_t>(length) >
               static_cast<uint64_t>(maxStart);
    }

    template <typename A, typename B>
    DAWN_FORCE_INLINE uint64_t Safe32x32(A a, B b) {
        static_assert(std::is_same<A, uint32_t>::value, "'a' must be uint32_t");
        static_assert(std::is_same<B, uint32_t>::value, "'b' must be uint32_t");
        return uint64_t(a) * uint64_t(b);
    }

    ResultOrError<uint64_t> ComputeRequiredBytesInCopy(const TexelBlockInfo& blockInfo,
                                                       const Extent3D& copySize,
                                                       uint32_t bytesPerRow,
                                                       uint32_t rowsPerImage) {
        ASSERT(copySize.width % blockInfo.width == 0);
        ASSERT(copySize.height % blockInfo.height == 0);
        uint32_t widthInBlocks = copySize.width / blockInfo.width;
        uint32_t heightInBlocks = copySize.height / blockInfo.height;
        uint64_t bytesInLastRow = Safe32x32(widthInBlocks, blockInfo.byteSize);

        if (copySize.depth == 0) {
            return 0;
        }

        // Check for potential overflows for the rest of the computations. We have the following
        // inequalities:
        //
        //   bytesInLastRow <= bytesPerRow
        //   heightInBlocks <= rowsPerImage
        //
        // So:
        //
        //   bytesInLastImage  = bytesPerRow * (heightInBlocks - 1) + bytesInLastRow
        //                    <= bytesPerRow * heightInBlocks
        //                    <= bytesPerRow * rowsPerImage
        //                    <= bytesPerImage
        //
        // This means that if the computation of depth * bytesPerImage doesn't overflow, none of the
        // computations for requiredBytesInCopy will. (and it's not a very pessimizing check)
        ASSERT(copySize.depth <= 1 || (bytesPerRow != wgpu::kCopyStrideUndefined &&
                                       rowsPerImage != wgpu::kCopyStrideUndefined));
        uint64_t bytesPerImage = Safe32x32(bytesPerRow, rowsPerImage);
        if (bytesPerImage > std::numeric_limits<uint64_t>::max() / copySize.depth) {
            return DAWN_VALIDATION_ERROR("requiredBytesInCopy is too large.");
        }

        uint64_t requiredBytesInCopy = bytesPerImage * (copySize.depth - 1);
        if (heightInBlocks > 0) {
            ASSERT(heightInBlocks <= 1 || bytesPerRow != wgpu::kCopyStrideUndefined);
            uint64_t bytesInLastImage = Safe32x32(bytesPerRow, heightInBlocks - 1) + bytesInLastRow;
            requiredBytesInCopy += bytesInLastImage;
        }
        return requiredBytesInCopy;
    }

    MaybeError ValidateCopySizeFitsInBuffer(const Ref<BufferBase>& buffer,
                                            uint64_t offset,
                                            uint64_t size) {
        uint64_t bufferSize = buffer->GetSize();
        bool fitsInBuffer = offset <= bufferSize && (size <= (bufferSize - offset));
        if (!fitsInBuffer) {
            return DAWN_VALIDATION_ERROR("Copy would overflow the buffer");
        }

        return {};
    }

    TextureDataLayout FixUpDeprecatedTextureDataLayoutOptions(
        DeviceBase* device,
        const TextureDataLayout& originalLayout,
        const TexelBlockInfo& blockInfo,
        const Extent3D& copyExtent) {
        // TODO(crbug.com/dawn/520): Remove deprecated functionality.
        TextureDataLayout layout = originalLayout;

        if (copyExtent.height != 0 && layout.rowsPerImage == 0) {
            if (copyExtent.depth > 1) {
                device->EmitDeprecationWarning(
                    "rowsPerImage soon must be non-zero if copy depth > 1 (it will no longer "
                    "default to the copy height).");
                ASSERT(copyExtent.height % blockInfo.height == 0);
                uint32_t heightInBlocks = copyExtent.height / blockInfo.height;
                layout.rowsPerImage = heightInBlocks;
            } else if (copyExtent.depth == 1) {
                device->EmitDeprecationWarning(
                    "rowsPerImage soon must be non-zero or unspecified if copy depth == 1 (it will "
                    "no longer default to the copy height).");
                layout.rowsPerImage = wgpu::kCopyStrideUndefined;
            }
        }

        // Only bother to fix-up for height == 1 && depth == 1.
        // The other cases that used to be allowed were zero-size copies.
        ASSERT(copyExtent.width % blockInfo.width == 0);
        uint32_t widthInBlocks = copyExtent.width / blockInfo.width;
        uint32_t bytesInLastRow = widthInBlocks * blockInfo.byteSize;
        if (copyExtent.height == 1 && copyExtent.depth == 1 &&
            bytesInLastRow > layout.bytesPerRow) {
            device->EmitDeprecationWarning(
                "Soon, even if copy height == 1, bytesPerRow must be >= the byte size of each row "
                "or left unspecified.");
            layout.bytesPerRow = wgpu::kCopyStrideUndefined;
        }
        return layout;
    }

    // Replace wgpu::kCopyStrideUndefined with real values, so backends don't have to think about
    // it.
    void ApplyDefaultTextureDataLayoutOptions(TextureDataLayout* layout,
                                              const TexelBlockInfo& blockInfo,
                                              const Extent3D& copyExtent) {
        ASSERT(layout != nullptr);
        ASSERT(copyExtent.height % blockInfo.height == 0);
        uint32_t heightInBlocks = copyExtent.height / blockInfo.height;

        if (layout->bytesPerRow == wgpu::kCopyStrideUndefined) {
            ASSERT(copyExtent.width % blockInfo.width == 0);
            uint32_t widthInBlocks = copyExtent.width / blockInfo.width;
            uint32_t bytesInLastRow = widthInBlocks * blockInfo.byteSize;

            ASSERT(heightInBlocks <= 1 && copyExtent.depth <= 1);
            layout->bytesPerRow = Align(bytesInLastRow, kTextureBytesPerRowAlignment);
        }
        if (layout->rowsPerImage == wgpu::kCopyStrideUndefined) {
            ASSERT(copyExtent.depth <= 1);
            layout->rowsPerImage = heightInBlocks;
        }
    }

    MaybeError ValidateLinearTextureData(const TextureDataLayout& layout,
                                         uint64_t byteSize,
                                         const TexelBlockInfo& blockInfo,
                                         const Extent3D& copyExtent) {
        ASSERT(copyExtent.height % blockInfo.height == 0);
        uint32_t heightInBlocks = copyExtent.height / blockInfo.height;

        if (copyExtent.depth > 1 && (layout.bytesPerRow == wgpu::kCopyStrideUndefined ||
                                     layout.rowsPerImage == wgpu::kCopyStrideUndefined)) {
            return DAWN_VALIDATION_ERROR(
                "If copy depth > 1, bytesPerRow and rowsPerImage must be specified.");
        }
        if (heightInBlocks > 1 && layout.bytesPerRow == wgpu::kCopyStrideUndefined) {
            return DAWN_VALIDATION_ERROR("If heightInBlocks > 1, bytesPerRow must be specified.");
        }

        // Validation for other members in layout:
        ASSERT(copyExtent.width % blockInfo.width == 0);
        uint32_t widthInBlocks = copyExtent.width / blockInfo.width;
        ASSERT(Safe32x32(widthInBlocks, blockInfo.byteSize) <=
               std::numeric_limits<uint32_t>::max());
        uint32_t bytesInLastRow = widthInBlocks * blockInfo.byteSize;

        // These != wgpu::kCopyStrideUndefined checks are technically redundant with the > checks,
        // but they should get optimized out.
        if (layout.bytesPerRow != wgpu::kCopyStrideUndefined &&
            bytesInLastRow > layout.bytesPerRow) {
            return DAWN_VALIDATION_ERROR("The byte size of each row must be <= bytesPerRow.");
        }
        if (layout.rowsPerImage != wgpu::kCopyStrideUndefined &&
            heightInBlocks > layout.rowsPerImage) {
            return DAWN_VALIDATION_ERROR(
                "The height of each image, in blocks, must be <= rowsPerImage.");
        }

        // We compute required bytes in copy after validating texel block alignments
        // because the divisibility conditions are necessary for the algorithm to be valid,
        // also the bytesPerRow bound is necessary to avoid overflows.
        uint64_t requiredBytesInCopy;
        DAWN_TRY_ASSIGN(requiredBytesInCopy,
                        ComputeRequiredBytesInCopy(blockInfo, copyExtent, layout.bytesPerRow,
                                                   layout.rowsPerImage));

        bool fitsInData =
            layout.offset <= byteSize && (requiredBytesInCopy <= (byteSize - layout.offset));
        if (!fitsInData) {
            return DAWN_VALIDATION_ERROR(
                "Required size for texture data layout exceeds the linear data size.");
        }

        return {};
    }

    MaybeError ValidateBufferCopyView(DeviceBase const* device,
                                      const BufferCopyView& bufferCopyView) {
        DAWN_TRY(device->ValidateObject(bufferCopyView.buffer));
        if (bufferCopyView.layout.bytesPerRow != wgpu::kCopyStrideUndefined) {
            if (bufferCopyView.layout.bytesPerRow % kTextureBytesPerRowAlignment != 0) {
                return DAWN_VALIDATION_ERROR("bytesPerRow must be a multiple of 256");
            }
        }

        return {};
    }

    MaybeError ValidateTextureCopyView(DeviceBase const* device,
                                       const TextureCopyView& textureCopy,
                                       const Extent3D& copySize) {
        const TextureBase* texture = textureCopy.texture;
        DAWN_TRY(device->ValidateObject(texture));
        if (textureCopy.mipLevel >= texture->GetNumMipLevels()) {
            return DAWN_VALIDATION_ERROR("mipLevel out of range");
        }

        if (SelectFormatAspects(texture->GetFormat(), textureCopy.aspect) == Aspect::None) {
            return DAWN_VALIDATION_ERROR("Texture does not have selected aspect for texture copy.");
        }

        if (texture->GetSampleCount() > 1 || texture->GetFormat().HasDepthOrStencil()) {
            Extent3D subresourceSize = texture->GetMipLevelPhysicalSize(textureCopy.mipLevel);
            ASSERT(texture->GetDimension() == wgpu::TextureDimension::e2D);
            if (textureCopy.origin.x != 0 || textureCopy.origin.y != 0 ||
                subresourceSize.width != copySize.width ||
                subresourceSize.height != copySize.height) {
                return DAWN_VALIDATION_ERROR(
                    "The entire subresource must be copied when using a depth/stencil texture, or "
                    "when sample count is greater than 1.");
            }
        }

        return {};
    }

    MaybeError ValidateTextureCopyRange(const TextureCopyView& textureCopy,
                                        const Extent3D& copySize) {
        // TODO(jiawei.shao@intel.com): add validations on the texture-to-texture copies within the
        // same texture.
        const TextureBase* texture = textureCopy.texture;

        // Validation for the copy being in-bounds:
        Extent3D mipSize = texture->GetMipLevelPhysicalSize(textureCopy.mipLevel);
        // For 2D textures, include the array layer as depth so it can be checked with other
        // dimensions.
        ASSERT(texture->GetDimension() == wgpu::TextureDimension::e2D);
        mipSize.depth = texture->GetArrayLayers();

        // All texture dimensions are in uint32_t so by doing checks in uint64_t we avoid
        // overflows.
        if (static_cast<uint64_t>(textureCopy.origin.x) + static_cast<uint64_t>(copySize.width) >
                static_cast<uint64_t>(mipSize.width) ||
            static_cast<uint64_t>(textureCopy.origin.y) + static_cast<uint64_t>(copySize.height) >
                static_cast<uint64_t>(mipSize.height) ||
            static_cast<uint64_t>(textureCopy.origin.z) + static_cast<uint64_t>(copySize.depth) >
                static_cast<uint64_t>(mipSize.depth)) {
            return DAWN_VALIDATION_ERROR("Touching outside of the texture");
        }

        // Validation for the texel block alignments:
        const Format& format = textureCopy.texture->GetFormat();
        if (format.isCompressed) {
            const TexelBlockInfo& blockInfo = format.GetAspectInfo(textureCopy.aspect).block;
            if (textureCopy.origin.x % blockInfo.width != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Offset.x must be a multiple of compressed texture format block width");
            }
            if (textureCopy.origin.y % blockInfo.height != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Offset.y must be a multiple of compressed texture format block height");
            }
            if (copySize.width % blockInfo.width != 0) {
                return DAWN_VALIDATION_ERROR(
                    "copySize.width must be a multiple of compressed texture format block width");
            }

            if (copySize.height % blockInfo.height != 0) {
                return DAWN_VALIDATION_ERROR(
                    "copySize.height must be a multiple of compressed texture format block height");
            }
        }

        return {};
    }

    // Always returns a single aspect (color, stencil, or depth).
    ResultOrError<Aspect> SingleAspectUsedByTextureCopyView(const TextureCopyView& view) {
        const Format& format = view.texture->GetFormat();
        switch (view.aspect) {
            case wgpu::TextureAspect::All:
                if (HasOneBit(format.aspects)) {
                    Aspect single = format.aspects;
                    return single;
                } else {
                    return DAWN_VALIDATION_ERROR(
                        "A single aspect must be selected for multi-planar formats in "
                        "texture <-> linear data copies");
                }
                break;
            case wgpu::TextureAspect::DepthOnly:
                ASSERT(format.aspects & Aspect::Depth);
                return Aspect::Depth;
            case wgpu::TextureAspect::StencilOnly:
                ASSERT(format.aspects & Aspect::Stencil);
                return Aspect::Stencil;
        }
    }

    MaybeError ValidateLinearToDepthStencilCopyRestrictions(const TextureCopyView& dst) {
        Aspect aspectUsed;
        DAWN_TRY_ASSIGN(aspectUsed, SingleAspectUsedByTextureCopyView(dst));
        if (aspectUsed == Aspect::Depth) {
            return DAWN_VALIDATION_ERROR("Cannot copy into the depth aspect of a texture");
        }

        return {};
    }

    MaybeError ValidateTextureToTextureCopyRestrictions(const TextureCopyView& src,
                                                        const TextureCopyView& dst,
                                                        const Extent3D& copySize) {
        const uint32_t srcSamples = src.texture->GetSampleCount();
        const uint32_t dstSamples = dst.texture->GetSampleCount();

        if (srcSamples != dstSamples) {
            return DAWN_VALIDATION_ERROR(
                "Source and destination textures must have matching sample counts.");
        }

        if (src.texture->GetFormat().format != dst.texture->GetFormat().format) {
            // Metal requires texture-to-texture copies be the same format
            return DAWN_VALIDATION_ERROR("Source and destination texture formats must match.");
        }

        // Metal cannot select a single aspect for texture-to-texture copies.
        const Format& format = src.texture->GetFormat();
        if (SelectFormatAspects(format, src.aspect) != format.aspects) {
            return DAWN_VALIDATION_ERROR(
                "Source aspect doesn't select all the aspects of the source format.");
        }
        if (SelectFormatAspects(format, dst.aspect) != format.aspects) {
            return DAWN_VALIDATION_ERROR(
                "Destination aspect doesn't select all the aspects of the destination format.");
        }

        if (src.texture == dst.texture && src.mipLevel == dst.mipLevel) {
            ASSERT(src.texture->GetDimension() == wgpu::TextureDimension::e2D &&
                   dst.texture->GetDimension() == wgpu::TextureDimension::e2D);
            if (IsRangeOverlapped(src.origin.z, dst.origin.z, copySize.depth)) {
                return DAWN_VALIDATION_ERROR(
                    "Copy subresources cannot be overlapped when copying within the same "
                    "texture.");
            }
        }

        return {};
    }

    MaybeError ValidateCanUseAs(const TextureBase* texture, wgpu::TextureUsage usage) {
        ASSERT(wgpu::HasZeroOrOneBits(usage));
        if (!(texture->GetUsage() & usage)) {
            return DAWN_VALIDATION_ERROR("texture doesn't have the required usage.");
        }

        return {};
    }

    MaybeError ValidateCanUseAs(const BufferBase* buffer, wgpu::BufferUsage usage) {
        ASSERT(wgpu::HasZeroOrOneBits(usage));
        if (!(buffer->GetUsage() & usage)) {
            return DAWN_VALIDATION_ERROR("buffer doesn't have the required usage.");
        }

        return {};
    }

}  // namespace dawn_native
