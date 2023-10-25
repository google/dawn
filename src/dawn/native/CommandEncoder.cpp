// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/CommandEncoder.h"

#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Math.h"
#include "dawn/native/ApplyClearColorValueWithDrawHelper.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/BlitBufferToDepthStencil.h"
#include "dawn/native/BlitColorToColorWithDraw.h"
#include "dawn/native/BlitDepthToDepth.h"
#include "dawn/native/BlitTextureToBuffer.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/CommandBufferStateTracker.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/ComputePassEncoder.h"
#include "dawn/native/Device.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/QueryHelper.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/Queue.h"
#include "dawn/native/RenderPassEncoder.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native {

namespace {

MaybeError ValidateB2BCopyAlignment(uint64_t dataSize, uint64_t srcOffset, uint64_t dstOffset) {
    // Copy size must be a multiple of 4 bytes on macOS.
    DAWN_INVALID_IF(dataSize % 4 != 0, "Copy size (%u) is not a multiple of 4.", dataSize);

    // SourceOffset and destinationOffset must be multiples of 4 bytes on macOS.
    DAWN_INVALID_IF(srcOffset % 4 != 0 || dstOffset % 4 != 0,
                    "Source offset (%u) or destination offset (%u) is not a multiple of 4 bytes,",
                    srcOffset, dstOffset);

    return {};
}

MaybeError ValidateTextureSampleCountInBufferCopyCommands(const TextureBase* texture) {
    DAWN_INVALID_IF(texture->GetSampleCount() > 1,
                    "%s sample count (%u) is not 1 when copying to or from a buffer.", texture,
                    texture->GetSampleCount());

    return {};
}

MaybeError ValidateLinearTextureCopyOffset(const TextureDataLayout& layout,
                                           const TexelBlockInfo& blockInfo,
                                           const bool hasDepthOrStencil) {
    if (hasDepthOrStencil) {
        // For depth-stencil texture, buffer offset must be a multiple of 4.
        DAWN_INVALID_IF(layout.offset % 4 != 0,
                        "Offset (%u) is not a multiple of 4 for depth/stencil texture.",
                        layout.offset);
    } else {
        DAWN_INVALID_IF(layout.offset % blockInfo.byteSize != 0,
                        "Offset (%u) is not a multiple of the texel block byte size (%u).",
                        layout.offset, blockInfo.byteSize);
    }
    return {};
}

MaybeError ValidateTextureFormatForTextureToBufferCopyInCompatibilityMode(
    const TextureBase* texture) {
    DAWN_INVALID_IF(texture->GetFormat().isCompressed,
                    "%s with format %s cannot be used as the source in a texture to buffer copy in "
                    "compatibility mode.",
                    texture, texture->GetFormat().format);
    return {};
}

MaybeError ValidateSourceTextureFormatForTextureToTextureCopyInCompatibilityMode(
    const TextureBase* texture) {
    DAWN_INVALID_IF(
        texture->GetFormat().isCompressed,
        "%s with format %s cannot be used as the source in a texture to texture copy in "
        "compatibility mode.",
        texture, texture->GetFormat().format);
    return {};
}

MaybeError ValidateTextureDepthStencilToBufferCopyRestrictions(const ImageCopyTexture& src) {
    Aspect aspectUsed;
    DAWN_TRY_ASSIGN(aspectUsed, SingleAspectUsedByImageCopyTexture(src));
    if (aspectUsed == Aspect::Depth) {
        switch (src.texture->GetFormat().format) {
            case wgpu::TextureFormat::Depth24Plus:
            case wgpu::TextureFormat::Depth24PlusStencil8:
                return DAWN_VALIDATION_ERROR(
                    "The depth aspect of %s format %s cannot be selected in a texture to "
                    "buffer copy.",
                    src.texture, src.texture->GetFormat().format);
            case wgpu::TextureFormat::Depth32Float:
            case wgpu::TextureFormat::Depth16Unorm:
            case wgpu::TextureFormat::Depth32FloatStencil8:
                break;

            default:
                DAWN_UNREACHABLE();
        }
    }

    return {};
}

MaybeError ValidateAttachmentArrayLayersAndLevelCount(const TextureViewBase* attachment) {
    // Currently we do not support layered rendering.
    DAWN_INVALID_IF(attachment->GetLayerCount() > 1,
                    "The layer count (%u) of %s used as attachment is greater than 1.",
                    attachment->GetLayerCount(), attachment);

    DAWN_INVALID_IF(attachment->GetLevelCount() > 1,
                    "The mip level count (%u) of %s used as attachment is greater than 1.",
                    attachment->GetLevelCount(), attachment);

    return {};
}

MaybeError ValidateOrSetAttachmentSize(const TextureViewBase* attachment,
                                       uint32_t* width,
                                       uint32_t* height) {
    Extent3D attachmentSize = attachment->GetSingleSubresourceVirtualSize();

    if (*width == 0) {
        DAWN_ASSERT(*height == 0);
        *width = attachmentSize.width;
        *height = attachmentSize.height;
        DAWN_ASSERT(*width != 0 && *height != 0);
    } else {
        DAWN_INVALID_IF(*width != attachmentSize.width || *height != attachmentSize.height,
                        "Attachment %s size (width: %u, height: %u) does not match the size of the "
                        "other attachments (width: %u, height: %u).",
                        attachment, attachmentSize.width, attachmentSize.height, *width, *height);
    }

    return {};
}

MaybeError ValidateOrSetColorAttachmentSampleCount(const TextureViewBase* colorAttachment,
                                                   uint32_t implicitSampleCount,
                                                   uint32_t* sampleCount) {
    uint32_t attachmentSampleCount = 0;
    std::string implicitPrefixStr;
    if (implicitSampleCount > 1) {
        DAWN_INVALID_IF(colorAttachment->GetTexture()->GetSampleCount() != 1,
                        "Color attachment %s sample count (%u) is not 1 when it has implicit "
                        "sample count (%u).",
                        colorAttachment, colorAttachment->GetTexture()->GetSampleCount(),
                        implicitSampleCount);

        attachmentSampleCount = implicitSampleCount;
        implicitPrefixStr = "implicit ";
    } else {
        attachmentSampleCount = colorAttachment->GetTexture()->GetSampleCount();
    }

    if (*sampleCount == 0) {
        *sampleCount = attachmentSampleCount;
        DAWN_ASSERT(*sampleCount != 0);
    } else {
        DAWN_INVALID_IF(
            *sampleCount != attachmentSampleCount,
            "Color attachment %s %ssample count (%u) does not match the sample count of the "
            "other attachments (%u).",
            colorAttachment, implicitPrefixStr, attachmentSampleCount, *sampleCount);
    }

    return {};
}

MaybeError ValidateResolveTarget(const DeviceBase* device,
                                 const RenderPassColorAttachment& colorAttachment,
                                 UsageValidationMode usageValidationMode) {
    if (colorAttachment.resolveTarget == nullptr) {
        return {};
    }

    const TextureViewBase* resolveTarget = colorAttachment.resolveTarget;
    const TextureViewBase* attachment = colorAttachment.view;
    DAWN_TRY(device->ValidateObject(colorAttachment.resolveTarget));
    DAWN_TRY(ValidateCanUseAs(colorAttachment.resolveTarget->GetTexture(),
                              wgpu::TextureUsage::RenderAttachment, usageValidationMode));

    DAWN_INVALID_IF(!attachment->GetTexture()->IsMultisampledTexture(),
                    "Cannot set %s as a resolve target when the color attachment %s has a sample "
                    "count of 1.",
                    resolveTarget, attachment);

    DAWN_INVALID_IF(resolveTarget->GetTexture()->IsMultisampledTexture(),
                    "Cannot use %s as resolve target. Sample count (%u) is greater than 1.",
                    resolveTarget, resolveTarget->GetTexture()->GetSampleCount());

    DAWN_INVALID_IF(resolveTarget->GetLayerCount() > 1,
                    "The resolve target %s array layer count (%u) is not 1.", resolveTarget,
                    resolveTarget->GetLayerCount());

    DAWN_INVALID_IF(resolveTarget->GetLevelCount() > 1,
                    "The resolve target %s mip level count (%u) is not 1.", resolveTarget,
                    resolveTarget->GetLevelCount());

    const Extent3D& colorTextureSize = attachment->GetSingleSubresourceVirtualSize();
    const Extent3D& resolveTextureSize = resolveTarget->GetSingleSubresourceVirtualSize();
    DAWN_INVALID_IF(colorTextureSize.width != resolveTextureSize.width ||
                        colorTextureSize.height != resolveTextureSize.height,
                    "The Resolve target %s size (width: %u, height: %u) does not match the color "
                    "attachment %s size (width: %u, height: %u).",
                    resolveTarget, resolveTextureSize.width, resolveTextureSize.height, attachment,
                    colorTextureSize.width, colorTextureSize.height);

    wgpu::TextureFormat resolveTargetFormat = resolveTarget->GetFormat().format;
    DAWN_INVALID_IF(
        resolveTargetFormat != attachment->GetFormat().format,
        "The resolve target %s format (%s) does not match the color attachment %s format "
        "(%s).",
        resolveTarget, resolveTargetFormat, attachment, attachment->GetFormat().format);
    DAWN_INVALID_IF(
        !resolveTarget->GetFormat().supportsResolveTarget,
        "The resolve target %s format (%s) does not support being used as resolve target.",
        resolveTarget, resolveTargetFormat);

    return {};
}

MaybeError ValidateColorAttachmentDepthSlice(const TextureViewBase* attachment,
                                             uint32_t depthSlice) {
    if (attachment->GetDimension() == wgpu::TextureViewDimension::e3D) {
        const Extent3D& attachmentSize = attachment->GetSingleSubresourceVirtualSize();

        DAWN_INVALID_IF(depthSlice >= attachmentSize.depthOrArrayLayers,
                        "The depth slice index (%u) of 3D %s used as attachment is >= the "
                        "depthOrArrayLayers (%u) of its subresource at mip level (%u).",
                        depthSlice, attachment, attachmentSize.depthOrArrayLayers,
                        attachment->GetBaseMipLevel());
    } else {
        DAWN_INVALID_IF(depthSlice != 0,
                        "The depth slice index (%u) of non-3D %s used as attachment is not 0.",
                        depthSlice, attachment);
    }

    return {};
}

MaybeError ValidateColorAttachmentRenderToSingleSampled(
    const DeviceBase* device,
    const RenderPassColorAttachment& colorAttachment,
    const DawnRenderPassColorAttachmentRenderToSingleSampled* msaaRenderToSingleSampledDesc) {
    DAWN_ASSERT(msaaRenderToSingleSampledDesc != nullptr);

    DAWN_INVALID_IF(
        !device->HasFeature(Feature::MSAARenderToSingleSampled),
        "The color attachment %s has implicit sample count while the %s feature is not enabled.",
        colorAttachment.view, ToAPI(Feature::MSAARenderToSingleSampled));

    DAWN_INVALID_IF(!IsValidSampleCount(msaaRenderToSingleSampledDesc->implicitSampleCount) ||
                        msaaRenderToSingleSampledDesc->implicitSampleCount <= 1,
                    "The color attachment %s's implicit sample count (%u) is not supported.",
                    colorAttachment.view, msaaRenderToSingleSampledDesc->implicitSampleCount);

    DAWN_INVALID_IF(!colorAttachment.view->GetTexture()->IsImplicitMSAARenderTextureViewSupported(),
                    "Color attachment %s was not created with %s usage, which is required for "
                    "having implicit sample count (%u).",
                    colorAttachment.view, wgpu::TextureUsage::TextureBinding,
                    msaaRenderToSingleSampledDesc->implicitSampleCount);

    DAWN_INVALID_IF(!colorAttachment.view->GetFormat().supportsResolveTarget,
                    "The color attachment %s format (%s) does not support being used with "
                    "implicit sample count (%u). The format does not support resolve.",
                    colorAttachment.view, colorAttachment.view->GetFormat().format,
                    msaaRenderToSingleSampledDesc->implicitSampleCount);

    DAWN_INVALID_IF(colorAttachment.resolveTarget != nullptr,
                    "Cannot set %s as a resolve target. No resolve target should be specified "
                    "for the color attachment %s with implicit sample count (%u).",
                    colorAttachment.resolveTarget, colorAttachment.view,
                    msaaRenderToSingleSampledDesc->implicitSampleCount);

    return {};
}

MaybeError ValidateRenderPassColorAttachment(DeviceBase* device,
                                             const RenderPassColorAttachment& colorAttachment,
                                             uint32_t* width,
                                             uint32_t* height,
                                             uint32_t* sampleCount,
                                             uint32_t* implicitSampleCount,
                                             UsageValidationMode usageValidationMode) {
    TextureViewBase* attachment = colorAttachment.view;
    if (attachment == nullptr) {
        return {};
    }

    DAWN_TRY(ValidateSingleSType(colorAttachment.nextInChain,
                                 wgpu::SType::DawnRenderPassColorAttachmentRenderToSingleSampled));

    const DawnRenderPassColorAttachmentRenderToSingleSampled* msaaRenderToSingleSampledDesc =
        nullptr;
    FindInChain(colorAttachment.nextInChain, &msaaRenderToSingleSampledDesc);
    if (msaaRenderToSingleSampledDesc) {
        DAWN_TRY(ValidateColorAttachmentRenderToSingleSampled(device, colorAttachment,
                                                              msaaRenderToSingleSampledDesc));
        *implicitSampleCount = msaaRenderToSingleSampledDesc->implicitSampleCount;
        // Note: we don't need to check whether the implicit sample count of different attachments
        // are the same. That already is done by indirectly comparing the sample count in
        // ValidateOrSetColorAttachmentSampleCount.
    }

    DAWN_TRY(device->ValidateObject(attachment));
    DAWN_TRY(ValidateCanUseAs(attachment->GetTexture(), wgpu::TextureUsage::RenderAttachment,
                              usageValidationMode));

    // Plane0 and Plane1 aspects for multiplanar texture views should be allowed as color
    // attachments.
    Aspect kRenderableAspects = Aspect::Color | Aspect::Plane0 | Aspect::Plane1;
    DAWN_INVALID_IF(
        !(attachment->GetAspects() & kRenderableAspects) || !attachment->GetFormat().isRenderable,
        "The color attachment %s format (%s) is not color renderable.", attachment,
        attachment->GetFormat().format);

    DAWN_TRY(ValidateLoadOp(colorAttachment.loadOp));
    DAWN_TRY(ValidateStoreOp(colorAttachment.storeOp));
    DAWN_INVALID_IF(colorAttachment.loadOp == wgpu::LoadOp::Undefined, "loadOp must be set.");
    DAWN_INVALID_IF(colorAttachment.storeOp == wgpu::StoreOp::Undefined, "storeOp must be set.");
    if (attachment->GetTexture()->GetUsage() & wgpu::TextureUsage::TransientAttachment) {
        DAWN_INVALID_IF(colorAttachment.loadOp != wgpu::LoadOp::Clear,
                        "The color attachment %s has the load op set to %s while its usage (%s) "
                        "has the transient attachment bit set.",
                        attachment, wgpu::LoadOp::Load, attachment->GetTexture()->GetUsage());
        DAWN_INVALID_IF(colorAttachment.storeOp != wgpu::StoreOp::Discard,
                        "The color attachment %s has the store op set to %s while its usage (%s) "
                        "has the transient attachment bit set.",
                        attachment, wgpu::StoreOp::Store, attachment->GetTexture()->GetUsage());
    }

    const dawn::native::Color& clearValue = colorAttachment.clearValue;
    if (colorAttachment.loadOp == wgpu::LoadOp::Clear) {
        DAWN_INVALID_IF(std::isnan(clearValue.r) || std::isnan(clearValue.g) ||
                            std::isnan(clearValue.b) || std::isnan(clearValue.a),
                        "Color clear value (%s) contains a NaN.", &clearValue);
    }

    DAWN_TRY(
        ValidateOrSetColorAttachmentSampleCount(attachment, *implicitSampleCount, sampleCount));

    if (*implicitSampleCount <= 1) {
        // This step is skipped if implicitSampleCount > 1, because in that case, there shoudn't be
        // any explicit resolveTarget specified.
        DAWN_TRY(ValidateResolveTarget(device, colorAttachment, usageValidationMode));
    }

    DAWN_TRY(ValidateColorAttachmentDepthSlice(attachment, colorAttachment.depthSlice));
    DAWN_TRY(ValidateAttachmentArrayLayersAndLevelCount(attachment));
    DAWN_TRY(ValidateOrSetAttachmentSize(attachment, width, height));

    return {};
}

MaybeError ValidateRenderPassDepthStencilAttachment(
    DeviceBase* device,
    const RenderPassDepthStencilAttachment* depthStencilAttachment,
    uint32_t* width,
    uint32_t* height,
    uint32_t* sampleCount,
    UsageValidationMode usageValidationMode) {
    DAWN_ASSERT(depthStencilAttachment != nullptr);

    TextureViewBase* attachment = depthStencilAttachment->view;
    DAWN_TRY(device->ValidateObject(attachment));
    DAWN_TRY(ValidateCanUseAs(attachment->GetTexture(), wgpu::TextureUsage::RenderAttachment,
                              usageValidationMode));

    // DS attachments must encompass all aspects of the texture, so we first check that this is
    // true, which means that in the rest of the function we can assume that the view's format is
    // the same as the texture's format.
    const Format& format = attachment->GetTexture()->GetFormat();
    DAWN_INVALID_IF(
        attachment->GetAspects() != format.aspects,
        "The depth stencil attachment %s must encompass all aspects of it's texture's format (%s).",
        attachment, format.format);
    DAWN_ASSERT(attachment->GetFormat().format == format.format);

    DAWN_INVALID_IF(!format.HasDepthOrStencil(),
                    "The depth stencil attachment %s format (%s) is not a depth stencil format.",
                    attachment, format.format);

    DAWN_INVALID_IF(!format.isRenderable,
                    "The depth stencil attachment %s format (%s) is not renderable.", attachment,
                    format.format);

    if (!device->IsToggleEnabled(Toggle::AllowUnsafeAPIs)) {
        DAWN_INVALID_IF(
            attachment->GetAspects() == (Aspect::Depth | Aspect::Stencil) &&
                depthStencilAttachment->depthReadOnly != depthStencilAttachment->stencilReadOnly,
            "depthReadOnly (%u) and stencilReadOnly (%u) must be the same when texture aspect "
            "is 'all'.",
            depthStencilAttachment->depthReadOnly, depthStencilAttachment->stencilReadOnly);
    }

    // Read only, or depth doesn't exist.
    if (depthStencilAttachment->depthReadOnly ||
        !IsSubset(Aspect::Depth, attachment->GetAspects())) {
        DAWN_INVALID_IF(depthStencilAttachment->depthLoadOp != wgpu::LoadOp::Undefined ||
                            depthStencilAttachment->depthStoreOp != wgpu::StoreOp::Undefined,
                        "Both depthLoadOp (%s) and depthStoreOp (%s) must not be set if the "
                        "attachment (%s) has no depth aspect or depthReadOnly (%u) is true.",
                        depthStencilAttachment->depthLoadOp, depthStencilAttachment->depthStoreOp,
                        attachment, depthStencilAttachment->depthReadOnly);
    } else {
        DAWN_TRY(ValidateLoadOp(depthStencilAttachment->depthLoadOp));
        DAWN_TRY(ValidateStoreOp(depthStencilAttachment->depthStoreOp));
        DAWN_INVALID_IF(depthStencilAttachment->depthLoadOp == wgpu::LoadOp::Undefined ||
                            depthStencilAttachment->depthStoreOp == wgpu::StoreOp::Undefined,
                        "Both depthLoadOp (%s) and depthStoreOp (%s) must be set if the attachment "
                        "(%s) has a depth aspect or depthReadOnly (%u) is false.",
                        depthStencilAttachment->depthLoadOp, depthStencilAttachment->depthStoreOp,
                        attachment, depthStencilAttachment->depthReadOnly);
    }

    // Read only, or stencil doesn't exist.
    if (depthStencilAttachment->stencilReadOnly ||
        !IsSubset(Aspect::Stencil, attachment->GetAspects())) {
        DAWN_INVALID_IF(depthStencilAttachment->stencilLoadOp != wgpu::LoadOp::Undefined ||
                            depthStencilAttachment->stencilStoreOp != wgpu::StoreOp::Undefined,
                        "Both stencilLoadOp (%s) and stencilStoreOp (%s) must not be set if the "
                        "attachment (%s) has no stencil aspect or stencilReadOnly (%u) is true.",
                        depthStencilAttachment->stencilLoadOp,
                        depthStencilAttachment->stencilStoreOp, attachment,
                        depthStencilAttachment->stencilReadOnly);
    } else {
        DAWN_TRY(ValidateLoadOp(depthStencilAttachment->stencilLoadOp));
        DAWN_TRY(ValidateStoreOp(depthStencilAttachment->stencilStoreOp));
        DAWN_INVALID_IF(depthStencilAttachment->stencilLoadOp == wgpu::LoadOp::Undefined ||
                            depthStencilAttachment->stencilStoreOp == wgpu::StoreOp::Undefined,
                        "Both stencilLoadOp (%s) and stencilStoreOp (%s) must be set if the "
                        "attachment (%s) has a stencil aspect or stencilReadOnly (%u) is false.",
                        depthStencilAttachment->stencilLoadOp,
                        depthStencilAttachment->stencilStoreOp, attachment,
                        depthStencilAttachment->stencilReadOnly);
    }

    if (depthStencilAttachment->depthLoadOp == wgpu::LoadOp::Clear &&
        IsSubset(Aspect::Depth, attachment->GetAspects())) {
        DAWN_INVALID_IF(
            std::isnan(depthStencilAttachment->depthClearValue),
            "depthClearValue (%f) must be set and must not be a NaN value if the attachment "
            "(%s) has a depth aspect and depthLoadOp is clear.",
            depthStencilAttachment->depthClearValue, attachment);
        DAWN_INVALID_IF(depthStencilAttachment->depthClearValue < 0.0f ||
                            depthStencilAttachment->depthClearValue > 1.0f,
                        "depthClearValue (%f) must be between 0.0 and 1.0 if the attachment (%s) "
                        "has a depth aspect and depthLoadOp is clear.",
                        depthStencilAttachment->depthClearValue, attachment);
    }

    // *sampleCount == 0 must only happen when there is no color attachment. In that case we
    // do not need to validate the sample count of the depth stencil attachment.
    const uint32_t depthStencilSampleCount = attachment->GetTexture()->GetSampleCount();
    if (*sampleCount != 0) {
        DAWN_INVALID_IF(
            depthStencilSampleCount != *sampleCount,
            "The depth stencil attachment %s sample count (%u) does not match the sample "
            "count of the other attachments (%u).",
            attachment, depthStencilSampleCount, *sampleCount);
    } else {
        *sampleCount = depthStencilSampleCount;
    }

    DAWN_TRY(ValidateAttachmentArrayLayersAndLevelCount(attachment));
    DAWN_TRY(ValidateOrSetAttachmentSize(attachment, width, height));

    return {};
}

MaybeError ValidateRenderPassPLS(DeviceBase* device,
                                 const RenderPassPixelLocalStorage* pls,
                                 uint32_t* width,
                                 uint32_t* height,
                                 uint32_t* sampleCount,
                                 uint32_t implicitSampleCount,
                                 UsageValidationMode usageValidationMode) {
    StackVector<StorageAttachmentInfoForValidation, 4> attachments;
    for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
        const RenderPassStorageAttachment& attachment = pls->storageAttachments[i];

        // Validate the attachment can be used as a storage attachment.
        DAWN_TRY(device->ValidateObject(attachment.storage));
        DAWN_TRY(ValidateCanUseAs(attachment.storage->GetTexture(),
                                  wgpu::TextureUsage::StorageAttachment, usageValidationMode));
        DAWN_TRY(ValidateAttachmentArrayLayersAndLevelCount(attachment.storage));
        DAWN_TRY(ValidateOrSetColorAttachmentSampleCount(attachment.storage, implicitSampleCount,
                                                         sampleCount));
        DAWN_TRY(ValidateOrSetAttachmentSize(attachment.storage, width, height));

        // Validate the load/storeOp and the clearValue.
        DAWN_TRY(ValidateLoadOp(attachment.loadOp));
        DAWN_TRY(ValidateStoreOp(attachment.storeOp));
        DAWN_INVALID_IF(attachment.loadOp == wgpu::LoadOp::Undefined,
                        "storageAttachments[%i].loadOp must be set.", i);
        DAWN_INVALID_IF(attachment.storeOp == wgpu::StoreOp::Undefined,
                        "storageAttachments[%i].storeOp must be set.", i);

        const dawn::native::Color& clearValue = attachment.clearValue;
        if (attachment.loadOp == wgpu::LoadOp::Clear) {
            DAWN_INVALID_IF(std::isnan(clearValue.r) || std::isnan(clearValue.g) ||
                                std::isnan(clearValue.b) || std::isnan(clearValue.a),
                            "storageAttachments[%i].clearValue (%s) contains a NaN.", i,
                            &clearValue);
        }

        attachments->push_back({attachment.offset, attachment.storage->GetFormat().format});
    }

