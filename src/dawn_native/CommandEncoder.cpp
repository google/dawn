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

#include "dawn_native/CommandEncoder.h"

#include "common/BitSetIterator.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/CommandBufferStateTracker.h"
#include "dawn_native/CommandValidation.h"
#include "dawn_native/Commands.h"
#include "dawn_native/ComputePassEncoder.h"
#include "dawn_native/Device.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/RenderPassEncoder.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/ValidationUtils_autogen.h"
#include "dawn_platform/DawnPlatform.h"
#include "dawn_platform/tracing/TraceEvent.h"

#include <cmath>
#include <map>

namespace dawn_native {

    namespace {

        MaybeError ValidateCopySizeFitsInTexture(const TextureCopy& textureCopy,
                                                 const Extent3D& copySize) {
            const TextureBase* texture = textureCopy.texture.Get();
            if (textureCopy.mipLevel >= texture->GetNumMipLevels()) {
                return DAWN_VALIDATION_ERROR("Copy mipLevel out of range");
            }

            if (textureCopy.arrayLayer >= texture->GetArrayLayers()) {
                return DAWN_VALIDATION_ERROR("Copy arrayLayer out of range");
            }

            Extent3D extent = texture->GetMipLevelPhysicalSize(textureCopy.mipLevel);

            // All texture dimensions are in uint32_t so by doing checks in uint64_t we avoid
            // overflows.
            if (uint64_t(textureCopy.origin.x) + uint64_t(copySize.width) >
                    static_cast<uint64_t>(extent.width) ||
                uint64_t(textureCopy.origin.y) + uint64_t(copySize.height) >
                    static_cast<uint64_t>(extent.height)) {
                return DAWN_VALIDATION_ERROR("Copy would touch outside of the texture");
            }

            // TODO(cwallez@chromium.org): Check the depth bound differently for 2D arrays and 3D
            // textures
            if (textureCopy.origin.z != 0 || copySize.depth > 1) {
                return DAWN_VALIDATION_ERROR("No support for z != 0 and depth > 1 for now");
            }

            return {};
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

        MaybeError ValidateCopySizeFitsInBuffer(const BufferCopy& bufferCopy, uint64_t dataSize) {
            return ValidateCopySizeFitsInBuffer(bufferCopy.buffer, bufferCopy.offset, dataSize);
        }

        MaybeError ValidateB2BCopySizeAlignment(uint64_t dataSize,
                                                uint64_t srcOffset,
                                                uint64_t dstOffset) {
            // Copy size must be a multiple of 4 bytes on macOS.
            if (dataSize % 4 != 0) {
                return DAWN_VALIDATION_ERROR("Copy size must be a multiple of 4 bytes");
            }

            // SourceOffset and destinationOffset must be multiples of 4 bytes on macOS.
            if (srcOffset % 4 != 0 || dstOffset % 4 != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Source offset and destination offset must be multiples of 4 bytes");
            }

            return {};
        }

        MaybeError ValidateTexelBufferOffset(const BufferCopy& bufferCopy, const Format& format) {
            if (bufferCopy.offset % format.blockByteSize != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Buffer offset must be a multiple of the texel or block size");
            }

            return {};
        }

        MaybeError ValidateImageHeight(const Format& format,
                                       uint32_t imageHeight,
                                       uint32_t copyHeight) {
            if (imageHeight < copyHeight) {
                return DAWN_VALIDATION_ERROR("Image height must not be less than the copy height.");
            }

            if (imageHeight % format.blockHeight != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Image height must be a multiple of compressed texture format block width");
            }

            return {};
        }

        MaybeError ValidateTextureSampleCountInCopyCommands(const TextureBase* texture) {
            if (texture->GetSampleCount() > 1) {
                return DAWN_VALIDATION_ERROR("The sample count of textures must be 1");
            }

            return {};
        }

        MaybeError ValidateEntireSubresourceCopied(const TextureCopy& src,
                                                   const TextureCopy& dst,
                                                   const Extent3D& copySize) {
            Extent3D srcSize = src.texture.Get()->GetSize();

            if (dst.origin.x != 0 || dst.origin.y != 0 || dst.origin.z != 0 ||
                srcSize.width != copySize.width || srcSize.height != copySize.height ||
                srcSize.depth != copySize.depth) {
                return DAWN_VALIDATION_ERROR(
                    "The entire subresource must be copied when using a depth/stencil texture or "
                    "when samples are greater than 1.");
            }

            return {};
        }

        MaybeError ValidateTextureToTextureCopyRestrictions(const TextureCopy& src,
                                                            const TextureCopy& dst,
                                                            const Extent3D& copySize) {
            const uint32_t srcSamples = src.texture.Get()->GetSampleCount();
            const uint32_t dstSamples = dst.texture.Get()->GetSampleCount();

            if (srcSamples != dstSamples) {
                return DAWN_VALIDATION_ERROR(
                    "Source and destination textures must have matching sample counts.");
            } else if (srcSamples > 1) {
                // D3D12 requires entire subresource to be copied when using CopyTextureRegion when
                // samples > 1.
                DAWN_TRY(ValidateEntireSubresourceCopied(src, dst, copySize));
            }

            if (src.texture.Get()->GetFormat().format != dst.texture.Get()->GetFormat().format) {
                // Metal requires texture-to-texture copies be the same format
                return DAWN_VALIDATION_ERROR("Source and destination texture formats must match.");
            }

            if (src.texture.Get()->GetFormat().HasDepthOrStencil()) {
                // D3D12 requires entire subresource to be copied when using CopyTextureRegion is
                // used with depth/stencil.
                DAWN_TRY(ValidateEntireSubresourceCopied(src, dst, copySize));
            }

            return {};
        }

        MaybeError ComputeTextureCopyBufferSize(const Format& textureFormat,
                                                const Extent3D& copySize,
                                                uint32_t rowPitch,
                                                uint32_t imageHeight,
                                                uint32_t* bufferSize) {
            ASSERT(imageHeight >= copySize.height);
            uint32_t blockByteSize = textureFormat.blockByteSize;
            uint32_t blockWidth = textureFormat.blockWidth;
            uint32_t blockHeight = textureFormat.blockHeight;

            // TODO(cwallez@chromium.org): check for overflows
            uint32_t slicePitch = rowPitch * imageHeight / blockWidth;
            uint32_t sliceSize = rowPitch * (copySize.height / blockHeight - 1) +
                                 (copySize.width / blockWidth) * blockByteSize;
            *bufferSize = (slicePitch * (copySize.depth - 1)) + sliceSize;

            return {};
        }

        uint32_t ComputeDefaultRowPitch(const Format& format, uint32_t width) {
            return width / format.blockWidth * format.blockByteSize;
        }

        MaybeError ValidateRowPitch(const Format& format,
                                    const Extent3D& copySize,
                                    uint32_t rowPitch) {
            if (rowPitch % kTextureRowPitchAlignment != 0) {
                return DAWN_VALIDATION_ERROR("Row pitch must be a multiple of 256");
            }

            if (rowPitch < copySize.width / format.blockWidth * format.blockByteSize) {
                return DAWN_VALIDATION_ERROR(
                    "Row pitch must not be less than the number of bytes per row");
            }

            return {};
        }

        MaybeError ValidateImageOrigin(const Format& format, const Origin3D& offset) {
            if (offset.x % format.blockWidth != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Offset.x must be a multiple of compressed texture format block width");
            }

            if (offset.y % format.blockHeight != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Offset.y must be a multiple of compressed texture format block height");
            }

            return {};
        }

        MaybeError ValidateImageCopySize(const Format& format, const Extent3D& extent) {
            if (extent.width % format.blockWidth != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Extent.width must be a multiple of compressed texture format block width");
            }

            if (extent.height % format.blockHeight != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Extent.height must be a multiple of compressed texture format block height");
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

        MaybeError ValidateCanUseAs(const TextureBase* texture, wgpu::TextureUsage usage) {
            ASSERT(wgpu::HasZeroOrOneBits(usage));
            if (!(texture->GetUsage() & usage)) {
                return DAWN_VALIDATION_ERROR("texture doesn't have the required usage.");
            }

            return {};
        }

        MaybeError ValidateAttachmentArrayLayersAndLevelCount(const TextureViewBase* attachment) {
            // Currently we do not support layered rendering.
            if (attachment->GetLayerCount() > 1) {
                return DAWN_VALIDATION_ERROR(
                    "The layer count of the texture view used as attachment cannot be greater than "
                    "1");
            }

            if (attachment->GetLevelCount() > 1) {
                return DAWN_VALIDATION_ERROR(
                    "The mipmap level count of the texture view used as attachment cannot be "
                    "greater than 1");
            }

            return {};
        }

        MaybeError ValidateOrSetAttachmentSize(const TextureViewBase* attachment,
                                               uint32_t* width,
                                               uint32_t* height) {
            const Extent3D& textureSize = attachment->GetTexture()->GetSize();
            const uint32_t attachmentWidth = textureSize.width >> attachment->GetBaseMipLevel();
            const uint32_t attachmentHeight = textureSize.height >> attachment->GetBaseMipLevel();

            if (*width == 0) {
                DAWN_ASSERT(*height == 0);
                *width = attachmentWidth;
                *height = attachmentHeight;
                DAWN_ASSERT(*width != 0 && *height != 0);
            } else if (*width != attachmentWidth || *height != attachmentHeight) {
                return DAWN_VALIDATION_ERROR("Attachment size mismatch");
            }

            return {};
        }

        MaybeError ValidateOrSetColorAttachmentSampleCount(const TextureViewBase* colorAttachment,
                                                           uint32_t* sampleCount) {
            if (*sampleCount == 0) {
                *sampleCount = colorAttachment->GetTexture()->GetSampleCount();
                DAWN_ASSERT(*sampleCount != 0);
            } else if (*sampleCount != colorAttachment->GetTexture()->GetSampleCount()) {
                return DAWN_VALIDATION_ERROR("Color attachment sample counts mismatch");
            }

            return {};
        }

        MaybeError ValidateResolveTarget(
            const DeviceBase* device,
            const RenderPassColorAttachmentDescriptor& colorAttachment) {
            if (colorAttachment.resolveTarget == nullptr) {
                return {};
            }

            const TextureViewBase* resolveTarget = colorAttachment.resolveTarget;
            const TextureViewBase* attachment = colorAttachment.attachment;
            DAWN_TRY(device->ValidateObject(colorAttachment.resolveTarget));

            if (!attachment->GetTexture()->IsMultisampledTexture()) {
                return DAWN_VALIDATION_ERROR(
                    "Cannot set resolve target when the sample count of the color attachment is 1");
            }

            if (resolveTarget->GetTexture()->IsMultisampledTexture()) {
                return DAWN_VALIDATION_ERROR("Cannot use multisampled texture as resolve target");
            }

            if (resolveTarget->GetLayerCount() > 1) {
                return DAWN_VALIDATION_ERROR(
                    "The array layer count of the resolve target must be 1");
            }

            if (resolveTarget->GetLevelCount() > 1) {
                return DAWN_VALIDATION_ERROR("The mip level count of the resolve target must be 1");
            }

            uint32_t colorAttachmentBaseMipLevel = attachment->GetBaseMipLevel();
            const Extent3D& colorTextureSize = attachment->GetTexture()->GetSize();
            uint32_t colorAttachmentWidth = colorTextureSize.width >> colorAttachmentBaseMipLevel;
            uint32_t colorAttachmentHeight = colorTextureSize.height >> colorAttachmentBaseMipLevel;

            uint32_t resolveTargetBaseMipLevel = resolveTarget->GetBaseMipLevel();
            const Extent3D& resolveTextureSize = resolveTarget->GetTexture()->GetSize();
            uint32_t resolveTargetWidth = resolveTextureSize.width >> resolveTargetBaseMipLevel;
            uint32_t resolveTargetHeight = resolveTextureSize.height >> resolveTargetBaseMipLevel;
            if (colorAttachmentWidth != resolveTargetWidth ||
                colorAttachmentHeight != resolveTargetHeight) {
                return DAWN_VALIDATION_ERROR(
                    "The size of the resolve target must be the same as the color attachment");
            }

            wgpu::TextureFormat resolveTargetFormat = resolveTarget->GetFormat().format;
            if (resolveTargetFormat != attachment->GetFormat().format) {
                return DAWN_VALIDATION_ERROR(
                    "The format of the resolve target must be the same as the color attachment");
            }

            return {};
        }

        MaybeError ValidateRenderPassColorAttachment(
            const DeviceBase* device,
            const RenderPassColorAttachmentDescriptor& colorAttachment,
            uint32_t* width,
            uint32_t* height,
            uint32_t* sampleCount) {
            DAWN_TRY(device->ValidateObject(colorAttachment.attachment));

            const TextureViewBase* attachment = colorAttachment.attachment;
            if (!attachment->GetFormat().IsColor() || !attachment->GetFormat().isRenderable) {
                return DAWN_VALIDATION_ERROR(
                    "The format of the texture view used as color attachment is not color "
                    "renderable");
            }

            DAWN_TRY(ValidateLoadOp(colorAttachment.loadOp));
            DAWN_TRY(ValidateStoreOp(colorAttachment.storeOp));

            if (colorAttachment.loadOp == wgpu::LoadOp::Clear) {
                if (std::isnan(colorAttachment.clearColor.r) ||
                    std::isnan(colorAttachment.clearColor.g) ||
                    std::isnan(colorAttachment.clearColor.b) ||
                    std::isnan(colorAttachment.clearColor.a)) {
                    return DAWN_VALIDATION_ERROR("Color clear value cannot contain NaN");
                }
            }

            DAWN_TRY(ValidateOrSetColorAttachmentSampleCount(attachment, sampleCount));

            DAWN_TRY(ValidateResolveTarget(device, colorAttachment));

            DAWN_TRY(ValidateAttachmentArrayLayersAndLevelCount(attachment));
            DAWN_TRY(ValidateOrSetAttachmentSize(attachment, width, height));

            return {};
        }

        MaybeError ValidateRenderPassDepthStencilAttachment(
            const DeviceBase* device,
            const RenderPassDepthStencilAttachmentDescriptor* depthStencilAttachment,
            uint32_t* width,
            uint32_t* height,
            uint32_t* sampleCount) {
            DAWN_ASSERT(depthStencilAttachment != nullptr);

            DAWN_TRY(device->ValidateObject(depthStencilAttachment->attachment));

            const TextureViewBase* attachment = depthStencilAttachment->attachment;
            if (!attachment->GetFormat().HasDepthOrStencil() ||
                !attachment->GetFormat().isRenderable) {
                return DAWN_VALIDATION_ERROR(
                    "The format of the texture view used as depth stencil attachment is not a "
                    "depth stencil format");
            }

            DAWN_TRY(ValidateLoadOp(depthStencilAttachment->depthLoadOp));
            DAWN_TRY(ValidateLoadOp(depthStencilAttachment->stencilLoadOp));
            DAWN_TRY(ValidateStoreOp(depthStencilAttachment->depthStoreOp));
            DAWN_TRY(ValidateStoreOp(depthStencilAttachment->stencilStoreOp));

            if (depthStencilAttachment->depthLoadOp == wgpu::LoadOp::Clear &&
                std::isnan(depthStencilAttachment->clearDepth)) {
                return DAWN_VALIDATION_ERROR("Depth clear value cannot be NaN");
            }

            // This validates that the depth storeOp and stencil storeOps are the same
            if (depthStencilAttachment->depthStoreOp != depthStencilAttachment->stencilStoreOp) {
                return DAWN_VALIDATION_ERROR(
                    "The depth storeOp and stencil storeOp are not the same");
            }

            // *sampleCount == 0 must only happen when there is no color attachment. In that case we
            // do not need to validate the sample count of the depth stencil attachment.
            const uint32_t depthStencilSampleCount = attachment->GetTexture()->GetSampleCount();
            if (*sampleCount != 0) {
                if (depthStencilSampleCount != *sampleCount) {
                    return DAWN_VALIDATION_ERROR("Depth stencil attachment sample counts mismatch");
                }
            } else {
                *sampleCount = depthStencilSampleCount;
            }

            DAWN_TRY(ValidateAttachmentArrayLayersAndLevelCount(attachment));
            DAWN_TRY(ValidateOrSetAttachmentSize(attachment, width, height));

            return {};
        }

        MaybeError ValidateRenderPassDescriptor(const DeviceBase* device,
                                                const RenderPassDescriptor* descriptor,
                                                uint32_t* width,
                                                uint32_t* height,
                                                uint32_t* sampleCount) {
            if (descriptor->colorAttachmentCount > kMaxColorAttachments) {
                return DAWN_VALIDATION_ERROR("Setting color attachments out of bounds");
            }

            for (uint32_t i = 0; i < descriptor->colorAttachmentCount; ++i) {
                DAWN_TRY(ValidateRenderPassColorAttachment(device, descriptor->colorAttachments[i],
                                                           width, height, sampleCount));
            }

            if (descriptor->depthStencilAttachment != nullptr) {
                DAWN_TRY(ValidateRenderPassDepthStencilAttachment(
                    device, descriptor->depthStencilAttachment, width, height, sampleCount));
            }

            if (descriptor->colorAttachmentCount == 0 &&
                descriptor->depthStencilAttachment == nullptr) {
                return DAWN_VALIDATION_ERROR("Cannot use render pass with no attachments.");
            }

            return {};
        }

        MaybeError ValidateComputePassDescriptor(const DeviceBase* device,
                                                 const ComputePassDescriptor* descriptor) {
            return {};
        }

    }  // namespace

    CommandEncoder::CommandEncoder(DeviceBase* device, const CommandEncoderDescriptor*)
        : ObjectBase(device), mEncodingContext(device, this) {
    }

    CommandBufferResourceUsage CommandEncoder::AcquireResourceUsages() {
        return CommandBufferResourceUsage{mEncodingContext.AcquirePassUsages(),
                                          std::move(mTopLevelBuffers),
                                          std::move(mTopLevelTextures)};
    }

    CommandIterator CommandEncoder::AcquireCommands() {
        return mEncodingContext.AcquireCommands();
    }

    // Implementation of the API's command recording methods

    ComputePassEncoder* CommandEncoder::BeginComputePass(const ComputePassDescriptor* descriptor) {
        DeviceBase* device = GetDevice();

        bool success =
            mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
                DAWN_TRY(ValidateComputePassDescriptor(device, descriptor));

                allocator->Allocate<BeginComputePassCmd>(Command::BeginComputePass);

                return {};
            });

        if (success) {
            ComputePassEncoder* passEncoder =
                new ComputePassEncoder(device, this, &mEncodingContext);
            mEncodingContext.EnterPass(passEncoder);
            return passEncoder;
        }

        return ComputePassEncoder::MakeError(device, this, &mEncodingContext);
    }

