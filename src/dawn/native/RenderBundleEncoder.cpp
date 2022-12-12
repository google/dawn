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

#include "dawn/native/RenderBundleEncoder.h"

#include <utility>

#include "dawn/common/StackContainer.h"
#include "dawn/native/CommandValidation.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/Format.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/ValidationUtils_autogen.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native {

MaybeError ValidateColorAttachmentFormat(const DeviceBase* device,
                                         wgpu::TextureFormat textureFormat) {
    DAWN_TRY(ValidateTextureFormat(textureFormat));
    const Format* format = nullptr;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(textureFormat));
    DAWN_INVALID_IF(!format->IsColor() || !format->isRenderable,
                    "Texture format %s is not color renderable.", textureFormat);
    return {};
}

MaybeError ValidateDepthStencilAttachmentFormat(const DeviceBase* device,
                                                wgpu::TextureFormat textureFormat,
                                                bool depthReadOnly,
                                                bool stencilReadOnly) {
    DAWN_TRY(ValidateTextureFormat(textureFormat));
    const Format* format = nullptr;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(textureFormat));
    DAWN_INVALID_IF(!format->HasDepthOrStencil() || !format->isRenderable,
                    "Texture format %s is not depth/stencil renderable.", textureFormat);

    DAWN_INVALID_IF(
        format->HasDepth() && format->HasStencil() && depthReadOnly != stencilReadOnly,
        "depthReadOnly (%u) and stencilReadOnly (%u) must be the same when format %s has "
        "both depth and stencil aspects.",
        depthReadOnly, stencilReadOnly, textureFormat);

    return {};
}

MaybeError ValidateRenderBundleEncoderDescriptor(DeviceBase* device,
                                                 const RenderBundleEncoderDescriptor* descriptor) {
    DAWN_INVALID_IF(!IsValidSampleCount(descriptor->sampleCount),
                    "Sample count (%u) is not supported.", descriptor->sampleCount);

    uint32_t maxColorAttachments = device->GetLimits().v1.maxColorAttachments;
    DAWN_INVALID_IF(descriptor->colorFormatsCount > maxColorAttachments,
                    "Color formats count (%u) exceeds maximum number of color attachements (%u).",
                    descriptor->colorFormatsCount, maxColorAttachments);

    bool allColorFormatsUndefined = true;
    ColorAttachmentFormats colorAttachmentFormats;
    for (uint32_t i = 0; i < descriptor->colorFormatsCount; ++i) {
        wgpu::TextureFormat format = descriptor->colorFormats[i];
        if (format != wgpu::TextureFormat::Undefined) {
            DAWN_TRY_CONTEXT(ValidateColorAttachmentFormat(device, format),
                             "validating colorFormats[%u]", i);
            colorAttachmentFormats->push_back(&device->GetValidInternalFormat(format));
            allColorFormatsUndefined = false;
        }
    }
    DAWN_TRY_CONTEXT(ValidateColorAttachmentBytesPerSample(device, colorAttachmentFormats),
                     "validating color attachment bytes per sample.");

    if (descriptor->depthStencilFormat != wgpu::TextureFormat::Undefined) {
        DAWN_TRY_CONTEXT(ValidateDepthStencilAttachmentFormat(
                             device, descriptor->depthStencilFormat, descriptor->depthReadOnly,
                             descriptor->stencilReadOnly),
                         "validating depthStencilFormat");
    } else {
        DAWN_INVALID_IF(
            allColorFormatsUndefined,
            "No color or depthStencil attachments specified. At least one is required.");
    }

    return {};
}

RenderBundleEncoder::RenderBundleEncoder(DeviceBase* device,
                                         const RenderBundleEncoderDescriptor* descriptor)
    : RenderEncoderBase(device,
                        descriptor->label,
                        &mBundleEncodingContext,
                        device->GetOrCreateAttachmentState(descriptor),
                        descriptor->depthReadOnly,
                        descriptor->stencilReadOnly),
      mBundleEncodingContext(device, this) {
    GetObjectTrackingList()->Track(this);
}

RenderBundleEncoder::RenderBundleEncoder(DeviceBase* device, ErrorTag errorTag)
    : RenderEncoderBase(device, &mBundleEncodingContext, errorTag),
      mBundleEncodingContext(device, this) {}

void RenderBundleEncoder::DestroyImpl() {
    RenderEncoderBase::DestroyImpl();
    mBundleEncodingContext.Destroy();
}

// static
Ref<RenderBundleEncoder> RenderBundleEncoder::Create(
    DeviceBase* device,
    const RenderBundleEncoderDescriptor* descriptor) {
    return AcquireRef(new RenderBundleEncoder(device, descriptor));
}

// static
RenderBundleEncoder* RenderBundleEncoder::MakeError(DeviceBase* device) {
    return new RenderBundleEncoder(device, ObjectBase::kError);
}

ObjectType RenderBundleEncoder::GetType() const {
    return ObjectType::RenderBundleEncoder;
}

CommandIterator RenderBundleEncoder::AcquireCommands() {
    return mBundleEncodingContext.AcquireCommands();
}

RenderBundleBase* RenderBundleEncoder::APIFinish(const RenderBundleDescriptor* descriptor) {
    RenderBundleBase* result = nullptr;

    if (GetDevice()->ConsumedError(FinishImpl(descriptor), &result, "calling %s.Finish(%s).", this,
                                   descriptor)) {
        return RenderBundleBase::MakeError(GetDevice());
    }

    return result;
}

ResultOrError<RenderBundleBase*> RenderBundleEncoder::FinishImpl(
    const RenderBundleDescriptor* descriptor) {
    // Even if mBundleEncodingContext.Finish() validation fails, calling it will mutate the
    // internal state of the encoding context. Subsequent calls to encode commands will generate
    // errors.
    DAWN_TRY(mBundleEncodingContext.Finish());

    RenderPassResourceUsage usages = mUsageTracker.AcquireResourceUsage();
    if (IsValidationEnabled()) {
        DAWN_TRY(GetDevice()->ValidateObject(this));
        DAWN_TRY(ValidateProgrammableEncoderEnd());
        DAWN_TRY(ValidateFinish(usages));
    }

    return new RenderBundleBase(this, descriptor, AcquireAttachmentState(), IsDepthReadOnly(),
                                IsStencilReadOnly(), std::move(usages),
                                std::move(mIndirectDrawMetadata));
}

MaybeError RenderBundleEncoder::ValidateFinish(const RenderPassResourceUsage& usages) const {
    TRACE_EVENT0(GetDevice()->GetPlatform(), Validation, "RenderBundleEncoder::ValidateFinish");
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_TRY(ValidateSyncScopeResourceUsage(usages));
    return {};
}

}  // namespace dawn::native
