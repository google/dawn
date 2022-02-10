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

#ifndef DAWNNATIVE_EXTERNALTEXTURE_H_
#define DAWNNATIVE_EXTERNALTEXTURE_H_

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/Subresource.h"

#include <array>

namespace dawn::native {

    class TextureViewBase;

    struct ExternalTextureParams {
        uint32_t numPlanes;
        float vr;
        float vg;
        float ub;
        float ug;
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

        MaybeError ValidateCanUseInSubmitNow() const;
        static ExternalTextureBase* MakeError(DeviceBase* device);

        void APIDestroy();

      protected:
        // Constructor used only for mocking and testing.
        ExternalTextureBase(DeviceBase* device);
        void DestroyImpl() override;

        ~ExternalTextureBase() override;

      private:
        ExternalTextureBase(DeviceBase* device, const ExternalTextureDescriptor* descriptor);

        enum class ExternalTextureState { Alive, Destroyed };
        ExternalTextureBase(DeviceBase* device, ObjectBase::ErrorTag tag);
        MaybeError Initialize(DeviceBase* device, const ExternalTextureDescriptor* descriptor);

        Ref<TextureBase> mDummyTexture;
        Ref<BufferBase> mParamsBuffer;
        std::array<Ref<TextureViewBase>, kMaxPlanesPerFormat> mTextureViews;

        ExternalTextureState mState;
    };
}  // namespace dawn::native

#endif  // DAWNNATIVE_EXTERNALTEXTURE_H_
