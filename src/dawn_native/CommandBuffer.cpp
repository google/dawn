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
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Format.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    CommandBufferBase::CommandBufferBase(CommandEncoder* encoder, const CommandBufferDescriptor*)
        : ObjectBase(encoder->GetDevice()), mResourceUsages(encoder->AcquireResourceUsages()) {
    }

    CommandBufferBase::CommandBufferBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ObjectBase(device, tag) {
    }

    // static
    CommandBufferBase* CommandBufferBase::MakeError(DeviceBase* device) {
        return new CommandBufferBase(device, ObjectBase::kError);
    }

    const CommandBufferResourceUsage& CommandBufferBase::GetResourceUsages() const {
        return mResourceUsages;
    }

    bool IsCompleteSubresourceCopiedTo(const TextureBase* texture,
                                       const Extent3D copySize,
                                       const uint32_t mipLevel) {
        Extent3D extent = texture->GetMipLevelPhysicalSize(mipLevel);

        if (extent.depth == copySize.depth && extent.width == copySize.width &&
            extent.height == copySize.height) {
            return true;
        }
        return false;
    }

    void LazyClearRenderPassAttachments(BeginRenderPassCmd* renderPass) {
        for (uint32_t i : IterateBitSet(renderPass->attachmentState->GetColorAttachmentsMask())) {
            auto& attachmentInfo = renderPass->colorAttachments[i];
            TextureViewBase* view = attachmentInfo.view.Get();
            bool hasResolveTarget = attachmentInfo.resolveTarget.Get() != nullptr;

            ASSERT(view->GetLayerCount() == 1);
            ASSERT(view->GetLevelCount() == 1);

            // If the loadOp is Load, but the subresource is not initialized, use Clear instead.
            if (attachmentInfo.loadOp == wgpu::LoadOp::Load &&
                !view->GetTexture()->IsSubresourceContentInitialized(
                    view->GetBaseMipLevel(), 1, view->GetBaseArrayLayer(), 1)) {
                attachmentInfo.loadOp = wgpu::LoadOp::Clear;
                attachmentInfo.clearColor = {0.f, 0.f, 0.f, 0.f};
            }

            if (hasResolveTarget) {
                // We need to set the resolve target to initialized so that it does not get
                // cleared later in the pipeline. The texture will be resolved from the
                // source color attachment, which will be correctly initialized.
                TextureViewBase* resolveView = attachmentInfo.resolveTarget.Get();
                resolveView->GetTexture()->SetIsSubresourceContentInitialized(
                    true, resolveView->GetBaseMipLevel(), resolveView->GetLevelCount(),
                    resolveView->GetBaseArrayLayer(), resolveView->GetLayerCount());
            }

            switch (attachmentInfo.storeOp) {
                case wgpu::StoreOp::Store:
                    view->GetTexture()->SetIsSubresourceContentInitialized(
                        true, view->GetBaseMipLevel(), 1, view->GetBaseArrayLayer(), 1);
                    break;

                case wgpu::StoreOp::Clear:
                    view->GetTexture()->SetIsSubresourceContentInitialized(
                        false, view->GetBaseMipLevel(), 1, view->GetBaseArrayLayer(), 1);
                    break;

                default:
                    UNREACHABLE();
                    break;
            }
        }

        if (renderPass->attachmentState->HasDepthStencilAttachment()) {
            auto& attachmentInfo = renderPass->depthStencilAttachment;
            TextureViewBase* view = attachmentInfo.view.Get();

            // If the depth stencil texture has not been initialized, we want to use loadop
            // clear to init the contents to 0's
            if (!view->GetTexture()->IsSubresourceContentInitialized(
                    view->GetBaseMipLevel(), view->GetLevelCount(), view->GetBaseArrayLayer(),
                    view->GetLayerCount())) {
                if (view->GetTexture()->GetFormat().HasDepth() &&
                    attachmentInfo.depthLoadOp == wgpu::LoadOp::Load) {
                    attachmentInfo.clearDepth = 0.0f;
                    attachmentInfo.depthLoadOp = wgpu::LoadOp::Clear;
                }
                if (view->GetTexture()->GetFormat().HasStencil() &&
                    attachmentInfo.stencilLoadOp == wgpu::LoadOp::Load) {
                    attachmentInfo.clearStencil = 0u;
                    attachmentInfo.stencilLoadOp = wgpu::LoadOp::Clear;
                }
            }

            // If these have different store ops, make them both Store because we can't track
            // initialized state separately yet. TODO(crbug.com/dawn/145)
            if (attachmentInfo.depthStoreOp != attachmentInfo.stencilStoreOp) {
                attachmentInfo.depthStoreOp = wgpu::StoreOp::Store;
                attachmentInfo.stencilStoreOp = wgpu::StoreOp::Store;
            }

            if (attachmentInfo.depthStoreOp == wgpu::StoreOp::Store &&
                attachmentInfo.stencilStoreOp == wgpu::StoreOp::Store) {
                view->GetTexture()->SetIsSubresourceContentInitialized(
                    true, view->GetBaseMipLevel(), view->GetLevelCount(), view->GetBaseArrayLayer(),
                    view->GetLayerCount());
            } else {
                ASSERT(attachmentInfo.depthStoreOp == wgpu::StoreOp::Clear &&
                       attachmentInfo.stencilStoreOp == wgpu::StoreOp::Clear);
                view->GetTexture()->SetIsSubresourceContentInitialized(
                    false, view->GetBaseMipLevel(), view->GetLevelCount(),
                    view->GetBaseArrayLayer(), view->GetLayerCount());
            }
        }
    }

}  // namespace dawn_native
