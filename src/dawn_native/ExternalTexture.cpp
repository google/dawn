// Copyright 2021 The Dawn Authors
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

#include "dawn_native/ExternalTexture.h"

#include "dawn_native/Device.h"
#include "dawn_native/ObjectType_autogen.h"
#include "dawn_native/Texture.h"

#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    MaybeError ValidateExternalTexturePlane(const TextureViewBase* textureView,
                                            wgpu::TextureFormat format) {
        if (textureView->GetFormat().format != format) {
            return DAWN_VALIDATION_ERROR(
                "The external texture descriptor specifies a texture format that is different from "
                "at least one of the passed texture views.");
        }

        DAWN_INVALID_IF(
            (textureView->GetTexture()->GetUsage() & wgpu::TextureUsage::TextureBinding) == 0,
            "The external texture plane (%s) usage (%s) doesn't include the required usage (%s)",
            textureView, textureView->GetTexture()->GetUsage(), wgpu::TextureUsage::TextureBinding);

        DAWN_INVALID_IF(textureView->GetDimension() != wgpu::TextureViewDimension::e2D,
                        "The external texture plane (%s) dimension (%s) is not 2D.", textureView,
                        textureView->GetDimension());

        DAWN_INVALID_IF(textureView->GetLevelCount() > 1,
                        "The external texture plane (%s) mip level count (%u) is not 1.",
                        textureView, textureView->GetLevelCount());

        DAWN_INVALID_IF(textureView->GetTexture()->GetSampleCount() != 1,
                        "The external texture plane (%s) sample count (%u) is not one.",
                        textureView, textureView->GetTexture()->GetSampleCount());

        return {};
    }

    MaybeError ValidateExternalTextureDescriptor(const DeviceBase* device,
                                                 const ExternalTextureDescriptor* descriptor) {
        ASSERT(descriptor);
        ASSERT(descriptor->plane0);

        DAWN_TRY(device->ValidateObject(descriptor->plane0));

        const Format* format;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->format));
        DAWN_UNUSED(format);

        switch (descriptor->format) {
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::BGRA8Unorm:
            case wgpu::TextureFormat::RGBA16Float:
                DAWN_TRY_CONTEXT(
                    ValidateExternalTexturePlane(descriptor->plane0, descriptor->format),
                    "validating plane0 against the external texture format (%s)",
                    descriptor->format);
                break;
            default:
                return DAWN_FORMAT_VALIDATION_ERROR(
                    "Format (%s) is not a supported external texture format.", descriptor->format);
        }

        return {};
    }

    // static
    ResultOrError<Ref<ExternalTextureBase>> ExternalTextureBase::Create(
        DeviceBase* device,
        const ExternalTextureDescriptor* descriptor) {
        Ref<ExternalTextureBase> externalTexture =
            AcquireRef(new ExternalTextureBase(device, descriptor));
        return std::move(externalTexture);
    }

    ExternalTextureBase::ExternalTextureBase(DeviceBase* device,
                                             const ExternalTextureDescriptor* descriptor)
        : ApiObjectBase(device, descriptor->label), mState(ExternalTextureState::Alive) {
        textureViews[0] = descriptor->plane0;
        TrackInDevice();
    }

    ExternalTextureBase::ExternalTextureBase(DeviceBase* device)
        : ApiObjectBase(device, kLabelNotImplemented), mState(ExternalTextureState::Alive) {
        TrackInDevice();
    }

    ExternalTextureBase::ExternalTextureBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ApiObjectBase(device, tag) {
    }

    const std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat>&
    ExternalTextureBase::GetTextureViews() const {
        return textureViews;
    }

    MaybeError ExternalTextureBase::ValidateCanUseInSubmitNow() const {
        ASSERT(!IsError());
        DAWN_INVALID_IF(mState == ExternalTextureState::Destroyed,
                        "Destroyed external texture %s is used in a submit.", this);
        return {};
    }

    void ExternalTextureBase::APIDestroy() {
        if (GetDevice()->ConsumedError(GetDevice()->ValidateObject(this))) {
            return;
        }
        Destroy();
    }

    void ExternalTextureBase::DestroyImpl() {
        mState = ExternalTextureState::Destroyed;
    }

    // static
    ExternalTextureBase* ExternalTextureBase::MakeError(DeviceBase* device) {
        return new ExternalTextureBase(device, ObjectBase::kError);
    }

    ObjectType ExternalTextureBase::GetType() const {
        return ObjectType::ExternalTexture;
    }

}  // namespace dawn_native
