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

#include "dawn/native/ExternalTexture.h"

#include <utility>

#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Queue.h"
#include "dawn/native/Texture.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

MaybeError ValidateExternalTexturePlane(const TextureViewBase* textureView) {
    DAWN_INVALID_IF(
        (textureView->GetTexture()->GetUsage() & wgpu::TextureUsage::TextureBinding) == 0,
        "The external texture plane (%s) usage (%s) doesn't include the required usage (%s)",
        textureView, textureView->GetTexture()->GetUsage(), wgpu::TextureUsage::TextureBinding);

    DAWN_INVALID_IF(textureView->GetDimension() != wgpu::TextureViewDimension::e2D,
                    "The external texture plane (%s) dimension (%s) is not 2D.", textureView,
                    textureView->GetDimension());

    DAWN_INVALID_IF(textureView->GetLevelCount() > 1,
                    "The external texture plane (%s) mip level count (%u) is not 1.", textureView,
                    textureView->GetLevelCount());

    DAWN_INVALID_IF(textureView->GetTexture()->GetSampleCount() != 1,
                    "The external texture plane (%s) sample count (%u) is not one.", textureView,
                    textureView->GetTexture()->GetSampleCount());

    return {};
}

MaybeError ValidateExternalTextureDescriptor(const DeviceBase* device,
                                             const ExternalTextureDescriptor* descriptor) {
    ASSERT(descriptor);
    ASSERT(descriptor->plane0);

    DAWN_TRY(device->ValidateObject(descriptor->plane0));

    wgpu::TextureFormat plane0Format = descriptor->plane0->GetFormat().format;

    if (descriptor->plane1) {
        DAWN_INVALID_IF(descriptor->colorSpace != wgpu::PredefinedColorSpace::Srgb,
                        "The specified color space (%s) is not %s.", descriptor->colorSpace,
                        wgpu::PredefinedColorSpace::Srgb);

        DAWN_TRY(device->ValidateObject(descriptor->plane1));
        wgpu::TextureFormat plane1Format = descriptor->plane1->GetFormat().format;

        DAWN_INVALID_IF(plane0Format != wgpu::TextureFormat::R8Unorm,
                        "The bi-planar external texture plane (%s) format (%s) is not %s.",
                        descriptor->plane0, plane0Format, wgpu::TextureFormat::R8Unorm);
        DAWN_INVALID_IF(plane1Format != wgpu::TextureFormat::RG8Unorm,
                        "The bi-planar external texture plane (%s) format (%s) is not %s.",
                        descriptor->plane1, plane1Format, wgpu::TextureFormat::RG8Unorm);

        DAWN_TRY(ValidateExternalTexturePlane(descriptor->plane0));
        DAWN_TRY(ValidateExternalTexturePlane(descriptor->plane1));
    } else {
        switch (plane0Format) {
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::BGRA8Unorm:
            case wgpu::TextureFormat::RGBA16Float:
                DAWN_TRY(ValidateExternalTexturePlane(descriptor->plane0));
                break;
            default:
                return DAWN_FORMAT_VALIDATION_ERROR(
                    "The external texture plane (%s) format (%s) is not a supported format "
                    "(%s, %s, %s).",
                    descriptor->plane0, plane0Format, wgpu::TextureFormat::RGBA8Unorm,
                    wgpu::TextureFormat::BGRA8Unorm, wgpu::TextureFormat::RGBA16Float);
        }
    }

    return {};
}

// static
ResultOrError<Ref<ExternalTextureBase>> ExternalTextureBase::Create(
    DeviceBase* device,
    const ExternalTextureDescriptor* descriptor) {
    Ref<ExternalTextureBase> externalTexture =
        AcquireRef(new ExternalTextureBase(device, descriptor));
    DAWN_TRY(externalTexture->Initialize(device, descriptor));
    return std::move(externalTexture);
}

ExternalTextureBase::ExternalTextureBase(DeviceBase* device,
                                         const ExternalTextureDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label), mState(ExternalTextureState::Alive) {
    TrackInDevice();
}