    return ValidatePLSInfo(device, pls->totalPixelLocalStorageSize,
                           {attachments->data(), attachments->size()});
}

MaybeError ValidateRenderPassDescriptor(DeviceBase* device,
                                        const RenderPassDescriptor* descriptor,
                                        uint32_t* width,
                                        uint32_t* height,
                                        uint32_t* sampleCount,
                                        uint32_t* implicitSampleCount,
                                        UsageValidationMode usageValidationMode) {
    DAWN_TRY(
        ValidateSTypes(descriptor->nextInChain, {{wgpu::SType::RenderPassDescriptorMaxDrawCount},
                                                 {wgpu::SType::RenderPassPixelLocalStorage}}));

    uint32_t maxColorAttachments = device->GetLimits().v1.maxColorAttachments;
    DAWN_INVALID_IF(
        descriptor->colorAttachmentCount > maxColorAttachments,
        "Color attachment count (%u) exceeds the maximum number of color attachments (%u).",
        descriptor->colorAttachmentCount, maxColorAttachments);

    bool anyColorAttachment = false;
    ColorAttachmentFormats colorAttachmentFormats;
    for (uint32_t i = 0; i < descriptor->colorAttachmentCount; ++i) {
        DAWN_TRY_CONTEXT(ValidateRenderPassColorAttachment(
                             device, descriptor->colorAttachments[i], width, height, sampleCount,
                             implicitSampleCount, usageValidationMode),
                         "validating colorAttachments[%u].", i);
        if (descriptor->colorAttachments[i].view) {
            anyColorAttachment = true;
            colorAttachmentFormats->push_back(&descriptor->colorAttachments[i].view->GetFormat());
        }
    }
    DAWN_TRY_CONTEXT(ValidateColorAttachmentBytesPerSample(device, colorAttachmentFormats),
                     "validating color attachment bytes per sample.");

    if (descriptor->depthStencilAttachment != nullptr) {
        DAWN_TRY_CONTEXT(ValidateRenderPassDepthStencilAttachment(
                             device, descriptor->depthStencilAttachment, width, height, sampleCount,
                             usageValidationMode),
                         "validating depthStencilAttachment.");
    }

    if (descriptor->occlusionQuerySet != nullptr) {
        DAWN_TRY(device->ValidateObject(descriptor->occlusionQuerySet));

        DAWN_INVALID_IF(descriptor->occlusionQuerySet->GetQueryType() != wgpu::QueryType::Occlusion,
                        "The occlusionQuerySet %s type (%s) is not %s.",
                        descriptor->occlusionQuerySet,
                        descriptor->occlusionQuerySet->GetQueryType(), wgpu::QueryType::Occlusion);
    }

    if (descriptor->timestampWrites != nullptr) {
        QuerySetBase* querySet = descriptor->timestampWrites->querySet;
        DAWN_ASSERT(querySet != nullptr);
        uint32_t beginningOfPassWriteIndex = descriptor->timestampWrites->beginningOfPassWriteIndex;
        uint32_t endOfPassWriteIndex = descriptor->timestampWrites->endOfPassWriteIndex;
        DAWN_TRY_CONTEXT(ValidatePassTimestampWrites(device, querySet, beginningOfPassWriteIndex,
                                                     endOfPassWriteIndex),
                         "validating timestampWrites.");
    }

    // Validation for any pixel local storage.
    size_t storageAttachmentCount = 0;
    const RenderPassPixelLocalStorage* pls = nullptr;
    FindInChain(descriptor->nextInChain, &pls);
    if (pls != nullptr) {
        storageAttachmentCount = pls->storageAttachmentCount;
        DAWN_TRY(ValidateRenderPassPLS(device, pls, width, height, sampleCount,
                                       *implicitSampleCount, usageValidationMode));
    }

    DAWN_INVALID_IF(!anyColorAttachment && descriptor->depthStencilAttachment == nullptr &&
                        storageAttachmentCount == 0,
                    "Render pass has no attachments.");

    if (*implicitSampleCount > 1) {
        // TODO(dawn:1710): support multiple attachments.
        DAWN_INVALID_IF(
            descriptor->colorAttachmentCount != 1,
            "colorAttachmentCount (%u) is not supported when the render pass has implicit sample "
            "count (%u). (Currently) colorAttachmentCount = 1 is supported.",
            descriptor->colorAttachmentCount, *implicitSampleCount);
        // TODO(dawn:1704): Consider supporting MSAARenderToSingleSampled + PLS
        DAWN_INVALID_IF(pls != nullptr,
                        "For now PLS is invalid to use with MSAARenderToSingleSampled.");
    }

    return {};
}