    RenderPassEncoder* CommandEncoder::BeginRenderPass(const RenderPassDescriptor* descriptor) {
        DeviceBase* device = GetDevice();

        PassResourceUsageTracker usageTracker;
        bool success =
            mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
                uint32_t width = 0;
                uint32_t height = 0;
                uint32_t sampleCount = 0;

                DAWN_TRY(ValidateRenderPassDescriptor(device, descriptor, &width, &height,
                                                      &sampleCount));

                ASSERT(width > 0 && height > 0 && sampleCount > 0);

                BeginRenderPassCmd* cmd =
                    allocator->Allocate<BeginRenderPassCmd>(Command::BeginRenderPass);

                cmd->attachmentState = device->GetOrCreateAttachmentState(descriptor);

                for (uint32_t i : IterateBitSet(cmd->attachmentState->GetColorAttachmentsMask())) {
                    TextureViewBase* view = descriptor->colorAttachments[i].attachment;
                    TextureViewBase* resolveTarget = descriptor->colorAttachments[i].resolveTarget;

                    cmd->colorAttachments[i].view = view;
                    cmd->colorAttachments[i].resolveTarget = resolveTarget;
                    cmd->colorAttachments[i].loadOp = descriptor->colorAttachments[i].loadOp;
                    cmd->colorAttachments[i].storeOp = descriptor->colorAttachments[i].storeOp;
                    cmd->colorAttachments[i].clearColor =
                        descriptor->colorAttachments[i].clearColor;

                    usageTracker.TextureUsedAs(view->GetTexture(),
                                               wgpu::TextureUsage::OutputAttachment);

                    if (resolveTarget != nullptr) {
                        usageTracker.TextureUsedAs(resolveTarget->GetTexture(),
                                                   wgpu::TextureUsage::OutputAttachment);
                    }
                }

                if (cmd->attachmentState->HasDepthStencilAttachment()) {
                    TextureViewBase* view = descriptor->depthStencilAttachment->attachment;

                    cmd->depthStencilAttachment.view = view;
                    cmd->depthStencilAttachment.clearDepth =
                        descriptor->depthStencilAttachment->clearDepth;
                    cmd->depthStencilAttachment.clearStencil =
                        descriptor->depthStencilAttachment->clearStencil;
                    cmd->depthStencilAttachment.depthLoadOp =
                        descriptor->depthStencilAttachment->depthLoadOp;
                    cmd->depthStencilAttachment.depthStoreOp =
                        descriptor->depthStencilAttachment->depthStoreOp;
                    cmd->depthStencilAttachment.stencilLoadOp =
                        descriptor->depthStencilAttachment->stencilLoadOp;
                    cmd->depthStencilAttachment.stencilStoreOp =
                        descriptor->depthStencilAttachment->stencilStoreOp;

                    usageTracker.TextureUsedAs(view->GetTexture(),
                                               wgpu::TextureUsage::OutputAttachment);
                }

                cmd->width = width;
                cmd->height = height;

                return {};
            });

