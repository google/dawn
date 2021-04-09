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

        if ((textureView->GetTexture()->GetUsage() & wgpu::TextureUsage::Sampled) !=
            wgpu::TextureUsage::Sampled) {
            return DAWN_VALIDATION_ERROR(
                "The external texture descriptor specifies a texture that was not created with "
                "TextureUsage::Sampled.");
        }

        if (textureView->GetDimension() != wgpu::TextureViewDimension::e2D) {
            return DAWN_VALIDATION_ERROR(
                "The external texture descriptor contains a texture view with a non-2D dimension.");
        }

        if (textureView->GetLevelCount() > 1) {
            return DAWN_VALIDATION_ERROR(
                "The external texture descriptor contains a texture view with a level count "
                "greater than 1.");
        }

        return {};
    }

    MaybeError ValidateExternalTextureDescriptor(const DeviceBase* device,
                                                 const ExternalTextureDescriptor* descriptor) {
        ASSERT(descriptor);
        ASSERT(descriptor->plane0);

        DAWN_TRY(device->ValidateObject(descriptor->plane0));

        const Format* format;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(descriptor->format));

        switch (descriptor->format) {
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::BGRA8Unorm:
            case wgpu::TextureFormat::RGBA16Float:
                DAWN_TRY(ValidateExternalTexturePlane(descriptor->plane0, descriptor->format));
                break;
            default:
                return DAWN_VALIDATION_ERROR(
                    "The external texture descriptor specifies an unsupported format.");
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
        : ObjectBase(device) {
        textureViews[0] = descriptor->plane0;
    }

    ExternalTextureBase::ExternalTextureBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ObjectBase(device, tag) {
    }

    std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat> ExternalTextureBase::GetTextureViews()
        const {
        return textureViews;
    }

    void ExternalTextureBase::APIDestroy() {
        if (GetDevice()->ConsumedError(GetDevice()->ValidateObject(this))) {
            return;
        }
        ASSERT(!IsError());
    }

    // static
    ExternalTextureBase* ExternalTextureBase::MakeError(DeviceBase* device) {
        return new ExternalTextureBase(device, ObjectBase::kError);
    }

}  // namespace dawn_native