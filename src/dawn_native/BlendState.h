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

#ifndef DAWNNATIVE_BLENDSTATE_H_
#define DAWNNATIVE_BLENDSTATE_H_

#include "dawn_native/Builder.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    class BlendStateBase : public ObjectBase {
      public:
        BlendStateBase(BlendStateBuilder* builder);

        struct BlendInfo {
            struct BlendOpFactor {
                dawn::BlendOperation operation = dawn::BlendOperation::Add;
                dawn::BlendFactor srcFactor = dawn::BlendFactor::One;
                dawn::BlendFactor dstFactor = dawn::BlendFactor::Zero;
            };

            bool blendEnabled = false;
            BlendOpFactor alphaBlend;
            BlendOpFactor colorBlend;
            dawn::ColorWriteMask colorWriteMask = dawn::ColorWriteMask::All;
        };

        const BlendInfo& GetBlendInfo() const;

      private:
        BlendInfo mBlendInfo;
    };

    class BlendStateBuilder : public Builder<BlendStateBase> {
      public:
        BlendStateBuilder(DeviceBase* device);

        // Dawn API
        void SetBlendEnabled(bool blendEnabled);
        void SetAlphaBlend(dawn::BlendOperation blendOperation,
                           dawn::BlendFactor srcFactor,
                           dawn::BlendFactor dstFactor);
        void SetColorBlend(dawn::BlendOperation blendOperation,
                           dawn::BlendFactor srcFactor,
                           dawn::BlendFactor dstFactor);
        void SetColorWriteMask(dawn::ColorWriteMask colorWriteMask);

      private:
        friend class BlendStateBase;

        BlendStateBase* GetResultImpl() override;

        int mPropertiesSet = 0;

        BlendStateBase::BlendInfo mBlendInfo;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_BLENDSTATE_H_
