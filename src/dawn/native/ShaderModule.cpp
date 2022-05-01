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
#include "dawn/common/HashUtils.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/ChainUtils_autogen.h"
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

tint::transform::VertexFormat ToTintVertexFormat(wgpu::VertexFormat format) {
    switch (format) {
        case wgpu::VertexFormat::Uint8x2:
            return tint::transform::VertexFormat::kUint8x2;
        case wgpu::VertexFormat::Uint8x4:
            return tint::transform::VertexFormat::kUint8x4;
        case wgpu::VertexFormat::Sint8x2:
            return tint::transform::VertexFormat::kSint8x2;
        case wgpu::VertexFormat::Sint8x4:
            return tint::transform::VertexFormat::kSint8x4;
        case wgpu::VertexFormat::Unorm8x2:
            return tint::transform::VertexFormat::kUnorm8x2;
        case wgpu::VertexFormat::Unorm8x4:
            return tint::transform::VertexFormat::kUnorm8x4;
        case wgpu::VertexFormat::Snorm8x2:
            return tint::transform::VertexFormat::kSnorm8x2;
        case wgpu::VertexFormat::Snorm8x4:
            return tint::transform::VertexFormat::kSnorm8x4;
        case wgpu::VertexFormat::Uint16x2:
            return tint::transform::VertexFormat::kUint16x2;
        case wgpu::VertexFormat::Uint16x4:
            return tint::transform::VertexFormat::kUint16x4;
        case wgpu::VertexFormat::Sint16x2:
            return tint::transform::VertexFormat::kSint16x2;
        case wgpu::VertexFormat::Sint16x4:
            return tint::transform::VertexFormat::kSint16x4;
        case wgpu::VertexFormat::Unorm16x2:
            return tint::transform::VertexFormat::kUnorm16x2;
        case wgpu::VertexFormat::Unorm16x4:
            return tint::transform::VertexFormat::kUnorm16x4;
        case wgpu::VertexFormat::Snorm16x2:
            return tint::transform::VertexFormat::kSnorm16x2;
        case wgpu::VertexFormat::Snorm16x4:
            return tint::transform::VertexFormat::kSnorm16x4;
        case wgpu::VertexFormat::Float16x2:
            return tint::transform::VertexFormat::kFloat16x2;
        case wgpu::VertexFormat::Float16x4:
            return tint::transform::VertexFormat::kFloat16x4;
        case wgpu::VertexFormat::Float32:
            return tint::transform::VertexFormat::kFloat32;
        case wgpu::VertexFormat::Float32x2:
            return tint::transform::VertexFormat::kFloat32x2;
        case wgpu::VertexFormat::Float32x3:
            return tint::transform::VertexFormat::kFloat32x3;
        case wgpu::VertexFormat::Float32x4:
            return tint::transform::VertexFormat::kFloat32x4;
        case wgpu::VertexFormat::Uint32:
            return tint::transform::VertexFormat::kUint32;
        case wgpu::VertexFormat::Uint32x2:
            return tint::transform::VertexFormat::kUint32x2;
        case wgpu::VertexFormat::Uint32x3:
            return tint::transform::VertexFormat::kUint32x3;
        case wgpu::VertexFormat::Uint32x4:
            return tint::transform::VertexFormat::kUint32x4;
        case wgpu::VertexFormat::Sint32:
            return tint::transform::VertexFormat::kSint32;
        case wgpu::VertexFormat::Sint32x2:
            return tint::transform::VertexFormat::kSint32x2;
        case wgpu::VertexFormat::Sint32x3:
            return tint::transform::VertexFormat::kSint32x3;
        case wgpu::VertexFormat::Sint32x4:
            return tint::transform::VertexFormat::kSint32x4;

        case wgpu::VertexFormat::Undefined:
            break;
    }
    UNREACHABLE();
}

tint::transform::VertexStepMode ToTintVertexStepMode(wgpu::VertexStepMode mode) {
    switch (mode) {
        case wgpu::VertexStepMode::Vertex:
            return tint::transform::VertexStepMode::kVertex;
        case wgpu::VertexStepMode::Instance:
            return tint::transform::VertexStepMode::kInstance;
    }
    UNREACHABLE();
}

