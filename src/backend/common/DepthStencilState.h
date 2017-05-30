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
                nxt::CompareFunction compareFunction = nxt::CompareFunction::Less;
                nxt::DepthWriteMode depthWriteMode = nxt::DepthWriteMode::Enabled;
            };

            struct StencilInfo {
                nxt::CompareFunction compareFunction = nxt::CompareFunction::Always;
				nxt::StencilOperation stencilFail = nxt::StencilOperation::Keep;
				nxt::StencilOperation depthFail = nxt::StencilOperation::Keep;
				nxt::StencilOperation stencilPass = nxt::StencilOperation::Keep;
                uint32_t readMask = 0xff;
                uint32_t writeMask = 0xff;
            };

			bool DepthIsEnabled() const;
			bool StencilIsEnabled() const;
            const DepthInfo& GetDepth() const;
            const StencilInfo& GetStencil(nxt::Face face) const;

        private:
			bool depthEnabled = false;
			bool stencilEnabled = false;
            DepthInfo depthInfo;
            StencilInfo stencilInfos[2];
    };

    class DepthStencilStateBuilder : public Builder<DepthStencilStateBase> {
        public:
            DepthStencilStateBuilder(DeviceBase* device);

            // NXT API
			void SetDepthEnabled(bool depthEnabled);
			void SetDepthCompareFunction(nxt::CompareFunction depthCompareFunction);
            void SetDepthWrite(nxt::DepthWriteMode depthWriteMode);
			void SetStencilEnabled(bool stencilEnabled);
            void SetStencilOperation(nxt::Face face, nxt::StencilOperation stencilFail, 
                    nxt::StencilOperation depthFail, nxt::StencilOperation stencilPass);
            void SetStencilCompareFunction(nxt::Face face, nxt::CompareFunction stencilCompareFunction);
            void SetStencilMask(nxt::Face face, uint32_t readMask, uint32_t writeMask);

        private:
            friend class DepthStencilStateBase;

            DepthStencilStateBase* GetResultImpl() override;

			bool depthEnabled;
			bool stencilEnabled;
            DepthStencilStateBase::DepthInfo depthInfo;
            DepthStencilStateBase::StencilInfo stencilInfos[2];
    };

}

#endif // BACKEND_COMMON_DEPTHSTENCILSTATE_H_
