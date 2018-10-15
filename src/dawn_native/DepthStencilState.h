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

#ifndef DAWNNATIVE_DEPTHSTENCILSTATE_H_
#define DAWNNATIVE_DEPTHSTENCILSTATE_H_

#include "dawn_native/Builder.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn_native/dawn_platform.h"

namespace dawn_native {

    class DepthStencilStateBase : public ObjectBase {
      public:
        DepthStencilStateBase(DepthStencilStateBuilder* builder);

        struct DepthInfo {
            dawn::CompareFunction compareFunction = dawn::CompareFunction::Always;
            bool depthWriteEnabled = false;
        };

        struct StencilFaceInfo {
            dawn::CompareFunction compareFunction = dawn::CompareFunction::Always;
            dawn::StencilOperation stencilFail = dawn::StencilOperation::Keep;
            dawn::StencilOperation depthFail = dawn::StencilOperation::Keep;
            dawn::StencilOperation depthStencilPass = dawn::StencilOperation::Keep;
        };

        struct StencilInfo {
            StencilFaceInfo back;
            StencilFaceInfo front;
            uint32_t readMask = 0xff;
            uint32_t writeMask = 0xff;
        };

        bool StencilTestEnabled() const;
        const DepthInfo& GetDepth() const;
        const StencilInfo& GetStencil() const;

      private:
        DepthInfo mDepthInfo;
        StencilInfo mStencilInfo;
    };

    class DepthStencilStateBuilder : public Builder<DepthStencilStateBase> {
      public:
        DepthStencilStateBuilder(DeviceBase* device);

        // Dawn API
        void SetDepthCompareFunction(dawn::CompareFunction depthCompareFunction);
        void SetDepthWriteEnabled(bool enabled);
        void SetStencilFunction(dawn::Face face,
                                dawn::CompareFunction stencilCompareFunction,
                                dawn::StencilOperation stencilFail,
                                dawn::StencilOperation depthFail,
                                dawn::StencilOperation depthStencilPass);
        void SetStencilMask(uint32_t readMask, uint32_t writeMask);

      private:
        friend class DepthStencilStateBase;

        DepthStencilStateBase* GetResultImpl() override;

        int mPropertiesSet = 0;

        DepthStencilStateBase::DepthInfo mDepthInfo;
        DepthStencilStateBase::StencilInfo mStencilInfo;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_DEPTHSTENCILSTATE_H_
