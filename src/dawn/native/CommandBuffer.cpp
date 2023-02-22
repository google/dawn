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

#include "dawn/native/CommandBuffer.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Format.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

CommandBufferBase::CommandBufferBase(CommandEncoder* encoder,
                                     const CommandBufferDescriptor* descriptor)
    : ApiObjectBase(encoder->GetDevice(), descriptor->label),
      mCommands(encoder->AcquireCommands()),
      mResourceUsages(encoder->AcquireResourceUsages()) {
    GetObjectTrackingList()->Track(this);
}

CommandBufferBase::CommandBufferBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag) {}

// static
CommandBufferBase* CommandBufferBase::MakeError(DeviceBase* device) {
    return new CommandBufferBase(device, ObjectBase::kError);
}

ObjectType CommandBufferBase::GetType() const {
    return ObjectType::CommandBuffer;
}

MaybeError CommandBufferBase::ValidateCanUseInSubmitNow() const {
    ASSERT(!IsError());

    DAWN_INVALID_IF(!IsAlive(), "%s cannot be submitted more than once.", this);
    return {};
}

void CommandBufferBase::DestroyImpl() {
    FreeCommands(&mCommands);
    mResourceUsages = {};
}

const CommandBufferResourceUsage& CommandBufferBase::GetResourceUsages() const {
    return mResourceUsages;
}

CommandIterator* CommandBufferBase::GetCommandIteratorForTesting() {
    return &mCommands;
}

bool IsCompleteSubresourceCopiedTo(const TextureBase* texture,
                                   const Extent3D copySize,
                                   const uint32_t mipLevel) {
    Extent3D extent = texture->GetMipLevelSingleSubresourcePhysicalSize(mipLevel);

    switch (texture->GetDimension()) {
        case wgpu::TextureDimension::e1D:
            return extent.width == copySize.width;
        case wgpu::TextureDimension::e2D:
            return extent.width == copySize.width && extent.height == copySize.height;
        case wgpu::TextureDimension::e3D:
            return extent.width == copySize.width && extent.height == copySize.height &&
                   extent.depthOrArrayLayers == copySize.depthOrArrayLayers;
    }

    UNREACHABLE();
}

SubresourceRange GetSubresourcesAffectedByCopy(const TextureCopy& copy, const Extent3D& copySize) {
    switch (copy.texture->GetDimension()) {
        case wgpu::TextureDimension::e1D:
            ASSERT(copy.origin.z == 0 && copySize.depthOrArrayLayers == 1);
            ASSERT(copy.mipLevel == 0);
            return {copy.aspect, {0, 1}, {0, 1}};
        case wgpu::TextureDimension::e2D:
            return {copy.aspect, {copy.origin.z, copySize.depthOrArrayLayers}, {copy.mipLevel, 1}};
        case wgpu::TextureDimension::e3D:
            return {copy.aspect, {0, 1}, {copy.mipLevel, 1}};
    }

    UNREACHABLE();
}

