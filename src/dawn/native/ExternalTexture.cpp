// Copyright 2021 The Dawn & Tint Authors
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

#include "dawn/native/ExternalTexture.h"

#include <algorithm>
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
    DAWN_ASSERT(descriptor);
    DAWN_ASSERT(descriptor->plane0);

    DAWN_TRY(device->ValidateObject(descriptor->plane0));

    DAWN_INVALID_IF(!descriptor->gamutConversionMatrix,
                    "The gamut conversion matrix must be non-null.");

    DAWN_INVALID_IF(!descriptor->srcTransferFunctionParameters,
                    "The source transfer function parameters must be non-null.");

    DAWN_INVALID_IF(!descriptor->dstTransferFunctionParameters,
                    "The destination transfer function parameters must be non-null.");

    DAWN_TRY(ValidateExternalTexturePlane(descriptor->plane0));

    auto CheckPlaneFormat = [](const DeviceBase* device, const Format& format,
                               uint32_t requiredComponentCount) -> MaybeError {
        DAWN_INVALID_IF(format.aspects != Aspect::Color, "The format (%s) is not a color format.",
                        format.format);
        DAWN_INVALID_IF(!IsSubset(SampleTypeBit::Float,
                                  format.GetAspectInfo(Aspect::Color).supportedSampleTypes),
                        "The format (%s) is not filterable float.", format.format);
        DAWN_INVALID_IF(format.componentCount != requiredComponentCount,
                        "The format (%s) component count (%u) is not %u.", format.format,
                        requiredComponentCount, format.componentCount);
        return {};
    };

    if (descriptor->plane1) {
        DAWN_INVALID_IF(
            !descriptor->yuvToRgbConversionMatrix,
            "When more than one plane is set, the YUV-to-RGB conversion matrix must be non-null.");

        DAWN_TRY(device->ValidateObject(descriptor->plane1));
        DAWN_TRY(ValidateExternalTexturePlane(descriptor->plane1));

        // Y + UV case.
        DAWN_TRY_CONTEXT(CheckPlaneFormat(device, descriptor->plane0->GetFormat(), 1),
                         "validating the format of plane 0 (%s)", descriptor->plane0);
        DAWN_TRY_CONTEXT(CheckPlaneFormat(device, descriptor->plane1->GetFormat(), 2),
                         "validating the format of plane 1 (%s)", descriptor->plane1);
    } else {
        // RGBA case.
        DAWN_TRY_CONTEXT(CheckPlaneFormat(device, descriptor->plane0->GetFormat(), 4),
                         "validating the format of plane 0 (%s)", descriptor->plane0);
    }

    DAWN_INVALID_IF(descriptor->visibleSize.width == 0 || descriptor->visibleSize.height == 0,
                    "VisibleSize %s have 0 on width or height.", &descriptor->visibleSize);

    const Extent3D textureSize = descriptor->plane0->GetSingleSubresourceVirtualSize();
    DAWN_INVALID_IF(descriptor->visibleSize.width > textureSize.width ||
                        descriptor->visibleSize.height > textureSize.height,
                    "VisibleSize %s is exceed the texture size, defined by Plane0 size (%u, %u).",
                    &descriptor->visibleSize, textureSize.width, textureSize.height);
    DAWN_INVALID_IF(
        descriptor->visibleOrigin.x > textureSize.width - descriptor->visibleSize.width ||
            descriptor->visibleOrigin.y > textureSize.height - descriptor->visibleSize.height,
        "VisibleRect[Origin: %s, Size: %s] is exceed the texture size, defined by "
        "Plane0 size (%u, %u).",
        &descriptor->visibleOrigin, &descriptor->visibleSize, textureSize.width,
        textureSize.height);

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
    : ApiObjectBase(device, descriptor->label),
      mVisibleOrigin(descriptor->visibleOrigin),
      mVisibleSize(descriptor->visibleSize),
      mState(ExternalTextureState::Active) {
    GetObjectTrackingList()->Track(this);
}

