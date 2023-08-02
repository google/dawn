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

#include "dawn/native/ShaderModule.h"

#include <algorithm>
#include <sstream>

#include "absl/strings/str_format.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Constants.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CompilationMessages.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/TintUtils.h"

#include "tint/tint.h"

namespace dawn::native {

namespace {

ResultOrError<SingleShaderStage> TintPipelineStageToShaderStage(
    tint::inspector::PipelineStage stage) {
    switch (stage) {
        case tint::inspector::PipelineStage::kVertex:
            return SingleShaderStage::Vertex;
        case tint::inspector::PipelineStage::kFragment:
            return SingleShaderStage::Fragment;
        case tint::inspector::PipelineStage::kCompute:
            return SingleShaderStage::Compute;
    }
    UNREACHABLE();
}

BindingInfoType TintResourceTypeToBindingInfoType(
    tint::inspector::ResourceBinding::ResourceType type) {
    switch (type) {
        case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
        case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
            return BindingInfoType::Buffer;
        case tint::inspector::ResourceBinding::ResourceType::kSampler:
        case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
            return BindingInfoType::Sampler;
        case tint::inspector::ResourceBinding::ResourceType::kSampledTexture:
        case tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture:
        case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
        case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
            return BindingInfoType::Texture;
        case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
            return BindingInfoType::StorageTexture;
        case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
            return BindingInfoType::ExternalTexture;

        default:
            UNREACHABLE();
            return BindingInfoType::Buffer;
    }
}

wgpu::TextureFormat TintImageFormatToTextureFormat(
    tint::inspector::ResourceBinding::TexelFormat format) {
    switch (format) {
        case tint::inspector::ResourceBinding::TexelFormat::kR32Uint:
            return wgpu::TextureFormat::R32Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kR32Sint:
            return wgpu::TextureFormat::R32Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kR32Float:
            return wgpu::TextureFormat::R32Float;
        case tint::inspector::ResourceBinding::TexelFormat::kBgra8Unorm:
            return wgpu::TextureFormat::BGRA8Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Unorm:
            return wgpu::TextureFormat::RGBA8Unorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Snorm:
            return wgpu::TextureFormat::RGBA8Snorm;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Uint:
            return wgpu::TextureFormat::RGBA8Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Sint:
            return wgpu::TextureFormat::RGBA8Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Uint:
            return wgpu::TextureFormat::RG32Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Sint:
            return wgpu::TextureFormat::RG32Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Float:
            return wgpu::TextureFormat::RG32Float;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Uint:
            return wgpu::TextureFormat::RGBA16Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Sint:
            return wgpu::TextureFormat::RGBA16Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Float:
            return wgpu::TextureFormat::RGBA16Float;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Uint:
            return wgpu::TextureFormat::RGBA32Uint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Sint:
            return wgpu::TextureFormat::RGBA32Sint;
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Float:
            return wgpu::TextureFormat::RGBA32Float;
        case tint::inspector::ResourceBinding::TexelFormat::kNone:
            return wgpu::TextureFormat::Undefined;

        default:
            UNREACHABLE();
            return wgpu::TextureFormat::Undefined;
    }
}

wgpu::TextureViewDimension TintTextureDimensionToTextureViewDimension(
    tint::inspector::ResourceBinding::TextureDimension dim) {
    switch (dim) {
        case tint::inspector::ResourceBinding::TextureDimension::k1d:
            return wgpu::TextureViewDimension::e1D;
        case tint::inspector::ResourceBinding::TextureDimension::k2d:
            return wgpu::TextureViewDimension::e2D;
        case tint::inspector::ResourceBinding::TextureDimension::k2dArray:
            return wgpu::TextureViewDimension::e2DArray;
        case tint::inspector::ResourceBinding::TextureDimension::k3d:
            return wgpu::TextureViewDimension::e3D;
        case tint::inspector::ResourceBinding::TextureDimension::kCube:
            return wgpu::TextureViewDimension::Cube;
        case tint::inspector::ResourceBinding::TextureDimension::kCubeArray:
            return wgpu::TextureViewDimension::CubeArray;
        case tint::inspector::ResourceBinding::TextureDimension::kNone:
            return wgpu::TextureViewDimension::Undefined;
    }
    UNREACHABLE();
}

SampleTypeBit TintSampledKindToSampleTypeBit(tint::inspector::ResourceBinding::SampledKind s) {
    switch (s) {
        case tint::inspector::ResourceBinding::SampledKind::kSInt:
            return SampleTypeBit::Sint;
        case tint::inspector::ResourceBinding::SampledKind::kUInt:
            return SampleTypeBit::Uint;
        case tint::inspector::ResourceBinding::SampledKind::kFloat:
            return SampleTypeBit::Float | SampleTypeBit::UnfilterableFloat;
        case tint::inspector::ResourceBinding::SampledKind::kUnknown:
            return SampleTypeBit::None;
    }
    UNREACHABLE();
}

ResultOrError<TextureComponentType> TintComponentTypeToTextureComponentType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kF32:
        case tint::inspector::ComponentType::kF16:
            return TextureComponentType::Float;
        case tint::inspector::ComponentType::kI32:
            return TextureComponentType::Sint;
        case tint::inspector::ComponentType::kU32:
            return TextureComponentType::Uint;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    UNREACHABLE();
}

ResultOrError<VertexFormatBaseType> TintComponentTypeToVertexFormatBaseType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kF32:
        case tint::inspector::ComponentType::kF16:
            return VertexFormatBaseType::Float;
        case tint::inspector::ComponentType::kI32:
            return VertexFormatBaseType::Sint;
        case tint::inspector::ComponentType::kU32:
            return VertexFormatBaseType::Uint;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    UNREACHABLE();
}

ResultOrError<wgpu::BufferBindingType> TintResourceTypeToBufferBindingType(
    tint::inspector::ResourceBinding::ResourceType resource_type) {
    switch (resource_type) {
        case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
            return wgpu::BufferBindingType::Uniform;
        case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
            return wgpu::BufferBindingType::Storage;
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
            return wgpu::BufferBindingType::ReadOnlyStorage;
        default:
            return DAWN_VALIDATION_ERROR("Attempted to convert non-buffer resource type");
    }
    UNREACHABLE();
}