void LazyClearRenderPassAttachments(BeginRenderPassCmd* renderPass) {
    for (ColorAttachmentIndex i :
         IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
        auto& attachmentInfo = renderPass->colorAttachments[i];
        TextureViewBase* view = attachmentInfo.view.Get();
        bool hasResolveTarget = attachmentInfo.resolveTarget != nullptr;

        ASSERT(view->GetLayerCount() == 1);
        ASSERT(view->GetLevelCount() == 1);
        SubresourceRange range = view->GetSubresourceRange();

        // If the loadOp is Load, but the subresource is not initialized, use Clear instead.
        if (attachmentInfo.loadOp == wgpu::LoadOp::Load &&
            !view->GetTexture()->IsSubresourceContentInitialized(range)) {
            attachmentInfo.loadOp = wgpu::LoadOp::Clear;
            attachmentInfo.clearColor = {0.f, 0.f, 0.f, 0.f};
        }

        if (hasResolveTarget) {
            // We need to set the resolve target to initialized so that it does not get
            // cleared later in the pipeline. The texture will be resolved from the
            // source color attachment, which will be correctly initialized.
            TextureViewBase* resolveView = attachmentInfo.resolveTarget.Get();
            ASSERT(resolveView->GetLayerCount() == 1);
            ASSERT(resolveView->GetLevelCount() == 1);
            resolveView->GetTexture()->SetIsSubresourceContentInitialized(
                true, resolveView->GetSubresourceRange());
        }

        switch (attachmentInfo.storeOp) {
            case wgpu::StoreOp::Store:
                view->GetTexture()->SetIsSubresourceContentInitialized(true, range);
                break;

            case wgpu::StoreOp::Discard:
                view->GetTexture()->SetIsSubresourceContentInitialized(false, range);
                break;

            case wgpu::StoreOp::Undefined:
                UNREACHABLE();
                break;
        }
    }

    if (renderPass->attachmentState->HasDepthStencilAttachment()) {
        auto& attachmentInfo = renderPass->depthStencilAttachment;
        TextureViewBase* view = attachmentInfo.view.Get();
        ASSERT(view->GetLayerCount() == 1);
        ASSERT(view->GetLevelCount() == 1);
        SubresourceRange range = view->GetSubresourceRange();

        SubresourceRange depthRange = range;
        depthRange.aspects = range.aspects & Aspect::Depth;

        SubresourceRange stencilRange = range;
        stencilRange.aspects = range.aspects & Aspect::Stencil;

        // If the depth stencil texture has not been initialized, we want to use loadop
        // clear to init the contents to 0's
        if (!view->GetTexture()->IsSubresourceContentInitialized(depthRange) &&
            attachmentInfo.depthLoadOp == wgpu::LoadOp::Load) {
            attachmentInfo.clearDepth = 0.0f;
            attachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
        }

        if (!view->GetTexture()->IsSubresourceContentInitialized(stencilRange) &&
            attachmentInfo.stencilLoadOp == wgpu::LoadOp::Load) {
            attachmentInfo.clearStencil = 0u;
            attachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
        }

        view->GetTexture()->SetIsSubresourceContentInitialized(
            attachmentInfo.depthStoreOp == wgpu::StoreOp::Store, depthRange);

        view->GetTexture()->SetIsSubresourceContentInitialized(
            attachmentInfo.stencilStoreOp == wgpu::StoreOp::Store, stencilRange);
    }
}

bool IsFullBufferOverwrittenInTextureToBufferCopy(const CopyTextureToBufferCmd* copy) {
    ASSERT(copy != nullptr);

    if (copy->destination.offset > 0) {
        // The copy doesn't touch the start of the buffer.
        return false;
    }

    const TextureBase* texture = copy->source.texture.Get();
    const TexelBlockInfo& blockInfo = texture->GetFormat().GetAspectInfo(copy->source.aspect).block;
    const uint64_t widthInBlocks = copy->copySize.width / blockInfo.width;
    const uint64_t heightInBlocks = copy->copySize.height / blockInfo.height;
    const bool multiSlice = copy->copySize.depthOrArrayLayers > 1;
    const bool multiRow = multiSlice || heightInBlocks > 1;

    if (multiSlice && copy->destination.rowsPerImage > heightInBlocks) {
        // There are gaps between slices that aren't overwritten
        return false;
    }

    const uint64_t copyTextureDataSizePerRow = widthInBlocks * blockInfo.byteSize;
    if (multiRow && copy->destination.bytesPerRow > copyTextureDataSizePerRow) {
        // There are gaps between rows that aren't overwritten
        return false;
    }

    // After the above checks, we're sure the copy has no gaps.
    // Now, compute the total number of bytes written.
    const uint64_t writtenBytes =
        ComputeRequiredBytesInCopy(blockInfo, copy->copySize, copy->destination.bytesPerRow,
                                   copy->destination.rowsPerImage)
            .AcquireSuccess();
    if (!copy->destination.buffer->IsFullBufferRange(copy->destination.offset, writtenBytes)) {
        // The written bytes don't cover the whole buffer.
        return false;
    }

    return true;
}

std::array<float, 4> ConvertToFloatColor(dawn::native::Color color) {
    const std::array<float, 4> outputValue = {
        static_cast<float>(color.r), static_cast<float>(color.g), static_cast<float>(color.b),
        static_cast<float>(color.a)};
    return outputValue;
}
std::array<int32_t, 4> ConvertToSignedIntegerColor(dawn::native::Color color) {
    const std::array<int32_t, 4> outputValue = {
        static_cast<int32_t>(color.r), static_cast<int32_t>(color.g), static_cast<int32_t>(color.b),
        static_cast<int32_t>(color.a)};
    return outputValue;
}

std::array<uint32_t, 4> ConvertToUnsignedIntegerColor(dawn::native::Color color) {
    const std::array<uint32_t, 4> outputValue = {
        static_cast<uint32_t>(color.r), static_cast<uint32_t>(color.g),
        static_cast<uint32_t>(color.b), static_cast<uint32_t>(color.a)};
    return outputValue;
}

}  // namespace dawn::native
