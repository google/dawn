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
#include "common/Math.h"
#include "dawn_native/BindGroup.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBuffer.h"
#include "dawn_native/CommandBufferStateTracker.h"
#include "dawn_native/CommandValidation.h"
#include "dawn_native/Commands.h"
#include "dawn_native/ComputePassEncoder.h"
#include "dawn_native/Device.h"
#include "dawn_native/ErrorData.h"
#include "dawn_native/QueryHelper.h"
#include "dawn_native/QuerySet.h"
#include "dawn_native/Queue.h"
#include "dawn_native/RenderPassEncoder.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/ValidationUtils_autogen.h"
#include "dawn_platform/DawnPlatform.h"
#include "dawn_platform/tracing/TraceEvent.h"

#include <cmath>
#include <map>

namespace dawn_native {

    namespace {

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

        MaybeError ValidateTextureSampleCountInBufferCopyCommands(const TextureBase* texture) {
            if (texture->GetSampleCount() > 1) {
                return DAWN_VALIDATION_ERROR(
                    "The sample count of textures must be 1 when copying between buffers and "
                    "textures");
            }

            return {};
        }

        MaybeError ValidateLinearTextureCopyOffset(const TextureDataLayout& layout,
                                                   const TexelBlockInfo& blockInfo) {
            if (layout.offset % blockInfo.byteSize != 0) {
                return DAWN_VALIDATION_ERROR(
                    "offset must be a multiple of the texel block byte size.");
            }
            return {};
        }