MaybeError ValidateComputePassDescriptor(const DeviceBase* device,
                                         const ComputePassDescriptor* descriptor) {
    if (descriptor == nullptr) {
        return {};
    }

    if (descriptor->timestampWrites != nullptr) {
        QuerySetBase* querySet = descriptor->timestampWrites->querySet;
        DAWN_ASSERT(querySet != nullptr);
        uint32_t beginningOfPassWriteIndex = descriptor->timestampWrites->beginningOfPassWriteIndex;
        uint32_t endOfPassWriteIndex = descriptor->timestampWrites->endOfPassWriteIndex;
        DAWN_TRY_CONTEXT(ValidatePassTimestampWrites(device, querySet, beginningOfPassWriteIndex,
                                                     endOfPassWriteIndex),
                         "validating timestampWrites.");
    }

    return {};
}

MaybeError ValidateQuerySetResolve(const QuerySetBase* querySet,
                                   uint32_t firstQuery,
                                   uint32_t queryCount,
                                   const BufferBase* destination,
                                   uint64_t destinationOffset) {
    DAWN_INVALID_IF(firstQuery >= querySet->GetQueryCount(),
                    "First query (%u) exceeds the number of queries (%u) in %s.", firstQuery,
                    querySet->GetQueryCount(), querySet);

    DAWN_INVALID_IF(
        queryCount > querySet->GetQueryCount() - firstQuery,
        "The query range (firstQuery: %u, queryCount: %u) exceeds the number of queries "
        "(%u) in %s.",
        firstQuery, queryCount, querySet->GetQueryCount(), querySet);

    DAWN_INVALID_IF(destinationOffset % kQueryResolveAlignment != 0,
                    "The destination buffer %s offset (%u) is not a multiple of %u.", destination,
                    destinationOffset, kQueryResolveAlignment);

    uint64_t bufferSize = destination->GetSize();
    // The destination buffer must have enough storage, from destination offset, to contain
    // the result of resolved queries
    bool fitsInBuffer =
        destinationOffset <= bufferSize &&
        (static_cast<uint64_t>(queryCount) * sizeof(uint64_t) <= (bufferSize - destinationOffset));
    DAWN_INVALID_IF(
        !fitsInBuffer,
        "The resolved %s data size (%u) would not fit in %s with size %u at the offset %u.",
        querySet, static_cast<uint64_t>(queryCount) * sizeof(uint64_t), destination, bufferSize,
        destinationOffset);

    return {};
}

MaybeError EncodeTimestampsToNanosecondsConversion(CommandEncoder* encoder,
                                                   QuerySetBase* querySet,
                                                   uint32_t firstQuery,
                                                   uint32_t queryCount,
                                                   BufferBase* destination,
                                                   uint64_t destinationOffset) {
    DeviceBase* device = encoder->GetDevice();

    // The availability got from query set is a reference to vector<bool>, need to covert
    // bool to uint32_t due to a user input in pipeline must not contain a bool type in
    // WGSL.
    std::vector<uint32_t> availability{querySet->GetQueryAvailability().begin(),
                                       querySet->GetQueryAvailability().end()};

    // Timestamp availability storage buffer
    BufferDescriptor availabilityDesc = {};
    availabilityDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
    availabilityDesc.size = querySet->GetQueryCount() * sizeof(uint32_t);
    Ref<BufferBase> availabilityBuffer;
    DAWN_TRY_ASSIGN(availabilityBuffer, device->CreateBuffer(&availabilityDesc));

    DAWN_TRY(device->GetQueue()->WriteBuffer(availabilityBuffer.Get(), 0, availability.data(),
                                             availability.size() * sizeof(uint32_t)));

    const uint32_t quantization_mask = (device->IsToggleEnabled(Toggle::TimestampQuantization))
                                           ? kTimestampQuantizationMask
                                           : 0xFFFFFFFF;

    // Timestamp params uniform buffer
    TimestampParams params(firstQuery, queryCount, static_cast<uint32_t>(destinationOffset),
                           quantization_mask, device->GetTimestampPeriodInNS());

    BufferDescriptor parmsDesc = {};
    parmsDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    parmsDesc.size = sizeof(params);
    Ref<BufferBase> paramsBuffer;
    DAWN_TRY_ASSIGN(paramsBuffer, device->CreateBuffer(&parmsDesc));

    DAWN_TRY(device->GetQueue()->WriteBuffer(paramsBuffer.Get(), 0, &params, sizeof(params)));

    // In the internal shader to convert timestamps to nanoseconds, we can ensure no uninitialized
    // data will be read and the full buffer range will be filled with valid data.
    if (!destination->IsDataInitialized() &&
        destination->IsFullBufferRange(firstQuery, sizeof(uint64_t) * queryCount)) {
        destination->SetIsDataInitialized();
    }

    return EncodeConvertTimestampsToNanoseconds(encoder, destination, availabilityBuffer.Get(),
                                                paramsBuffer.Get());
}

