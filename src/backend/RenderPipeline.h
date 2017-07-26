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

#ifndef BACKEND_RENDERPIPELINE_H_
#define BACKEND_RENDERPIPELINE_H_

#include "backend/Pipeline.h"

#include "nxt/nxtcpp.h"

namespace backend {

    class RenderPipelineBase : public RefCounted, public PipelineBase {
        public:
            RenderPipelineBase(RenderPipelineBuilder* builder);

            DepthStencilStateBase* GetDepthStencilState();
            InputStateBase* GetInputState();
            nxt::PrimitiveTopology GetPrimitiveTopology() const;
            RenderPassBase* GetRenderPass();
            uint32_t GetSubPass();

        private:
            Ref<DepthStencilStateBase> depthStencilState;
            Ref<InputStateBase> inputState;
            nxt::PrimitiveTopology primitiveTopology;
            Ref<RenderPassBase> renderPass;
            uint32_t subpass;
    };

    class RenderPipelineBuilder : public Builder<RenderPipelineBase>, public PipelineBuilder {
        public:
            RenderPipelineBuilder(DeviceBase* device);

            // NXT API
            void SetDepthStencilState(DepthStencilStateBase* depthStencilState);
            void SetPrimitiveTopology(nxt::PrimitiveTopology primitiveTopology);
            void SetInputState(InputStateBase* inputState);
            void SetSubpass(RenderPassBase* renderPass, uint32_t subpass);

        private:
            friend class RenderPipelineBase;

            RenderPipelineBase* GetResultImpl() override;

            Ref<DepthStencilStateBase> depthStencilState;
            Ref<InputStateBase> inputState;
            // TODO(enga@google.com): Remove default when we validate that all required properties are set
            nxt::PrimitiveTopology primitiveTopology = nxt::PrimitiveTopology::TriangleList;
            Ref<RenderPassBase> renderPass;
            uint32_t subpass;
    };

}

#endif // BACKEND_RENDERPIPELINE_H_
