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

#include "backend/RenderPipeline.h"

#include "backend/Device.h"
#include "backend/DepthStencilState.h"
#include "backend/InputState.h"
#include "backend/RenderPass.h"

namespace backend {

    // RenderPipelineBase

    RenderPipelineBase::RenderPipelineBase(RenderPipelineBuilder* builder)
        : PipelineBase(builder),
          depthStencilState(std::move(builder->depthStencilState)),
          inputState(std::move(builder->inputState)),
          primitiveTopology(builder->primitiveTopology),
          renderPass(std::move(builder->renderPass)), subpass(builder->subpass) {

        if (GetStageMask() != (nxt::ShaderStageBit::Vertex | nxt::ShaderStageBit::Fragment)) {
            builder->HandleError("Render pipeline should have exactly a vertex and fragment stage");
            return;
        }

        if (!renderPass) {
            builder->HandleError("Pipeline render pass not set");
            return;
        }

        // TODO(kainino@chromium.org): Need to verify the pipeline against its render subpass.

        if ((builder->GetStageInfo(nxt::ShaderStage::Vertex).module->GetUsedVertexAttributes() & ~inputState->GetAttributesSetMask()).any()) {
            builder->HandleError("Pipeline vertex stage uses inputs not in the input state");
            return;
        }
    }

    DepthStencilStateBase* RenderPipelineBase::GetDepthStencilState() {
        return depthStencilState.Get();
    }

    InputStateBase* RenderPipelineBase::GetInputState() {
        return inputState.Get();
    }

    nxt::PrimitiveTopology RenderPipelineBase::GetPrimitiveTopology() const {
        return primitiveTopology;
    }

    RenderPassBase* RenderPipelineBase::GetRenderPass() {
        return renderPass.Get();
    }

    uint32_t RenderPipelineBase::GetSubPass() {
        return subpass;
    }

    // RenderPipelineBuilder

    RenderPipelineBuilder::RenderPipelineBuilder(DeviceBase* device)
        : Builder(device), PipelineBuilder(this) {
    }

    RenderPipelineBase* RenderPipelineBuilder::GetResultImpl() {
        // TODO(cwallez@chromium.org): the layout should be required, and put the default objects in the device
        if (!inputState) {
            inputState = device->CreateInputStateBuilder()->GetResult();
        }
        if (!depthStencilState) {
            depthStencilState = device->CreateDepthStencilStateBuilder()->GetResult();
        }

        return device->CreateRenderPipeline(this);
    }

    void RenderPipelineBuilder::SetDepthStencilState(DepthStencilStateBase* depthStencilState) {
        this->depthStencilState = depthStencilState;
    }

    void RenderPipelineBuilder::SetInputState(InputStateBase* inputState) {
        this->inputState = inputState;
    }

    void RenderPipelineBuilder::SetPrimitiveTopology(nxt::PrimitiveTopology primitiveTopology) {
        this->primitiveTopology = primitiveTopology;
    }

    void RenderPipelineBuilder::SetSubpass(RenderPassBase* renderPass, uint32_t subpass) {
        this->renderPass = renderPass;
        this->subpass = subpass;
    }

}