ResultOrError<wgpu::StorageTextureAccess> TintResourceTypeToStorageTextureAccess(
    tint::inspector::ResourceBinding::ResourceType resource_type) {
    switch (resource_type) {
        case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
            return wgpu::StorageTextureAccess::WriteOnly;
        default:
            return DAWN_VALIDATION_ERROR("Attempted to convert non-storage texture resource type");
    }
    UNREACHABLE();
}

ResultOrError<InterStageComponentType> TintComponentTypeToInterStageComponentType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kF32:
            return InterStageComponentType::F32;
        case tint::inspector::ComponentType::kI32:
            return InterStageComponentType::I32;
        case tint::inspector::ComponentType::kU32:
            return InterStageComponentType::U32;
        case tint::inspector::ComponentType::kF16:
            return InterStageComponentType::F16;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    UNREACHABLE();
}

ResultOrError<uint32_t> TintCompositionTypeToInterStageComponentCount(
    tint::inspector::CompositionType type) {
    switch (type) {
        case tint::inspector::CompositionType::kScalar:
            return 1u;
        case tint::inspector::CompositionType::kVec2:
            return 2u;
        case tint::inspector::CompositionType::kVec3:
            return 3u;
        case tint::inspector::CompositionType::kVec4:
            return 4u;
        case tint::inspector::CompositionType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempt to convert 'Unknown' composition type from Tint");
    }
    UNREACHABLE();
}

ResultOrError<InterpolationType> TintInterpolationTypeToInterpolationType(
    tint::inspector::InterpolationType type) {
    switch (type) {
        case tint::inspector::InterpolationType::kPerspective:
            return InterpolationType::Perspective;
        case tint::inspector::InterpolationType::kLinear:
            return InterpolationType::Linear;
        case tint::inspector::InterpolationType::kFlat:
            return InterpolationType::Flat;
        case tint::inspector::InterpolationType::kUnknown:
            return DAWN_VALIDATION_ERROR(
                "Attempted to convert 'Unknown' interpolation type from Tint");
    }
    UNREACHABLE();
}

ResultOrError<InterpolationSampling> TintInterpolationSamplingToInterpolationSamplingType(
    tint::inspector::InterpolationSampling type) {
    switch (type) {
        case tint::inspector::InterpolationSampling::kNone:
            return InterpolationSampling::None;
        case tint::inspector::InterpolationSampling::kCenter:
            return InterpolationSampling::Center;
        case tint::inspector::InterpolationSampling::kCentroid:
            return InterpolationSampling::Centroid;
        case tint::inspector::InterpolationSampling::kSample:
            return InterpolationSampling::Sample;
        case tint::inspector::InterpolationSampling::kUnknown:
            return DAWN_VALIDATION_ERROR(
                "Attempted to convert 'Unknown' interpolation sampling type from Tint");
    }
    UNREACHABLE();
}

EntryPointMetadata::Override::Type FromTintOverrideType(tint::inspector::Override::Type type) {
    switch (type) {
        case tint::inspector::Override::Type::kBool:
            return EntryPointMetadata::Override::Type::Boolean;
        case tint::inspector::Override::Type::kFloat32:
            return EntryPointMetadata::Override::Type::Float32;
        case tint::inspector::Override::Type::kFloat16:
            return EntryPointMetadata::Override::Type::Float16;
        case tint::inspector::Override::Type::kInt32:
            return EntryPointMetadata::Override::Type::Int32;
        case tint::inspector::Override::Type::kUint32:
            return EntryPointMetadata::Override::Type::Uint32;
    }
    UNREACHABLE();
}

ResultOrError<tint::Program> ParseWGSL(const tint::Source::File* file,
                                       OwnedCompilationMessages* outMessages) {
#if TINT_BUILD_WGSL_READER
    tint::Program program = tint::wgsl::reader::Parse(file);
    if (outMessages != nullptr) {
        DAWN_TRY(outMessages->AddMessages(program.Diagnostics()));
    }
    if (!program.IsValid()) {
        return DAWN_VALIDATION_ERROR("Tint WGSL reader failure: %s\n", program.Diagnostics().str());
    }

    return std::move(program);
#else
    return DAWN_VALIDATION_ERROR("TINT_BUILD_WGSL_READER is not defined.");
#endif
}

#if TINT_BUILD_SPV_READER
ResultOrError<tint::Program> ParseSPIRV(const std::vector<uint32_t>& spirv,
                                        OwnedCompilationMessages* outMessages,
                                        const DawnShaderModuleSPIRVOptionsDescriptor* optionsDesc) {
    tint::spirv::reader::Options options;
    if (optionsDesc) {
        options.allow_non_uniform_derivatives = optionsDesc->allowNonUniformDerivatives;
    }
    tint::Program program = tint::spirv::reader::Parse(spirv, options);
    if (outMessages != nullptr) {
        DAWN_TRY(outMessages->AddMessages(program.Diagnostics()));
    }
    if (!program.IsValid()) {
        return DAWN_VALIDATION_ERROR("Tint SPIR-V reader failure:\nParser: %s\n",
                                     program.Diagnostics().str());
    }

    return std::move(program);
}
#endif  // TINT_BUILD_SPV_READER

std::vector<uint64_t> GetBindGroupMinBufferSizes(const BindingGroupInfoMap& shaderBindings,
                                                 const BindGroupLayoutBase* layout) {
    std::vector<uint64_t> requiredBufferSizes(layout->GetUnverifiedBufferCount());
    uint32_t packedIdx = 0;

    for (BindingIndex bindingIndex{0}; bindingIndex < layout->GetBufferCount(); ++bindingIndex) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);
        if (bindingInfo.buffer.minBindingSize != 0) {
            // Skip bindings that have minimum buffer size set in the layout
            continue;
        }

        ASSERT(packedIdx < requiredBufferSizes.size());
        const auto& shaderInfo = shaderBindings.find(bindingInfo.binding);
        if (shaderInfo != shaderBindings.end()) {
            requiredBufferSizes[packedIdx] = shaderInfo->second.buffer.minBindingSize;
        } else {
            // We have to include buffers if they are included in the bind group's
            // packed vector. We don't actually need to check these at draw time, so
            // if this is a problem in the future we can optimize it further.
            requiredBufferSizes[packedIdx] = 0;
        }
        ++packedIdx;
    }

    return requiredBufferSizes;
}