// Error external texture cannot be used in bind group.
ExternalTextureBase::ExternalTextureBase(DeviceBase* device,
                                         ObjectBase::ErrorTag tag,
                                         const char* label)
    : ApiObjectBase(device, tag, label), mState(ExternalTextureState::Destroyed) {}

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

    ExternalTextureParams params;
    params.numPlanes = descriptor->plane1 == nullptr ? 1 : 2;

    params.doYuvToRgbConversionOnly = descriptor->doYuvToRgbConversionOnly ? 1 : 0;

    // YUV-to-RGB conversion is performed by multiplying the source YUV values with a 4x3 matrix
    // passed from Chromium. The matrix was originally sourced from /skia/src/core/SkYUVMath.cpp.
    // This matrix is only used in multiplanar scenarios.
    if (params.numPlanes == 2) {
        DAWN_ASSERT(descriptor->yuvToRgbConversionMatrix);
        const float* yMat = descriptor->yuvToRgbConversionMatrix;
        std::copy(yMat, yMat + 12, params.yuvToRgbConversionMatrix.begin());
    }

    // Gamut correction is performed by multiplying a 3x3 matrix passed from Chromium. The
    // matrix was computed by multiplying the appropriate source and destination gamut
    // matrices sourced from ui/gfx/color_space.cc.
    const float* gMat = descriptor->gamutConversionMatrix;
    params.gamutConversionMatrix = {gMat[0], gMat[1], gMat[2], 0.0f,  //
                                    gMat[3], gMat[4], gMat[5], 0.0f,  //
                                    gMat[6], gMat[7], gMat[8], 0.0f};

    // Gamma decode/encode is performed by the logic:
    //    if (abs(v) < params.D) {
    //        return sign(v) * (params.C * abs(v) + params.F);
    //    }
    //    return pow(A * x + B, G) + E
    //
    // Constants are passed from Chromium and originally sourced from ui/gfx/color_space.cc
    const float* srcFn = descriptor->srcTransferFunctionParameters;
    std::copy(srcFn, srcFn + 7, params.gammaDecodingParams.begin());

    const float* dstFn = descriptor->dstTransferFunctionParameters;
    std::copy(dstFn, dstFn + 7, params.gammaEncodingParams.begin());

    // Unlike WGSL, which stores matrices in column vectors, the following arithmetic uses row
    // vectors, so elements are stored in the following order:
    // ┌         ┐
    // │ 0, 1, 2 │
    // │ 3, 4, 5 │
    // └         ┘
    // The matrix is transposed at the end.
    using mat2x3 = std::array<float, 6>;

    // Multiplies the two mat2x3 matrices, by treating the RHS matrix as a mat3x3 where the last row
    // is [0, 0, 1].
    auto Mul = [&](const mat2x3& lhs, const mat2x3& rhs) {
        auto& a = lhs[0];
        auto& b = lhs[1];
        auto& c = lhs[2];
        auto& d = lhs[3];
        auto& e = lhs[4];
        auto& f = lhs[5];
        auto& g = rhs[0];
        auto& h = rhs[1];
        auto& i = rhs[2];
        auto& j = rhs[3];
        auto& k = rhs[4];
        auto& l = rhs[5];
        // ┌         ┐   ┌         ┐
        // │ a, b, c │   │ g, h, i │
        // │ d, e, f │ x │ j, k, l │
        // └         ┘   │ 0, 0, 1 │
        //               └         ┘
        return mat2x3{
            a * g + b * j,      //
            a * h + b * k,      //
            a * i + b * l + c,  //
            d * g + e * j,      //
            d * h + e * k,      //
            d * i + e * l + f,  //
        };
    };

    auto Scale = [&](const mat2x3& m, float x, float y) {
        return Mul(mat2x3{x, 0, 0, 0, y, 0}, m);
    };

    auto Translate = [&](const mat2x3& m, float x, float y) {
        return Mul(mat2x3{1, 0, x, 0, 1, y}, m);
    };

    mat2x3 coordTransformMatrix = {
        1, 0, 0,  //
        0, 1, 0,  //
    };

    // Offset the coordinates so the center texel is at the origin, so we can apply rotations and
    // y-flips. After translation, coordinates range from [-0.5 .. +0.5] in both U and V.
    coordTransformMatrix = Translate(coordTransformMatrix, -0.5, -0.5);

    // If the texture needs flipping, mirror in Y.
    if (descriptor->flipY) {
        coordTransformMatrix = Scale(coordTransformMatrix, 1, -1);
    }

    // Apply rotations as needed.
    switch (descriptor->rotation) {
        case wgpu::ExternalTextureRotation::Rotate0Degrees:
            break;
        case wgpu::ExternalTextureRotation::Rotate90Degrees:
            coordTransformMatrix = Mul(mat2x3{0, -1, 0,   // x' = -y
                                              +1, 0, 0},  // y' = x
                                       coordTransformMatrix);
            break;
        case wgpu::ExternalTextureRotation::Rotate180Degrees:
            coordTransformMatrix = Mul(mat2x3{-1, 0, 0,   // x' = -x
                                              0, -1, 0},  // y' = -y
                                       coordTransformMatrix);
            break;
        case wgpu::ExternalTextureRotation::Rotate270Degrees:
            coordTransformMatrix = Mul(mat2x3{0, +1, 0,   // x' = y
                                              -1, 0, 0},  // y' = -x
                                       coordTransformMatrix);
            break;
    }

    // Offset the coordinates so the bottom-left texel is at origin.
    // After translation, coordinates range from [0 .. 1] in both U and V.
    coordTransformMatrix = Translate(coordTransformMatrix, 0.5, 0.5);

    // Calculate scale factors and offsets from the specified visibleSize.
    DAWN_ASSERT(descriptor->visibleSize.width > 0);
    DAWN_ASSERT(descriptor->visibleSize.height > 0);
    uint32_t frameWidth = descriptor->plane0->GetSingleSubresourceVirtualSize().width;
    uint32_t frameHeight = descriptor->plane0->GetSingleSubresourceVirtualSize().height;
    float xScale =
        static_cast<float>(descriptor->visibleSize.width) / static_cast<float>(frameWidth);
    float yScale =
        static_cast<float>(descriptor->visibleSize.height) / static_cast<float>(frameHeight);
    float xOffset =
        static_cast<float>(descriptor->visibleOrigin.x) / static_cast<float>(frameWidth);
    float yOffset =
        static_cast<float>(descriptor->visibleOrigin.y) / static_cast<float>(frameHeight);

    // Finally, scale and translate based on the visible rect. This applies cropping.
    coordTransformMatrix = Scale(coordTransformMatrix, xScale, yScale);
    coordTransformMatrix = Translate(coordTransformMatrix, xOffset, yOffset);

    // Transpose the mat2x3 into column vectors for use by WGSL.
    params.coordTransformMatrix[0] = coordTransformMatrix[0];
    params.coordTransformMatrix[1] = coordTransformMatrix[3];
    params.coordTransformMatrix[2] = coordTransformMatrix[1];
    params.coordTransformMatrix[3] = coordTransformMatrix[4];
    params.coordTransformMatrix[4] = coordTransformMatrix[2];
    params.coordTransformMatrix[5] = coordTransformMatrix[5];

    DAWN_TRY(device->GetQueue()->WriteBuffer(mParamsBuffer.Get(), 0, &params,
                                             sizeof(ExternalTextureParams)));

    return {};
}