        MaybeError ValidateTextureDepthStencilToBufferCopyRestrictions(const TextureCopyView& src) {
            Aspect aspectUsed;
            DAWN_TRY_ASSIGN(aspectUsed, SingleAspectUsedByTextureCopyView(src));
            if (aspectUsed == Aspect::Depth) {
                switch (src.texture->GetFormat().format) {
                    case wgpu::TextureFormat::Depth24Plus:
                    case wgpu::TextureFormat::Depth24PlusStencil8:
                        return DAWN_VALIDATION_ERROR(
                            "The depth aspect of depth24plus texture cannot be selected in a "
                            "texture to buffer copy");
                        break;
                    case wgpu::TextureFormat::Depth32Float:
                        break;

                    default:
                        UNREACHABLE();
                }
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
            const Extent3D& attachmentSize =
                attachment->GetTexture()->GetMipLevelVirtualSize(attachment->GetBaseMipLevel());

            if (*width == 0) {
                DAWN_ASSERT(*height == 0);
                *width = attachmentSize.width;
                *height = attachmentSize.height;
                DAWN_ASSERT(*width != 0 && *height != 0);
            } else if (*width != attachmentSize.width || *height != attachmentSize.height) {
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
            DAWN_TRY(ValidateCanUseAs(colorAttachment.resolveTarget->GetTexture(),
                                      wgpu::TextureUsage::RenderAttachment));

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
            DAWN_TRY(ValidateCanUseAs(colorAttachment.attachment->GetTexture(),
                                      wgpu::TextureUsage::RenderAttachment));

            const TextureViewBase* attachment = colorAttachment.attachment;
            if (!(attachment->GetAspects() & Aspect::Color) ||
                !attachment->GetFormat().isRenderable) {
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
            DAWN_TRY(ValidateCanUseAs(depthStencilAttachment->attachment->GetTexture(),
                                      wgpu::TextureUsage::RenderAttachment));

            const TextureViewBase* attachment = depthStencilAttachment->attachment;
            if ((attachment->GetAspects() & (Aspect::Depth | Aspect::Stencil)) == Aspect::None ||
                !attachment->GetFormat().isRenderable) {
                return DAWN_VALIDATION_ERROR(
                    "The format of the texture view used as depth stencil attachment is not a "
                    "depth stencil format");
            }

            DAWN_TRY(ValidateLoadOp(depthStencilAttachment->depthLoadOp));
            DAWN_TRY(ValidateLoadOp(depthStencilAttachment->stencilLoadOp));
            DAWN_TRY(ValidateStoreOp(depthStencilAttachment->depthStoreOp));
            DAWN_TRY(ValidateStoreOp(depthStencilAttachment->stencilStoreOp));

            if (attachment->GetAspects() == (Aspect::Depth | Aspect::Stencil) &&
                depthStencilAttachment->depthReadOnly != depthStencilAttachment->stencilReadOnly) {
                return DAWN_VALIDATION_ERROR(
                    "depthReadOnly and stencilReadOnly must be the same when texture aspect is "
                    "'all'");
            }

            if (depthStencilAttachment->depthReadOnly &&
                (depthStencilAttachment->depthLoadOp != wgpu::LoadOp::Load ||
                 depthStencilAttachment->depthStoreOp != wgpu::StoreOp::Store)) {
                return DAWN_VALIDATION_ERROR(
                    "depthLoadOp must be load and depthStoreOp must be store when depthReadOnly "
                    "is true.");
            }

            if (depthStencilAttachment->stencilReadOnly &&
                (depthStencilAttachment->stencilLoadOp != wgpu::LoadOp::Load ||
                 depthStencilAttachment->stencilStoreOp != wgpu::StoreOp::Store)) {
                return DAWN_VALIDATION_ERROR(
                    "stencilLoadOp must be load and stencilStoreOp must be store when "
                    "stencilReadOnly "
                    "is true.");
            }

            if (depthStencilAttachment->depthLoadOp == wgpu::LoadOp::Clear &&
                std::isnan(depthStencilAttachment->clearDepth)) {
                return DAWN_VALIDATION_ERROR("Depth clear value cannot be NaN");
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

            if (descriptor->occlusionQuerySet != nullptr) {
                DAWN_TRY(device->ValidateObject(descriptor->occlusionQuerySet));

                // Occlusion query has not been implemented completely. Disallow it as unsafe until
                // the implementaion is completed.
                if (device->IsToggleEnabled(Toggle::DisallowUnsafeAPIs)) {
                    return DAWN_VALIDATION_ERROR(
                        "Occlusion query is disallowed because it has not been implemented "
                        "completely.");
                }

                if (descriptor->occlusionQuerySet->GetQueryType() != wgpu::QueryType::Occlusion) {
                    return DAWN_VALIDATION_ERROR("The type of query set must be Occlusion");
                }
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

        MaybeError ValidateQuerySetResolve(const QuerySetBase* querySet,
                                           uint32_t firstQuery,
                                           uint32_t queryCount,
                                           const BufferBase* destination,
                                           uint64_t destinationOffset) {
            if (firstQuery >= querySet->GetQueryCount()) {
                return DAWN_VALIDATION_ERROR("Query index out of bounds");
            }

            if (queryCount > querySet->GetQueryCount() - firstQuery) {
                return DAWN_VALIDATION_ERROR(
                    "The sum of firstQuery and queryCount exceeds the number of queries in query "
                    "set");
            }

            // TODO(hao.x.li@intel.com): Validate that the queries between [firstQuery, firstQuery +
            // queryCount - 1] must be available(written by query operations).

            // The destinationOffset must be a multiple of 8 bytes on D3D12 and Vulkan
            if (destinationOffset % 8 != 0) {
                return DAWN_VALIDATION_ERROR(
                    "The alignment offset into the destination buffer must be a multiple of 8 "
                    "bytes");
            }

            uint64_t bufferSize = destination->GetSize();
            // The destination buffer must have enough storage, from destination offset, to contain
            // the result of resolved queries
            bool fitsInBuffer = destinationOffset <= bufferSize &&
                                (static_cast<uint64_t>(queryCount) * sizeof(uint64_t) <=
                                 (bufferSize - destinationOffset));
            if (!fitsInBuffer) {
                return DAWN_VALIDATION_ERROR("The resolved query data would overflow the buffer");
            }

            return {};
        }

        void EncodeTimestampsToNanosecondsConversion(CommandEncoder* encoder,
                                                     QuerySetBase* querySet,
                                                     uint32_t queryCount,
                                                     BufferBase* destination,
                                                     uint64_t destinationOffset) {
            DeviceBase* device = encoder->GetDevice();

            std::vector<uint32_t> availability;
            auto it = encoder->GetQueryAvailabilityMap().find(querySet);
            if (it != encoder->GetQueryAvailabilityMap().end()) {
                availability = {it->second.begin(), it->second.end()};
            } else {
                availability.resize(querySet->GetQueryCount());
            }

            // Timestamp availability storage buffer
            BufferDescriptor availabilityDesc = {};
            availabilityDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            availabilityDesc.size = querySet->GetQueryCount() * sizeof(uint32_t);
            Ref<BufferBase> availabilityBuffer =
                AcquireRef(device->CreateBuffer(&availabilityDesc));
            device->GetDefaultQueue()->WriteBuffer(availabilityBuffer.Get(), 0, availability.data(),
                                                   querySet->GetQueryCount() * sizeof(uint32_t));

            // Timestamp params uniform buffer
            TimestampParams params = {queryCount, static_cast<uint32_t>(destinationOffset),
                                      device->GetTimestampPeriodInNS()};
            BufferDescriptor parmsDesc = {};
            parmsDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
            parmsDesc.size = sizeof(params);
            Ref<BufferBase> paramsBuffer = AcquireRef(device->CreateBuffer(&parmsDesc));
            device->GetDefaultQueue()->WriteBuffer(paramsBuffer.Get(), 0, &params, sizeof(params));

            EncodeConvertTimestampsToNanoseconds(encoder, destination, availabilityBuffer.Get(),
                                                 paramsBuffer.Get());
        }

    }  // namespace

    CommandEncoder::CommandEncoder(DeviceBase* device, const CommandEncoderDescriptor*)
        : ObjectBase(device), mEncodingContext(device, this) {
    }

    CommandBufferResourceUsage CommandEncoder::AcquireResourceUsages() {
        return CommandBufferResourceUsage{mEncodingContext.AcquirePassUsages(),
                                          std::move(mTopLevelBuffers), std::move(mTopLevelTextures),
                                          std::move(mUsedQuerySets)};
    }

    CommandIterator CommandEncoder::AcquireCommands() {
        return mEncodingContext.AcquireCommands();
    }

    void CommandEncoder::TrackUsedQuerySet(QuerySetBase* querySet) {
        mUsedQuerySets.insert(querySet);
    }

    void CommandEncoder::TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex) {
        DAWN_ASSERT(querySet != nullptr);

        if (GetDevice()->IsValidationEnabled()) {
            TrackUsedQuerySet(querySet);
        }

        // Gets the iterator for that querySet or create a new vector of bool set to false
        // if the querySet wasn't registered.
        auto it = mQueryAvailabilityMap.emplace(querySet, querySet->GetQueryCount()).first;
        it->second[queryIndex] = 1;
    }

    const QueryAvailabilityMap& CommandEncoder::GetQueryAvailabilityMap() const {
        return mQueryAvailabilityMap;
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

        uint32_t width = 0;
        uint32_t height = 0;
        bool success =
            mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
                uint32_t sampleCount = 0;

                DAWN_TRY(ValidateRenderPassDescriptor(device, descriptor, &width, &height,
                                                      &sampleCount));

                ASSERT(width > 0 && height > 0 && sampleCount > 0);

                BeginRenderPassCmd* cmd =
                    allocator->Allocate<BeginRenderPassCmd>(Command::BeginRenderPass);

                cmd->attachmentState = device->GetOrCreateAttachmentState(descriptor);

                for (ColorAttachmentIndex index :
                     IterateBitSet(cmd->attachmentState->GetColorAttachmentsMask())) {
                    uint8_t i = static_cast<uint8_t>(index);
                    TextureViewBase* view = descriptor->colorAttachments[i].attachment;
                    TextureViewBase* resolveTarget = descriptor->colorAttachments[i].resolveTarget;

                    cmd->colorAttachments[index].view = view;
                    cmd->colorAttachments[index].resolveTarget = resolveTarget;
                    cmd->colorAttachments[index].loadOp = descriptor->colorAttachments[i].loadOp;
                    cmd->colorAttachments[index].storeOp = descriptor->colorAttachments[i].storeOp;
                    cmd->colorAttachments[index].clearColor =
                        descriptor->colorAttachments[i].clearColor;

                    usageTracker.TextureViewUsedAs(view, wgpu::TextureUsage::RenderAttachment);

                    if (resolveTarget != nullptr) {
                        usageTracker.TextureViewUsedAs(resolveTarget,
                                                       wgpu::TextureUsage::RenderAttachment);
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

                    usageTracker.TextureViewUsedAs(view, wgpu::TextureUsage::RenderAttachment);
                }

                cmd->width = width;
                cmd->height = height;

                return {};
            });

        if (success) {
            RenderPassEncoder* passEncoder =
                new RenderPassEncoder(device, this, &mEncodingContext, std::move(usageTracker),
                                      descriptor->occlusionQuerySet, width, height);
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

            // Skip noop copies. Some backends validation rules disallow them.
            if (size != 0) {
                CopyBufferToBufferCmd* copy =
                    allocator->Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
                copy->source = source;
                copy->sourceOffset = sourceOffset;
                copy->destination = destination;
                copy->destinationOffset = destinationOffset;
                copy->size = size;
            }

            return {};
        });
    }

    void CommandEncoder::CopyBufferToTexture(const BufferCopyView* source,
                                             const TextureCopyView* destination,
                                             const Extent3D* copySize) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateBufferCopyView(GetDevice(), *source));
                DAWN_TRY(ValidateCanUseAs(source->buffer, wgpu::BufferUsage::CopySrc));

                DAWN_TRY(ValidateTextureCopyView(GetDevice(), *destination, *copySize));
                DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::CopyDst));
                DAWN_TRY(ValidateTextureSampleCountInBufferCopyCommands(destination->texture));