// Load resolve texture to MSAA attachment if needed.
MaybeError ApplyMSAARenderToSingleSampledLoadOp(DeviceBase* device,
                                                RenderPassEncoder* renderPassEncoder,
                                                const RenderPassDescriptor* renderPassDescriptor,
                                                uint32_t implicitSampleCount) {
    // TODO(dawn:1710): support multiple attachments.
    DAWN_ASSERT(renderPassDescriptor->colorAttachmentCount == 1);
    if (renderPassDescriptor->colorAttachments[0].loadOp != wgpu::LoadOp::Load) {
        return {};
    }

    // TODO(dawn:1710): support loading resolve texture on platforms that don't support reading
    // it in fragment shader such as vulkan.
    DAWN_ASSERT(device->IsResolveTextureBlitWithDrawSupported());

    // Read implicit resolve texture in fragment shader and copy to the implicit MSAA attachment.
    return BlitMSAARenderToSingleSampledColorWithDraw(device, renderPassEncoder,
                                                      renderPassDescriptor, implicitSampleCount);
}
// Tracks the temporary resolve attachments used when the AlwaysResolveIntoZeroLevelAndLayer toggle
// is active so that the results can be copied from the temporary resolve attachment into the
// intended target after the render pass is complete. Also used by the
// ResolveMultipleAttachmentInSeparatePasses toggle to track resolves that need to be done in their
// own passes.
struct TemporaryResolveAttachment {
    TemporaryResolveAttachment(Ref<TextureViewBase> src,
                               Ref<TextureViewBase> dst,
                               wgpu::StoreOp storeOp = wgpu::StoreOp::Store)
        : copySrc(std::move(src)), copyDst(std::move(dst)), storeOp(storeOp) {}

    Ref<TextureViewBase> copySrc;
    Ref<TextureViewBase> copyDst;
    wgpu::StoreOp storeOp;
};

bool ShouldUseTextureToBufferBlit(const DeviceBase* device,
                                  const Format& format,
                                  const Aspect& aspect) {
    // Snorm
    if (format.IsSnorm() && device->IsToggleEnabled(Toggle::UseBlitForSnormTextureToBufferCopy)) {
        return true;
    }
    // BGRA8Unorm
    if (format.format == wgpu::TextureFormat::BGRA8Unorm &&
        device->IsToggleEnabled(Toggle::UseBlitForBGRA8UnormTextureToBufferCopy)) {
        return true;
    }
    // RGB9E5Ufloat
    if (format.format == wgpu::TextureFormat::RGB9E5Ufloat &&
        device->IsToggleEnabled(Toggle::UseBlitForRGB9E5UfloatTextureCopy)) {
        return true;
    }
    // Depth
    if (aspect == Aspect::Depth &&
        ((format.format == wgpu::TextureFormat::Depth16Unorm &&
          device->IsToggleEnabled(Toggle::UseBlitForDepth16UnormTextureToBufferCopy)) ||
         (format.format == wgpu::TextureFormat::Depth32Float &&
          device->IsToggleEnabled(Toggle::UseBlitForDepth32FloatTextureToBufferCopy)))) {
        return true;
    }
    // Stencil
    if (aspect == Aspect::Stencil &&
        device->IsToggleEnabled(Toggle::UseBlitForStencilTextureToBufferCopy)) {
        return true;
    }
    return false;
}

}  // namespace

Color ClampClearColorValueToLegalRange(const Color& originalColor, const Format& format) {
    const AspectInfo& aspectInfo = format.GetAspectInfo(Aspect::Color);
    double minValue = 0;
    double maxValue = 0;
    switch (aspectInfo.baseType) {
        case TextureComponentType::Float: {
            return originalColor;
        }
        case TextureComponentType::Sint: {
            const uint32_t bitsPerComponent =
                (aspectInfo.block.byteSize * 8 / format.componentCount);
            maxValue =
                static_cast<double>((static_cast<uint64_t>(1) << (bitsPerComponent - 1)) - 1);
            minValue = -static_cast<double>(static_cast<uint64_t>(1) << (bitsPerComponent - 1));
            break;
        }
        case TextureComponentType::Uint: {
            const uint32_t bitsPerComponent =
                (aspectInfo.block.byteSize * 8 / format.componentCount);
            maxValue = static_cast<double>((static_cast<uint64_t>(1) << bitsPerComponent) - 1);
            break;
        }
    }

    return {std::clamp(originalColor.r, minValue, maxValue),
            std::clamp(originalColor.g, minValue, maxValue),
            std::clamp(originalColor.b, minValue, maxValue),
            std::clamp(originalColor.a, minValue, maxValue)};
}

MaybeError ValidateCommandEncoderDescriptor(const DeviceBase* device,
                                            const CommandEncoderDescriptor* descriptor) {
    DAWN_TRY(ValidateSingleSType(descriptor->nextInChain,
                                 wgpu::SType::DawnEncoderInternalUsageDescriptor));

    const DawnEncoderInternalUsageDescriptor* internalUsageDesc = nullptr;
    FindInChain(descriptor->nextInChain, &internalUsageDesc);

    DAWN_INVALID_IF(internalUsageDesc != nullptr &&
                        !device->APIHasFeature(wgpu::FeatureName::DawnInternalUsages),
                    "%s is not available.", wgpu::FeatureName::DawnInternalUsages);
    return {};
}

// static
Ref<CommandEncoder> CommandEncoder::Create(DeviceBase* device,
                                           const CommandEncoderDescriptor* descriptor) {
    return AcquireRef(new CommandEncoder(device, descriptor));
}

// static
CommandEncoder* CommandEncoder::MakeError(DeviceBase* device, const char* label) {
    return new CommandEncoder(device, ObjectBase::kError, label);
}

CommandEncoder::CommandEncoder(DeviceBase* device, const CommandEncoderDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label), mEncodingContext(device, this) {
    GetObjectTrackingList()->Track(this);

    const DawnEncoderInternalUsageDescriptor* internalUsageDesc = nullptr;
    FindInChain(descriptor->nextInChain, &internalUsageDesc);

    if (internalUsageDesc != nullptr && internalUsageDesc->useInternalUsages) {
        mUsageValidationMode = UsageValidationMode::Internal;
    } else {
        mUsageValidationMode = UsageValidationMode::Default;
    }
}

CommandEncoder::CommandEncoder(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label)
    : ApiObjectBase(device, tag, label),
      mEncodingContext(device, this),
      mUsageValidationMode(UsageValidationMode::Default) {
    GetObjectTrackingList()->Track(this);

    mEncodingContext.HandleError(DAWN_VALIDATION_ERROR("%s is invalid.", this));
}

ObjectType CommandEncoder::GetType() const {
    return ObjectType::CommandEncoder;
}

void CommandEncoder::DestroyImpl() {
    mEncodingContext.Destroy();
}

CommandBufferResourceUsage CommandEncoder::AcquireResourceUsages() {
    return CommandBufferResourceUsage{
        mEncodingContext.AcquireRenderPassUsages(), mEncodingContext.AcquireComputePassUsages(),
        std::move(mTopLevelBuffers), std::move(mTopLevelTextures), std::move(mUsedQuerySets)};
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

    // Set the query at queryIndex to available for resolving in query set.
    querySet->SetQueryAvailability(queryIndex, true);
}

// Implementation of the API's command recording methods

ComputePassEncoder* CommandEncoder::APIBeginComputePass(const ComputePassDescriptor* descriptor) {
    // This function will create new object, need to lock the Device.
    auto deviceLock(GetDevice()->GetScopedLock());

    return BeginComputePass(descriptor).Detach();
}

Ref<ComputePassEncoder> CommandEncoder::BeginComputePass(const ComputePassDescriptor* descriptor) {
    DeviceBase* device = GetDevice();
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

    bool success = mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            DAWN_TRY(ValidateComputePassDescriptor(device, descriptor));

            BeginComputePassCmd* cmd =
                allocator->Allocate<BeginComputePassCmd>(Command::BeginComputePass);

            if (descriptor == nullptr) {
                return {};
            }
            cmd->label = std::string(descriptor->label ? descriptor->label : "");

            if (descriptor->timestampWrites != nullptr) {
                QuerySetBase* querySet = descriptor->timestampWrites->querySet;
                uint32_t beginningOfPassWriteIndex =
                    descriptor->timestampWrites->beginningOfPassWriteIndex;
                uint32_t endOfPassWriteIndex = descriptor->timestampWrites->endOfPassWriteIndex;

                cmd->timestampWrites.querySet = querySet;
                cmd->timestampWrites.beginningOfPassWriteIndex = beginningOfPassWriteIndex;
                cmd->timestampWrites.endOfPassWriteIndex = endOfPassWriteIndex;
                if (beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
                    TrackQueryAvailability(querySet, beginningOfPassWriteIndex);
                }
                if (endOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
                    TrackQueryAvailability(querySet, endOfPassWriteIndex);
                }
            }
            return {};
        },
        "encoding %s.BeginComputePass(%s).", this, descriptor);

    if (success) {
        const ComputePassDescriptor defaultDescriptor = {};
        if (descriptor == nullptr) {
            descriptor = &defaultDescriptor;
        }

        Ref<ComputePassEncoder> passEncoder =
            ComputePassEncoder::Create(device, descriptor, this, &mEncodingContext);
        mEncodingContext.EnterPass(passEncoder.Get());
        return passEncoder;
    }

    return ComputePassEncoder::MakeError(device, this, &mEncodingContext,
                                         descriptor ? descriptor->label : nullptr);
}

RenderPassEncoder* CommandEncoder::APIBeginRenderPass(const RenderPassDescriptor* descriptor) {
    // This function will create new object, need to lock the Device.
    auto deviceLock(GetDevice()->GetScopedLock());

    return BeginRenderPass(descriptor).Detach();
}