ExternalTextureBase::ExternalTextureBase(DeviceBase* device)
    : ApiObjectBase(device, kLabelNotImplemented), mState(ExternalTextureState::Alive) {
    TrackInDevice();
}

ExternalTextureBase::ExternalTextureBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag) {}

ExternalTextureBase::~ExternalTextureBase() = default;

MaybeError ExternalTextureBase::Initialize(DeviceBase* device,
                                           const ExternalTextureDescriptor* descriptor) {
    // Store any passed in TextureViews associated with individual planes.
    mTextureViews[0] = descriptor->plane0;

    if (descriptor->plane1) {
        mTextureViews[1] = descriptor->plane1;
    } else {
        DAWN_TRY_ASSIGN(mTextureViews[1],
                        device->GetOrCreatePlaceholderTextureViewForExternalTexture());
    }

    // We must create a buffer to store parameters needed by a shader that operates on this
    // external texture.
    BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(ExternalTextureParams);
    bufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    bufferDesc.label = "Dawn_External_Texture_Params_Buffer";

    DAWN_TRY_ASSIGN(mParamsBuffer, device->CreateBuffer(&bufferDesc));

    // Dawn & Tint's YUV-to-RGB conversion implementation is a simple 3x4 matrix multiplication
    // using a standard conversion matrix. These matrices can be found in
    // chromium/src/third_party/skia/src/core/SkYUVMath.cpp
    ExternalTextureParams params;
    params.numPlanes = descriptor->plane1 == nullptr ? 1 : 2;

    // TODO(dawn:1082): Make this field configurable from outside of Dawn.
    // Conversion matrix for BT.709 limited range. Columns 1, 2 and 3 are copied
    // directly from the corresponding matrix in SkYUVMath.cpp. Column 4 is the range
    // bias (for RGB) found in column 5 of the same SkYUVMath.cpp matrix.
    params.yuvToRgbConversionMatrix = {1.164384f, 0.0f,       1.792741f,  -0.972945f,
                                       1.164384f, -0.213249f, -0.532909f, 0.301483f,
                                       1.164384f, 2.112402f,  0.0f,       -1.133402f};

    // TODO(dawn:1082): Make this field configurable from outside of Dawn.
    // Use an identity matrix when converting BT.709 to sRGB because they shared the
    // same primaries.
    params.gamutConversionMatrix = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
                                    0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    switch (descriptor->colorSpace) {
        case wgpu::PredefinedColorSpace::Undefined:
            // Undefined color space should eventually produce an error. For now, these
            // constants will effectively perform no gamma correction so tests can continue
            // passing.
            params.gammaDecodingParams = {1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0};
            params.gammaEncodingParams = {1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0};
            break;
        case wgpu::PredefinedColorSpace::Srgb:
            // TODO(dawn:1082): Make this field configurable from outside of Dawn.
            // These are the inverted parameters as specified by Rec. ITU-R BT.1886 for BT.709
            params.gammaDecodingParams = {2.2, 1.0 / 1.099, 0.099 / 1.099, 1 / 4.5, 0.081,
                                          0.0, 0.0};

            // Constants for sRGB transfer function pulled from
            // https://en.wikipedia.org/wiki/SRGB
            params.gammaEncodingParams = {
                1 / 2.4, 1.137119 /*1.055^2.4*/, 0.0, 12.92, 0.0031308, -0.055, 0.0};
            break;
    }

    DAWN_TRY(device->GetQueue()->WriteBuffer(mParamsBuffer.Get(), 0, &params,
                                             sizeof(ExternalTextureParams)));

    return {};
}

const std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat>& ExternalTextureBase::GetTextureViews()
    const {
    return mTextureViews;
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

BufferBase* ExternalTextureBase::GetParamsBuffer() const {
    return mParamsBuffer.Get();
}

ObjectType ExternalTextureBase::GetType() const {
    return ObjectType::ExternalTexture;
}

}  // namespace dawn::native