MaybeError ValidateCompatibilityOfSingleBindingWithLayout(const DeviceBase* device,
                                                          const BindGroupLayoutBase* layout,
                                                          SingleShaderStage entryPointStage,
                                                          BindingNumber bindingNumber,
                                                          const ShaderBindingInfo& shaderInfo) {
    const BindGroupLayoutInternalBase::BindingMap& layoutBindings = layout->GetBindingMap();

    // An external texture binding found in the shader will later be expanded into multiple
    // bindings at compile time. This expansion will have already happened in the bgl - so
    // the shader and bgl will always mismatch at this point. Expansion info is contained in
    // the bgl object, so we can still verify the bgl used to have an external texture in
    // the slot corresponding to the shader reflection.
    if (shaderInfo.bindingType == BindingInfoType::ExternalTexture) {
        // If an external texture binding used to exist in the bgl, it will be found as a
        // key in the ExternalTextureBindingExpansions map.
        ExternalTextureBindingExpansionMap expansions =
            layout->GetExternalTextureBindingExpansionMap();
        std::map<BindingNumber, dawn_native::ExternalTextureBindingExpansion>::iterator it =
            expansions.find(bindingNumber);
        // TODO(dawn:563): Provide info about the binding types.
        DAWN_INVALID_IF(it == expansions.end(),
                        "Binding type in the shader (texture_external) doesn't match the "
                        "type in the layout.");

        return {};
    }

    const auto& bindingIt = layoutBindings.find(bindingNumber);
    DAWN_INVALID_IF(bindingIt == layoutBindings.end(), "Binding doesn't exist in %s.", layout);

    BindingIndex bindingIndex(bindingIt->second);
    const BindingInfo& layoutInfo = layout->GetBindingInfo(bindingIndex);

    DAWN_INVALID_IF(layoutInfo.bindingType != shaderInfo.bindingType,
                    "Binding type in the shader (%s) doesn't match the type in the layout (%s).",
                    shaderInfo.bindingType, layoutInfo.bindingType);

    ExternalTextureBindingExpansionMap expansions = layout->GetExternalTextureBindingExpansionMap();
    DAWN_INVALID_IF(expansions.find(bindingNumber) != expansions.end(),
                    "Binding type (buffer vs. texture vs. sampler vs. external) doesn't "
                    "match the type in the layout.");

    DAWN_INVALID_IF((layoutInfo.visibility & StageBit(entryPointStage)) == 0,
                    "Entry point's stage (%s) is not in the binding visibility in the layout (%s).",
                    StageBit(entryPointStage), layoutInfo.visibility);

    switch (layoutInfo.bindingType) {
        case BindingInfoType::Texture: {
            DAWN_INVALID_IF(
                layoutInfo.texture.multisampled != shaderInfo.texture.multisampled,
                "Binding multisampled flag (%u) doesn't match the layout's multisampled "
                "flag (%u)",
                layoutInfo.texture.multisampled, shaderInfo.texture.multisampled);

            // TODO(dawn:563): Provide info about the sample types.
            SampleTypeBit requiredType;
            if (layoutInfo.texture.sampleType == kInternalResolveAttachmentSampleType) {
                // If the layout's texture's sample type is kInternalResolveAttachmentSampleType,
                // then the shader's compatible sample types must contain float.
                requiredType = SampleTypeBit::UnfilterableFloat;
            } else {
                requiredType = SampleTypeToSampleTypeBit(layoutInfo.texture.sampleType);
            }

            DAWN_INVALID_IF(!(shaderInfo.texture.compatibleSampleTypes & requiredType),
                            "The sample type in the shader is not compatible with the "
                            "sample type of the layout.");

            DAWN_INVALID_IF(
                layoutInfo.texture.viewDimension != shaderInfo.texture.viewDimension,
                "The shader's binding dimension (%s) doesn't match the shader's binding "
                "dimension (%s).",
                layoutInfo.texture.viewDimension, shaderInfo.texture.viewDimension);
            break;
        }

        case BindingInfoType::StorageTexture: {
            ASSERT(layoutInfo.storageTexture.format != wgpu::TextureFormat::Undefined);
            ASSERT(shaderInfo.storageTexture.format != wgpu::TextureFormat::Undefined);

            DAWN_INVALID_IF(layoutInfo.storageTexture.access != shaderInfo.storageTexture.access,
                            "The layout's binding access (%s) isn't compatible with the shader's "
                            "binding access (%s).",
                            layoutInfo.storageTexture.access, shaderInfo.storageTexture.access);

            DAWN_INVALID_IF(layoutInfo.storageTexture.format != shaderInfo.storageTexture.format,
                            "The layout's binding format (%s) doesn't match the shader's binding "
                            "format (%s).",
                            layoutInfo.storageTexture.format, shaderInfo.storageTexture.format);

            DAWN_INVALID_IF(
                layoutInfo.storageTexture.viewDimension != shaderInfo.storageTexture.viewDimension,
                "The layout's binding dimension (%s) doesn't match the "
                "shader's binding dimension (%s).",
                layoutInfo.storageTexture.viewDimension, shaderInfo.storageTexture.viewDimension);
            break;
        }

        case BindingInfoType::Buffer: {
            // Binding mismatch between shader and bind group is invalid. For example, a
            // writable binding in the shader with a readonly storage buffer in the bind
            // group layout is invalid. For internal usage with internal shaders, a storage
            // binding in the shader with an internal storage buffer in the bind group
            // layout is also valid.
            bool validBindingConversion =
                (layoutInfo.buffer.type == kInternalStorageBufferBinding &&
                 shaderInfo.buffer.type == wgpu::BufferBindingType::Storage);

            DAWN_INVALID_IF(
                layoutInfo.buffer.type != shaderInfo.buffer.type && !validBindingConversion,
                "The buffer type in the shader (%s) is not compatible with the type in the "
                "layout (%s).",
                shaderInfo.buffer.type, layoutInfo.buffer.type);

            DAWN_INVALID_IF(layoutInfo.buffer.minBindingSize != 0 &&
                                shaderInfo.buffer.minBindingSize > layoutInfo.buffer.minBindingSize,
                            "The shader uses more bytes of the buffer (%u) than the layout's "
                            "minBindingSize (%u).",
                            shaderInfo.buffer.minBindingSize, layoutInfo.buffer.minBindingSize);
            break;
        }

        case BindingInfoType::Sampler:
            DAWN_INVALID_IF(
                (layoutInfo.sampler.type == wgpu::SamplerBindingType::Comparison) !=
                    shaderInfo.sampler.isComparison,
                "The sampler type in the shader (comparison: %u) doesn't match the type in "
                "the layout (comparison: %u).",
                shaderInfo.sampler.isComparison,
                layoutInfo.sampler.type == wgpu::SamplerBindingType::Comparison);
            break;

        case BindingInfoType::ExternalTexture: {
            UNREACHABLE();
            break;
        }
    }

    return {};
}
MaybeError ValidateCompatibilityWithBindGroupLayout(DeviceBase* device,
                                                    BindGroupIndex group,
                                                    const EntryPointMetadata& entryPoint,
                                                    const BindGroupLayoutBase* layout) {
    // Iterate over all bindings used by this group in the shader, and find the
    // corresponding binding in the BindGroupLayout, if it exists.
    for (const auto& [bindingId, bindingInfo] : entryPoint.bindings[group]) {
        DAWN_TRY_CONTEXT(ValidateCompatibilityOfSingleBindingWithLayout(
                             device, layout, entryPoint.stage, bindingId, bindingInfo),
                         "validating that the entry-point's declaration for @group(%u) "
                         "@binding(%u) matches %s",
                         static_cast<uint32_t>(group), static_cast<uint32_t>(bindingId), layout);
    }

    return {};
}

