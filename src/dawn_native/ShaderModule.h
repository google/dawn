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
#include "common/ityp_array.h"
#include "dawn_native/BindingInfo.h"
#include "dawn_native/CachedObject.h"
#include "dawn_native/Error.h"
#include "dawn_native/Format.h"
#include "dawn_native/Forward.h"
#include "dawn_native/PerStage.h"

#include "dawn_native/dawn_platform.h"

#include <bitset>
#include <map>
#include <vector>

namespace spirv_cross {
    class Compiler;
}

namespace dawn_native {

    struct EntryPointMetadata;

    MaybeError ValidateShaderModuleDescriptor(DeviceBase* device,
                                              const ShaderModuleDescriptor* descriptor);
    MaybeError ValidateCompatibilityWithPipelineLayout(const EntryPointMetadata& entryPoint,
                                                       const PipelineLayoutBase* layout);

    RequiredBufferSizes ComputeRequiredBufferSizesForLayout(const EntryPointMetadata& entryPoint,
                                                            const PipelineLayoutBase* layout);

    // Contains all the reflection data for a valid (ShaderModule, entryPoint, stage). They are
    // stored in the ShaderModuleBase and destroyed only when the shader module is destroyed so
    // pointers to EntryPointMetadata are safe to store as long as you also keep a Ref to the
    // ShaderModuleBase.
    struct EntryPointMetadata {
        EntryPointMetadata();

        // Per-binding shader metadata contains some SPIRV specific information in addition to
        // most of the frontend per-binding information.
        struct ShaderBindingInfo : BindingInfo {
            // The SPIRV ID of the resource.
            uint32_t id;
            uint32_t base_type_id;

          private:
            // Disallow access to unused members.
            using BindingInfo::hasDynamicOffset;
            using BindingInfo::visibility;
        };

        // bindings[G][B] is the reflection data for the binding defined with
        // [[group=G, binding=B]] in WGSL / SPIRV.
        using BindingGroupInfoMap = std::map<BindingNumber, ShaderBindingInfo>;
        using BindingInfo = ityp::array<BindGroupIndex, BindingGroupInfoMap, kMaxBindGroups>;
        BindingInfo bindings;

        // The set of vertex attributes this entryPoint uses.
        std::bitset<kMaxVertexAttributes> usedVertexAttributes;

        // An array to record the basic types (float, int and uint) of the fragment shader outputs
        // or Format::Type::Other means the fragment shader output is unused.
        using FragmentOutputBaseTypes = std::array<Format::Type, kMaxColorAttachments>;
        FragmentOutputBaseTypes fragmentOutputFormatBaseTypes;

        // The shader stage for this binding, TODO(dawn:216): can likely be removed once we
        // properly support multiple entrypoints per ShaderModule.
        SingleShaderStage stage;
    };

    class ShaderModuleBase : public CachedObject {
      public:
        ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor* descriptor);
        ~ShaderModuleBase() override;

        static ShaderModuleBase* MakeError(DeviceBase* device);

        // Return true iff the module has an entrypoint called `entryPoint` for stage `stage`.
        bool HasEntryPoint(const std::string& entryPoint, SingleShaderStage stage) const;

        // Returns the metadata for the given `entryPoint` and `stage`. HasEntryPoint with the same
        // arguments must be true.
        const EntryPointMetadata& GetEntryPoint(const std::string& entryPoint,
                                                SingleShaderStage stage) const;

        // TODO make this member protected, it is only used outside of child classes in DeviceNull.
        MaybeError ExtractSpirvInfo(const spirv_cross::Compiler& compiler);

        // Functors necessary for the unordered_set<ShaderModuleBase*>-based cache.
        struct HashFunc {
            size_t operator()(const ShaderModuleBase* module) const;
        };
        struct EqualityFunc {
            bool operator()(const ShaderModuleBase* a, const ShaderModuleBase* b) const;
        };

        const std::vector<uint32_t>& GetSpirv() const;

#ifdef DAWN_ENABLE_WGSL
        ResultOrError<std::vector<uint32_t>> GeneratePullingSpirv(
            const VertexStateDescriptor& vertexState,
            const std::string& entryPoint,
            uint32_t pullingBufferBindingSet) const;
#endif

      protected:
        MaybeError InitializeBase();

        // Allows backends to get the stage for the "main" entrypoint while they are transitioned to
        // support multiple entrypoints.
        // TODO(dawn:216): Remove this once the transition is complete.
        SingleShaderStage GetMainEntryPointStageForTransition() const;

      private:
        ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        ResultOrError<std::unique_ptr<EntryPointMetadata>> ExtractSpirvInfoImpl(
            const spirv_cross::Compiler& compiler);

        enum class Type { Undefined, Spirv, Wgsl };
        Type mType;
        std::vector<uint32_t> mSpirv;
        std::string mWgsl;

        std::unique_ptr<EntryPointMetadata> mMainEntryPoint;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_SHADERMODULE_H_