Ref<RenderPassEncoder> CommandEncoder::BeginRenderPass(const RenderPassDescriptor* descriptor) {
    DeviceBase* device = GetDevice();
    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

    RenderPassResourceUsageTracker usageTracker;

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t sampleCount = 0;
    // The implicit multisample count used by MSAA render to single sampled.
    uint32_t implicitSampleCount = 0;
    bool depthReadOnly = false;
    bool stencilReadOnly = false;
    Ref<AttachmentState> attachmentState;

    std::function<void()> passEndCallback = nullptr;

    bool success = mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            DAWN_TRY(ValidateRenderPassDescriptor(device, descriptor, &width, &height, &sampleCount,
                                                  &implicitSampleCount, mUsageValidationMode));

            DAWN_ASSERT(width > 0 && height > 0 && sampleCount > 0 &&
                        (implicitSampleCount == 0 || implicitSampleCount == sampleCount));

            mEncodingContext.WillBeginRenderPass();
            BeginRenderPassCmd* cmd =
                allocator->Allocate<BeginRenderPassCmd>(Command::BeginRenderPass);
            cmd->label = std::string(descriptor->label ? descriptor->label : "");

            cmd->attachmentState = device->GetOrCreateAttachmentState(descriptor);
            attachmentState = cmd->attachmentState;

            for (ColorAttachmentIndex index :
                 IterateBitSet(cmd->attachmentState->GetColorAttachmentsMask())) {
                uint8_t i = static_cast<uint8_t>(index);
                TextureViewBase* colorTarget;
                TextureViewBase* resolveTarget;

                if (implicitSampleCount <= 1) {
                    colorTarget = descriptor->colorAttachments[i].view;
                    resolveTarget = descriptor->colorAttachments[i].resolveTarget;

                    cmd->colorAttachments[index].view = colorTarget;
                    cmd->colorAttachments[index].depthSlice =
                        descriptor->colorAttachments[i].depthSlice;
                    cmd->colorAttachments[index].loadOp = descriptor->colorAttachments[i].loadOp;
                    cmd->colorAttachments[index].storeOp = descriptor->colorAttachments[i].storeOp;
                } else {
                    // We use an implicit MSAA texture and resolve to the client supplied
                    // attachment.
                    resolveTarget = descriptor->colorAttachments[i].view;
                    Ref<TextureViewBase> implicitMSAATargetRef;
                    DAWN_TRY_ASSIGN(implicitMSAATargetRef,
                                    device->CreateImplicitMSAARenderTextureViewFor(
                                        resolveTarget, implicitSampleCount));
                    colorTarget = implicitMSAATargetRef.Get();

                    cmd->colorAttachments[index].view = std::move(implicitMSAATargetRef);
                    cmd->colorAttachments[index].depthSlice = 0;
                    cmd->colorAttachments[index].loadOp = wgpu::LoadOp::Clear;
                    cmd->colorAttachments[index].storeOp = wgpu::StoreOp::Discard;
                }

                cmd->colorAttachments[index].resolveTarget = resolveTarget;

                Color color = descriptor->colorAttachments[i].clearValue;
                cmd->colorAttachments[index].clearColor =
                    ClampClearColorValueToLegalRange(color, colorTarget->GetFormat());

                usageTracker.TextureViewUsedAs(colorTarget, wgpu::TextureUsage::RenderAttachment);

                if (resolveTarget != nullptr) {
                    usageTracker.TextureViewUsedAs(resolveTarget,
                                                   wgpu::TextureUsage::RenderAttachment);
                }
            }

            if (cmd->attachmentState->HasDepthStencilAttachment()) {
                TextureViewBase* view = descriptor->depthStencilAttachment->view;
                TextureBase* attachment = view->GetTexture();
                cmd->depthStencilAttachment.view = view;
                // Range that will be modified per aspect to track the usage.
                SubresourceRange usageRange = view->GetSubresourceRange();

                switch (descriptor->depthStencilAttachment->depthLoadOp) {
                    case wgpu::LoadOp::Clear:
                        cmd->depthStencilAttachment.clearDepth =
                            descriptor->depthStencilAttachment->depthClearValue;
                        break;
                    case wgpu::LoadOp::Load:
                    case wgpu::LoadOp::Undefined:
                        // Set depthClearValue to 0 if it is the load op is not clear.
                        // The default value NaN may be invalid in the backend.
                        cmd->depthStencilAttachment.clearDepth = 0.f;
                        break;
                }

                // GPURenderPassDepthStencilAttachment.stencilClearValue will be converted to
                // the type of the stencil aspect of view by taking the same number of LSBs as
                // the number of bits in the stencil aspect of one texel block of view.
                DAWN_ASSERT(!(view->GetFormat().aspects & Aspect::Stencil) ||
                            view->GetFormat().GetAspectInfo(Aspect::Stencil).block.byteSize == 1u);
                cmd->depthStencilAttachment.clearStencil =
                    descriptor->depthStencilAttachment->stencilClearValue & 0xFF;

                // Depth aspect:
                //  - Copy parameters for the aspect, reyifing the values when it is not present or
                //  readonly.
                //  - Export depthReadOnly to the outside of the depth-stencil attachment handling.
                //  - Track the usage of this aspect.
                depthReadOnly = descriptor->depthStencilAttachment->depthReadOnly;

                cmd->depthStencilAttachment.depthReadOnly = false;
                cmd->depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Load;
                cmd->depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
                if (attachment->GetFormat().HasDepth()) {
                    cmd->depthStencilAttachment.depthReadOnly = depthReadOnly;
                    if (!depthReadOnly) {
                        cmd->depthStencilAttachment.depthLoadOp =
                            descriptor->depthStencilAttachment->depthLoadOp;
                        cmd->depthStencilAttachment.depthStoreOp =
                            descriptor->depthStencilAttachment->depthStoreOp;
                    }

                    usageRange.aspects = Aspect::Depth;
                    usageTracker.TextureRangeUsedAs(attachment, usageRange,
                                                    depthReadOnly
                                                        ? kReadOnlyRenderAttachment
                                                        : wgpu::TextureUsage::RenderAttachment);
                }

                // Stencil aspect:
                //  - Copy parameters for the aspect, reyifing the values when it is not present or
                //  readonly.
                //  - Export stencilReadOnly to the outside of the depth-stencil attachment
                //  handling.
                //  - Track the usage of this aspect.
                stencilReadOnly = descriptor->depthStencilAttachment->stencilReadOnly;

                cmd->depthStencilAttachment.stencilReadOnly = false;
                cmd->depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Load;
                cmd->depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
                if (attachment->GetFormat().HasStencil()) {
                    cmd->depthStencilAttachment.stencilReadOnly = stencilReadOnly;
                    if (!stencilReadOnly) {
                        cmd->depthStencilAttachment.stencilLoadOp =
                            descriptor->depthStencilAttachment->stencilLoadOp;
                        cmd->depthStencilAttachment.stencilStoreOp =
                            descriptor->depthStencilAttachment->stencilStoreOp;
                    }

                    usageRange.aspects = Aspect::Stencil;
                    usageTracker.TextureRangeUsedAs(attachment, usageRange,
                                                    stencilReadOnly
                                                        ? kReadOnlyRenderAttachment
                                                        : wgpu::TextureUsage::RenderAttachment);
                }
            }

            cmd->width = width;
            cmd->height = height;

            cmd->occlusionQuerySet = descriptor->occlusionQuerySet;

            if (descriptor->timestampWrites != nullptr) {
                QuerySetBase* querySet = descriptor->timestampWrites->querySet;
                uint32_t beginningOfPassWriteIndex =
                    descriptor->timestampWrites->beginningOfPassWriteIndex;
                uint32_t endOfPassWriteIndex = descriptor->timestampWrites->endOfPassWriteIndex;

                cmd->timestampWrites.querySet = querySet;
                cmd->timestampWrites.beginningOfPassWriteIndex = beginningOfPassWriteIndex;
                cmd->timestampWrites.endOfPassWriteIndex = endOfPassWriteIndex;
                if (beginningOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
                    TrackQueryAvailability(querySet, beginningOfPassWriteIndex);
                    // Track the query availability with true on render pass again for rewrite
                    // validation and query reset on Vulkan
                    usageTracker.TrackQueryAvailability(querySet, beginningOfPassWriteIndex);
                }
                if (endOfPassWriteIndex != wgpu::kQuerySetIndexUndefined) {
                    TrackQueryAvailability(querySet, endOfPassWriteIndex);
                    // Track the query availability with true on render pass again for rewrite
                    // validation and query reset on Vulkan
                    usageTracker.TrackQueryAvailability(querySet, endOfPassWriteIndex);
                }
            }

            const RenderPassPixelLocalStorage* pls = nullptr;
            FindInChain(descriptor->nextInChain, &pls);
            if (pls != nullptr) {
                for (size_t i = 0; i < pls->storageAttachmentCount; i++) {
                    usageTracker.TextureViewUsedAs(pls->storageAttachments[i].storage,
                                                   wgpu::TextureUsage::StorageAttachment);
                }
            }

            DAWN_TRY_ASSIGN(passEndCallback,
                            ApplyRenderPassWorkarounds(device, &usageTracker, cmd));

            return {};
        },
        "encoding %s.BeginRenderPass(%s).", this, descriptor);

    if (success) {
        Ref<RenderPassEncoder> passEncoder =
            RenderPassEncoder::Create(device, descriptor, this, &mEncodingContext,
                                      std::move(usageTracker), std::move(attachmentState), width,
                                      height, depthReadOnly, stencilReadOnly, passEndCallback);

        mEncodingContext.EnterPass(passEncoder.Get());

        MaybeError error;

        if (implicitSampleCount > 1) {
            error = ApplyMSAARenderToSingleSampledLoadOp(device, passEncoder.Get(), descriptor,
                                                         implicitSampleCount);
        } else if (ShouldApplyClearBigIntegerColorValueWithDraw(device, descriptor)) {
            // This is skipped if implicitSampleCount > 1. Because implicitSampleCount > 1 is only
            // supported for non-integer textures.
            error = ApplyClearBigIntegerColorValueWithDraw(passEncoder.Get(), descriptor);
        }

        if (device->ConsumedError(std::move(error))) {
            return RenderPassEncoder::MakeError(device, this, &mEncodingContext,
                                                descriptor ? descriptor->label : nullptr);
        }

        return passEncoder;
    }

    return RenderPassEncoder::MakeError(device, this, &mEncodingContext,
                                        descriptor ? descriptor->label : nullptr);
}

