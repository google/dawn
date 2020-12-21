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
#include "dawn_native/IntegerTypes.h"
#include "dawn_native/PerStage.h"
#include "dawn_native/dawn_platform.h"

#include <bitset>
#include <map>
#include <unordered_map>
#include <vector>

namespace tint {

    namespace ast {
        class Module;
    }  // namespace ast

    namespace transform {
        class Manager;
        class VertexPulling;
    }  // namespace transform

}  // namespace tint

namespace spirv_cross {
    class Compiler;
}

namespace dawn_native {

    struct EntryPointMetadata;

    // A map from name to EntryPointMetadata.
    using EntryPointMetadataTable =
        std::unordered_map<std::string, std::unique_ptr<EntryPointMetadata>>;

    struct ShaderModuleParseResult {
        ShaderModuleParseResult();
        ~ShaderModuleParseResult();
        ShaderModuleParseResult(ShaderModuleParseResult&& rhs);
        ShaderModuleParseResult& operator=(ShaderModuleParseResult&& rhs);

#ifdef DAWN_ENABLE_WGSL
        std::unique_ptr<tint::ast::Module> tintModule;
#endif
        std::vector<uint32_t> spirv;
    };

    ResultOrError<ShaderModuleParseResult> ValidateShaderModuleDescriptor(
        DeviceBase* device,
        const ShaderModuleDescriptor* descriptor);
    MaybeError ValidateCompatibilityWithPipelineLayout(DeviceBase* device,
                                                       const EntryPointMetadata& entryPoint,
                                                       const PipelineLayoutBase* layout);

    RequiredBufferSizes ComputeRequiredBufferSizesForLayout(const EntryPointMetadata& entryPoint,
                                                            const PipelineLayoutBase* layout);
#ifdef DAWN_ENABLE_WGSL
    ResultOrError<tint::ast::Module> RunTransforms(tint::transform::Manager* manager,
                                                   tint::ast::Module* module);

    std::unique_ptr<tint::transform::VertexPulling> MakeVertexPullingTransform(
        const VertexStateDescriptor& vertexState,
        const std::string& entryPoint,
        BindGroupIndex pullingBufferBindingSet);
#endif

    // Contains all the reflection data for a valid (ShaderModule, entryPoint, stage). They are
    // stored in the ShaderModuleBase and destroyed only when the shader module is destroyed so
    // pointers to EntryPointMetadata are safe to store as long as you also keep a Ref to the
    // ShaderModuleBase.
    struct EntryPointMetadata {
        // Per-binding shader metadata contains some SPIRV specific information in addition to
        // most of the frontend per-binding information.
        struct ShaderBindingInfo : BindingInfo {
            // The SPIRV ID of the resource.
            uint32_t id;
            uint32_t base_type_id;

          private:
            // Disallow access to unused members.
            using BindingInfo::visibility;
        };

        // bindings[G][B] is the reflection data for the binding defined with
        // [[group=G, binding=B]] in WGSL / SPIRV.
        using BindingGroupInfoMap = std::map<BindingNumber, ShaderBindingInfo>;
        using BindingInfo = ityp::array<BindGroupIndex, BindingGroupInfoMap, kMaxBindGroups>;
        BindingInfo bindings;

        // The set of vertex attributes this entryPoint uses.
        std::bitset<kMaxVertexAttributes> usedVertexAttributes;

        // An array to record the basic types (float, int and uint) of the fragment shader outputs.
        ityp::array<ColorAttachmentIndex, wgpu::TextureComponentType, kMaxColorAttachments>
            fragmentOutputFormatBaseTypes;
        ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> fragmentOutputsWritten;

        // The local workgroup size declared for a compute entry point (or 0s otehrwise).
        Origin3D localWorkgroupSize;

        // The shader stage for this binding.
        SingleShaderStage stage;
    };

    class ShaderModuleBase : public CachedObject {
      public:
        ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor* descriptor);
        ~ShaderModuleBase() override;

        static ShaderModuleBase* MakeError(DeviceBase* device);

        // Return true iff the module has an entrypoint called `entryPoint`.
        bool HasEntryPoint(const std::string& entryPoint) const;

        // Returns the metadata for the given `entryPoint`. HasEntryPoint with the same argument
        // must be true.
        const EntryPointMetadata& GetEntryPoint(const std::string& entryPoint) const;

        // Functions necessary for the unordered_set<ShaderModuleBase*>-based cache.
        size_t ComputeContentHash() override;

        struct EqualityFunc {
            bool operator()(const ShaderModuleBase* a, const ShaderModuleBase* b) const;
        };

        const std::vector<uint32_t>& GetSpirv() const;

#ifdef DAWN_ENABLE_WGSL
        ResultOrError<std::vector<uint32_t>> GeneratePullingSpirv(
            const std::vector<uint32_t>& spirv,
            const VertexStateDescriptor& vertexState,
            const std::string& entryPoint,
            BindGroupIndex pullingBufferBindingSet) const;

        ResultOrError<std::vector<uint32_t>> GeneratePullingSpirv(
            tint::ast::Module* module,
            const VertexStateDescriptor& vertexState,
            const std::string& entryPoint,
            BindGroupIndex pullingBufferBindingSet) const;
#endif

      protected:
        MaybeError InitializeBase(ShaderModuleParseResult* parseResult);

      private:
        ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag);

        enum class Type { Undefined, Spirv, Wgsl };
        Type mType;
        std::vector<uint32_t> mOriginalSpirv;
        std::vector<uint32_t> mSpirv;
        std::string mWgsl;

        EntryPointMetadataTable mEntryPoints;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_SHADERMODULE_H_
