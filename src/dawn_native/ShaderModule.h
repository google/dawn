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
#include "dawn_native/CachedObject.h"
#include "dawn_native/Error.h"
#include "dawn_native/Format.h"
#include "dawn_native/Forward.h"
#include "dawn_native/PerStage.h"

#include "dawn_native/dawn_platform.h"

#include "spvc/spvc.hpp"

#include <array>
#include <bitset>
#include <vector>

namespace spirv_cross {
    class Compiler;
}

namespace dawn_native {

    MaybeError ValidateShaderModuleDescriptor(DeviceBase* device,
                                              const ShaderModuleDescriptor* descriptor);

    class ShaderModuleBase : public CachedObject {
      public:
        ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor* descriptor);
        ~ShaderModuleBase() override;

        static ShaderModuleBase* MakeError(DeviceBase* device);

        void ExtractSpirvInfo(const spirv_cross::Compiler& compiler);

        struct BindingInfo {
            // The SPIRV ID of the resource.
            uint32_t id;
            uint32_t base_type_id;
            wgpu::BindingType type;
            // Match the defaults in BindGroupLayoutDescriptor
            wgpu::TextureViewDimension textureDimension = wgpu::TextureViewDimension::Undefined;
            Format::Type textureComponentType = Format::Type::Float;
            bool multisampled = false;
            bool used = false;
        };
        using ModuleBindingInfo =
            std::array<std::array<BindingInfo, kMaxBindingsPerGroup>, kMaxBindGroups>;

        const ModuleBindingInfo& GetBindingInfo() const;
        const std::bitset<kMaxVertexAttributes>& GetUsedVertexAttributes() const;
        SingleShaderStage GetExecutionModel() const;

        // An array to record the basic types (float, int and uint) of the fragment shader outputs
        // or Format::Type::Other means the fragment shader output is unused.
        using FragmentOutputBaseTypes = std::array<Format::Type, kMaxColorAttachments>;
        const FragmentOutputBaseTypes& GetFragmentOutputBaseTypes() const;

        bool IsCompatibleWithPipelineLayout(const PipelineLayoutBase* layout) const;

        // Functors necessary for the unordered_set<ShaderModuleBase*>-based cache.
        struct HashFunc {
            size_t operator()(const ShaderModuleBase* module) const;
        };
        struct EqualityFunc {
            bool operator()(const ShaderModuleBase* a, const ShaderModuleBase* b) const;
        };

      protected:
        shaderc_spvc::Context mSpvcContext;

      private:
        ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        bool IsCompatibleWithBindGroupLayout(size_t group, const BindGroupLayoutBase* layout) const;

        // TODO(cwallez@chromium.org): The code is only stored for deduplication. We could maybe
        // store a cryptographic hash of the code instead?
        std::vector<uint32_t> mCode;

        ModuleBindingInfo mBindingInfo;
        std::bitset<kMaxVertexAttributes> mUsedVertexAttributes;
        SingleShaderStage mExecutionModel;

        FragmentOutputBaseTypes mFragmentOutputFormatBaseTypes;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_SHADERMODULE_H_