// This function handles render pass workarounds. Because some cases may require
// multiple workarounds, it applies any workarounds one by one and calls itself
// recursively to handle the next workaround if needed.
ResultOrError<std::function<void()>> CommandEncoder::ApplyRenderPassWorkarounds(
    DeviceBase* device,
    RenderPassResourceUsageTracker* usageTracker,
    BeginRenderPassCmd* cmd,
    std::function<void()> passEndCallback) {
    // dawn:1550
    // Handle toggle ResolveMultipleAttachmentInSeparatePasses. This identifies passes where there
    // are multiple MSAA color targets and at least one of them has a resolve target. If that's the
    // case then the resolves are deferred by removing the resolve targets and forcing the storeOp
    // to Store. After the pass has ended an new pass is recorded for each resolve target that
    // resolves it separately.
    if (device->IsToggleEnabled(Toggle::ResolveMultipleAttachmentInSeparatePasses) &&
        cmd->attachmentState->GetColorAttachmentsMask().count() > 1) {
        bool splitResolvesIntoSeparatePasses = false;

        // This workaround needs to apply if there are multiple MSAA color targets (checked above)
        // and at least one resolve target.
        for (ColorAttachmentIndex i :
             IterateBitSet(cmd->attachmentState->GetColorAttachmentsMask())) {
            if (cmd->colorAttachments[i].resolveTarget.Get() != nullptr) {
                splitResolvesIntoSeparatePasses = true;
                break;
            }
        }

        if (splitResolvesIntoSeparatePasses) {
            std::vector<TemporaryResolveAttachment> temporaryResolveAttachments;

            for (ColorAttachmentIndex i :
                 IterateBitSet(cmd->attachmentState->GetColorAttachmentsMask())) {
                auto& attachmentInfo = cmd->colorAttachments[i];
                TextureViewBase* resolveTarget = attachmentInfo.resolveTarget.Get();
                if (resolveTarget != nullptr) {
                    // Save the color and resolve targets together for an explicit resolve pass
                    // after this one ends, then remove the resolve target from this pass and
                    // force the storeOp to Store.
                    temporaryResolveAttachments.emplace_back(attachmentInfo.view.Get(),
                                                             resolveTarget, attachmentInfo.storeOp);
                    attachmentInfo.storeOp = wgpu::StoreOp::Store;
                    attachmentInfo.resolveTarget = nullptr;
                }
            }

            // Check for other workarounds that need to be applied recursively.
            return ApplyRenderPassWorkarounds(
                device, usageTracker, cmd,
                [this, passEndCallback = std::move(passEndCallback),
                 temporaryResolveAttachments = std::move(temporaryResolveAttachments)]() -> void {
                    // Called once the render pass has been ended.
                    // Handles any separate resolve passes needed for the
                    // ResolveMultipleAttachmentInSeparatePasses workaround immediately after the
                    // render pass ends and before any additional commands are recorded.
                    for (auto& deferredResolve : temporaryResolveAttachments) {
                        RenderPassColorAttachment attachment = {};
                        attachment.view = deferredResolve.copySrc.Get();
                        attachment.resolveTarget = deferredResolve.copyDst.Get();
                        attachment.loadOp = wgpu::LoadOp::Load;
                        attachment.storeOp = deferredResolve.storeOp;

                        RenderPassDescriptor resolvePass = {};
                        resolvePass.colorAttachmentCount = 1;
                        resolvePass.colorAttachments = &attachment;

                        // Begin and end an empty render pass to force the resolve.
                        Ref<RenderPassEncoder> encoder = this->BeginRenderPass(&resolvePass);
                        encoder->End();
                    }

                    // If there were any other callbacks in the workaround stack, call the next one.
                    if (passEndCallback) {
                        passEndCallback();
                    }
                });
        }
    }

    // dawn:56, dawn:1569
    // Handle Toggle AlwaysResolveIntoZeroLevelAndLayer. This swaps out the given resolve attachment
    // for a temporary one that has no layers or mip levels. The results are copied from the
    // temporary attachment into the given attachment when the render pass ends. (Handled at the
    // bottom of this branch)
    if (device->IsToggleEnabled(Toggle::AlwaysResolveIntoZeroLevelAndLayer)) {
        std::vector<TemporaryResolveAttachment> temporaryResolveAttachments;

        for (ColorAttachmentIndex index :
             IterateBitSet(cmd->attachmentState->GetColorAttachmentsMask())) {
            TextureViewBase* resolveTarget = cmd->colorAttachments[index].resolveTarget.Get();

            if (resolveTarget != nullptr && (resolveTarget->GetBaseMipLevel() != 0 ||
                                             resolveTarget->GetBaseArrayLayer() != 0)) {
                // Create a temporary texture to resolve into
                // TODO(dawn:1618): Defer allocation of temporary textures till submit time.
                TextureDescriptor descriptor = {};
                descriptor.usage =
                    wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
                descriptor.format = resolveTarget->GetFormat().format;
                descriptor.size = resolveTarget->GetSingleSubresourceVirtualSize();
                descriptor.dimension = wgpu::TextureDimension::e2D;
                descriptor.mipLevelCount = 1;

                // We are creating new resources. Device must already be locked via
                // APIBeginRenderPass -> ApplyRenderPassWorkarounds.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                Ref<TextureBase> temporaryResolveTexture;
                Ref<TextureViewBase> temporaryResolveView;
                {
                    DAWN_ASSERT(device->IsLockedByCurrentThreadIfNeeded());

                    DAWN_TRY_ASSIGN(temporaryResolveTexture, device->CreateTexture(&descriptor));

                    TextureViewDescriptor viewDescriptor = {};
                    DAWN_TRY_ASSIGN(
                        temporaryResolveView,
                        device->CreateTextureView(temporaryResolveTexture.Get(), &viewDescriptor));
                }

                // Save the temporary and given render targets together for copying after
                // the render pass ends.
                temporaryResolveAttachments.emplace_back(temporaryResolveView, resolveTarget);

                // Replace the given resolve attachment with the temporary one.
                usageTracker->TextureViewUsedAs(temporaryResolveView.Get(),
                                                wgpu::TextureUsage::RenderAttachment);
                cmd->colorAttachments[index].resolveTarget = temporaryResolveView;
            }
        }

        if (temporaryResolveAttachments.size()) {
            // Check for other workarounds that need to be applied recursively.
            return ApplyRenderPassWorkarounds(
                device, usageTracker, cmd,
                [this, passEndCallback = std::move(passEndCallback),
                 temporaryResolveAttachments = std::move(temporaryResolveAttachments)]() -> void {
                    // Called once the render pass has been ended.
                    // Handle any copies needed for the AlwaysResolveIntoZeroLevelAndLayer
                    // workaround immediately after the render pass ends and before any additional
                    // commands are recorded.
                    for (auto& copyTarget : temporaryResolveAttachments) {
                        ImageCopyTexture srcImageCopyTexture = {};
                        srcImageCopyTexture.texture = copyTarget.copySrc->GetTexture();
                        srcImageCopyTexture.aspect = wgpu::TextureAspect::All;
                        srcImageCopyTexture.mipLevel = 0;
                        srcImageCopyTexture.origin = {0, 0, 0};

                        ImageCopyTexture dstImageCopyTexture = {};
                        dstImageCopyTexture.texture = copyTarget.copyDst->GetTexture();
                        dstImageCopyTexture.aspect = wgpu::TextureAspect::All;
                        dstImageCopyTexture.mipLevel = copyTarget.copyDst->GetBaseMipLevel();
                        dstImageCopyTexture.origin = {0, 0,
                                                      copyTarget.copyDst->GetBaseArrayLayer()};

                        Extent3D extent3D = copyTarget.copySrc->GetSingleSubresourceVirtualSize();

                        auto internalUsageScope = MakeInternalUsageScope();
                        this->APICopyTextureToTexture(&srcImageCopyTexture, &dstImageCopyTexture,
                                                      &extent3D);
                    }

                    // If there were any other callbacks in the workaround stack, call the next one.
                    if (passEndCallback) {
                        passEndCallback();
                    }
                });
        }
    }

    return std::move(passEndCallback);
}

void CommandEncoder::APICopyBufferToBuffer(BufferBase* source,
                                           uint64_t sourceOffset,
                                           BufferBase* destination,
                                           uint64_t destinationOffset,
                                           uint64_t size) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source));
                DAWN_TRY(GetDevice()->ValidateObject(destination));

                DAWN_INVALID_IF(source == destination,
                                "Source and destination are the same buffer (%s).", source);

                DAWN_TRY_CONTEXT(ValidateCopySizeFitsInBuffer(source, sourceOffset, size),
                                 "validating source %s copy size.", source);
                DAWN_TRY_CONTEXT(ValidateCopySizeFitsInBuffer(destination, destinationOffset, size),
                                 "validating destination %s copy size.", destination);
                DAWN_TRY(ValidateB2BCopyAlignment(size, sourceOffset, destinationOffset));

                DAWN_TRY_CONTEXT(ValidateCanUseAs(source, wgpu::BufferUsage::CopySrc),
                                 "validating source %s usage.", source);
                DAWN_TRY_CONTEXT(ValidateCanUseAs(destination, wgpu::BufferUsage::CopyDst),
                                 "validating destination %s usage.", destination);
            }

            mTopLevelBuffers.insert(source);
            mTopLevelBuffers.insert(destination);

            CopyBufferToBufferCmd* copy =
                allocator->Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
            copy->source = source;
            copy->sourceOffset = sourceOffset;
            copy->destination = destination;
            copy->destinationOffset = destinationOffset;
            copy->size = size;

            return {};
        },
        "encoding %s.CopyBufferToBuffer(%s, %u, %s, %u, %u).", this, source, sourceOffset,
        destination, destinationOffset, size);
}

// The internal version of APICopyBufferToBuffer which validates against mAllocatedSize instead of
// mSize of buffers.
void CommandEncoder::InternalCopyBufferToBufferWithAllocatedSize(BufferBase* source,
                                                                 uint64_t sourceOffset,
                                                                 BufferBase* destination,
                                                                 uint64_t destinationOffset,
                                                                 uint64_t size) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source));
                DAWN_TRY(GetDevice()->ValidateObject(destination));

                DAWN_INVALID_IF(source == destination,
                                "Source and destination are the same buffer (%s).", source);

                DAWN_TRY_CONTEXT(ValidateCopySizeFitsInBuffer(source, sourceOffset, size,
                                                              BufferSizeType::AllocatedSize),
                                 "validating source %s copy size against allocated size.", source);
                DAWN_TRY_CONTEXT(ValidateCopySizeFitsInBuffer(destination, destinationOffset, size,
                                                              BufferSizeType::AllocatedSize),
                                 "validating destination %s copy size against allocated size.",
                                 destination);
                DAWN_TRY(ValidateB2BCopyAlignment(size, sourceOffset, destinationOffset));

                DAWN_TRY_CONTEXT(ValidateCanUseAs(source, wgpu::BufferUsage::CopySrc),
                                 "validating source %s usage.", source);
                DAWN_TRY_CONTEXT(ValidateCanUseAs(destination, wgpu::BufferUsage::CopyDst),
                                 "validating destination %s usage.", destination);
            }

            mTopLevelBuffers.insert(source);
            mTopLevelBuffers.insert(destination);

            CopyBufferToBufferCmd* copy =
                allocator->Allocate<CopyBufferToBufferCmd>(Command::CopyBufferToBuffer);
            copy->source = source;
            copy->sourceOffset = sourceOffset;
            copy->destination = destination;
            copy->destinationOffset = destinationOffset;
            copy->size = size;

            return {};
        },
        "encoding internal %s.CopyBufferToBuffer(%s, %u, %s, %u, %u).", this, source, sourceOffset,
        destination, destinationOffset, size);
}