const std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat>& ExternalTextureBase::GetTextureViews()
    const {
    return mTextureViews;
}

MaybeError ExternalTextureBase::ValidateCanUseInSubmitNow() const {
    DAWN_ASSERT(!IsError());
    DAWN_INVALID_IF(mState != ExternalTextureState::Active,
                    "External texture %s used in a submit is not active.", this);

    for (uint32_t i = 0; i < kMaxPlanesPerFormat; ++i) {
        if (mTextureViews[i] != nullptr) {
            DAWN_TRY_CONTEXT(mTextureViews[i]->GetTexture()->ValidateCanUseInSubmitNow(),
                             "Validate plane %u of %s can be used in a submit.", i, this);
        }
    }
    return {};
}

MaybeError ExternalTextureBase::ValidateRefresh() {
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_INVALID_IF(mState == ExternalTextureState::Destroyed, "%s is destroyed.", this);
    return {};
}

MaybeError ExternalTextureBase::ValidateExpire() {
    DAWN_TRY(GetDevice()->ValidateObject(this));
    DAWN_INVALID_IF(mState != ExternalTextureState::Active, "%s is not active.", this);
    return {};
}

void ExternalTextureBase::APIRefresh() {
    if (GetDevice()->ConsumedError(ValidateRefresh(), "calling %s.Refresh()", this)) {
        return;
    }
    mState = ExternalTextureState::Active;
}

void ExternalTextureBase::APIExpire() {
    if (GetDevice()->ConsumedError(ValidateExpire(), "calling %s.Expire()", this)) {
        return;
    }
    mState = ExternalTextureState::Expired;
}

void ExternalTextureBase::APIDestroy() {
    Destroy();
}

void ExternalTextureBase::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the texture is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the texture.
    // - It may be called when the last ref to the texture is dropped and the texture
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the texture since there are no other live refs.
    mState = ExternalTextureState::Destroyed;
}

// static
ExternalTextureBase* ExternalTextureBase::MakeError(DeviceBase* device, const char* label) {
    return new ExternalTextureBase(device, ObjectBase::kError, label);
}

BufferBase* ExternalTextureBase::GetParamsBuffer() const {
    return mParamsBuffer.Get();
}

ObjectType ExternalTextureBase::GetType() const {
    return ObjectType::ExternalTexture;
}

const Extent2D& ExternalTextureBase::GetVisibleSize() const {
    DAWN_ASSERT(!IsError());
    return mVisibleSize;
}

const Origin2D& ExternalTextureBase::GetVisibleOrigin() const {
    DAWN_ASSERT(!IsError());
    return mVisibleOrigin;
}

}  // namespace dawn::native