ResultOrError<std::unique_ptr<EntryPointMetadata>> ReflectEntryPointUsingTint(
    const DeviceBase* device,
    tint::inspector::Inspector* inspector,
    const tint::inspector::EntryPoint& entryPoint) {
    std::unique_ptr<EntryPointMetadata> metadata = std::make_unique<EntryPointMetadata>();

    // Returns the invalid argument, and if it is true additionally store the formatted
    // error in metadata.infringedLimits. This is to delay the emission of these validation
    // errors until the entry point is used.
#define DelayedInvalidIf(invalid, ...)                                              \
    ([&] {                                                                          \
        if (invalid) {                                                              \
            metadata->infringedLimitErrors.push_back(absl::StrFormat(__VA_ARGS__)); \
        }                                                                           \
        return invalid;                                                             \
    })()

    if (!entryPoint.overrides.empty()) {
        const auto& name2Id = inspector->GetNamedOverrideIds();

        for (auto& c : entryPoint.overrides) {
            auto id = name2Id.at(c.name);
            EntryPointMetadata::Override override = {id, FromTintOverrideType(c.type),
                                                     c.is_initialized};

            std::string identifier = c.is_id_specified ? std::to_string(override.id.value) : c.name;
            metadata->overrides[identifier] = override;

            if (!c.is_initialized) {
                auto [_, inserted] =
                    metadata->uninitializedOverrides.emplace(std::move(identifier));
                // The insertion should have taken place
                ASSERT(inserted);
            } else {
                auto [_, inserted] = metadata->initializedOverrides.emplace(std::move(identifier));
                // The insertion should have taken place
                ASSERT(inserted);
            }
        }
    }

    DAWN_TRY_ASSIGN(metadata->stage, TintPipelineStageToShaderStage(entryPoint.stage));

    if (metadata->stage == SingleShaderStage::Compute) {
        metadata->usesNumWorkgroups = entryPoint.num_workgroups_used;
    }

    const CombinedLimits& limits = device->GetLimits();
    const uint32_t maxVertexAttributes = limits.v1.maxVertexAttributes;
    const uint32_t maxInterStageShaderVariables = limits.v1.maxInterStageShaderVariables;
    const uint32_t maxInterStageShaderComponents = limits.v1.maxInterStageShaderComponents;
    if (metadata->stage == SingleShaderStage::Vertex) {
        for (const auto& inputVar : entryPoint.input_variables) {
            uint32_t unsanitizedLocation = inputVar.location_attribute;
            if (DelayedInvalidIf(unsanitizedLocation >= maxVertexAttributes,
                                 "Vertex input variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u)",
                                 inputVar.name, unsanitizedLocation, maxVertexAttributes)) {
                continue;
            }

            VertexAttributeLocation location(static_cast<uint8_t>(unsanitizedLocation));
            DAWN_TRY_ASSIGN(metadata->vertexInputBaseTypes[location],
                            TintComponentTypeToVertexFormatBaseType(inputVar.component_type));
            metadata->usedVertexInputs.set(location);
        }

        uint32_t totalInterStageShaderComponents = 0;
        for (const auto& outputVar : entryPoint.output_variables) {
            EntryPointMetadata::InterStageVariableInfo variable;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToInterStageComponentType(outputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         outputVar.composition_type));
            DAWN_TRY_ASSIGN(variable.interpolationType,
                            TintInterpolationTypeToInterpolationType(outputVar.interpolation_type));
            DAWN_TRY_ASSIGN(variable.interpolationSampling,
                            TintInterpolationSamplingToInterpolationSamplingType(
                                outputVar.interpolation_sampling));
            totalInterStageShaderComponents += variable.componentCount;

            uint32_t location = outputVar.location_attribute;
            if (DelayedInvalidIf(location >= maxInterStageShaderVariables,
                                 "Vertex output variable \"%s\" has a location (%u) that "
                                 "is greater than or equal to (%u).",
                                 outputVar.name, location, maxInterStageShaderVariables)) {
                continue;
            }

            metadata->usedInterStageVariables.set(location);
            metadata->interStageVariables[location] = variable;
        }

        metadata->totalInterStageShaderComponents = totalInterStageShaderComponents;
        DelayedInvalidIf(totalInterStageShaderComponents > maxInterStageShaderComponents,
                         "Total vertex output components count (%u) exceeds the maximum (%u).",
                         totalInterStageShaderComponents, maxInterStageShaderComponents);
    }

    if (metadata->stage == SingleShaderStage::Fragment) {
        uint32_t totalInterStageShaderComponents = 0;
        for (const auto& inputVar : entryPoint.input_variables) {
            EntryPointMetadata::InterStageVariableInfo variable;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToInterStageComponentType(inputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         inputVar.composition_type));
            DAWN_TRY_ASSIGN(variable.interpolationType,
                            TintInterpolationTypeToInterpolationType(inputVar.interpolation_type));
            DAWN_TRY_ASSIGN(variable.interpolationSampling,
                            TintInterpolationSamplingToInterpolationSamplingType(
                                inputVar.interpolation_sampling));
            totalInterStageShaderComponents += variable.componentCount;

            uint32_t location = inputVar.location_attribute;
            if (DelayedInvalidIf(location >= maxInterStageShaderVariables,
                                 "Fragment input variable \"%s\" has a location (%u) that "
                                 "is greater than or equal to (%u).",
                                 inputVar.name, location, maxInterStageShaderVariables)) {
                continue;
            }

            metadata->usedInterStageVariables.set(location);
            metadata->interStageVariables[location] = variable;
        }

        if (entryPoint.front_facing_used) {
            totalInterStageShaderComponents += 1;
        }
        if (entryPoint.input_sample_mask_used) {
            totalInterStageShaderComponents += 1;
        }
        metadata->usesSampleMaskOutput = entryPoint.output_sample_mask_used;
        if (entryPoint.sample_index_used) {
            totalInterStageShaderComponents += 1;
        }
        metadata->usesFragDepth = entryPoint.frag_depth_used;

        metadata->totalInterStageShaderComponents = totalInterStageShaderComponents;
        DelayedInvalidIf(totalInterStageShaderComponents > maxInterStageShaderComponents,
                         "Total fragment input components count (%u) exceeds the maximum (%u).",
                         totalInterStageShaderComponents, maxInterStageShaderComponents);

        uint32_t maxColorAttachments = limits.v1.maxColorAttachments;
        for (const auto& outputVar : entryPoint.output_variables) {
            EntryPointMetadata::FragmentOutputVariableInfo variable;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToTextureComponentType(outputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         outputVar.composition_type));
            ASSERT(variable.componentCount <= 4);

            uint32_t unsanitizedAttachment = outputVar.location_attribute;
            if (DelayedInvalidIf(unsanitizedAttachment >= maxColorAttachments,
                                 "Fragment output variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u).",
                                 outputVar.name, unsanitizedAttachment, maxColorAttachments)) {
                continue;
            }

            ColorAttachmentIndex attachment(static_cast<uint8_t>(unsanitizedAttachment));
            metadata->fragmentOutputVariables[attachment] = variable;
            metadata->fragmentOutputsWritten.set(attachment);
        }
    }

    for (const tint::inspector::ResourceBinding& resource :
         inspector->GetResourceBindings(entryPoint.name)) {
        ShaderBindingInfo info;

        info.bindingType = TintResourceTypeToBindingInfoType(resource.resource_type);

        switch (info.bindingType) {
            case BindingInfoType::Buffer:
                info.buffer.minBindingSize = resource.size;
                DAWN_TRY_ASSIGN(info.buffer.type,
                                TintResourceTypeToBufferBindingType(resource.resource_type));
                break;
            case BindingInfoType::Sampler:
                switch (resource.resource_type) {
                    case tint::inspector::ResourceBinding::ResourceType::kSampler:
                        info.sampler.isComparison = false;
                        break;
                    case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
                        info.sampler.isComparison = true;
                        break;
                    default:
                        UNREACHABLE();
                }
                break;
            case BindingInfoType::Texture:
                info.texture.viewDimension =
                    TintTextureDimensionToTextureViewDimension(resource.dim);
                if (resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kDepthTexture ||
                    resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture) {
                    info.texture.compatibleSampleTypes = SampleTypeBit::Depth;
                } else {
                    info.texture.compatibleSampleTypes =
                        TintSampledKindToSampleTypeBit(resource.sampled_kind);
                }
                info.texture.multisampled =
                    resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture ||
                    resource.resource_type ==
                        tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture;

                break;
            case BindingInfoType::StorageTexture:
                DAWN_TRY_ASSIGN(info.storageTexture.access,
                                TintResourceTypeToStorageTextureAccess(resource.resource_type));
                info.storageTexture.format = TintImageFormatToTextureFormat(resource.image_format);
                info.storageTexture.viewDimension =
                    TintTextureDimensionToTextureViewDimension(resource.dim);

                DAWN_INVALID_IF(info.storageTexture.format == wgpu::TextureFormat::BGRA8Unorm &&
                                    !device->HasFeature(Feature::BGRA8UnormStorage),
                                "BGRA8Unorm storage textures are not supported if optional feature "
                                "bgra8unorm-storage is not supported.");

                break;
            case BindingInfoType::ExternalTexture:
                break;
            default:
                return DAWN_VALIDATION_ERROR("Unknown binding type in Shader");
        }

        BindingNumber bindingNumber(resource.binding);
        BindGroupIndex bindGroupIndex(resource.bind_group);

        if (DelayedInvalidIf(bindGroupIndex >= kMaxBindGroupsTyped,
                             "The entry-point uses a binding with a group decoration (%u) "
                             "that exceeds the maximum (%u).",
                             resource.bind_group, kMaxBindGroups) ||
            DelayedInvalidIf(bindingNumber >= kMaxBindingsPerBindGroupTyped,
                             "Binding number (%u) exceeds the maxBindingsPerBindGroup limit (%u).",
                             uint32_t(bindingNumber), kMaxBindingsPerBindGroup)) {
            continue;
        }

        const auto& [binding, inserted] =
            metadata->bindings[bindGroupIndex].emplace(bindingNumber, info);
        DAWN_INVALID_IF(!inserted,
                        "Entry-point has a duplicate binding for (group:%u, binding:%u).",
                        resource.binding, resource.bind_group);
    }

    auto samplerTextureUses = inspector->GetSamplerTextureUses(entryPoint.name);
    metadata->samplerTexturePairs.reserve(samplerTextureUses.Length());
    std::transform(samplerTextureUses.begin(), samplerTextureUses.end(),
                   std::back_inserter(metadata->samplerTexturePairs),
                   [](const tint::inspector::SamplerTexturePair& pair) {
                       EntryPointMetadata::SamplerTexturePair result;
                       result.sampler = {BindGroupIndex(pair.sampler_binding_point.group),
                                         BindingNumber(pair.sampler_binding_point.binding)};
                       result.texture = {BindGroupIndex(pair.texture_binding_point.group),
                                         BindingNumber(pair.texture_binding_point.binding)};
                       return result;
                   });

