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

#include "dawn_native/RenderBundleEncoder.h"

#include "dawn_native/CommandValidation.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/Format.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/ValidationUtils_autogen.h"

namespace dawn_native {

    MaybeError ValidateColorAttachmentFormat(const DeviceBase* device,
                                             dawn::TextureFormat textureFormat) {
        DAWN_TRY(ValidateTextureFormat(textureFormat));
        const Format* format = nullptr;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(textureFormat));
        if (!format->IsColor() || !format->isRenderable) {
            return DAWN_VALIDATION_ERROR(
                "The color attachment texture format is not color renderable");
        }
        return {};
    }

    MaybeError ValidateDepthStencilAttachmentFormat(const DeviceBase* device,
                                                    dawn::TextureFormat textureFormat) {
        DAWN_TRY(ValidateTextureFormat(textureFormat));
        const Format* format = nullptr;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(textureFormat));
        if (!format->HasDepthOrStencil() || !format->isRenderable) {
            return DAWN_VALIDATION_ERROR(
                "The depth stencil attachment texture format is not a renderable depth/stencil "
                "format");
        }
        return {};
    }

    MaybeError ValidateRenderBundleEncoderDescriptor(
        const DeviceBase* device,
        const RenderBundleEncoderDescriptor* descriptor) {
        if (!IsValidSampleCount(descriptor->sampleCount)) {
            return DAWN_VALIDATION_ERROR("Sample count is not supported");
        }

        if (descriptor->colorFormatsCount > kMaxColorAttachments) {
            return DAWN_VALIDATION_ERROR("Color formats count exceeds maximum");
        }

        if (descriptor->colorFormatsCount == 0 &&
            descriptor->depthStencilFormat == dawn::TextureFormat::None) {
            return DAWN_VALIDATION_ERROR("Should have at least one attachment format");
        }

        for (uint32_t i = 0; i < descriptor->colorFormatsCount; ++i) {
            DAWN_TRY(ValidateColorAttachmentFormat(device, descriptor->colorFormats[i]));
        }

        if (descriptor->depthStencilFormat != dawn::TextureFormat::None) {
            DAWN_TRY(ValidateDepthStencilAttachmentFormat(device, descriptor->depthStencilFormat));
        }

        return {};
    }

    RenderBundleEncoderBase::RenderBundleEncoderBase(
        DeviceBase* device,
        const RenderBundleEncoderDescriptor* descriptor)
        : RenderEncoderBase(device, &mEncodingContext),
          mEncodingContext(device, this),
          mAttachmentState(device->GetOrCreateAttachmentState(descriptor)) {
    }

    RenderBundleEncoderBase::RenderBundleEncoderBase(DeviceBase* device, ErrorTag errorTag)
        : RenderEncoderBase(device, &mEncodingContext, errorTag), mEncodingContext(device, this) {
    }

    // static
    RenderBundleEncoderBase* RenderBundleEncoderBase::MakeError(DeviceBase* device) {
        return new RenderBundleEncoderBase(device, ObjectBase::kError);
    }

    const AttachmentState* RenderBundleEncoderBase::GetAttachmentState() const {
        return mAttachmentState.Get();
    }

    CommandIterator RenderBundleEncoderBase::AcquireCommands() {
        return mEncodingContext.AcquireCommands();
    }

    RenderBundleBase* RenderBundleEncoderBase::Finish(const RenderBundleDescriptor* descriptor) {
        if (GetDevice()->ConsumedError(ValidateFinish(descriptor))) {
            return RenderBundleBase::MakeError(GetDevice());
        }
        ASSERT(!IsError());

        return new RenderBundleBase(this, descriptor, mAttachmentState.Get(),
                                    std::move(mResourceUsage));
    }

    MaybeError RenderBundleEncoderBase::ValidateFinish(const RenderBundleDescriptor* descriptor) {
        DAWN_TRY(GetDevice()->ValidateObject(this));

        // Even if Finish() validation fails, calling it will mutate the internal state of the
        // encoding context. Subsequent calls to encode commands will generate errors.
        DAWN_TRY(mEncodingContext.Finish());

        CommandIterator* commands = mEncodingContext.GetIterator();

        DAWN_TRY(ValidateRenderBundle(commands, mAttachmentState.Get(), &mResourceUsage));
        return {};
    }

}  // namespace dawn_native