        if (success) {
            RenderPassEncoder* passEncoder =
                new RenderPassEncoder(device, this, &mEncodingContext, std::move(usageTracker));
            mEncodingContext.EnterPass(passEncoder);
            return passEncoder;
        }

        return RenderPassEncoder::MakeError(device, this, &mEncodingContext);
    }

    void CommandEncoder::CopyBufferToBuffer(BufferBase* source,
                                            uint64_t sourceOffset,
                                            BufferBase* destination,
                                            uint64_t destinationOffset,
                                            uint64_t size) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            DAWN_TRY(GetDevice()->ValidateObject(source));
            DAWN_TRY(GetDevice()->ValidateObject(destination));

            CopyBufferToBufferCmd* copy =
                allocator->Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
            copy->source = source;
            copy->sourceOffset = sourceOffset;
            copy->destination = destination;
            copy->destinationOffset = destinationOffset;
            copy->size = size;

            if (GetDevice()->IsValidationEnabled()) {
                mTopLevelBuffers.insert(source);
                mTopLevelBuffers.insert(destination);
            }
            return {};
        });
    }

    void CommandEncoder::CopyBufferToTexture(const BufferCopyView* source,
                                             const TextureCopyView* destination,
                                             const Extent3D* copySize) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            DAWN_TRY(GetDevice()->ValidateObject(source->buffer));
            DAWN_TRY(GetDevice()->ValidateObject(destination->texture));

            CopyBufferToTextureCmd* copy =
                allocator->Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
            copy->source.buffer = source->buffer;
            copy->source.offset = source->offset;
            copy->destination.texture = destination->texture;
            copy->destination.origin = destination->origin;
            copy->copySize = *copySize;
            copy->destination.mipLevel = destination->mipLevel;
            copy->destination.arrayLayer = destination->arrayLayer;
            if (source->rowPitch == 0) {
                copy->source.rowPitch =
                    ComputeDefaultRowPitch(destination->texture->GetFormat(), copySize->width);
            } else {
                copy->source.rowPitch = source->rowPitch;
            }
            if (source->imageHeight == 0) {
                copy->source.imageHeight = copySize->height;
            } else {
                copy->source.imageHeight = source->imageHeight;
            }

            if (GetDevice()->IsValidationEnabled()) {
                mTopLevelBuffers.insert(source->buffer);
                mTopLevelTextures.insert(destination->texture);
            }
            return {};
        });
    }

    void CommandEncoder::CopyTextureToBuffer(const TextureCopyView* source,
                                             const BufferCopyView* destination,
                                             const Extent3D* copySize) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            DAWN_TRY(GetDevice()->ValidateObject(source->texture));
            DAWN_TRY(GetDevice()->ValidateObject(destination->buffer));

            CopyTextureToBufferCmd* copy =
                allocator->Allocate<CopyTextureToBufferCmd>(Command::CopyTextureToBuffer);
            copy->source.texture = source->texture;
            copy->source.origin = source->origin;
            copy->copySize = *copySize;
            copy->source.mipLevel = source->mipLevel;
            copy->source.arrayLayer = source->arrayLayer;
            copy->destination.buffer = destination->buffer;
            copy->destination.offset = destination->offset;
            if (destination->rowPitch == 0) {
                copy->destination.rowPitch =
                    ComputeDefaultRowPitch(source->texture->GetFormat(), copySize->width);
            } else {
                copy->destination.rowPitch = destination->rowPitch;
            }
            if (destination->imageHeight == 0) {
                copy->destination.imageHeight = copySize->height;
            } else {
                copy->destination.imageHeight = destination->imageHeight;
            }

            if (GetDevice()->IsValidationEnabled()) {
                mTopLevelTextures.insert(source->texture);
                mTopLevelBuffers.insert(destination->buffer);
            }
            return {};
        });
    }

    void CommandEncoder::CopyTextureToTexture(const TextureCopyView* source,
                                              const TextureCopyView* destination,
                                              const Extent3D* copySize) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            DAWN_TRY(GetDevice()->ValidateObject(source->texture));
            DAWN_TRY(GetDevice()->ValidateObject(destination->texture));

            CopyTextureToTextureCmd* copy =
                allocator->Allocate<CopyTextureToTextureCmd>(Command::CopyTextureToTexture);
            copy->source.texture = source->texture;
            copy->source.origin = source->origin;
            copy->source.mipLevel = source->mipLevel;
            copy->source.arrayLayer = source->arrayLayer;
            copy->destination.texture = destination->texture;
            copy->destination.origin = destination->origin;
            copy->destination.mipLevel = destination->mipLevel;
            copy->destination.arrayLayer = destination->arrayLayer;
            copy->copySize = *copySize;

            if (GetDevice()->IsValidationEnabled()) {
                mTopLevelTextures.insert(source->texture);
                mTopLevelTextures.insert(destination->texture);
            }
            return {};
        });
    }

    void CommandEncoder::InsertDebugMarker(const char* groupLabel) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            InsertDebugMarkerCmd* cmd =
                allocator->Allocate<InsertDebugMarkerCmd>(Command::InsertDebugMarker);
            cmd->length = strlen(groupLabel);

            char* label = allocator->AllocateData<char>(cmd->length + 1);
            memcpy(label, groupLabel, cmd->length + 1);

            return {};
        });
    }

    void CommandEncoder::PopDebugGroup() {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            allocator->Allocate<PopDebugGroupCmd>(Command::PopDebugGroup);

            return {};
        });
    }

    void CommandEncoder::PushDebugGroup(const char* groupLabel) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            PushDebugGroupCmd* cmd =
                allocator->Allocate<PushDebugGroupCmd>(Command::PushDebugGroup);
            cmd->length = strlen(groupLabel);

            char* label = allocator->AllocateData<char>(cmd->length + 1);
            memcpy(label, groupLabel, cmd->length + 1);

            return {};
        });
    }

    CommandBufferBase* CommandEncoder::Finish(const CommandBufferDescriptor* descriptor) {
        DeviceBase* device = GetDevice();
        // Even if mEncodingContext.Finish() validation fails, calling it will mutate the internal
        // state of the encoding context. The internal state is set to finished, and subsequent
        // calls to encode commands will generate errors.
        if (device->ConsumedError(mEncodingContext.Finish()) ||
            device->ConsumedError(device->ValidateIsAlive()) ||
            (device->IsValidationEnabled() &&
             device->ConsumedError(ValidateFinish(mEncodingContext.GetIterator(),
                                                  mEncodingContext.GetPassUsages())))) {
            return CommandBufferBase::MakeError(device);
        }
        ASSERT(!IsError());
        return device->CreateCommandBuffer(this, descriptor);
    }

    // Implementation of the command buffer validation that can be precomputed before submit
    MaybeError CommandEncoder::ValidateFinish(CommandIterator* commands,
                                              const PerPassUsages& perPassUsages) const {
        TRACE_EVENT0(GetDevice()->GetPlatform(), Validation, "CommandEncoder::ValidateFinish");
        DAWN_TRY(GetDevice()->ValidateObject(this));

        for (const PassResourceUsage& passUsage : perPassUsages) {
            DAWN_TRY(ValidatePassResourceUsage(passUsage));
        }

        uint64_t debugGroupStackSize = 0;

        commands->Reset();
        Command type;
        while (commands->NextCommandId(&type)) {
            switch (type) {
                case Command::BeginComputePass: {
                    commands->NextCommand<BeginComputePassCmd>();
                    DAWN_TRY(ValidateComputePass(commands));
                    break;
                }

                case Command::BeginRenderPass: {
                    const BeginRenderPassCmd* cmd = commands->NextCommand<BeginRenderPassCmd>();
                    DAWN_TRY(ValidateRenderPass(commands, cmd));
                    break;
                }

                case Command::CopyBufferToBuffer: {
                    const CopyBufferToBufferCmd* copy =
                        commands->NextCommand<CopyBufferToBufferCmd>();

                    DAWN_TRY(
                        ValidateCopySizeFitsInBuffer(copy->source, copy->sourceOffset, copy->size));
                    DAWN_TRY(ValidateCopySizeFitsInBuffer(copy->destination,
                                                          copy->destinationOffset, copy->size));
                    DAWN_TRY(ValidateB2BCopySizeAlignment(copy->size, copy->sourceOffset,
                                                          copy->destinationOffset));

                    DAWN_TRY(ValidateCanUseAs(copy->source.Get(), wgpu::BufferUsage::CopySrc));
                    DAWN_TRY(ValidateCanUseAs(copy->destination.Get(), wgpu::BufferUsage::CopyDst));
                    break;
                }

                case Command::CopyBufferToTexture: {
                    const CopyBufferToTextureCmd* copy =
                        commands->NextCommand<CopyBufferToTextureCmd>();

                    DAWN_TRY(
                        ValidateTextureSampleCountInCopyCommands(copy->destination.texture.Get()));

                    DAWN_TRY(ValidateImageHeight(copy->destination.texture->GetFormat(),
                                                 copy->source.imageHeight, copy->copySize.height));
                    DAWN_TRY(ValidateImageOrigin(copy->destination.texture->GetFormat(),
                                                 copy->destination.origin));
                    DAWN_TRY(ValidateImageCopySize(copy->destination.texture->GetFormat(),
                                                   copy->copySize));

                    uint32_t bufferCopySize = 0;
                    DAWN_TRY(ValidateRowPitch(copy->destination.texture->GetFormat(),
                                              copy->copySize, copy->source.rowPitch));

                    DAWN_TRY(ComputeTextureCopyBufferSize(
                        copy->destination.texture->GetFormat(), copy->copySize,
                        copy->source.rowPitch, copy->source.imageHeight, &bufferCopySize));

                    DAWN_TRY(ValidateCopySizeFitsInTexture(copy->destination, copy->copySize));
                    DAWN_TRY(ValidateCopySizeFitsInBuffer(copy->source, bufferCopySize));
                    DAWN_TRY(ValidateTexelBufferOffset(copy->source,
                                                       copy->destination.texture->GetFormat()));

                    DAWN_TRY(
                        ValidateCanUseAs(copy->source.buffer.Get(), wgpu::BufferUsage::CopySrc));
                    DAWN_TRY(ValidateCanUseAs(copy->destination.texture.Get(),
                                              wgpu::TextureUsage::CopyDst));
                    break;
                }

                case Command::CopyTextureToBuffer: {
                    const CopyTextureToBufferCmd* copy =
                        commands->NextCommand<CopyTextureToBufferCmd>();

                    DAWN_TRY(ValidateTextureSampleCountInCopyCommands(copy->source.texture.Get()));

                    DAWN_TRY(ValidateImageHeight(copy->source.texture->GetFormat(),
                                                 copy->destination.imageHeight,
                                                 copy->copySize.height));
                    DAWN_TRY(ValidateImageOrigin(copy->source.texture->GetFormat(),
                                                 copy->source.origin));
                    DAWN_TRY(
                        ValidateImageCopySize(copy->source.texture->GetFormat(), copy->copySize));

                    uint32_t bufferCopySize = 0;
                    DAWN_TRY(ValidateRowPitch(copy->source.texture->GetFormat(), copy->copySize,
                                              copy->destination.rowPitch));
                    DAWN_TRY(ComputeTextureCopyBufferSize(
                        copy->source.texture->GetFormat(), copy->copySize,
                        copy->destination.rowPitch, copy->destination.imageHeight,
                        &bufferCopySize));

                    DAWN_TRY(ValidateCopySizeFitsInTexture(copy->source, copy->copySize));
                    DAWN_TRY(ValidateCopySizeFitsInBuffer(copy->destination, bufferCopySize));
                    DAWN_TRY(ValidateTexelBufferOffset(copy->destination,
                                                       copy->source.texture->GetFormat()));

                    DAWN_TRY(
                        ValidateCanUseAs(copy->source.texture.Get(), wgpu::TextureUsage::CopySrc));
                    DAWN_TRY(ValidateCanUseAs(copy->destination.buffer.Get(),
                                              wgpu::BufferUsage::CopyDst));
                    break;
                }

                case Command::CopyTextureToTexture: {
                    const CopyTextureToTextureCmd* copy =
                        commands->NextCommand<CopyTextureToTextureCmd>();

                    DAWN_TRY(ValidateTextureToTextureCopyRestrictions(
                        copy->source, copy->destination, copy->copySize));

                    DAWN_TRY(ValidateImageOrigin(copy->source.texture->GetFormat(),
                                                 copy->source.origin));
                    DAWN_TRY(
                        ValidateImageCopySize(copy->source.texture->GetFormat(), copy->copySize));
                    DAWN_TRY(ValidateImageOrigin(copy->destination.texture->GetFormat(),
                                                 copy->destination.origin));
                    DAWN_TRY(ValidateImageCopySize(copy->destination.texture->GetFormat(),
                                                   copy->copySize));

                    DAWN_TRY(ValidateCopySizeFitsInTexture(copy->source, copy->copySize));
                    DAWN_TRY(ValidateCopySizeFitsInTexture(copy->destination, copy->copySize));

                    DAWN_TRY(
                        ValidateCanUseAs(copy->source.texture.Get(), wgpu::TextureUsage::CopySrc));
                    DAWN_TRY(ValidateCanUseAs(copy->destination.texture.Get(),
                                              wgpu::TextureUsage::CopyDst));
                    break;
                }

                case Command::InsertDebugMarker: {
                    const InsertDebugMarkerCmd* cmd = commands->NextCommand<InsertDebugMarkerCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    break;
                }

                case Command::PopDebugGroup: {
                    commands->NextCommand<PopDebugGroupCmd>();
                    DAWN_TRY(ValidateCanPopDebugGroup(debugGroupStackSize));
                    debugGroupStackSize--;
                    break;
                }

                case Command::PushDebugGroup: {
                    const PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    debugGroupStackSize++;
                    break;
                }
                default:
                    return DAWN_VALIDATION_ERROR("Command disallowed outside of a pass");
            }
        }

        DAWN_TRY(ValidateFinalDebugGroupStackSize(debugGroupStackSize));

        return {};
    }

}  // namespace dawn_native