#undef DelayedInvalidIf
    return std::move(metadata);
}

MaybeError ValidateWGSLProgramExtension(const DeviceBase* device,
                                        const WGSLExtensionSet* enabledExtensions,
                                        OwnedCompilationMessages* outMessages) {
    const WGSLExtensionSet& extensionAllowList = device->GetWGSLExtensionAllowList();

    bool hasDisallowedExtension = false;
    tint::diag::List messages;

    for (const std::string& extension : *enabledExtensions) {
        if (extensionAllowList.count(extension)) {
            continue;
        }
        hasDisallowedExtension = true;
        messages.add_error(tint::diag::System::Program,
                           "Extension " + extension + " is not allowed on the Device.");
    }

    if (hasDisallowedExtension) {
        if (outMessages != nullptr) {
            DAWN_TRY(outMessages->AddMessages(messages));
        }
        return DAWN_MAKE_ERROR(InternalErrorType::Validation,
                               "Shader module uses extension(s) not enabled for its device.");
    }

    return {};
}

MaybeError ReflectShaderUsingTint(const DeviceBase* device,
                                  const tint::Program* program,
                                  OwnedCompilationMessages* compilationMessages,
                                  EntryPointMetadataTable* entryPointMetadataTable,
                                  WGSLExtensionSet* enabledWGSLExtensions) {
    ASSERT(program->IsValid());

    tint::inspector::Inspector inspector(program);

    ASSERT(enabledWGSLExtensions->empty());
    auto usedExtensionNames = inspector.GetUsedExtensionNames();
    for (std::string name : usedExtensionNames) {
        enabledWGSLExtensions->insert(name);
    }
    DAWN_TRY(ValidateWGSLProgramExtension(device, enabledWGSLExtensions, compilationMessages));

    std::vector<tint::inspector::EntryPoint> entryPoints = inspector.GetEntryPoints();
    DAWN_INVALID_IF(inspector.has_error(), "Tint Reflection failure: Inspector: %s\n",
                    inspector.error());

    for (const tint::inspector::EntryPoint& entryPoint : entryPoints) {
        std::unique_ptr<EntryPointMetadata> metadata;
        DAWN_TRY_ASSIGN_CONTEXT(metadata,
                                ReflectEntryPointUsingTint(device, &inspector, entryPoint),
                                "processing entry point \"%s\".", entryPoint.name);

        ASSERT(entryPointMetadataTable->count(entryPoint.name) == 0);
        (*entryPointMetadataTable)[entryPoint.name] = std::move(metadata);
    }
    return {};
}
}  // anonymous namespace