void CommandEncoder::APICopyBufferToTexture(const ImageCopyBuffer* source,
                                            const ImageCopyTexture* destination,
                                            const Extent3D* copySize) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateImageCopyBuffer(GetDevice(), *source));
                DAWN_TRY_CONTEXT(ValidateCanUseAs(source->buffer, wgpu::BufferUsage::CopySrc),
                                 "validating source %s usage.", source->buffer);

                DAWN_TRY(ValidateImageCopyTexture(GetDevice(), *destination, *copySize));
                DAWN_TRY_CONTEXT(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::CopyDst,
                                                  mUsageValidationMode),
                                 "validating destination %s usage.", destination->texture);
                DAWN_TRY(ValidateTextureSampleCountInBufferCopyCommands(destination->texture));

                DAWN_TRY(ValidateLinearToDepthStencilCopyRestrictions(*destination));
                // We validate texture copy range before validating linear texture data,
                // because in the latter we divide copyExtent.width by blockWidth and
                // copyExtent.height by blockHeight while the divisibility conditions are
                // checked in validating texture copy range.
                DAWN_TRY(ValidateTextureCopyRange(GetDevice(), *destination, *copySize));
            }
            const TexelBlockInfo& blockInfo =
                destination->texture->GetFormat().GetAspectInfo(destination->aspect).block;
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateLinearTextureCopyOffset(
                    source->layout, blockInfo,
                    destination->texture->GetFormat().HasDepthOrStencil()));
                DAWN_TRY(ValidateLinearTextureData(source->layout, source->buffer->GetSize(),
                                                   blockInfo, *copySize));
            }

            mTopLevelBuffers.insert(source->buffer);
            mTopLevelTextures.insert(destination->texture);

            TextureDataLayout srcLayout = source->layout;
            ApplyDefaultTextureDataLayoutOptions(&srcLayout, blockInfo, *copySize);

            TextureCopy dst;
            dst.texture = destination->texture;
            dst.origin = destination->origin;
            dst.mipLevel = destination->mipLevel;
            dst.aspect = ConvertAspect(destination->texture->GetFormat(), destination->aspect);

            if (dst.aspect == Aspect::Depth &&
                GetDevice()->IsToggleEnabled(Toggle::UseBlitForBufferToDepthTextureCopy)) {
                // The below function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                DAWN_TRY_CONTEXT(
                    BlitBufferToDepth(GetDevice(), this, source->buffer, srcLayout, dst, *copySize),
                    "copying from %s to depth aspect of %s using blit workaround.", source->buffer,
                    dst.texture.Get());
                return {};
            } else if (dst.aspect == Aspect::Stencil &&
                       GetDevice()->IsToggleEnabled(Toggle::UseBlitForBufferToStencilTextureCopy)) {
                // The below function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                DAWN_TRY_CONTEXT(BlitBufferToStencil(GetDevice(), this, source->buffer, srcLayout,
                                                     dst, *copySize),
                                 "copying from %s to stencil aspect of %s using blit workaround.",
                                 source->buffer, dst.texture.Get());
                return {};
            }

            CopyBufferToTextureCmd* copy =
                allocator->Allocate<CopyBufferToTextureCmd>(Command::CopyBufferToTexture);
            copy->source.buffer = source->buffer;
            copy->source.offset = srcLayout.offset;
            copy->source.bytesPerRow = srcLayout.bytesPerRow;
            copy->source.rowsPerImage = srcLayout.rowsPerImage;
            copy->destination = dst;
            copy->copySize = *copySize;

            return {};
        },
        "encoding %s.CopyBufferToTexture(%s, %s, %s).", this, source->buffer, destination->texture,
        copySize);
}

void CommandEncoder::APICopyTextureToBuffer(const ImageCopyTexture* source,
                                            const ImageCopyBuffer* destination,
                                            const Extent3D* copySize) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateImageCopyTexture(GetDevice(), *source, *copySize));
                DAWN_TRY_CONTEXT(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc,
                                                  mUsageValidationMode),
                                 "validating source %s usage.", source->texture);
                DAWN_TRY(ValidateTextureSampleCountInBufferCopyCommands(source->texture));
                DAWN_TRY(ValidateTextureDepthStencilToBufferCopyRestrictions(*source));

                DAWN_TRY(ValidateImageCopyBuffer(GetDevice(), *destination));
                DAWN_TRY_CONTEXT(ValidateCanUseAs(destination->buffer, wgpu::BufferUsage::CopyDst),
                                 "validating destination %s usage.", destination->buffer);

                // We validate texture copy range before validating linear texture data,
                // because in the latter we divide copyExtent.width by blockWidth and
                // copyExtent.height by blockHeight while the divisibility conditions are
                // checked in validating texture copy range.
                DAWN_TRY(ValidateTextureCopyRange(GetDevice(), *source, *copySize));

                if (GetDevice()->IsCompatibilityMode()) {
                    DAWN_TRY(ValidateTextureFormatForTextureToBufferCopyInCompatibilityMode(
                        source->texture));
                }
            }
            const TexelBlockInfo& blockInfo =
                source->texture->GetFormat().GetAspectInfo(source->aspect).block;
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateLinearTextureCopyOffset(
                    destination->layout, blockInfo,
                    source->texture->GetFormat().HasDepthOrStencil()));
                DAWN_TRY(ValidateLinearTextureData(
                    destination->layout, destination->buffer->GetSize(), blockInfo, *copySize));
            }

            mTopLevelTextures.insert(source->texture);
            mTopLevelBuffers.insert(destination->buffer);

            TextureDataLayout dstLayout = destination->layout;
            ApplyDefaultTextureDataLayoutOptions(&dstLayout, blockInfo, *copySize);

            if (copySize->width == 0 || copySize->height == 0 ||
                copySize->depthOrArrayLayers == 0) {
                // Noop copy but is valid, simply skip encoding any command.
                return {};
            }

            auto format = source->texture->GetFormat();
            auto aspect = ConvertAspect(format, source->aspect);

            // Workaround to use compute pass to emulate texture to buffer copy
            if (ShouldUseTextureToBufferBlit(GetDevice(), format, aspect)) {
                // This function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                TextureCopy src;
                src.texture = source->texture;
                src.origin = source->origin;
                src.mipLevel = source->mipLevel;
                src.aspect = aspect;

                BufferCopy dst;
                dst.buffer = destination->buffer;
                dst.bytesPerRow = destination->layout.bytesPerRow;
                dst.rowsPerImage = destination->layout.rowsPerImage;
                dst.offset = destination->layout.offset;
                DAWN_TRY_CONTEXT(BlitTextureToBuffer(GetDevice(), this, src, dst, *copySize),
                                 "copying texture %s to %s using blit workaround.",
                                 src.texture.Get(), destination->buffer);

                return {};
            }

            CopyTextureToBufferCmd* t2b =
                allocator->Allocate<CopyTextureToBufferCmd>(Command::CopyTextureToBuffer);
            t2b->source.texture = source->texture;
            t2b->source.origin = source->origin;
            t2b->source.mipLevel = source->mipLevel;
            t2b->source.aspect = ConvertAspect(source->texture->GetFormat(), source->aspect);
            t2b->destination.buffer = destination->buffer;
            t2b->destination.offset = dstLayout.offset;
            t2b->destination.bytesPerRow = dstLayout.bytesPerRow;
            t2b->destination.rowsPerImage = dstLayout.rowsPerImage;
            t2b->copySize = *copySize;

            return {};
        },
        "encoding %s.CopyTextureToBuffer(%s, %s, %s).", this, source->texture, destination->buffer,
        copySize);
}

void CommandEncoder::APICopyTextureToTexture(const ImageCopyTexture* source,
                                             const ImageCopyTexture* destination,
                                             const Extent3D* copySize) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(source->texture));
                DAWN_TRY(GetDevice()->ValidateObject(destination->texture));

                DAWN_INVALID_IF(source->texture->GetFormat().IsMultiPlanar() ||
                                    destination->texture->GetFormat().IsMultiPlanar(),
                                "Copying between a multiplanar texture and another texture is "
                                "currently not allowed.");
                DAWN_TRY_CONTEXT(ValidateImageCopyTexture(GetDevice(), *source, *copySize),
                                 "validating source %s.", source->texture);
                DAWN_TRY_CONTEXT(ValidateImageCopyTexture(GetDevice(), *destination, *copySize),
                                 "validating destination %s.", destination->texture);

                DAWN_TRY_CONTEXT(ValidateTextureCopyRange(GetDevice(), *source, *copySize),
                                 "validating source %s copy range.", source->texture);
                DAWN_TRY_CONTEXT(ValidateTextureCopyRange(GetDevice(), *destination, *copySize),
                                 "validating source %s copy range.", destination->texture);

                DAWN_TRY(
                    ValidateTextureToTextureCopyRestrictions(*source, *destination, *copySize));

                DAWN_TRY(ValidateCanUseAs(source->texture, wgpu::TextureUsage::CopySrc,
                                          mUsageValidationMode));
                DAWN_TRY(ValidateCanUseAs(destination->texture, wgpu::TextureUsage::CopyDst,
                                          mUsageValidationMode));

                if (GetDevice()->IsCompatibilityMode()) {
                    DAWN_TRY(ValidateSourceTextureFormatForTextureToTextureCopyInCompatibilityMode(
                        source->texture));
                }
            }

            mTopLevelTextures.insert(source->texture);
            mTopLevelTextures.insert(destination->texture);

            Aspect aspect = ConvertAspect(source->texture->GetFormat(), source->aspect);
            DAWN_ASSERT(aspect ==
                        ConvertAspect(destination->texture->GetFormat(), destination->aspect));

            TextureCopy src;
            src.texture = source->texture;
            src.origin = source->origin;
            src.mipLevel = source->mipLevel;
            src.aspect = aspect;

            TextureCopy dst;
            dst.texture = destination->texture;
            dst.origin = destination->origin;
            dst.mipLevel = destination->mipLevel;
            dst.aspect = aspect;

            // Emulate RGB9E5Ufloat T2T copy by calling a T2B copy and then a B2T copy
            // for OpenGL/ES.
            if (src.texture->GetFormat().baseFormat == wgpu::TextureFormat::RGB9E5Ufloat &&
                GetDevice()->IsToggleEnabled(Toggle::UseBlitForRGB9E5UfloatTextureCopy)) {
                DAWN_ASSERT(dst.texture->GetFormat().baseFormat ==
                            wgpu::TextureFormat::RGB9E5Ufloat);

                // Calculate needed buffer size to hold copied texel data.
                const TexelBlockInfo& blockInfo =
                    source->texture->GetFormat().GetAspectInfo(aspect).block;
                const uint64_t bytesPerRow =
                    Align(4 * copySize->width, kTextureBytesPerRowAlignment);
                const uint64_t rowsPerImage = copySize->height;
                uint64_t requiredBytes;
                DAWN_TRY_ASSIGN(
                    requiredBytes,
                    ComputeRequiredBytesInCopy(blockInfo, *copySize, bytesPerRow, rowsPerImage));

                // Create an intermediate dst buffer.
                BufferDescriptor descriptor = {};
                descriptor.size = Align(requiredBytes, 4);
                descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
                Ref<BufferBase> intermediateBuffer;
                DAWN_TRY_ASSIGN(intermediateBuffer, GetDevice()->CreateBuffer(&descriptor));

                ImageCopyBuffer buf;
                buf.buffer = intermediateBuffer.Get();
                buf.layout.bytesPerRow = bytesPerRow;
                buf.layout.rowsPerImage = rowsPerImage;

                APICopyTextureToBuffer(source, &buf, copySize);
                APICopyBufferToTexture(&buf, destination, copySize);

                return {};
            }

            const bool blitDepth =
                (aspect & Aspect::Depth) &&
                GetDevice()->IsToggleEnabled(
                    Toggle::UseBlitForDepthTextureToTextureCopyToNonzeroSubresource) &&
                (dst.mipLevel > 0 || dst.origin.z > 0 || copySize->depthOrArrayLayers > 1);

            // If we're not using a blit, or there are aspects other than depth,
            // issue the copy. This is because if there's also stencil, we still need the copy
            // command to copy the stencil portion.
            if (!blitDepth || aspect != Aspect::Depth) {
                CopyTextureToTextureCmd* copy =
                    allocator->Allocate<CopyTextureToTextureCmd>(Command::CopyTextureToTexture);
                copy->source = src;
                copy->destination = dst;
                copy->copySize = *copySize;
            }

            // Use a blit to copy the depth aspect.
            if (blitDepth) {
                // This function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                DAWN_TRY_CONTEXT(BlitDepthToDepth(GetDevice(), this, src, dst, *copySize),
                                 "copying depth aspect from %s to %s using blit workaround.",
                                 source->texture, destination->texture);
            }

            return {};
        },
        "encoding %s.CopyTextureToTexture(%s, %s, %s).", this, source->texture,
        destination->texture, copySize);
}

