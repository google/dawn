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

#ifndef SRC_DAWN_NATIVE_EXTERNALTEXTURE_H_
#define SRC_DAWN_NATIVE_EXTERNALTEXTURE_H_

#include <array>

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/Subresource.h"

namespace dawn::native {

class TextureViewBase;

struct ExternalTextureParams {
    uint32_t numPlanes;
    // TODO(crbug.com/dawn/1466): Only go as few steps as necessary.
    uint32_t doYuvToRgbConversionOnly;
    std::array<uint32_t, 2> padding;
    std::array<float, 12> yuvToRgbConversionMatrix;
    std::array<float, 8> gammaDecodingParams = {};
    std::array<float, 8> gammaEncodingParams = {};
    std::array<float, 12> gamutConversionMatrix = {};
    std::array<float, 6> coordTransformMatrix = {};
};

MaybeError ValidateExternalTextureDescriptor(const DeviceBase* device,
                                             const ExternalTextureDescriptor* descriptor);

class ExternalTextureBase : public ApiObjectBase {
  public:
    static ResultOrError<Ref<ExternalTextureBase>> Create(
        DeviceBase* device,
        const ExternalTextureDescriptor* descriptor);

    BufferBase* GetParamsBuffer() const;
    const std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat>& GetTextureViews() const;
    ObjectType GetType() const override;

    const Extent2D& GetVisibleSize() const;
    const Origin2D& GetVisibleOrigin() const;

    MaybeError ValidateCanUseInSubmitNow() const;
    static ExternalTextureBase* MakeError(DeviceBase* device);

    void APIDestroy();

  protected:
    ExternalTextureBase(DeviceBase* device, const ExternalTextureDescriptor* descriptor);
    void DestroyImpl() override;

    MaybeError Initialize(DeviceBase* device, const ExternalTextureDescriptor* descriptor);

    ~ExternalTextureBase() override;

  private:
    enum class ExternalTextureState { Alive, Destroyed };
    ExternalTextureBase(DeviceBase* device, ObjectBase::ErrorTag tag);

    Ref<TextureBase> mPlaceholderTexture;
    Ref<BufferBase> mParamsBuffer;
    std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat> mTextureViews;

    // TODO(dawn:1082) Use the visible size and origin in the external texture shader
    // code to sample video content.
    Origin2D mVisibleOrigin;
    Extent2D mVisibleSize;

    ExternalTextureState mState;
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_EXTERNALTEXTURE_H_