ResultOrError<Extent3D> ValidateComputeStageWorkgroupSize(
    const tint::Program& program,
    const char* entryPointName,
    const LimitsForCompilationRequest& limits) {
    tint::inspector::Inspector inspector(&program);
    // At this point the entry point must exist and must have workgroup size values.
    tint::inspector::EntryPoint entryPoint = inspector.GetEntryPoint(entryPointName);
    ASSERT(entryPoint.workgroup_size.has_value());
    const tint::inspector::WorkgroupSize& workgroup_size = entryPoint.workgroup_size.value();

    DAWN_INVALID_IF(workgroup_size.x < 1 || workgroup_size.y < 1 || workgroup_size.z < 1,
                    "Entry-point uses workgroup_size(%u, %u, %u) that are below the "
                    "minimum allowed (1, 1, 1).",
                    workgroup_size.x, workgroup_size.y, workgroup_size.z);

    DAWN_INVALID_IF(workgroup_size.x > limits.maxComputeWorkgroupSizeX ||
                        workgroup_size.y > limits.maxComputeWorkgroupSizeY ||
                        workgroup_size.z > limits.maxComputeWorkgroupSizeZ,
                    "Entry-point uses workgroup_size(%u, %u, %u) that exceeds the "
                    "maximum allowed (%u, %u, %u).",
                    workgroup_size.x, workgroup_size.y, workgroup_size.z,
                    limits.maxComputeWorkgroupSizeX, limits.maxComputeWorkgroupSizeY,
                    limits.maxComputeWorkgroupSizeZ);

    uint64_t numInvocations =
        static_cast<uint64_t>(workgroup_size.x) * workgroup_size.y * workgroup_size.z;
    DAWN_INVALID_IF(numInvocations > limits.maxComputeInvocationsPerWorkgroup,
                    "The total number of workgroup invocations (%u) exceeds the "
                    "maximum allowed (%u).",
                    numInvocations, limits.maxComputeInvocationsPerWorkgroup);

    const size_t workgroupStorageSize = inspector.GetWorkgroupStorageSize(entryPointName);
    DAWN_INVALID_IF(workgroupStorageSize > limits.maxComputeWorkgroupStorageSize,
                    "The total use of workgroup storage (%u bytes) is larger than "
                    "the maximum allowed (%u bytes).",
                    workgroupStorageSize, limits.maxComputeWorkgroupStorageSize);

    return Extent3D{workgroup_size.x, workgroup_size.y, workgroup_size.z};
}

ShaderModuleParseResult::ShaderModuleParseResult() = default;
ShaderModuleParseResult::~ShaderModuleParseResult() = default;

ShaderModuleParseResult::ShaderModuleParseResult(ShaderModuleParseResult&& rhs) = default;

ShaderModuleParseResult& ShaderModuleParseResult::operator=(ShaderModuleParseResult&& rhs) =
    default;

bool ShaderModuleParseResult::HasParsedShader() const {
    return tintProgram != nullptr;
}

// TintSource is a PIMPL container for a tint::Source::File, which needs to be kept alive for as
// long as tint diagnostics are inspected / printed.
class TintSource {
  public:
    template <typename... ARGS>
    explicit TintSource(ARGS&&... args) : file(std::forward<ARGS>(args)...) {}

    tint::Source::File file;
};

