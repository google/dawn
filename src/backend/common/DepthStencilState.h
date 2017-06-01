// Copyright 2017 The NXT Authors
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

#ifndef BACKEND_COMMON_DEPTHSTENCILSTATE_H_
#define BACKEND_COMMON_DEPTHSTENCILSTATE_H_

#include "Forward.h"
#include "Builder.h"
#include "RefCounted.h"

#include "nxt/nxtcpp.h"

namespace backend {


    class DepthStencilStateBase : public RefCounted {
        public:
            DepthStencilStateBase(DepthStencilStateBuilder* builder);

            struct DepthInfo {
                nxt::CompareFunction compareFunction = nxt::CompareFunction::Always;
                bool depthWriteEnabled = false;
            };

            struct StencilFaceInfo {
                nxt::CompareFunction compareFunction = nxt::CompareFunction::Always;
                nxt::StencilOperation stencilFail = nxt::StencilOperation::Keep;
                nxt::StencilOperation depthFail = nxt::StencilOperation::Keep;
                nxt::StencilOperation depthStencilPass = nxt::StencilOperation::Keep;
                uint32_t mask = 0xff;
            };

            struct StencilInfo {
                bool stencilTestEnabled;
                StencilFaceInfo back;
                StencilFaceInfo front;
            };

            bool DepthTestEnabled() const;
            bool StencilTestEnabled() const;
            const DepthInfo& GetDepth() const;
            const StencilInfo& GetStencil() const;

        private:
            DepthInfo depthInfo;
            StencilInfo stencilInfo;
    };

    class DepthStencilStateBuilder : public Builder<DepthStencilStateBase> {
        public:
            DepthStencilStateBuilder(DeviceBase* device);

            // NXT API
            void SetDepthCompareFunction(nxt::CompareFunction depthCompareFunction);
            void SetDepthWriteEnabled(bool enabled);
            void SetStencilFunction(nxt::Face face, nxt::CompareFunction stencilCompareFunction,
                nxt::StencilOperation stencilFail, nxt::StencilOperation depthFail, nxt::StencilOperation depthStencilPass);
            void SetStencilMask(nxt::Face face, uint32_t mask);

        private:
            friend class DepthStencilStateBase;

            DepthStencilStateBase* GetResultImpl() override;

            int propertiesSet = 0;

            DepthStencilStateBase::DepthInfo depthInfo;
            DepthStencilStateBase::StencilInfo stencilInfo;
    };

}

#endif // BACKEND_COMMON_DEPTHSTENCILSTATE_H_