void CommandEncoder::APIClearBuffer(BufferBase* buffer, uint64_t offset, uint64_t size) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(buffer));

                uint64_t bufferSize = buffer->GetSize();
                DAWN_INVALID_IF(offset > bufferSize,
                                "Buffer offset (%u) is larger than the size (%u) of %s.", offset,
                                bufferSize, buffer);

                uint64_t remainingSize = bufferSize - offset;
                if (size == wgpu::kWholeSize) {
                    size = remainingSize;
                } else {
                    DAWN_INVALID_IF(size > remainingSize,
                                    "Buffer range (offset: %u, size: %u) doesn't fit in "
                                    "the size (%u) of %s.",
                                    offset, size, bufferSize, buffer);
                }

                DAWN_TRY_CONTEXT(ValidateCanUseAs(buffer, wgpu::BufferUsage::CopyDst),
                                 "validating buffer %s usage.", buffer);

                // Size must be a multiple of 4 bytes on macOS.
                DAWN_INVALID_IF(size % 4 != 0, "Fill size (%u) is not a multiple of 4 bytes.",
                                size);

                // Offset must be multiples of 4 bytes on macOS.
                DAWN_INVALID_IF(offset % 4 != 0, "Offset (%u) is not a multiple of 4 bytes,",
                                offset);

            } else {
                if (size == wgpu::kWholeSize) {
                    DAWN_ASSERT(buffer->GetSize() >= offset);
                    size = buffer->GetSize() - offset;
                }
            }

            mTopLevelBuffers.insert(buffer);

            ClearBufferCmd* cmd = allocator->Allocate<ClearBufferCmd>(Command::ClearBuffer);
            cmd->buffer = buffer;
            cmd->offset = offset;
            cmd->size = size;

            return {};
        },
        "encoding %s.ClearBuffer(%s, %u, %u).", this, buffer, offset, size);
}

void CommandEncoder::APIInjectValidationError(const char* message) {
    if (mEncodingContext.CheckCurrentEncoder(this)) {
        mEncodingContext.HandleError(DAWN_MAKE_ERROR(InternalErrorType::Validation, message));
    }
}

void CommandEncoder::APIInsertDebugMarker(const char* groupLabel) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            InsertDebugMarkerCmd* cmd =
                allocator->Allocate<InsertDebugMarkerCmd>(Command::InsertDebugMarker);
            cmd->length = strlen(groupLabel);

            char* label = allocator->AllocateData<char>(cmd->length + 1);
            memcpy(label, groupLabel, cmd->length + 1);

            return {};
        },
        "encoding %s.InsertDebugMarker(\"%s\").", this, groupLabel);
}

void CommandEncoder::APIPopDebugGroup() {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_INVALID_IF(mDebugGroupStackSize == 0,
                                "PopDebugGroup called when no debug groups are currently pushed.");
            }
            allocator->Allocate<PopDebugGroupCmd>(Command::PopDebugGroup);
            mDebugGroupStackSize--;
            mEncodingContext.PopDebugGroupLabel();

            return {};
        },
        "encoding %s.PopDebugGroup().", this);
}

void CommandEncoder::APIPushDebugGroup(const char* groupLabel) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            PushDebugGroupCmd* cmd =
                allocator->Allocate<PushDebugGroupCmd>(Command::PushDebugGroup);
            cmd->length = strlen(groupLabel);

            char* label = allocator->AllocateData<char>(cmd->length + 1);
            memcpy(label, groupLabel, cmd->length + 1);

            mDebugGroupStackSize++;
            mEncodingContext.PushDebugGroupLabel(groupLabel);

            return {};
        },
        "encoding %s.PushDebugGroup(\"%s\").", this, groupLabel);
}

void CommandEncoder::APIResolveQuerySet(QuerySetBase* querySet,
                                        uint32_t firstQuery,
                                        uint32_t queryCount,
                                        BufferBase* destination,
                                        uint64_t destinationOffset) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(GetDevice()->ValidateObject(querySet));
                DAWN_TRY(GetDevice()->ValidateObject(destination));

                DAWN_TRY(ValidateQuerySetResolve(querySet, firstQuery, queryCount, destination,
                                                 destinationOffset));

                DAWN_TRY(ValidateCanUseAs(destination, wgpu::BufferUsage::QueryResolve));

                TrackUsedQuerySet(querySet);
            }

            mTopLevelBuffers.insert(destination);

            ResolveQuerySetCmd* cmd =
                allocator->Allocate<ResolveQuerySetCmd>(Command::ResolveQuerySet);
            cmd->querySet = querySet;
            cmd->firstQuery = firstQuery;
            cmd->queryCount = queryCount;
            cmd->destination = destination;
            cmd->destinationOffset = destinationOffset;

            // Encode internal compute pipeline for timestamp query
            if (querySet->GetQueryType() == wgpu::QueryType::Timestamp &&
                !GetDevice()->IsToggleEnabled(Toggle::DisableTimestampQueryConversion)) {
                // The below function might create new resources. Need to lock the Device.
                // TODO(crbug.com/dawn/1618): In future, all temp resources should be created at
                // Command Submit time, so the locking would be removed from here at that point.
                auto deviceLock(GetDevice()->GetScopedLock());

                DAWN_TRY(EncodeTimestampsToNanosecondsConversion(
                    this, querySet, firstQuery, queryCount, destination, destinationOffset));
            }

            return {};
        },
        "encoding %s.ResolveQuerySet(%s, %u, %u, %s, %u).", this, querySet, firstQuery, queryCount,
        destination, destinationOffset);
}

void CommandEncoder::APIWriteBuffer(BufferBase* buffer,
                                    uint64_t bufferOffset,
                                    const uint8_t* data,
                                    uint64_t size) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateWriteBuffer(GetDevice(), buffer, bufferOffset, size));
            }

            WriteBufferCmd* cmd = allocator->Allocate<WriteBufferCmd>(Command::WriteBuffer);
            cmd->buffer = buffer;
            cmd->offset = bufferOffset;
            cmd->size = size;

            uint8_t* inlinedData = allocator->AllocateData<uint8_t>(size);
            memcpy(inlinedData, data, size);

            mTopLevelBuffers.insert(buffer);

            return {};
        },
        "encoding %s.WriteBuffer(%s, %u, ..., %u).", this, buffer, bufferOffset, size);
}

void CommandEncoder::APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex) {
    mEncodingContext.TryEncode(
        this,
        [&](CommandAllocator* allocator) -> MaybeError {
            if (GetDevice()->IsValidationEnabled()) {
                DAWN_TRY(ValidateTimestampQuery(GetDevice(), querySet, queryIndex));
            }

            TrackQueryAvailability(querySet, queryIndex);

            WriteTimestampCmd* cmd =
                allocator->Allocate<WriteTimestampCmd>(Command::WriteTimestamp);
            cmd->querySet = querySet;
            cmd->queryIndex = queryIndex;

            return {};
        },
        "encoding %s.WriteTimestamp(%s, %u).", this, querySet, queryIndex);
}

CommandBufferBase* CommandEncoder::APIFinish(const CommandBufferDescriptor* descriptor) {
    // This function will create new object, need to lock the Device.
    auto deviceLock(GetDevice()->GetScopedLock());

    Ref<CommandBufferBase> commandBuffer;
    if (GetDevice()->ConsumedError(Finish(descriptor), &commandBuffer)) {
        CommandBufferBase* errorCommandBuffer =
            CommandBufferBase::MakeError(GetDevice(), descriptor ? descriptor->label : nullptr);
        errorCommandBuffer->SetEncoderLabel(this->GetLabel());
        return errorCommandBuffer;
    }
    DAWN_ASSERT(!IsError());
    return commandBuffer.Detach();
}

ResultOrError<Ref<CommandBufferBase>> CommandEncoder::Finish(
    const CommandBufferDescriptor* descriptor) {
    DeviceBase* device = GetDevice();

    // Even if mEncodingContext.Finish() validation fails, calling it will mutate the internal
    // state of the encoding context. The internal state is set to finished, and subsequent
    // calls to encode commands will generate errors.
    DAWN_TRY(mEncodingContext.Finish());
    DAWN_TRY(device->ValidateIsAlive());

    if (device->IsValidationEnabled()) {
        DAWN_TRY(ValidateFinish());
    }

    const CommandBufferDescriptor defaultDescriptor = {};
    if (descriptor == nullptr) {
        descriptor = &defaultDescriptor;
    }

    return device->CreateCommandBuffer(this, descriptor);
}

// Implementation of the command buffer validation that can be precomputed before submit
MaybeError CommandEncoder::ValidateFinish() const {
    TRACE_EVENT0(GetDevice()->GetPlatform(), Validation, "CommandEncoder::ValidateFinish");
    DAWN_TRY(GetDevice()->ValidateObject(this));

    for (const RenderPassResourceUsage& passUsage : mEncodingContext.GetRenderPassUsages()) {
        DAWN_TRY_CONTEXT(ValidateSyncScopeResourceUsage(passUsage),
                         "validating render pass usage.");
    }

    for (const ComputePassResourceUsage& passUsage : mEncodingContext.GetComputePassUsages()) {
        for (const SyncScopeResourceUsage& scope : passUsage.dispatchUsages) {
            DAWN_TRY_CONTEXT(ValidateSyncScopeResourceUsage(scope),
                             "validating compute pass usage.");
        }
    }

    DAWN_INVALID_IF(
        mDebugGroupStackSize != 0,
        "PushDebugGroup called %u time(s) without a corresponding PopDebugGroup prior to "
        "calling Finish.",
        mDebugGroupStackSize);

    return {};
}

CommandEncoder::InternalUsageScope CommandEncoder::MakeInternalUsageScope() {
    return InternalUsageScope(this);
}

CommandEncoder::InternalUsageScope::InternalUsageScope(CommandEncoder* encoder)
    : mEncoder(encoder), mUsageValidationMode(mEncoder->mUsageValidationMode) {
    mEncoder->mUsageValidationMode = UsageValidationMode::Internal;
}

CommandEncoder::InternalUsageScope::~InternalUsageScope() {
    mEncoder->mUsageValidationMode = mUsageValidationMode;
}

}  // namespace dawn::native
