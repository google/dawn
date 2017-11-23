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

#include "backend/Pipeline.h"

#include "backend/Device.h"
#include "backend/DepthStencilState.h"
#include "backend/InputState.h"
#include "backend/PipelineLayout.h"
#include "backend/RenderPass.h"
#include "backend/ShaderModule.h"

namespace backend {

    // PipelineBase

    PipelineBase::PipelineBase(PipelineBuilder* builder)
        : mStageMask(builder->mStageMask), mLayout(std::move(builder->mLayout)) {
        if (!mLayout) {
            mLayout = builder->GetParentBuilder()->GetDevice()->CreatePipelineLayoutBuilder()->GetResult();
        }

        auto FillPushConstants = [](const ShaderModuleBase* module, PushConstantInfo* info) {
            const auto& moduleInfo = module->GetPushConstants();
            info->mask = moduleInfo.mask;

            for (uint32_t i = 0; i < moduleInfo.names.size(); i++) {
                uint32_t size = moduleInfo.sizes[i];
                if (size == 0) {
                    continue;
                }

                for (uint32_t offset = 0; offset < size; offset++) {
                    info->types[i + offset] = moduleInfo.types[i];
                }
                i += size - 1;
            }
        };

        for (auto stageBit : IterateStages(builder->mStageMask)) {
            if (!builder->mStages[stageBit].module->IsCompatibleWithPipelineLayout(mLayout.Get())) {
                builder->GetParentBuilder()->HandleError("Stage not compatible with layout");
                return;
            }

            FillPushConstants(builder->mStages[stageBit].module.Get(), &mPushConstants[stageBit]);
        }
    }

    const PipelineBase::PushConstantInfo& PipelineBase::GetPushConstants(nxt::ShaderStage stage) const {
        return mPushConstants[stage];
    }

    nxt::ShaderStageBit PipelineBase::GetStageMask() const {
        return mStageMask;
    }

    PipelineLayoutBase* PipelineBase::GetLayout() {
        return mLayout.Get();
    }

    // PipelineBuilder

    PipelineBuilder::PipelineBuilder(BuilderBase* parentBuilder)
        : mParentBuilder(parentBuilder), mStageMask(static_cast<nxt::ShaderStageBit>(0)) {
    }

    const PipelineBuilder::StageInfo& PipelineBuilder::GetStageInfo(nxt::ShaderStage stage) const {
        ASSERT(mStageMask & StageBit(stage));
        return mStages[stage];
    }

    BuilderBase* PipelineBuilder::GetParentBuilder() const {
        return mParentBuilder;
    }

    void PipelineBuilder::SetLayout(PipelineLayoutBase* layout) {
        mLayout = layout;
    }

    void PipelineBuilder::SetStage(nxt::ShaderStage stage, ShaderModuleBase* module, const char* entryPoint) {
        if (entryPoint != std::string("main")) {
            mParentBuilder->HandleError("Currently the entry point has to be main()");
            return;
        }

        if (stage != module->GetExecutionModel()) {
            mParentBuilder->HandleError("Setting module with wrong execution model");
            return;
        }

        nxt::ShaderStageBit bit = StageBit(stage);
        if (mStageMask & bit) {
            mParentBuilder->HandleError("Setting already set stage");
            return;
        }
        mStageMask |= bit;

        mStages[stage].module = module;
        mStages[stage].entryPoint = entryPoint;
    }

}
