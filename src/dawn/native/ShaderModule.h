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

#ifndef SRC_DAWN_NATIVE_SHADERMODULE_H_
#define SRC_DAWN_NATIVE_SHADERMODULE_H_

#include <bitset>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/common/ityp_array.h"
#include "dawn/native/BindingInfo.h"
#include "dawn/native/CachedObject.h"
#include "dawn/native/CompilationMessages.h"
#include "dawn/native/Error.h"
#include "dawn/native/Format.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/Limits.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/VertexFormat.h"
#include "dawn/native/dawn_platform.h"
#include "tint/override_id.h"

namespace tint {

class Program;

namespace transform {
class DataMap;
class Manager;
class Transform;
class VertexPulling;
}  // namespace transform

}  // namespace tint

namespace dawn::native {

using WGSLExtensionSet = std::unordered_set<std::string>;
struct EntryPointMetadata;

// Base component type of an inter-stage variable
enum class InterStageComponentType {
    I32,
    U32,
    F32,
    F16,
};

enum class InterpolationType {
    Perspective,
    Linear,
    Flat,
};

enum class InterpolationSampling {
    None,
    Center,
    Centroid,
    Sample,
};

// Use map to make sure constant keys are sorted for creating shader cache keys
using PipelineConstantEntries = std::map<std::string, double>;

// A map from name to EntryPointMetadata.
using EntryPointMetadataTable =
    std::unordered_map<std::string, std::unique_ptr<EntryPointMetadata>>;

// Source for a tint program
class TintSource;

struct ShaderModuleParseResult {
    ShaderModuleParseResult();
    ~ShaderModuleParseResult();
    ShaderModuleParseResult(ShaderModuleParseResult&& rhs);
    ShaderModuleParseResult& operator=(ShaderModuleParseResult&& rhs);

    bool HasParsedShader() const;

    std::unique_ptr<tint::Program> tintProgram;
    std::unique_ptr<TintSource> tintSource;
};

MaybeError ValidateAndParseShaderModule(DeviceBase* device,
                                        const ShaderModuleDescriptor* descriptor,
                                        ShaderModuleParseResult* parseResult,
                                        OwnedCompilationMessages* outMessages);
MaybeError ValidateCompatibilityWithPipelineLayout(DeviceBase* device,
                                                   const EntryPointMetadata& entryPoint,
                                                   const PipelineLayoutBase* layout);

// Return extent3D with workgroup size dimension info if it is valid
// width = x, height = y, depthOrArrayLength = z
ResultOrError<Extent3D> ValidateComputeStageWorkgroupSize(
    const tint::Program& program,
    const char* entryPointName,
    const LimitsForCompilationRequest& limits);

RequiredBufferSizes ComputeRequiredBufferSizesForLayout(const EntryPointMetadata& entryPoint,
                                                        const PipelineLayoutBase* layout);
ResultOrError<tint::Program> RunTransforms(tint::transform::Transform* transform,
                                           const tint::Program* program,
                                           const tint::transform::DataMap& inputs,
                                           tint::transform::DataMap* outputs,
                                           OwnedCompilationMessages* messages);

// Mirrors wgpu::SamplerBindingLayout but instead stores a single boolean
// for isComparison instead of a wgpu::SamplerBindingType enum.
struct ShaderSamplerBindingInfo {
    bool isComparison;
};

// Mirrors wgpu::TextureBindingLayout but instead has a set of compatible sampleTypes
// instead of a single enum.
struct ShaderTextureBindingInfo {
    SampleTypeBit compatibleSampleTypes;
    wgpu::TextureViewDimension viewDimension;
    bool multisampled;
};

// Per-binding shader metadata contains some SPIRV specific information in addition to
// most of the frontend per-binding information.
struct ShaderBindingInfo {
    // The SPIRV ID of the resource.
    uint32_t id;
    uint32_t base_type_id;

    BindingNumber binding;
    BindingInfoType bindingType;

    BufferBindingLayout buffer;
    ShaderSamplerBindingInfo sampler;
    ShaderTextureBindingInfo texture;
    StorageTextureBindingLayout storageTexture;
};

using BindingGroupInfoMap = std::map<BindingNumber, ShaderBindingInfo>;
using BindingInfoArray = ityp::array<BindGroupIndex, BindingGroupInfoMap, kMaxBindGroups>;

// The WebGPU override variables only support these scalar types
union OverrideScalar {
    // Use int32_t for boolean to initialize the full 32bit
    int32_t b;
    float f32;
    int32_t i32;
    uint32_t u32;
};

// Contains all the reflection data for a valid (ShaderModule, entryPoint, stage). They are
// stored in the ShaderModuleBase and destroyed only when the shader program is destroyed so
// pointers to EntryPointMetadata are safe to store as long as you also keep a Ref to the
// ShaderModuleBase.
struct EntryPointMetadata {
    // It is valid for a shader to contain entry points that go over limits. To keep this
    // structure with packed arrays and bitsets, we still validate against limits when
    // doing reflection, but store the errors in this vector, for later use if the application
    // tries to use the entry point.
    std::vector<std::string> infringedLimitErrors;