MaybeError ValidateAndParseShaderModule(DeviceBase* device,
                                        const ShaderModuleDescriptor* descriptor,
                                        ShaderModuleParseResult* parseResult,
                                        OwnedCompilationMessages* outMessages) {
    ASSERT(parseResult != nullptr);

    const ChainedStruct* chainedDescriptor = descriptor->nextInChain;
    DAWN_INVALID_IF(chainedDescriptor == nullptr,
                    "Shader module descriptor missing chained descriptor");

// A WGSL (or SPIR-V, if enabled) subdescriptor is required, and a Dawn-specific SPIR-V options
// descriptor is allowed when using SPIR-V.
#if TINT_BUILD_SPV_READER
    DAWN_TRY(ValidateSTypes(
        chainedDescriptor,
        {{wgpu::SType::ShaderModuleSPIRVDescriptor, wgpu::SType::ShaderModuleWGSLDescriptor},
         {wgpu::SType::DawnShaderModuleSPIRVOptionsDescriptor}}));
#else
    DAWN_TRY(ValidateSingleSType(chainedDescriptor, wgpu::SType::ShaderModuleWGSLDescriptor));
#endif

    ScopedTintICEHandler scopedICEHandler(device);

    const ShaderModuleWGSLDescriptor* wgslDesc = nullptr;
    FindInChain(chainedDescriptor, &wgslDesc);

    const DawnShaderModuleSPIRVOptionsDescriptor* spirvOptions = nullptr;
    FindInChain(chainedDescriptor, &spirvOptions);

    DAWN_INVALID_IF(wgslDesc != nullptr && spirvOptions != nullptr,
                    "SPIR-V options descriptor not valid with WGSL descriptor");

#if TINT_BUILD_SPV_READER
    const ShaderModuleSPIRVDescriptor* spirvDesc = nullptr;
    FindInChain(chainedDescriptor, &spirvDesc);

    DAWN_INVALID_IF(spirvOptions != nullptr && spirvDesc == nullptr,
                    "SPIR-V options descriptor can only be used with SPIR-V input");

    // We have a temporary toggle to force the SPIRV ingestion to go through a WGSL
    // intermediate step. It is done by switching the spirvDesc for a wgslDesc below.
    ShaderModuleWGSLDescriptor newWgslDesc;
    std::string newWgslCode;
    if (spirvDesc && device->IsToggleEnabled(Toggle::ForceWGSLStep)) {
#if TINT_BUILD_WGSL_WRITER
        std::vector<uint32_t> spirv(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);
        tint::Program program;
        DAWN_TRY_ASSIGN(program, ParseSPIRV(spirv, outMessages, spirvOptions));

        tint::wgsl::writer::Options options;
        auto result = tint::wgsl::writer::Generate(&program, options);
        DAWN_INVALID_IF(!result, "Tint WGSL failure: Generator: %s", result.Failure());

        newWgslCode = std::move(result->wgsl);
        newWgslDesc.code = newWgslCode.c_str();

        spirvDesc = nullptr;
        wgslDesc = &newWgslDesc;
#else
        device->EmitLog(
            WGPULoggingType_Info,
            "Toggle::ForceWGSLStep skipped because TINT_BUILD_WGSL_WRITER is not defined\n");
#endif  // TINT_BUILD_WGSL_WRITER
    }

    if (spirvDesc) {
        DAWN_INVALID_IF(device->IsToggleEnabled(Toggle::DisallowSpirv), "SPIR-V is disallowed.");

        std::vector<uint32_t> spirv(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);
        tint::Program program;
        DAWN_TRY_ASSIGN(program, ParseSPIRV(spirv, outMessages, spirvOptions));
        parseResult->tintProgram = std::make_unique<tint::Program>(std::move(program));

        return {};
    }
#endif  // TINT_BUILD_SPV_READER

    ASSERT(wgslDesc != nullptr);

    auto tintSource = std::make_unique<TintSource>("", wgslDesc->code);

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "// Dumped WGSL:" << std::endl << wgslDesc->code;
        device->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    tint::Program program;
    DAWN_TRY_ASSIGN(program, ParseWGSL(&tintSource->file, outMessages));
    parseResult->tintProgram = std::make_unique<tint::Program>(std::move(program));
    parseResult->tintSource = std::move(tintSource);

    return {};
}

RequiredBufferSizes ComputeRequiredBufferSizesForLayout(const EntryPointMetadata& entryPoint,
                                                        const PipelineLayoutBase* layout) {
    RequiredBufferSizes bufferSizes;
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        bufferSizes[group] = GetBindGroupMinBufferSizes(entryPoint.bindings[group],
                                                        layout->GetBindGroupLayout(group));
    }

    return bufferSizes;
}

ResultOrError<tint::Program> RunTransforms(tint::ast::transform::Manager* transformManager,
                                           const tint::Program* program,
                                           const tint::ast::transform::DataMap& inputs,
                                           tint::ast::transform::DataMap* outputs,
                                           OwnedCompilationMessages* outMessages) {
    tint::ast::transform::DataMap transform_outputs;
    tint::Program result = transformManager->Run(program, inputs, transform_outputs);
    if (outMessages != nullptr) {
        DAWN_TRY(outMessages->AddMessages(result.Diagnostics()));
    }
    DAWN_INVALID_IF(!result.IsValid(), "Tint program failure: %s\n", result.Diagnostics().str());
    if (outputs != nullptr) {
        *outputs = std::move(transform_outputs);
    }
    return std::move(result);
}

