// Copyright 2018 The Dawn Authors
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

#include "dawn_native/RenderPassEncoder.h"

#include "common/Constants.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/RenderBundle.h"
#include "dawn_native/RenderPipeline.h"

#include <math.h>
#include <cstring>

namespace dawn_native {

    RenderPassEncoderBase::RenderPassEncoderBase(DeviceBase* device,
                                                 CommandEncoderBase* commandEncoder,
                                                 EncodingContext* encodingContext)
        : RenderEncoderBase(device, encodingContext), mCommandEncoder(commandEncoder) {
    }

    RenderPassEncoderBase::RenderPassEncoderBase(DeviceBase* device,
                                                 CommandEncoderBase* commandEncoder,
                                                 EncodingContext* encodingContext,
                                                 ErrorTag errorTag)
        : RenderEncoderBase(device, encodingContext, errorTag), mCommandEncoder(commandEncoder) {
    }

    RenderPassEncoderBase* RenderPassEncoderBase::MakeError(DeviceBase* device,
                                                            CommandEncoderBase* commandEncoder,
                                                            EncodingContext* encodingContext) {
        return new RenderPassEncoderBase(device, commandEncoder, encodingContext,
                                         ObjectBase::kError);
    }

    void RenderPassEncoderBase::EndPass() {
        if (mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
                allocator->Allocate<EndRenderPassCmd>(Command::EndRenderPass);

                return {};
            })) {
            mEncodingContext->ExitPass(this);
        }
    }

    void RenderPassEncoderBase::SetStencilReference(uint32_t reference) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            SetStencilReferenceCmd* cmd =
                allocator->Allocate<SetStencilReferenceCmd>(Command::SetStencilReference);
            cmd->reference = reference;

            return {};
        });
    }

    void RenderPassEncoderBase::SetBlendColor(const Color* color) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            SetBlendColorCmd* cmd = allocator->Allocate<SetBlendColorCmd>(Command::SetBlendColor);
            cmd->color = *color;

            return {};
        });
    }

    void RenderPassEncoderBase::SetViewport(float x,
                                            float y,
                                            float width,
                                            float height,
                                            float minDepth,
                                            float maxDepth) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if ((isnan(x) || isnan(y) || isnan(width) || isnan(height) || isnan(minDepth) ||
                 isnan(maxDepth))) {
                return DAWN_VALIDATION_ERROR("NaN is not allowed.");
            }

            // TODO(yunchao.he@intel.com): there are more restrictions for x, y, width and height in
            // Vulkan, and height can be a negative value in Vulkan 1.1. Revisit this part later
            // (say, for WebGPU v1).
            if (width <= 0 || height <= 0) {
                return DAWN_VALIDATION_ERROR("Width and height must be greater than 0.");
            }

            if (minDepth < 0 || minDepth > 1 || maxDepth < 0 || maxDepth > 1) {
                return DAWN_VALIDATION_ERROR("minDepth and maxDepth must be in [0, 1].");
            }

            SetViewportCmd* cmd = allocator->Allocate<SetViewportCmd>(Command::SetViewport);
            cmd->x = x;
            cmd->y = y;
            cmd->width = width;
            cmd->height = height;
            cmd->minDepth = minDepth;
            cmd->maxDepth = maxDepth;

            return {};
        });
    }

    void RenderPassEncoderBase::SetScissorRect(uint32_t x,
                                               uint32_t y,
                                               uint32_t width,
                                               uint32_t height) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            if (width == 0 || height == 0) {
                return DAWN_VALIDATION_ERROR("Width and height must be greater than 0.");
            }

            SetScissorRectCmd* cmd =
                allocator->Allocate<SetScissorRectCmd>(Command::SetScissorRect);
            cmd->x = x;
            cmd->y = y;
            cmd->width = width;
            cmd->height = height;

            return {};
        });
    }

    void RenderPassEncoderBase::ExecuteBundles(uint32_t count,
                                               RenderBundleBase* const* renderBundles) {
        mEncodingContext->TryEncode(this, [&](CommandAllocator* allocator) -> MaybeError {
            for (uint32_t i = 0; i < count; ++i) {
                DAWN_TRY(GetDevice()->ValidateObject(renderBundles[i]));
            }

            ExecuteBundlesCmd* cmd =
                allocator->Allocate<ExecuteBundlesCmd>(Command::ExecuteBundles);
            cmd->count = count;

            Ref<RenderBundleBase>* bundles = allocator->AllocateData<Ref<RenderBundleBase>>(count);
            for (uint32_t i = 0; i < count; ++i) {
                bundles[i] = renderBundles[i];
            }

            return {};
        });
    }

}  // namespace dawn_native