    // bindings[G][B] is the reflection data for the binding defined with
    // @group(G) @binding(B) in WGSL / SPIRV.
    BindingInfoArray bindings;

    struct SamplerTexturePair {
        BindingSlot sampler;
        BindingSlot texture;
    };
    std::vector<SamplerTexturePair> samplerTexturePairs;

    // The set of vertex attributes this entryPoint uses.
    ityp::array<VertexAttributeLocation, VertexFormatBaseType, kMaxVertexAttributes>
        vertexInputBaseTypes;
    ityp::bitset<VertexAttributeLocation, kMaxVertexAttributes> usedVertexInputs;

    // An array to record the basic types (float, int and uint) of the fragment shader outputs.
    struct FragmentOutputVariableInfo {
        wgpu::TextureComponentType baseType;
        uint8_t componentCount;
    };
    ityp::array<ColorAttachmentIndex, FragmentOutputVariableInfo, kMaxColorAttachments>
        fragmentOutputVariables;
    ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> fragmentOutputsWritten;

    struct InterStageVariableInfo {
        InterStageComponentType baseType;
        uint32_t componentCount;
        InterpolationType interpolationType;
        InterpolationSampling interpolationSampling;
    };
    // Now that we only support vertex and fragment stages, there can't be both inter-stage
    // inputs and outputs in one shader stage.
    std::bitset<kMaxInterStageShaderVariables> usedInterStageVariables;
    std::array<InterStageVariableInfo, kMaxInterStageShaderVariables> interStageVariables;
    uint32_t totalInterStageShaderComponents;

    // The shader stage for this binding.
    SingleShaderStage stage;

    struct Override {
        tint::OverrideId id;

        // Match tint::inspector::Override::Type
        // Bool is defined as a macro on linux X11 and cannot compile
        enum class Type { Boolean, Float32, Uint32, Int32 } type;

        // If the constant doesn't not have an initializer in the shader
        // Then it is required for the pipeline stage to have a constant record to initialize a
        // value
        bool isInitialized;
    };

    using OverridesMap = std::unordered_map<std::string, Override>;

    // Map identifier to override variable
    // Identifier is unique: either the variable name or the numeric ID if specified
    OverridesMap overrides;

    // Override variables that are not initialized in shaders
    // They need value initialization from pipeline stage or it is a validation error
    std::unordered_set<std::string> uninitializedOverrides;

    // Store constants with shader initialized values as well
    // This is used by metal backend to set values with default initializers that are not
    // overridden
    std::unordered_set<std::string> initializedOverrides;

    bool usesNumWorkgroups = false;
    bool usesFragDepth = false;
    // Used at render pipeline validation.
    bool usesSampleMaskOutput = false;
};

class ShaderModuleBase : public ApiObjectBase, public CachedObject {
  public:
    ShaderModuleBase(DeviceBase* device,
                     const ShaderModuleDescriptor* descriptor,
                     ApiObjectBase::UntrackedByDeviceTag tag);
    ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor* descriptor);
    ~ShaderModuleBase() override;

    static Ref<ShaderModuleBase> MakeError(DeviceBase* device);

    ObjectType GetType() const override;

    // Return true iff the program has an entrypoint called `entryPoint`.
    bool HasEntryPoint(const std::string& entryPoint) const;

    // Return the metadata for the given `entryPoint`. HasEntryPoint with the same argument
    // must be true.
    const EntryPointMetadata& GetEntryPoint(const std::string& entryPoint) const;

    // Functions necessary for the unordered_set<ShaderModuleBase*>-based cache.
    size_t ComputeContentHash() override;

    struct EqualityFunc {
        bool operator()(const ShaderModuleBase* a, const ShaderModuleBase* b) const;
    };

    // This returns tint program before running transforms.
    const tint::Program* GetTintProgram() const;

    void APIGetCompilationInfo(wgpu::CompilationInfoCallback callback, void* userdata);

    void InjectCompilationMessages(std::unique_ptr<OwnedCompilationMessages> compilationMessages);

    OwnedCompilationMessages* GetCompilationMessages() const;

  protected:
    void DestroyImpl() override;

    MaybeError InitializeBase(ShaderModuleParseResult* parseResult,
                              OwnedCompilationMessages* compilationMessages);

  private:
    ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag);

    // The original data in the descriptor for caching.
    enum class Type { Undefined, Spirv, Wgsl };
    Type mType;
    std::vector<uint32_t> mOriginalSpirv;
    std::string mWgsl;

    EntryPointMetadataTable mEntryPoints;
    WGSLExtensionSet mEnabledWGSLExtensions;
    std::unique_ptr<tint::Program> mTintProgram;
    std::unique_ptr<TintSource> mTintSource;  // Keep the tint::Source::File alive

    std::unique_ptr<OwnedCompilationMessages> mCompilationMessages;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SHADERMODULE_H_
