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

        // TODO(jiawei.shao@intel.com): add validations on the texture-to-texture copies within the
        // same texture.
        MaybeError ValidateCopySizeFitsInTexture(const TextureCopyView& textureCopy,
                                                 const Extent3D& copySize) {
            const TextureBase* texture = textureCopy.texture;
            if (textureCopy.mipLevel >= texture->GetNumMipLevels()) {
                return DAWN_VALIDATION_ERROR("Copy mipLevel out of range");
            }

            if (static_cast<uint64_t>(textureCopy.arrayLayer) +
                    static_cast<uint64_t>(copySize.depth) >
                static_cast<uint64_t>(texture->GetArrayLayers())) {
                return DAWN_VALIDATION_ERROR("Copy arrayLayer out of range");
            }

            Extent3D extent = texture->GetMipLevelPhysicalSize(textureCopy.mipLevel);

            // All texture dimensions are in uint32_t so by doing checks in uint64_t we avoid
            // overflows.
            if (static_cast<uint64_t>(textureCopy.origin.x) +
                        static_cast<uint64_t>(copySize.width) >
                    static_cast<uint64_t>(extent.width) ||
                static_cast<uint64_t>(textureCopy.origin.y) +
                        static_cast<uint64_t>(copySize.height) >
                    static_cast<uint64_t>(extent.height)) {
                return DAWN_VALIDATION_ERROR("Copy would touch outside of the texture");
            }

            // TODO(cwallez@chromium.org): Check the depth bound differently for 3D textures.
            if (textureCopy.origin.z != 0) {
                return DAWN_VALIDATION_ERROR("No support for z != 0 for now");
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

        MaybeError ValidateCopySizeFitsInBuffer(const BufferCopyView& bufferCopy,
                                                uint64_t dataSize) {
            return ValidateCopySizeFitsInBuffer(bufferCopy.buffer, bufferCopy.offset, dataSize);
        }

        MaybeError ValidateB2BCopyAlignment(uint64_t dataSize,
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

        MaybeError ValidateTexelBufferOffset(const BufferCopyView& bufferCopy,
                                             const Format& format) {
            if (bufferCopy.offset % format.blockByteSize != 0) {
                return DAWN_VALIDATION_ERROR(
                    "Buffer offset must be a multiple of the texel or block size");
            }

            return {};
        }

        MaybeError ValidateRowsPerImage(const Format& format,
                                        uint32_t rowsPerImage,
                                        uint32_t copyHeight) {
            if (rowsPerImage < copyHeight) {
                return DAWN_VALIDATION_ERROR("rowsPerImage must not be less than the copy height.");
            }

            if (rowsPerImage % format.blockHeight != 0) {
                return DAWN_VALIDATION_ERROR(
                    "rowsPerImage must be a multiple of compressed texture format block height");
            }

            return {};
        }

        MaybeError ValidateTextureSampleCountInCopyCommands(const TextureBase* texture) {
            if (texture->GetSampleCount() > 1) {
                return DAWN_VALIDATION_ERROR("The sample count of textures must be 1");
            }

            return {};
        }

        MaybeError ValidateEntireSubresourceCopied(const TextureCopyView& src,
                                                   const TextureCopyView& dst,
                                                   const Extent3D& copySize) {
            Extent3D srcSize = src.texture->GetSize();

            if (dst.origin.x != 0 || dst.origin.y != 0 || dst.origin.z != 0 ||
                srcSize.width != copySize.width || srcSize.height != copySize.height ||
                srcSize.depth != copySize.depth) {
                return DAWN_VALIDATION_ERROR(
                    "The entire subresource must be copied when using a depth/stencil texture or "
                    "when samples are greater than 1.");
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
            } else if (srcSamples > 1) {
                // D3D12 requires entire subresource to be copied when using CopyTextureRegion when
                // samples > 1.
                DAWN_TRY(ValidateEntireSubresourceCopied(src, dst, copySize));
            }

            if (src.texture->GetFormat().format != dst.texture->GetFormat().format) {
                // Metal requires texture-to-texture copies be the same format
                return DAWN_VALIDATION_ERROR("Source and destination texture formats must match.");
            }

            if (src.texture->GetFormat().HasDepthOrStencil()) {
                // D3D12 requires entire subresource to be copied when using CopyTextureRegion is
                // used with depth/stencil.
                DAWN_TRY(ValidateEntireSubresourceCopied(src, dst, copySize));
            }

            return {};
        }

        MaybeError ComputeTextureCopyBufferSize(const Format& textureFormat,
                                                const Extent3D& copySize,
                                                uint32_t bytesPerRow,
                                                uint32_t rowsPerImage,
                                                uint32_t* bufferSize) {
            ASSERT(rowsPerImage >= copySize.height);
            uint32_t blockByteSize = textureFormat.blockByteSize;
            uint32_t blockWidth = textureFormat.blockWidth;
            uint32_t blockHeight = textureFormat.blockHeight;

            // TODO(cwallez@chromium.org): check for overflows
            uint32_t slicePitch = bytesPerRow * rowsPerImage / blockWidth;
            uint32_t sliceSize = bytesPerRow * (copySize.height / blockHeight - 1) +
                                 (copySize.width / blockWidth) * blockByteSize;
            *bufferSize = (slicePitch * (copySize.depth - 1)) + sliceSize;

            return {};
        }

        uint32_t ComputeDefaultBytesPerRow(const Format& format, uint32_t width) {
            return width / format.blockWidth * format.blockByteSize;
        }

        MaybeError ValidateBytesPerRow(const Format& format,
                                       const Extent3D& copySize,
                                       uint32_t bytesPerRow) {
            if (bytesPerRow % kTextureBytesPerRowAlignment != 0) {
                return DAWN_VALIDATION_ERROR("bytesPerRow must be a multiple of 256");
            }

            if (bytesPerRow < copySize.width / format.blockWidth * format.blockByteSize) {
                return DAWN_VALIDATION_ERROR(
                    "bytesPerRow must not be less than the number of bytes per row");
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

        PassResourceUsageTracker usageTracker(PassType::Render);
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

                    usageTracker.TextureViewUsedAs(view, wgpu::TextureUsage::OutputAttachment);

                    if (resolveTarget != nullptr) {
                        usageTracker.TextureViewUsedAs(resolveTarget,
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

                    usageTracker.TextureViewUsedAs(view, wgpu::TextureUsage::OutputAttachment);
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
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source));
                DAWN_TRY(GetDevice()->ValidateObject(destination));

                if (source == destination) {
                    return DAWN_VALIDATION_ERROR(
                        "Source and destination cannot be the same buffer.");
                }

                DAWN_TRY(ValidateCopySizeFitsInBuffer(source, sourceOffset, size));
                DAWN_TRY(ValidateCopySizeFitsInBuffer(destination, destinationOffset, size));
                DAWN_TRY(ValidateB2BCopyAlignment(size, sourceOffset, destinationOffset));

                DAWN_TRY(ValidateCanUseAs(source, wgpu::BufferUsage::CopySrc));
                DAWN_TRY(ValidateCanUseAs(destination, wgpu::BufferUsage::CopyDst));

                mTopLevelBuffers.insert(source);
                mTopLevelBuffers.insert(destination);
            }

            CopyBufferToBufferCmd* copy =
                allocator->Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
            copy->source = source;
            copy->sourceOffset = sourceOffset;
            copy->destination = destination;
            copy->destinationOffset = destinationOffset;
            copy->size = size;

            return {};
        });
    }

    void CommandEncoder::CopyBufferToTexture(const BufferCopyView* source,
                                             const TextureCopyView* destination,
                                             const Extent3D* copySize) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            // Validate objects before doing the defaulting.
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source->buffer));
                DAWN_TRY(GetDevice()->ValidateObject(destination->texture));
            }

            // Compute default values for bytesPerRow/rowsPerImage
            uint32_t defaultedBytesPerRow = source->bytesPerRow;
            if (defaultedBytesPerRow == 0) {
                defaultedBytesPerRow =
                    ComputeDefaultBytesPerRow(destination->texture->GetFormat(), copySize->width);
            }

            uint32_t defaultedRowsPerImage = source->rowsPerImage;
            if (defaultedRowsPerImage == 0) {
                defaultedRowsPerImage = copySize->height;
            }

            // Perform the rest of the validation using the default values.
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateTextureSampleCountInCopyCommands(destination->texture));

                DAWN_TRY(ValidateRowsPerImage(destination->texture->GetFormat(),
                                              defaultedRowsPerImage, copySize->height));
                DAWN_TRY(
                    ValidateImageOrigin(destination->texture->GetFormat(), destination->origin));
                DAWN_TRY(ValidateImageCopySize(destination->texture->GetFormat(), *copySize));

                uint32_t bufferCopySize = 0;
                DAWN_TRY(ValidateBytesPerRow(destination->texture->GetFormat(), *copySize,
                                             defaultedBytesPerRow));

                DAWN_TRY(ComputeTextureCopyBufferSize(destination->texture->GetFormat(), *copySize,
                                                      defaultedBytesPerRow, defaultedRowsPerImage,
                                                      &bufferCopySize));

                DAWN_TRY(ValidateCopySizeFitsInTexture(*destination, *copySize));
                DAWN_TRY(ValidateCopySizeFitsInBuffer(*source, bufferCopySize));
                DAWN_TRY(ValidateTexelBufferOffset(*source, destination->texture->GetFormat()));

                DAWN_TRY(ValidateCanUseAs(source->buffer, wgpu::BufferUsage::CopySrc));
                DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::CopyDst));

                mTopLevelBuffers.insert(source->buffer);
                mTopLevelTextures.insert(destination->texture);
            }

            // Record the copy command.
            CopyBufferToTextureCmd* copy =
                allocator->Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
            copy->source.buffer = source->buffer;
            copy->source.offset = source->offset;
            copy->source.bytesPerRow = defaultedBytesPerRow;
            copy->source.rowsPerImage = defaultedRowsPerImage;
            copy->destination.texture = destination->texture;
            copy->destination.origin = destination->origin;
            copy->copySize = *copySize;
            copy->destination.mipLevel = destination->mipLevel;
            copy->destination.arrayLayer = destination->arrayLayer;

            return {};
        });
    }

    void CommandEncoder::CopyTextureToBuffer(const TextureCopyView* source,
                                             const BufferCopyView* destination,
                                             const Extent3D* copySize) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            // Validate objects before doing the defaulting.
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source->texture));
                DAWN_TRY(GetDevice()->ValidateObject(destination->buffer));
            }

            // Compute default values for bytesPerRow/rowsPerImage
            uint32_t defaultedBytesPerRow = destination->bytesPerRow;
            if (defaultedBytesPerRow == 0) {
                defaultedBytesPerRow =
                    ComputeDefaultBytesPerRow(source->texture->GetFormat(), copySize->width);
            }

            uint32_t defaultedRowsPerImage = destination->rowsPerImage;
            if (defaultedRowsPerImage == 0) {
                defaultedRowsPerImage = copySize->height;
            }

            // Perform the rest of the validation using the default values.
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateTextureSampleCountInCopyCommands(source->texture));

                DAWN_TRY(ValidateRowsPerImage(source->texture->GetFormat(), defaultedRowsPerImage,
                                              copySize->height));
                DAWN_TRY(ValidateImageOrigin(source->texture->GetFormat(), source->origin));
                DAWN_TRY(ValidateImageCopySize(source->texture->GetFormat(), *copySize));

                uint32_t bufferCopySize = 0;
                DAWN_TRY(ValidateBytesPerRow(source->texture->GetFormat(), *copySize,
                                             defaultedBytesPerRow));
                DAWN_TRY(ComputeTextureCopyBufferSize(source->texture->GetFormat(), *copySize,
                                                      defaultedBytesPerRow, defaultedRowsPerImage,
                                                      &bufferCopySize));

                DAWN_TRY(ValidateCopySizeFitsInTexture(*source, *copySize));
                DAWN_TRY(ValidateCopySizeFitsInBuffer(*destination, bufferCopySize));
                DAWN_TRY(ValidateTexelBufferOffset(*destination, source->texture->GetFormat()));

                DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc));
                DAWN_TRY(ValidateCanUseAs(destination->buffer, wgpu::BufferUsage::CopyDst));

                mTopLevelTextures.insert(source->texture);
                mTopLevelBuffers.insert(destination->buffer);
            }

            // Record the copy command.
            CopyTextureToBufferCmd* copy =
                allocator->Allocate<CopyTextureToBufferCmd>(Command::CopyTextureToBuffer);
            copy->source.texture = source->texture;
            copy->source.origin = source->origin;
            copy->copySize = *copySize;
            copy->source.mipLevel = source->mipLevel;
            copy->source.arrayLayer = source->arrayLayer;
            copy->destination.buffer = destination->buffer;
            copy->destination.offset = destination->offset;
            copy->destination.bytesPerRow = defaultedBytesPerRow;
            copy->destination.rowsPerImage = defaultedRowsPerImage;

            return {};
        });
    }

    void CommandEncoder::CopyTextureToTexture(const TextureCopyView* source,
                                              const TextureCopyView* destination,
                                              const Extent3D* copySize) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source->texture));
                DAWN_TRY(GetDevice()->ValidateObject(destination->texture));

                DAWN_TRY(
                    ValidateTextureToTextureCopyRestrictions(*source, *destination, *copySize));

                DAWN_TRY(ValidateImageOrigin(source->texture->GetFormat(), source->origin));
                DAWN_TRY(ValidateImageCopySize(source->texture->GetFormat(), *copySize));
                DAWN_TRY(
                    ValidateImageOrigin(destination->texture->GetFormat(), destination->origin));
                DAWN_TRY(ValidateImageCopySize(destination->texture->GetFormat(), *copySize));

                DAWN_TRY(ValidateCopySizeFitsInTexture(*source, *copySize));
                DAWN_TRY(ValidateCopySizeFitsInTexture(*destination, *copySize));

                DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc));
                DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::CopyDst));

                mTopLevelTextures.insert(source->texture);
                mTopLevelTextures.insert(destination->texture);
            }

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
                    commands->NextCommand<CopyBufferToBufferCmd>();
                    break;
                }

                case Command::CopyBufferToTexture: {
                    commands->NextCommand<CopyBufferToTextureCmd>();
                    break;
                }

                case Command::CopyTextureToBuffer: {
                    commands->NextCommand<CopyTextureToBufferCmd>();
                    break;
                }

                case Command::CopyTextureToTexture: {
                    commands->NextCommand<CopyTextureToTextureCmd>();
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