ResultOrError<SingleShaderStage> TintPipelineStageToShaderStage(tint::ast::PipelineStage stage) {
    switch (stage) {
        case tint::ast::PipelineStage::kVertex:
            return SingleShaderStage::Vertex;
        case tint::ast::PipelineStage::kFragment:
            return SingleShaderStage::Fragment;
        case tint::ast::PipelineStage::kCompute:
            return SingleShaderStage::Compute;
        case tint::ast::PipelineStage::kNone:
            break;
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

ResultOrError<wgpu::TextureComponentType> TintComponentTypeToTextureComponentType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kFloat:
            return wgpu::TextureComponentType::Float;
        case tint::inspector::ComponentType::kSInt:
            return wgpu::TextureComponentType::Sint;
        case tint::inspector::ComponentType::kUInt:
            return wgpu::TextureComponentType::Uint;
        case tint::inspector::ComponentType::kUnknown:
            return DAWN_VALIDATION_ERROR("Attempted to convert 'Unknown' component type from Tint");
    }
    UNREACHABLE();
}

ResultOrError<VertexFormatBaseType> TintComponentTypeToVertexFormatBaseType(
    tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kFloat:
            return VertexFormatBaseType::Float;
        case tint::inspector::ComponentType::kSInt:
            return VertexFormatBaseType::Sint;
        case tint::inspector::ComponentType::kUInt:
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
        case tint::inspector::ComponentType::kFloat:
            return InterStageComponentType::Float;
        case tint::inspector::ComponentType::kSInt:
            return InterStageComponentType::Sint;
        case tint::inspector::ComponentType::kUInt:
            return InterStageComponentType::Uint;
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

EntryPointMetadata::OverridableConstant::Type FromTintOverridableConstantType(
    tint::inspector::OverridableConstant::Type type) {
    switch (type) {
        case tint::inspector::OverridableConstant::Type::kBool:
            return EntryPointMetadata::OverridableConstant::Type::Boolean;
        case tint::inspector::OverridableConstant::Type::kFloat32:
            return EntryPointMetadata::OverridableConstant::Type::Float32;
        case tint::inspector::OverridableConstant::Type::kInt32:
            return EntryPointMetadata::OverridableConstant::Type::Int32;
        case tint::inspector::OverridableConstant::Type::kUint32:
            return EntryPointMetadata::OverridableConstant::Type::Uint32;
    }
    UNREACHABLE();
}

ResultOrError<tint::Program> ParseWGSL(const tint::Source::File* file,
                                       OwnedCompilationMessages* outMessages) {
#if TINT_BUILD_WGSL_READER
    tint::Program program = tint::reader::wgsl::Parse(file);
    if (outMessages != nullptr) {
        outMessages->AddMessages(program.Diagnostics());
    }
    if (!program.IsValid()) {
        return DAWN_FORMAT_VALIDATION_ERROR("Tint WGSL reader failure:\nParser: %s\nShader:\n%s\n",
                                            program.Diagnostics().str(), file->content.data);
    }

    return std::move(program);
#else
    return DAWN_FORMAT_VALIDATION_ERROR("TINT_BUILD_WGSL_READER is not defined.");
#endif
}

ResultOrError<tint::Program> ParseSPIRV(const std::vector<uint32_t>& spirv,
                                        OwnedCompilationMessages* outMessages) {
#if TINT_BUILD_SPV_READER
    tint::Program program = tint::reader::spirv::Parse(spirv);
    if (outMessages != nullptr) {
        outMessages->AddMessages(program.Diagnostics());
    }
    if (!program.IsValid()) {
        return DAWN_FORMAT_VALIDATION_ERROR("Tint SPIR-V reader failure:\nParser: %s\n",
                                            program.Diagnostics().str());
    }

    return std::move(program);
#else
    return DAWN_FORMAT_VALIDATION_ERROR("TINT_BUILD_SPV_READER is not defined.");

#endif
}

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
    const BindGroupLayoutBase::BindingMap& layoutBindings = layout->GetBindingMap();

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

    // TODO(dawn:563): Provide info about the binding types.
    DAWN_INVALID_IF(
        layoutInfo.bindingType != shaderInfo.bindingType,
        "Binding type (buffer vs. texture vs. sampler vs. external) doesn't match the type "
        "in the layout.");

    ExternalTextureBindingExpansionMap expansions = layout->GetExternalTextureBindingExpansionMap();
    DAWN_INVALID_IF(expansions.find(bindingNumber) != expansions.end(),
                    "Binding type (buffer vs. texture vs. sampler vs. external) doesn't "
                    "match the type in the layout.");

    // TODO(dawn:563): Provide info about the visibility.
    DAWN_INVALID_IF((layoutInfo.visibility & StageBit(entryPointStage)) == 0,
                    "Entry point's stage is not in the binding visibility in the layout (%s)",
                    layoutInfo.visibility);

    switch (layoutInfo.bindingType) {
        case BindingInfoType::Texture: {
            DAWN_INVALID_IF(
                layoutInfo.texture.multisampled != shaderInfo.texture.multisampled,
                "Binding multisampled flag (%u) doesn't match the layout's multisampled "
                "flag (%u)",
                layoutInfo.texture.multisampled, shaderInfo.texture.multisampled);

            // TODO(dawn:563): Provide info about the sample types.
            DAWN_INVALID_IF((SampleTypeToSampleTypeBit(layoutInfo.texture.sampleType) &
                             shaderInfo.texture.compatibleSampleTypes) == 0,
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
    const CombinedLimits& limits = device->GetLimits();
    constexpr uint32_t kMaxInterStageShaderLocation = kMaxInterStageShaderVariables - 1;

    std::unique_ptr<EntryPointMetadata> metadata = std::make_unique<EntryPointMetadata>();

    // Returns the invalid argument, and if it is true additionally store the formatted
    // error in metadata.infringedLimits. This is to delay the emission of these validation
    // errors until the entry point is used.
#define DelayedInvalidIf(invalid, ...)                                              \
    ([&]() {                                                                        \
        if (invalid) {                                                              \
            metadata->infringedLimitErrors.push_back(absl::StrFormat(__VA_ARGS__)); \
        }                                                                           \
        return invalid;                                                             \
    })()

    if (!entryPoint.overridable_constants.empty()) {
        DAWN_INVALID_IF(device->IsToggleEnabled(Toggle::DisallowUnsafeAPIs),
                        "Pipeline overridable constants are disallowed because they "
                        "are partially implemented.");

        const auto& name2Id = inspector->GetConstantNameToIdMap();
        const auto& id2Scalar = inspector->GetConstantIDs();

        for (auto& c : entryPoint.overridable_constants) {
            uint32_t id = name2Id.at(c.name);
            OverridableConstantScalar defaultValue;
            if (c.is_initialized) {
                // if it is initialized, the scalar must exist
                const auto& scalar = id2Scalar.at(id);
                if (scalar.IsBool()) {
                    defaultValue.b = scalar.AsBool();
                } else if (scalar.IsU32()) {
                    defaultValue.u32 = scalar.AsU32();
                } else if (scalar.IsI32()) {
                    defaultValue.i32 = scalar.AsI32();
                } else if (scalar.IsFloat()) {
                    defaultValue.f32 = scalar.AsFloat();
                } else {
                    UNREACHABLE();
                }
            }
            EntryPointMetadata::OverridableConstant constant = {
                id, FromTintOverridableConstantType(c.type), c.is_initialized, defaultValue};

            std::string identifier =
                c.is_numeric_id_specified ? std::to_string(constant.id) : c.name;
            metadata->overridableConstants[identifier] = constant;

            if (!c.is_initialized) {
                auto [_, inserted] =
                    metadata->uninitializedOverridableConstants.emplace(std::move(identifier));
                // The insertion should have taken place
                ASSERT(inserted);
            } else {
                auto [_, inserted] =
                    metadata->initializedOverridableConstants.emplace(std::move(identifier));
                // The insertion should have taken place
                ASSERT(inserted);
            }
        }
    }

    DAWN_TRY_ASSIGN(metadata->stage, TintPipelineStageToShaderStage(entryPoint.stage));

    if (metadata->stage == SingleShaderStage::Compute) {
        DelayedInvalidIf(entryPoint.workgroup_size_x > limits.v1.maxComputeWorkgroupSizeX ||
                             entryPoint.workgroup_size_y > limits.v1.maxComputeWorkgroupSizeY ||
                             entryPoint.workgroup_size_z > limits.v1.maxComputeWorkgroupSizeZ,
                         "Entry-point uses workgroup_size(%u, %u, %u) that exceeds the "
                         "maximum allowed (%u, %u, %u).",
                         entryPoint.workgroup_size_x, entryPoint.workgroup_size_y,
                         entryPoint.workgroup_size_z, limits.v1.maxComputeWorkgroupSizeX,
                         limits.v1.maxComputeWorkgroupSizeY, limits.v1.maxComputeWorkgroupSizeZ);

        // Dimensions have already been validated against their individual limits above.
        // Cast to uint64_t to avoid overflow in this multiplication.
        uint64_t numInvocations = static_cast<uint64_t>(entryPoint.workgroup_size_x) *
                                  entryPoint.workgroup_size_y * entryPoint.workgroup_size_z;
        DelayedInvalidIf(numInvocations > limits.v1.maxComputeInvocationsPerWorkgroup,
                         "The total number of workgroup invocations (%u) exceeds the "
                         "maximum allowed (%u).",
                         numInvocations, limits.v1.maxComputeInvocationsPerWorkgroup);

        const size_t workgroupStorageSize = inspector->GetWorkgroupStorageSize(entryPoint.name);
        DelayedInvalidIf(workgroupStorageSize > limits.v1.maxComputeWorkgroupStorageSize,
                         "The total use of workgroup storage (%u bytes) is larger than "
                         "the maximum allowed (%u bytes).",
                         workgroupStorageSize, limits.v1.maxComputeWorkgroupStorageSize);

        metadata->localWorkgroupSize.x = entryPoint.workgroup_size_x;
        metadata->localWorkgroupSize.y = entryPoint.workgroup_size_y;
        metadata->localWorkgroupSize.z = entryPoint.workgroup_size_z;

        metadata->usesNumWorkgroups = entryPoint.num_workgroups_used;
    }

    if (metadata->stage == SingleShaderStage::Vertex) {
        for (const auto& inputVar : entryPoint.input_variables) {
            uint32_t unsanitizedLocation = inputVar.location_decoration;
            if (DelayedInvalidIf(unsanitizedLocation >= kMaxVertexAttributes,
                                 "Vertex input variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u)",
                                 inputVar.name, unsanitizedLocation, kMaxVertexAttributes)) {
                continue;
            }

            VertexAttributeLocation location(static_cast<uint8_t>(unsanitizedLocation));
            DAWN_TRY_ASSIGN(metadata->vertexInputBaseTypes[location],
                            TintComponentTypeToVertexFormatBaseType(inputVar.component_type));
            metadata->usedVertexInputs.set(location);
        }

        // [[position]] must be declared in a vertex shader but is not exposed as an
        // output variable by Tint so we directly add its components to the total.
        uint32_t totalInterStageShaderComponents = 4;
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

            uint32_t location = outputVar.location_decoration;
            if (DelayedInvalidIf(location > kMaxInterStageShaderLocation,
                                 "Vertex output variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u).",
                                 outputVar.name, location, kMaxInterStageShaderLocation)) {
                continue;
            }

            metadata->usedInterStageVariables.set(location);
            metadata->interStageVariables[location] = variable;
        }

        DelayedInvalidIf(totalInterStageShaderComponents > kMaxInterStageShaderComponents,
                         "Total vertex output components count (%u) exceeds the maximum (%u).",
                         totalInterStageShaderComponents, kMaxInterStageShaderComponents);
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

            uint32_t location = inputVar.location_decoration;
            if (DelayedInvalidIf(location > kMaxInterStageShaderLocation,
                                 "Fragment input variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u).",
                                 inputVar.name, location, kMaxInterStageShaderLocation)) {
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
        if (entryPoint.sample_index_used) {
            totalInterStageShaderComponents += 1;
        }
        if (entryPoint.input_position_used) {
            totalInterStageShaderComponents += 4;
        }

        DelayedInvalidIf(totalInterStageShaderComponents > kMaxInterStageShaderComponents,
                         "Total fragment input components count (%u) exceeds the maximum (%u).",
                         totalInterStageShaderComponents, kMaxInterStageShaderComponents);

        for (const auto& outputVar : entryPoint.output_variables) {
            EntryPointMetadata::FragmentOutputVariableInfo variable;
            DAWN_TRY_ASSIGN(variable.baseType,
                            TintComponentTypeToTextureComponentType(outputVar.component_type));
            DAWN_TRY_ASSIGN(variable.componentCount, TintCompositionTypeToInterStageComponentCount(
                                                         outputVar.composition_type));
            ASSERT(variable.componentCount <= 4);

            uint32_t unsanitizedAttachment = outputVar.location_decoration;
            if (DelayedInvalidIf(unsanitizedAttachment >= kMaxColorAttachments,
                                 "Fragment output variable \"%s\" has a location (%u) that "
                                 "exceeds the maximum (%u).",
                                 outputVar.name, unsanitizedAttachment, kMaxColorAttachments)) {
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
                info.buffer.minBindingSize = resource.size_no_padding;
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
            DelayedInvalidIf(bindingNumber > kMaxBindingNumberTyped,
                             "Binding number (%u) exceeds the maximum binding number (%u).",
                             uint32_t(bindingNumber), uint32_t(kMaxBindingNumberTyped))) {
            continue;
        }

        const auto& [binding, inserted] =
            metadata->bindings[bindGroupIndex].emplace(bindingNumber, info);
        DAWN_INVALID_IF(!inserted,
                        "Entry-point has a duplicate binding for (group:%u, binding:%u).",
                        resource.binding, resource.bind_group);
    }

    std::vector<tint::inspector::SamplerTexturePair> samplerTextureUses =
        inspector->GetSamplerTextureUses(entryPoint.name);
    metadata->samplerTexturePairs.reserve(samplerTextureUses.size());
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

ResultOrError<EntryPointMetadataTable> ReflectShaderUsingTint(const DeviceBase* device,
                                                              const tint::Program* program) {
    ASSERT(program->IsValid());

    tint::inspector::Inspector inspector(program);
    std::vector<tint::inspector::EntryPoint> entryPoints = inspector.GetEntryPoints();
    DAWN_INVALID_IF(inspector.has_error(), "Tint Reflection failure: Inspector: %s\n",
                    inspector.error());

    EntryPointMetadataTable result;

    for (const tint::inspector::EntryPoint& entryPoint : entryPoints) {
        std::unique_ptr<EntryPointMetadata> metadata;
        DAWN_TRY_ASSIGN_CONTEXT(metadata,
                                ReflectEntryPointUsingTint(device, &inspector, entryPoint),
                                "processing entry point \"%s\".", entryPoint.name);

        ASSERT(result.count(entryPoint.name) == 0);
        result[entryPoint.name] = std::move(metadata);
    }
    return std::move(result);
}
}  // anonymous namespace

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

MaybeError ValidateShaderModuleDescriptor(DeviceBase* device,
                                          const ShaderModuleDescriptor* descriptor,
                                          ShaderModuleParseResult* parseResult,
                                          OwnedCompilationMessages* outMessages) {
    ASSERT(parseResult != nullptr);

    const ChainedStruct* chainedDescriptor = descriptor->nextInChain;
    DAWN_INVALID_IF(chainedDescriptor == nullptr,
                    "Shader module descriptor missing chained descriptor");

    // For now only a single SPIRV or WGSL subdescriptor is allowed.
    DAWN_TRY(ValidateSingleSType(chainedDescriptor, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                 wgpu::SType::ShaderModuleWGSLDescriptor));

    ScopedTintICEHandler scopedICEHandler(device);

    const ShaderModuleSPIRVDescriptor* spirvDesc = nullptr;
    FindInChain(chainedDescriptor, &spirvDesc);
    const ShaderModuleWGSLDescriptor* wgslDesc = nullptr;
    FindInChain(chainedDescriptor, &wgslDesc);

    // We have a temporary toggle to force the SPIRV ingestion to go through a WGSL
    // intermediate step. It is done by switching the spirvDesc for a wgslDesc below.
    ShaderModuleWGSLDescriptor newWgslDesc;
    std::string newWgslCode;
    if (spirvDesc && device->IsToggleEnabled(Toggle::ForceWGSLStep)) {
#if TINT_BUILD_WGSL_WRITER
        std::vector<uint32_t> spirv(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);
        tint::Program program;
        DAWN_TRY_ASSIGN(program, ParseSPIRV(spirv, outMessages));

        tint::writer::wgsl::Options options;
        auto result = tint::writer::wgsl::Generate(&program, options);
        DAWN_INVALID_IF(!result.success, "Tint WGSL failure: Generator: %s", result.error);

        newWgslCode = std::move(result.wgsl);
        newWgslDesc.source = newWgslCode.c_str();

        spirvDesc = nullptr;
        wgslDesc = &newWgslDesc;
#else
        device->EmitLog(
            WGPULoggingType_Info,
            "Toggle::ForceWGSLStep skipped because TINT_BUILD_WGSL_WRITER is not defined\n");
#endif
    }

    if (spirvDesc) {
        DAWN_INVALID_IF(device->IsToggleEnabled(Toggle::DisallowSpirv), "SPIR-V is disallowed.");

        std::vector<uint32_t> spirv(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);
        tint::Program program;
        DAWN_TRY_ASSIGN(program, ParseSPIRV(spirv, outMessages));
        parseResult->tintProgram = std::make_unique<tint::Program>(std::move(program));
    } else if (wgslDesc) {
        auto tintSource = std::make_unique<TintSource>("", wgslDesc->source);

        if (device->IsToggleEnabled(Toggle::DumpShaders)) {
            std::ostringstream dumpedMsg;
            dumpedMsg << "// Dumped WGSL:" << std::endl << wgslDesc->source;
            device->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
        }

        tint::Program program;
        DAWN_TRY_ASSIGN(program, ParseWGSL(&tintSource->file, outMessages));
        parseResult->tintProgram = std::make_unique<tint::Program>(std::move(program));
        parseResult->tintSource = std::move(tintSource);
    }

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

ResultOrError<tint::Program> RunTransforms(tint::transform::Transform* transform,
                                           const tint::Program* program,
                                           const tint::transform::DataMap& inputs,
                                           tint::transform::DataMap* outputs,
                                           OwnedCompilationMessages* outMessages) {
    tint::transform::Output output = transform->Run(program, inputs);
    if (outMessages != nullptr) {
        outMessages->AddMessages(output.program.Diagnostics());
    }
    DAWN_INVALID_IF(!output.program.IsValid(), "Tint program failure: %s\n",
                    output.program.Diagnostics().str());
    if (outputs != nullptr) {
        *outputs = std::move(output.data);
    }
    return std::move(output.program);
}

void AddVertexPullingTransformConfig(const RenderPipelineBase& renderPipeline,
                                     const std::string& entryPoint,
                                     BindGroupIndex pullingBufferBindingSet,
                                     tint::transform::DataMap* transformInputs) {
    tint::transform::VertexPulling::Config cfg;
    cfg.entry_point_name = entryPoint;
    cfg.pulling_group = static_cast<uint32_t>(pullingBufferBindingSet);

    cfg.vertex_state.resize(renderPipeline.GetVertexBufferCount());
    for (VertexBufferSlot slot : IterateBitSet(renderPipeline.GetVertexBufferSlotsUsed())) {
        const VertexBufferInfo& dawnInfo = renderPipeline.GetVertexBuffer(slot);
        tint::transform::VertexBufferLayoutDescriptor* tintInfo =
            &cfg.vertex_state[static_cast<uint8_t>(slot)];

        tintInfo->array_stride = dawnInfo.arrayStride;
        tintInfo->step_mode = ToTintVertexStepMode(dawnInfo.stepMode);
    }

    for (VertexAttributeLocation location :
         IterateBitSet(renderPipeline.GetAttributeLocationsUsed())) {
        const VertexAttributeInfo& dawnInfo = renderPipeline.GetAttribute(location);
        tint::transform::VertexAttributeDescriptor tintInfo;
        tintInfo.format = ToTintVertexFormat(dawnInfo.format);
        tintInfo.offset = dawnInfo.offset;
        tintInfo.shader_location = static_cast<uint32_t>(static_cast<uint8_t>(location));

        uint8_t vertexBufferSlot = static_cast<uint8_t>(dawnInfo.vertexBufferSlot);
        cfg.vertex_state[vertexBufferSlot].attributes.push_back(tintInfo);
    }

    transformInputs->Add<tint::transform::VertexPulling::Config>(cfg);
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

        // Uint/sint can't be statically used with a sampler, so they any
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
        mWgsl = std::string(wgslDesc->source);
    }
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor* descriptor)
    : ShaderModuleBase(device, descriptor, kUntrackedByDevice) {
    TrackInDevice();
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device)
    : ApiObjectBase(device, kLabelNotImplemented) {
    TrackInDevice();
}

ShaderModuleBase::ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag), mType(Type::Undefined) {}

ShaderModuleBase::~ShaderModuleBase() = default;

void ShaderModuleBase::DestroyImpl() {
    if (IsCachedReference()) {
        // Do not uncache the actual cached object if we are a blueprint.
        GetDevice()->UncacheShaderModule(this);
    }
}

// static
Ref<ShaderModuleBase> ShaderModuleBase::MakeError(DeviceBase* device) {
    return AcquireRef(new ShaderModuleBase(device, ObjectBase::kError));
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

// static
void ShaderModuleBase::AddExternalTextureTransform(const PipelineLayoutBase* layout,
                                                   tint::transform::Manager* transformManager,
                                                   tint::transform::DataMap* transformInputs) {
    tint::transform::MultiplanarExternalTexture::BindingsMap newBindingsMap;
    for (BindGroupIndex i : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayoutBase* bgl = layout->GetBindGroupLayout(i);

        for (const auto& expansion : bgl->GetExternalTextureBindingExpansionMap()) {
            newBindingsMap[{static_cast<uint32_t>(i),
                            static_cast<uint32_t>(expansion.second.plane0)}] = {
                {static_cast<uint32_t>(i), static_cast<uint32_t>(expansion.second.plane1)},
                {static_cast<uint32_t>(i), static_cast<uint32_t>(expansion.second.params)}};
        }
    }

    if (!newBindingsMap.empty()) {
        transformManager->Add<tint::transform::MultiplanarExternalTexture>();
        transformInputs->Add<tint::transform::MultiplanarExternalTexture::NewBindingPoints>(
            newBindingsMap);
    }
}

MaybeError ShaderModuleBase::InitializeBase(ShaderModuleParseResult* parseResult) {
    mTintProgram = std::move(parseResult->tintProgram);
    mTintSource = std::move(parseResult->tintSource);

    DAWN_TRY_ASSIGN(mEntryPoints, ReflectShaderUsingTint(GetDevice(), mTintProgram.get()));
    return {};
}

size_t PipelineLayoutEntryPointPairHashFunc::operator()(
    const PipelineLayoutEntryPointPair& pair) const {
    size_t hash = 0;
    HashCombine(&hash, pair.first, pair.second);
    return hash;
}

}  // namespace dawn::native