MaybeError ValidateCompatibilityWithPipelineLayout(DeviceBase* device,
                                                   const EntryPointMetadata& entryPoint,
                                                   const PipelineLayoutBase* layout) {
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        DAWN_TRY_CONTEXT(ValidateCompatibilityWithBindGroupLayout(
                             device, group, entryPoint, layout->GetBindGroupLayout(group)),
                         "validating the entry-point's compatibility for group %u with %s",
                         static_cast<uint32_t>(group), layout->GetBindGroupLayout(group));
    }

    for (BindGroupIndex group : IterateBitSet(~layout->GetBindGroupLayoutsMask())) {
        DAWN_INVALID_IF(entryPoint.bindings[group].size() > 0,
                        "The entry-point uses bindings in group %u but %s doesn't have a "
                        "BindGroupLayout for this index",
                        static_cast<uint32_t>(group), layout);
    }

    // Validate that filtering samplers are not used with unfilterable textures.
    for (const auto& pair : entryPoint.samplerTexturePairs) {
        const BindGroupLayoutBase* samplerBGL = layout->GetBindGroupLayout(pair.sampler.group);
        const BindingInfo& samplerInfo =
            samplerBGL->GetBindingInfo(samplerBGL->GetBindingIndex(pair.sampler.binding));
        if (samplerInfo.sampler.type != wgpu::SamplerBindingType::Filtering) {
            continue;
        }
        const BindGroupLayoutBase* textureBGL = layout->GetBindGroupLayout(pair.texture.group);
        const BindingInfo& textureInfo =
            textureBGL->GetBindingInfo(textureBGL->GetBindingIndex(pair.texture.binding));

        ASSERT(textureInfo.bindingType != BindingInfoType::Buffer &&
               textureInfo.bindingType != BindingInfoType::Sampler &&
               textureInfo.bindingType != BindingInfoType::StorageTexture);

        if (textureInfo.bindingType != BindingInfoType::Texture) {
            continue;
        }

        // Uint/Sint can't be statically used with a sampler, so they any
        // texture bindings reflected must be float or depth textures. If
        // the shader uses a float/depth texture but the bind group layout
        // specifies a uint/sint texture binding,
        // |ValidateCompatibilityWithBindGroupLayout| will fail since the
        // sampleType does not match.
        ASSERT(textureInfo.texture.sampleType != wgpu::TextureSampleType::Undefined &&
               textureInfo.texture.sampleType != wgpu::TextureSampleType::Uint &&
               textureInfo.texture.sampleType != wgpu::TextureSampleType::Sint);

        DAWN_INVALID_IF(
            textureInfo.texture.sampleType == wgpu::TextureSampleType::UnfilterableFloat,
            "Texture binding (group:%u, binding:%u) is %s but used statically with a sampler "
            "(group:%u, binding:%u) that's %s",
            static_cast<uint32_t>(pair.texture.group), static_cast<uint32_t>(pair.texture.binding),
            wgpu::TextureSampleType::UnfilterableFloat, static_cast<uint32_t>(pair.sampler.group),
            static_cast<uint32_t>(pair.sampler.binding), wgpu::SamplerBindingType::Filtering);
    }

    return {};
}

// ShaderModuleBase

ShaderModuleBase::ShaderModuleBase(DeviceBase* device,
                                   const ShaderModuleDescriptor* descriptor,
                                   ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label), mType(Type::Undefined) {
    ASSERT(descriptor->nextInChain != nullptr);
    const ShaderModuleSPIRVDescriptor* spirvDesc = nullptr;
    FindInChain(descriptor->nextInChain, &spirvDesc);
    const ShaderModuleWGSLDescriptor* wgslDesc = nullptr;
    FindInChain(descriptor->nextInChain, &wgslDesc);
    ASSERT(spirvDesc || wgslDesc);

    if (spirvDesc) {
        mType = Type::Spirv;
        mOriginalSpirv.assign(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);
    } else if (wgslDesc) {
        mType = Type::Wgsl;
        mWgsl = std::string(wgslDesc->code);
    }
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor* descriptor)
    : ShaderModuleBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label)
    : ApiObjectBase(device, tag, label), mType(Type::Undefined) {}

ShaderModuleBase::~ShaderModuleBase() = default;

void ShaderModuleBase::DestroyImpl() {
    Uncache();
}

// static
Ref<ShaderModuleBase> ShaderModuleBase::MakeError(DeviceBase* device, const char* label) {
    return AcquireRef(new ShaderModuleBase(device, ObjectBase::kError, label));
}

ObjectType ShaderModuleBase::GetType() const {
    return ObjectType::ShaderModule;
}

bool ShaderModuleBase::HasEntryPoint(const std::string& entryPoint) const {
    return mEntryPoints.count(entryPoint) > 0;
}

const EntryPointMetadata& ShaderModuleBase::GetEntryPoint(const std::string& entryPoint) const {
    ASSERT(HasEntryPoint(entryPoint));
    return *mEntryPoints.at(entryPoint);
}

size_t ShaderModuleBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    recorder.Record(mType);
    recorder.Record(mOriginalSpirv);
    recorder.Record(mWgsl);
    return recorder.GetContentHash();
}

bool ShaderModuleBase::EqualityFunc::operator()(const ShaderModuleBase* a,
                                                const ShaderModuleBase* b) const {
    return a->mType == b->mType && a->mOriginalSpirv == b->mOriginalSpirv && a->mWgsl == b->mWgsl;
}

const tint::Program* ShaderModuleBase::GetTintProgram() const {
    ASSERT(mTintProgram);
    return mTintProgram.get();
}

void ShaderModuleBase::APIGetCompilationInfo(wgpu::CompilationInfoCallback callback,
                                             void* userdata) {
    if (callback == nullptr) {
        return;
    }

    callback(WGPUCompilationInfoRequestStatus_Success, mCompilationMessages->GetCompilationInfo(),
             userdata);
}

void ShaderModuleBase::InjectCompilationMessages(
    std::unique_ptr<OwnedCompilationMessages> compilationMessages) {
    // TODO(dawn:944): ensure the InjectCompilationMessages is properly handled for shader
    // module returned from cache.
    // InjectCompilationMessages should be called only once for a shader module, after it is
    // created. However currently InjectCompilationMessages may be called on a shader module
    // returned from cache rather than newly created, and violate the rule. We just skip the
    // injection in this case for now, but a proper solution including ensure the cache goes
    // before the validation is required.
    if (mCompilationMessages != nullptr) {
        return;
    }
    // Move the compilationMessages into the shader module and emit the tint errors and warnings
    mCompilationMessages = std::move(compilationMessages);

    // Emit the formatted Tint errors and warnings within the moved compilationMessages
    const std::vector<std::string>& formattedTintMessages =
        mCompilationMessages->GetFormattedTintMessages();
    if (formattedTintMessages.empty()) {
        return;
    }
    std::ostringstream t;
    for (auto pMessage = formattedTintMessages.begin(); pMessage != formattedTintMessages.end();
         pMessage++) {
        if (pMessage != formattedTintMessages.begin()) {
            t << std::endl;
        }
        t << *pMessage;
    }
    this->GetDevice()->EmitLog(WGPULoggingType_Warning, t.str().c_str());
}

OwnedCompilationMessages* ShaderModuleBase::GetCompilationMessages() const {
    return mCompilationMessages.get();
}

MaybeError ShaderModuleBase::InitializeBase(ShaderModuleParseResult* parseResult,
                                            OwnedCompilationMessages* compilationMessages) {
    mTintProgram = std::move(parseResult->tintProgram);
    mTintSource = std::move(parseResult->tintSource);

    DAWN_TRY(ReflectShaderUsingTint(GetDevice(), mTintProgram.get(), compilationMessages,
                                    &mEntryPoints, &mEnabledWGSLExtensions));
    return {};
}

}  // namespace dawn::native
