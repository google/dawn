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

#include "dawn_native/CommandBuffer.h"

#include "common/BitSetIterator.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Format.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    CommandBufferBase::CommandBufferBase(CommandEncoder* encoder, const CommandBufferDescriptor*)
        : ObjectBase(encoder->GetDevice()),
          mCommands(encoder->AcquireCommands()),
          mResourceUsages(encoder->AcquireResourceUsages()) {
    }

    CommandBufferBase::CommandBufferBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ObjectBase(device, tag) {
    }

    CommandBufferBase::~CommandBufferBase() {
        Destroy();
    }

    // static
    CommandBufferBase* CommandBufferBase::MakeError(DeviceBase* device) {
        return new CommandBufferBase(device, ObjectBase::kError);
    }

    MaybeError CommandBufferBase::ValidateCanUseInSubmitNow() const {
        ASSERT(!IsError());

        if (mDestroyed) {
            return DAWN_VALIDATION_ERROR("Command buffer reused in submit");
        }
        return {};
    }

    void CommandBufferBase::Destroy() {
        FreeCommands(&mCommands);
        mResourceUsages = {};
        mDestroyed = true;
    }

    const CommandBufferResourceUsage& CommandBufferBase::GetResourceUsages() const {
        return mResourceUsages;
    }

    bool IsCompleteSubresourceCopiedTo(const TextureBase* texture,
                                       const Extent3D copySize,
                                       const uint32_t mipLevel) {
        Extent3D extent = texture->GetMipLevelPhysicalSize(mipLevel);

        ASSERT(texture->GetDimension() == wgpu::TextureDimension::e2D);
        if (extent.width == copySize.width && extent.height == copySize.height) {
            return true;
        }
        return false;
    }

    SubresourceRange GetSubresourcesAffectedByCopy(const TextureCopy& copy,
                                                   const Extent3D& copySize) {
        switch (copy.texture->GetDimension()) {
            case wgpu::TextureDimension::e2D:
                return {copy.aspect, {copy.origin.z, copySize.depth}, {copy.mipLevel, 1}};
            default:
                UNREACHABLE();
        }
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

                case wgpu::StoreOp::Clear:
                    view->GetTexture()->SetIsSubresourceContentInitialized(false, range);
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
            return false;
        }

        const TextureBase* texture = copy->source.texture.Get();
        const TexelBlockInfo& blockInfo =
            texture->GetFormat().GetAspectInfo(copy->source.aspect).block;
        const uint64_t heightInBlocks = copy->copySize.height / blockInfo.height;

        if (copy->destination.rowsPerImage > heightInBlocks) {
            return false;
        }

        const uint64_t copyTextureDataSizePerRow =
            copy->copySize.width / blockInfo.width * blockInfo.byteSize;
        if (copy->destination.bytesPerRow > copyTextureDataSizePerRow) {
            return false;
        }

        const uint64_t overwrittenRangeSize =
            copyTextureDataSizePerRow * heightInBlocks * copy->copySize.depth;
        if (copy->destination.buffer->GetSize() > overwrittenRangeSize) {
            return false;
        }

        return true;
    }

    std::array<float, 4> ConvertToFloatColor(dawn_native::Color color) {
        const std::array<float, 4> outputValue = {
            static_cast<float>(color.r), static_cast<float>(color.g), static_cast<float>(color.b),
            static_cast<float>(color.a)};
        return outputValue;
    }
    std::array<int32_t, 4> ConvertToSignedIntegerColor(dawn_native::Color color) {
        const std::array<int32_t, 4> outputValue = {
            static_cast<int32_t>(color.r), static_cast<int32_t>(color.g),
            static_cast<int32_t>(color.b), static_cast<int32_t>(color.a)};
        return outputValue;
    }

    std::array<uint32_t, 4> ConvertToUnsignedIntegerColor(dawn_native::Color color) {
        const std::array<uint32_t, 4> outputValue = {
            static_cast<uint32_t>(color.r), static_cast<uint32_t>(color.g),
            static_cast<uint32_t>(color.b), static_cast<uint32_t>(color.a)};
        return outputValue;
    }

}  // namespace dawn_native
