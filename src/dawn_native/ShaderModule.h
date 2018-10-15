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

#ifndef DAWNNATIVE_SHADERMODULE_H_
#define DAWNNATIVE_SHADERMODULE_H_

#include "common/Constants.h"
#include "dawn_native/Builder.h"
#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/ObjectBase.h"

#include "dawn_native/dawn_platform.h"

#include <array>
#include <bitset>
#include <vector>

namespace spirv_cross {
    class Compiler;
}

namespace dawn_native {

    MaybeError ValidateShaderModuleDescriptor(DeviceBase* device,
                                              const ShaderModuleDescriptor* descriptor);

    class ShaderModuleBase : public ObjectBase {
      public:
        ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor* descriptor);

        void ExtractSpirvInfo(const spirv_cross::Compiler& compiler);

        struct PushConstantInfo {
            std::bitset<kMaxPushConstants> mask;

            std::array<std::string, kMaxPushConstants> names;
            std::array<uint32_t, kMaxPushConstants> sizes;
            std::array<PushConstantType, kMaxPushConstants> types;
        };

        struct BindingInfo {
            // The SPIRV ID of the resource.
            uint32_t id;
            uint32_t base_type_id;
            dawn::BindingType type;
            bool used = false;
        };
        using ModuleBindingInfo =
            std::array<std::array<BindingInfo, kMaxBindingsPerGroup>, kMaxBindGroups>;

        const PushConstantInfo& GetPushConstants() const;
        const ModuleBindingInfo& GetBindingInfo() const;
        const std::bitset<kMaxVertexAttributes>& GetUsedVertexAttributes() const;
        dawn::ShaderStage GetExecutionModel() const;

        bool IsCompatibleWithPipelineLayout(const PipelineLayoutBase* layout);

      private:
        bool IsCompatibleWithBindGroupLayout(size_t group, const BindGroupLayoutBase* layout);

        PushConstantInfo mPushConstants = {};
        ModuleBindingInfo mBindingInfo;
        std::bitset<kMaxVertexAttributes> mUsedVertexAttributes;
        dawn::ShaderStage mExecutionModel;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_SHADERMODULE_H_