                DAWN_TRY(ValidateLinearToDepthStencilCopyRestrictions(*destination));
                // We validate texture copy range before validating linear texture data,
                // because in the latter we divide copyExtent.width by blockWidth and
                // copyExtent.height by blockHeight while the divisibility conditions are
                // checked in validating texture copy range.
                DAWN_TRY(ValidateTextureCopyRange(*destination, *copySize));
            }
            const TexelBlockInfo& blockInfo =
                destination->texture->GetFormat().GetAspectInfo(destination->aspect).block;
            TextureDataLayout srcLayout = FixUpDeprecatedTextureDataLayoutOptions(
                GetDevice(), source->layout, blockInfo, *copySize);
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateLinearTextureCopyOffset(srcLayout, blockInfo));
                DAWN_TRY(ValidateLinearTextureData(srcLayout, source->buffer->GetSize(), blockInfo,
                                                   *copySize));

                mTopLevelBuffers.insert(source->buffer);
                mTopLevelTextures.insert(destination->texture);
            }

            ApplyDefaultTextureDataLayoutOptions(&srcLayout, blockInfo, *copySize);

            // Skip noop copies.
            if (copySize->width != 0 && copySize->height != 0 && copySize->depth != 0) {
                // Record the copy command.
                CopyBufferToTextureCmd* copy =
                    allocator->Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
                copy->source.buffer = source->buffer;
                copy->source.offset = srcLayout.offset;
                copy->source.bytesPerRow = srcLayout.bytesPerRow;
                copy->source.rowsPerImage = srcLayout.rowsPerImage;
                copy->destination.texture = destination->texture;
                copy->destination.origin = destination->origin;
                copy->destination.mipLevel = destination->mipLevel;
                copy->destination.aspect =
                    ConvertAspect(destination->texture->GetFormat(), destination->aspect);
                copy->copySize = *copySize;
            }

            return {};
        });
    }

    void CommandEncoder::CopyTextureToBuffer(const TextureCopyView* source,
                                             const BufferCopyView* destination,
                                             const Extent3D* copySize) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateTextureCopyView(GetDevice(), *source, *copySize));
                DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc));
                DAWN_TRY(ValidateTextureSampleCountInBufferCopyCommands(source->texture));
                DAWN_TRY(ValidateTextureDepthStencilToBufferCopyRestrictions(*source));

                DAWN_TRY(ValidateBufferCopyView(GetDevice(), *destination));
                DAWN_TRY(ValidateCanUseAs(destination->buffer, wgpu::BufferUsage::CopyDst));

                // We validate texture copy range before validating linear texture data,
                // because in the latter we divide copyExtent.width by blockWidth and
                // copyExtent.height by blockHeight while the divisibility conditions are
                // checked in validating texture copy range.
                DAWN_TRY(ValidateTextureCopyRange(*source, *copySize));
            }
            const TexelBlockInfo& blockInfo =
                source->texture->GetFormat().GetAspectInfo(source->aspect).block;
            TextureDataLayout dstLayout = FixUpDeprecatedTextureDataLayoutOptions(
                GetDevice(), destination->layout, blockInfo, *copySize);
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateLinearTextureCopyOffset(dstLayout, blockInfo));
                DAWN_TRY(ValidateLinearTextureData(dstLayout, destination->buffer->GetSize(),
                                                   blockInfo, *copySize));

                mTopLevelTextures.insert(source->texture);
                mTopLevelBuffers.insert(destination->buffer);
            }

            ApplyDefaultTextureDataLayoutOptions(&dstLayout, blockInfo, *copySize);

            // Skip noop copies.
            if (copySize->width != 0 && copySize->height != 0 && copySize->depth != 0) {
                // Record the copy command.
                CopyTextureToBufferCmd* copy =
                    allocator->Allocate<CopyTextureToBufferCmd>(Command::CopyTextureToBuffer);
                copy->source.texture = source->texture;
                copy->source.origin = source->origin;
                copy->source.mipLevel = source->mipLevel;
                copy->source.aspect = ConvertAspect(source->texture->GetFormat(), source->aspect);
                copy->destination.buffer = destination->buffer;
                copy->destination.offset = dstLayout.offset;
                copy->destination.bytesPerRow = dstLayout.bytesPerRow;
                copy->destination.rowsPerImage = dstLayout.rowsPerImage;
                copy->copySize = *copySize;
            }

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

                DAWN_TRY(ValidateTextureCopyView(GetDevice(), *source, *copySize));
                DAWN_TRY(ValidateTextureCopyView(GetDevice(), *destination, *copySize));

                DAWN_TRY(
                    ValidateTextureToTextureCopyRestrictions(*source, *destination, *copySize));

                DAWN_TRY(ValidateTextureCopyRange(*source, *copySize));
                DAWN_TRY(ValidateTextureCopyRange(*destination, *copySize));

                DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc));
                DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::CopyDst));

                mTopLevelTextures.insert(source->texture);
                mTopLevelTextures.insert(destination->texture);
            }

            // Skip noop copies.
            if (copySize->width != 0 && copySize->height != 0 && copySize->depth != 0) {
                CopyTextureToTextureCmd* copy =
                    allocator->Allocate<CopyTextureToTextureCmd>(Command::CopyTextureToTexture);
                copy->source.texture = source->texture;
                copy->source.origin = source->origin;
                copy->source.mipLevel = source->mipLevel;
                copy->source.aspect = ConvertAspect(source->texture->GetFormat(), source->aspect);
                copy->destination.texture = destination->texture;
                copy->destination.origin = destination->origin;
                copy->destination.mipLevel = destination->mipLevel;
                copy->destination.aspect =
                    ConvertAspect(destination->texture->GetFormat(), destination->aspect);
                copy->copySize = *copySize;
            }

            return {};
        });
    }

    void CommandEncoder::InjectValidationError(const char* message) {
        if (mEncodingContext.CheckCurrentEncoder(this)) {
            mEncodingContext.HandleError(InternalErrorType::Validation, message);
        }
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
            if (GetDevice()->IsValidationEnabled()) {
                if (mDebugGroupStackSize == 0) {
                    return DAWN_VALIDATION_ERROR("Pop must be balanced by a corresponding Push.");
                }
            }
            allocator->Allocate<PopDebugGroupCmd>(Command::PopDebugGroup);
            mDebugGroupStackSize--;

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

            mDebugGroupStackSize++;

            return {};
        });
    }

    void CommandEncoder::ResolveQuerySet(QuerySetBase* querySet,
                                         uint32_t firstQuery,
                                         uint32_t queryCount,
                                         BufferBase* destination,
                                         uint64_t destinationOffset) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(querySet));
                DAWN_TRY(GetDevice()->ValidateObject(destination));

                DAWN_TRY(ValidateQuerySetResolve(querySet, firstQuery, queryCount, destination,
                                                 destinationOffset));

                DAWN_TRY(ValidateCanUseAs(destination, wgpu::BufferUsage::QueryResolve));

                TrackUsedQuerySet(querySet);
                mTopLevelBuffers.insert(destination);
            }

            ResolveQuerySetCmd* cmd =
                allocator->Allocate<ResolveQuerySetCmd>(Command::ResolveQuerySet);
            cmd->querySet = querySet;
            cmd->firstQuery = firstQuery;
            cmd->queryCount = queryCount;
            cmd->destination = destination;
            cmd->destinationOffset = destinationOffset;

            // Encode internal compute pipeline for timestamp query
            if (querySet->GetQueryType() == wgpu::QueryType::Timestamp) {
                EncodeTimestampsToNanosecondsConversion(this, querySet, queryCount, destination,
                                                        destinationOffset);
            }

            return {};
        });
    }

    void CommandEncoder::WriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex) {
        mEncodingContext.TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(querySet));
                DAWN_TRY(ValidateTimestampQuery(querySet, queryIndex));
            }

            TrackQueryAvailability(querySet, queryIndex);

            WriteTimestampCmd* cmd =
                allocator->Allocate<WriteTimestampCmd>(Command::WriteTimestamp);
            cmd->querySet = querySet;
            cmd->queryIndex = queryIndex;

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

        if (mDebugGroupStackSize != 0) {
            return DAWN_VALIDATION_ERROR("Each Push must be balanced by a corresponding Pop.");
        }

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
                    break;
                }

                case Command::PushDebugGroup: {
                    const PushDebugGroupCmd* cmd = commands->NextCommand<PushDebugGroupCmd>();
                    commands->NextData<char>(cmd->length + 1);
                    break;
                }

                case Command::ResolveQuerySet: {
                    commands->NextCommand<ResolveQuerySetCmd>();
                    break;
                }

                case Command::WriteTimestamp: {
                    commands->NextCommand<WriteTimestampCmd>();
                    break;
                }
                default:
                    return DAWN_VALIDATION_ERROR("Command disallowed outside of a pass");
            }
        }

        return {};
    }

}  // namespace dawn_native
