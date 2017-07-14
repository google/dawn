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

#ifndef BACKEND_PIPELINE_H_
#define BACKEND_PIPELINE_H_

#include "backend/Forward.h"
#include "backend/Builder.h"
#include "backend/PerStage.h"
#include "backend/PipelineLayout.h"
#include "backend/RefCounted.h"
#include "backend/ShaderModule.h"

#include "nxt/nxtcpp.h"

#include <array>
#include <bitset>

namespace backend {

    enum PushConstantType : uint8_t {
        Int,
        UInt,
        Float,
    };

    class PipelineBuilder;

    class PipelineBase {
        public:
            PipelineBase(PipelineBuilder* builder);

            struct PushConstantInfo {
                std::bitset<kMaxPushConstants> mask;
                std::array<PushConstantType, kMaxPushConstants> types;
            };
            const PushConstantInfo& GetPushConstants(nxt::ShaderStage stage) const;
            nxt::ShaderStageBit GetStageMask() const;

            PipelineLayoutBase* GetLayout();

        private:
            nxt::ShaderStageBit stageMask;
            Ref<PipelineLayoutBase> layout;
            PerStage<PushConstantInfo> pushConstants;
    };

    class PipelineBuilder {
        public:
            PipelineBuilder(BuilderBase* parentBuilder);

            struct StageInfo {
                std::string entryPoint;
                Ref<ShaderModuleBase> module;
            };
            const StageInfo& GetStageInfo(nxt::ShaderStage stage) const;
            BuilderBase* GetParentBuilder() const;

            // NXT API
            void SetLayout(PipelineLayoutBase* layout);
            void SetStage(nxt::ShaderStage stage, ShaderModuleBase* module, const char* entryPoint);

        private:
            friend class PipelineBase;

            BuilderBase* parentBuilder;
            Ref<PipelineLayoutBase> layout;
            nxt::ShaderStageBit stageMask;
            PerStage<StageInfo> stages;
    };

}

#endif // BACKEND_PIPELINE_H_
