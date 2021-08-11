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

#include "dawn_native/ShaderModule.h"

#include "common/Constants.h"
#include "common/HashUtils.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/ChainUtils_autogen.h"
#include "dawn_native/CompilationMessages.h"
#include "dawn_native/Device.h"
#include "dawn_native/ObjectContentHasher.h"
#include "dawn_native/Pipeline.h"
#include "dawn_native/PipelineLayout.h"
#include "dawn_native/RenderPipeline.h"
#include "dawn_native/TintUtils.h"

#include <tint/tint.h>

#include <sstream>

namespace dawn_native {

    namespace {

        std::string GetShaderDeclarationString(BindGroupIndex group, BindingNumber binding) {
            std::ostringstream ostream;
            ostream << "the shader module declaration at set " << static_cast<uint32_t>(group)
                    << " binding " << static_cast<uint32_t>(binding);
            return ostream.str();
        }

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
        }

        ResultOrError<SingleShaderStage> TintPipelineStageToShaderStage(
            tint::ast::PipelineStage stage) {
            switch (stage) {
                case tint::ast::PipelineStage::kVertex:
                    return SingleShaderStage::Vertex;
                case tint::ast::PipelineStage::kFragment:
                    return SingleShaderStage::Fragment;
                case tint::ast::PipelineStage::kCompute:
                    return SingleShaderStage::Compute;
                case tint::ast::PipelineStage::kNone:
                    UNREACHABLE();
            }
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
                case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageTexture:
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
            tint::inspector::ResourceBinding::ImageFormat format) {
            switch (format) {
                case tint::inspector::ResourceBinding::ImageFormat::kR8Unorm:
                    return wgpu::TextureFormat::R8Unorm;
                case tint::inspector::ResourceBinding::ImageFormat::kR8Snorm:
                    return wgpu::TextureFormat::R8Snorm;
                case tint::inspector::ResourceBinding::ImageFormat::kR8Uint:
                    return wgpu::TextureFormat::R8Uint;
                case tint::inspector::ResourceBinding::ImageFormat::kR8Sint:
                    return wgpu::TextureFormat::R8Sint;
                case tint::inspector::ResourceBinding::ImageFormat::kR16Uint:
                    return wgpu::TextureFormat::R16Uint;
                case tint::inspector::ResourceBinding::ImageFormat::kR16Sint:
                    return wgpu::TextureFormat::R16Sint;
                case tint::inspector::ResourceBinding::ImageFormat::kR16Float:
                    return wgpu::TextureFormat::R16Float;
                case tint::inspector::ResourceBinding::ImageFormat::kRg8Unorm:
                    return wgpu::TextureFormat::RG8Unorm;
                case tint::inspector::ResourceBinding::ImageFormat::kRg8Snorm:
                    return wgpu::TextureFormat::RG8Snorm;
                case tint::inspector::ResourceBinding::ImageFormat::kRg8Uint:
                    return wgpu::TextureFormat::RG8Uint;
                case tint::inspector::ResourceBinding::ImageFormat::kRg8Sint:
                    return wgpu::TextureFormat::RG8Sint;
                case tint::inspector::ResourceBinding::ImageFormat::kR32Uint:
                    return wgpu::TextureFormat::R32Uint;
                case tint::inspector::ResourceBinding::ImageFormat::kR32Sint:
                    return wgpu::TextureFormat::R32Sint;
                case tint::inspector::ResourceBinding::ImageFormat::kR32Float:
                    return wgpu::TextureFormat::R32Float;
                case tint::inspector::ResourceBinding::ImageFormat::kRg16Uint:
                    return wgpu::TextureFormat::RG16Uint;
                case tint::inspector::ResourceBinding::ImageFormat::kRg16Sint:
                    return wgpu::TextureFormat::RG16Sint;
                case tint::inspector::ResourceBinding::ImageFormat::kRg16Float:
                    return wgpu::TextureFormat::RG16Float;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba8Unorm:
                    return wgpu::TextureFormat::RGBA8Unorm;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba8UnormSrgb:
                    return wgpu::TextureFormat::RGBA8UnormSrgb;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba8Snorm:
                    return wgpu::TextureFormat::RGBA8Snorm;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba8Uint:
                    return wgpu::TextureFormat::RGBA8Uint;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba8Sint:
                    return wgpu::TextureFormat::RGBA8Sint;
                case tint::inspector::ResourceBinding::ImageFormat::kBgra8Unorm:
                    return wgpu::TextureFormat::BGRA8Unorm;
                case tint::inspector::ResourceBinding::ImageFormat::kBgra8UnormSrgb:
                    return wgpu::TextureFormat::BGRA8UnormSrgb;
                case tint::inspector::ResourceBinding::ImageFormat::kRgb10A2Unorm:
                    return wgpu::TextureFormat::RGB10A2Unorm;
                case tint::inspector::ResourceBinding::ImageFormat::kRg11B10Float:
                    return wgpu::TextureFormat::RG11B10Ufloat;
                case tint::inspector::ResourceBinding::ImageFormat::kRg32Uint:
                    return wgpu::TextureFormat::RG32Uint;
                case tint::inspector::ResourceBinding::ImageFormat::kRg32Sint:
                    return wgpu::TextureFormat::RG32Sint;
                case tint::inspector::ResourceBinding::ImageFormat::kRg32Float:
                    return wgpu::TextureFormat::RG32Float;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba16Uint:
                    return wgpu::TextureFormat::RGBA16Uint;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba16Sint:
                    return wgpu::TextureFormat::RGBA16Sint;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba16Float:
                    return wgpu::TextureFormat::RGBA16Float;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba32Uint:
                    return wgpu::TextureFormat::RGBA32Uint;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba32Sint:
                    return wgpu::TextureFormat::RGBA32Sint;
                case tint::inspector::ResourceBinding::ImageFormat::kRgba32Float:
                    return wgpu::TextureFormat::RGBA32Float;
                case tint::inspector::ResourceBinding::ImageFormat::kNone:
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
        }

        SampleTypeBit TintSampledKindToSampleTypeBit(
            tint::inspector::ResourceBinding::SampledKind s) {
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
                    return DAWN_VALIDATION_ERROR(
                        "Attempted to convert 'Unknown' component type from Tint");
            }
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
                    return DAWN_VALIDATION_ERROR(
                        "Attempted to convert 'Unknown' component type from Tint");
            }
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
        }

        ResultOrError<wgpu::StorageTextureAccess> TintResourceTypeToStorageTextureAccess(
            tint::inspector::ResourceBinding::ResourceType resource_type) {
            switch (resource_type) {
                case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageTexture:
                    return wgpu::StorageTextureAccess::ReadOnly;
                case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
                    return wgpu::StorageTextureAccess::WriteOnly;
                default:
                    return DAWN_VALIDATION_ERROR(
                        "Attempted to convert non-storage texture resource type");
            }
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
                    return DAWN_VALIDATION_ERROR(
                        "Attempted to convert 'Unknown' component type from Tint");
            }
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
                    return DAWN_VALIDATION_ERROR(
                        "Attempt to convert 'Unknown' composition type from Tint");
            }
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
        }

        ResultOrError<tint::Program> ParseWGSL(const tint::Source::File* file,
                                               OwnedCompilationMessages* outMessages) {
            std::ostringstream errorStream;
            errorStream << "Tint WGSL reader failure:" << std::endl;

            tint::Program program = tint::reader::wgsl::Parse(file);
            if (outMessages != nullptr) {
                outMessages->AddMessages(program.Diagnostics());
            }
            if (!program.IsValid()) {
                auto err = program.Diagnostics().str();
                errorStream << "Parser: " << err << std::endl
                            << "Shader: " << std::endl
                            << file->content << std::endl;
                return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
            }

            return std::move(program);
        }

        ResultOrError<tint::Program> ParseSPIRV(const std::vector<uint32_t>& spirv,
                                                OwnedCompilationMessages* outMessages) {
            std::ostringstream errorStream;
            errorStream << "Tint SPIRV reader failure:" << std::endl;

            tint::Program program = tint::reader::spirv::Parse(spirv);
            if (outMessages != nullptr) {
                outMessages->AddMessages(program.Diagnostics());
            }
            if (!program.IsValid()) {
                auto err = program.Diagnostics().str();
                errorStream << "Parser: " << err << std::endl;
                return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
            }

            return std::move(program);
        }

        std::vector<uint64_t> GetBindGroupMinBufferSizes(
            const EntryPointMetadata::BindingGroupInfoMap& shaderBindings,
            const BindGroupLayoutBase* layout) {
            std::vector<uint64_t> requiredBufferSizes(layout->GetUnverifiedBufferCount());
            uint32_t packedIdx = 0;

            for (BindingIndex bindingIndex{0}; bindingIndex < layout->GetBufferCount();
                 ++bindingIndex) {
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

        MaybeError ValidateCompatibilityWithBindGroupLayout(DeviceBase* device,
                                                            BindGroupIndex group,
                                                            const EntryPointMetadata& entryPoint,
                                                            const BindGroupLayoutBase* layout) {
            const BindGroupLayoutBase::BindingMap& layoutBindings = layout->GetBindingMap();

            // Iterate over all bindings used by this group in the shader, and find the
            // corresponding binding in the BindGroupLayout, if it exists.
            for (const auto& it : entryPoint.bindings[group]) {
                BindingNumber bindingNumber = it.first;
                const EntryPointMetadata::ShaderBindingInfo& shaderInfo = it.second;

                const auto& bindingIt = layoutBindings.find(bindingNumber);
                if (bindingIt == layoutBindings.end()) {
                    return DAWN_VALIDATION_ERROR("Missing bind group layout entry for " +
                                                 GetShaderDeclarationString(group, bindingNumber));
                }
                BindingIndex bindingIndex(bindingIt->second);
                const BindingInfo& layoutInfo = layout->GetBindingInfo(bindingIndex);

                if (layoutInfo.bindingType != shaderInfo.bindingType) {
                    return DAWN_VALIDATION_ERROR(
                        "The binding type of the bind group layout entry conflicts " +
                        GetShaderDeclarationString(group, bindingNumber));
                }

                if ((layoutInfo.visibility & StageBit(entryPoint.stage)) == 0) {
                    return DAWN_VALIDATION_ERROR("The bind group layout entry for " +
                                                 GetShaderDeclarationString(group, bindingNumber) +
                                                 " is not visible for the shader stage");
                }

                switch (layoutInfo.bindingType) {
                    case BindingInfoType::Texture: {
                        if (layoutInfo.texture.multisampled != shaderInfo.texture.multisampled) {
                            return DAWN_VALIDATION_ERROR(
                                "The texture multisampled flag of the bind group layout entry is "
                                "different from " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }

                        if ((SampleTypeToSampleTypeBit(layoutInfo.texture.sampleType) &
                             shaderInfo.texture.compatibleSampleTypes) == 0) {
                            return DAWN_VALIDATION_ERROR(
                                "The texture sampleType of the bind group layout entry is "
                                "not compatible with " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }

                        if (layoutInfo.texture.viewDimension != shaderInfo.texture.viewDimension) {
                            return DAWN_VALIDATION_ERROR(
                                "The texture viewDimension of the bind group layout entry is "
                                "different "
                                "from " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }
                        break;
                    }

                    case BindingInfoType::StorageTexture: {
                        ASSERT(layoutInfo.storageTexture.format != wgpu::TextureFormat::Undefined);
                        ASSERT(shaderInfo.storageTexture.format != wgpu::TextureFormat::Undefined);

                        if (layoutInfo.storageTexture.access != shaderInfo.storageTexture.access) {
                            return DAWN_VALIDATION_ERROR(
                                "The storageTexture access mode of the bind group layout entry is "
                                "different from " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }

                        if (layoutInfo.storageTexture.format != shaderInfo.storageTexture.format) {
                            return DAWN_VALIDATION_ERROR(
                                "The storageTexture format of the bind group layout entry is "
                                "different from " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }
                        if (layoutInfo.storageTexture.viewDimension !=
                            shaderInfo.storageTexture.viewDimension) {
                            return DAWN_VALIDATION_ERROR(
                                "The storageTexture viewDimension of the bind group layout entry "
                                "is different from " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }
                        break;
                    }

                    case BindingInfoType::ExternalTexture: {
                        if (shaderInfo.bindingType != BindingInfoType::ExternalTexture) {
                            return DAWN_VALIDATION_ERROR(
                                "The external texture bind group layout entry conflicts with " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }
                        break;
                    }

                    case BindingInfoType::Buffer: {
                        // Binding mismatch between shader and bind group is invalid. For example, a
                        // writable binding in the shader with a readonly storage buffer in the bind
                        // group layout is invalid. However, a readonly binding in the shader with a
                        // writable storage buffer in the bind group layout is valid, a storage
                        // binding in the shader with an internal storage buffer in the bind group
                        // layout is also valid.
                        bool validBindingConversion =
                            (layoutInfo.buffer.type == wgpu::BufferBindingType::Storage &&
                             shaderInfo.buffer.type == wgpu::BufferBindingType::ReadOnlyStorage) ||
                            (layoutInfo.buffer.type == kInternalStorageBufferBinding &&
                             shaderInfo.buffer.type == wgpu::BufferBindingType::Storage);

                        if (layoutInfo.buffer.type != shaderInfo.buffer.type &&
                            !validBindingConversion) {
                            return DAWN_VALIDATION_ERROR(
                                "The buffer type of the bind group layout entry conflicts " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }

                        if (layoutInfo.buffer.minBindingSize != 0 &&
                            shaderInfo.buffer.minBindingSize > layoutInfo.buffer.minBindingSize) {
                            return DAWN_VALIDATION_ERROR(
                                "The minimum buffer size of the bind group layout entry is smaller "
                                "than " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }
                        break;
                    }

                    case BindingInfoType::Sampler:
                        if ((layoutInfo.sampler.type == wgpu::SamplerBindingType::Comparison) !=
                            shaderInfo.sampler.isComparison) {
                            return DAWN_VALIDATION_ERROR(
                                "The sampler type of the bind group layout entry is "
                                "not compatible with " +
                                GetShaderDeclarationString(group, bindingNumber));
                        }
                }
            }

            return {};
        }

        ResultOrError<EntryPointMetadataTable> ReflectShaderUsingTint(
            DeviceBase*,
            const tint::Program* program) {
            ASSERT(program->IsValid());

            EntryPointMetadataTable result;
            std::ostringstream errorStream;
            errorStream << "Tint Reflection failure:" << std::endl;

            tint::inspector::Inspector inspector(program);
            auto entryPoints = inspector.GetEntryPoints();
            if (inspector.has_error()) {
                errorStream << "Inspector: " << inspector.error() << std::endl;
                return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
            }

            constexpr uint32_t kMaxInterStageShaderLocation = kMaxInterStageShaderVariables - 1;
            for (auto& entryPoint : entryPoints) {
                ASSERT(result.count(entryPoint.name) == 0);

                if (!entryPoint.overridable_constants.empty()) {
                    return DAWN_VALIDATION_ERROR(
                        "Pipeline overridable constants are not implemented yet");
                }

                auto metadata = std::make_unique<EntryPointMetadata>();

                DAWN_TRY_ASSIGN(metadata->stage, TintPipelineStageToShaderStage(entryPoint.stage));

                if (metadata->stage == SingleShaderStage::Compute) {
                    if (entryPoint.workgroup_size_x > kMaxComputeWorkgroupSizeX) {
                        errorStream << "Workgroup X dimension exceeds maximum allowed:"
                                    << entryPoint.workgroup_size_x << " > "
                                    << kMaxComputeWorkgroupSizeX;
                        return DAWN_VALIDATION_ERROR(errorStream.str());
                    }
                    if (entryPoint.workgroup_size_y > kMaxComputeWorkgroupSizeY) {
                        errorStream << "Workgroup Y dimension exceeds maximum allowed: "
                                    << entryPoint.workgroup_size_y << " > "
                                    << kMaxComputeWorkgroupSizeY;
                        return DAWN_VALIDATION_ERROR(errorStream.str());
                    }
                    if (entryPoint.workgroup_size_z > kMaxComputeWorkgroupSizeZ) {
                        errorStream << "Workgroup Z dimension exceeds maximum allowed: "
                                    << entryPoint.workgroup_size_z << " > "
                                    << kMaxComputeWorkgroupSizeZ;
                        return DAWN_VALIDATION_ERROR(errorStream.str());
                    }

                    // Dimensions have already been validated against their individual limits above.
                    // This assertion ensures that the product of such limited dimensions cannot
                    // possibly overflow a uint32_t.
                    static_assert(static_cast<uint64_t>(kMaxComputeWorkgroupSizeX) *
                                          kMaxComputeWorkgroupSizeY * kMaxComputeWorkgroupSizeZ <=
                                      std::numeric_limits<uint32_t>::max(),
                                  "Per-dimension workgroup size limits are too high");
                    uint32_t num_invocations = entryPoint.workgroup_size_x *
                                               entryPoint.workgroup_size_y *
                                               entryPoint.workgroup_size_z;
                    if (num_invocations > kMaxComputeWorkgroupInvocations) {
                        errorStream << "Number of workgroup invocations exceeds maximum allowed: "
                                    << num_invocations << " > " << kMaxComputeWorkgroupInvocations;
                        return DAWN_VALIDATION_ERROR(errorStream.str());
                    }

                    const size_t workgroup_storage_size =
                        inspector.GetWorkgroupStorageSize(entryPoint.name);
                    if (workgroup_storage_size > kMaxComputeWorkgroupStorageSize) {
                        errorStream << "Workgroup shared storage size for " << entryPoint.name
                                    << " exceeds the maximum allowed: " << workgroup_storage_size
                                    << " > " << kMaxComputeWorkgroupStorageSize;
                        return DAWN_VALIDATION_ERROR(errorStream.str());
                    }

                    metadata->localWorkgroupSize.x = entryPoint.workgroup_size_x;
                    metadata->localWorkgroupSize.y = entryPoint.workgroup_size_y;
                    metadata->localWorkgroupSize.z = entryPoint.workgroup_size_z;
                }

                if (metadata->stage == SingleShaderStage::Vertex) {
                    for (const auto& input_var : entryPoint.input_variables) {
                        if (!input_var.has_location_decoration) {
                            return DAWN_VALIDATION_ERROR(
                                "Need Location decoration on Vertex input");
                        }
                        uint32_t unsanitizedLocation = input_var.location_decoration;
                        if (DAWN_UNLIKELY(unsanitizedLocation >= kMaxVertexAttributes)) {
                            std::stringstream ss;
                            ss << "Attribute location (" << unsanitizedLocation << ") over limits";
                            return DAWN_VALIDATION_ERROR(ss.str());
                        }
                        VertexAttributeLocation location(static_cast<uint8_t>(unsanitizedLocation));
                        DAWN_TRY_ASSIGN(
                            metadata->vertexInputBaseTypes[location],
                            TintComponentTypeToVertexFormatBaseType(input_var.component_type));
                        metadata->usedVertexInputs.set(location);
                    }

                    // [[position]] must be declared in a vertex shader.
                    uint32_t totalInterStageShaderComponents = 4;
                    for (const auto& output_var : entryPoint.output_variables) {
                        if (DAWN_UNLIKELY(!output_var.has_location_decoration)) {
                            std::stringstream ss;
                            ss << "Missing location qualifier on vertex output, "
                               << output_var.name;
                            return DAWN_VALIDATION_ERROR(ss.str());
                        }
                        uint32_t location = output_var.location_decoration;
                        if (DAWN_UNLIKELY(location > kMaxInterStageShaderLocation)) {
                            std::stringstream ss;
                            ss << "Vertex output location (" << location << ") over limits";
                            return DAWN_VALIDATION_ERROR(ss.str());
                        }
                        metadata->usedInterStageVariables.set(location);
                        DAWN_TRY_ASSIGN(
                            metadata->interStageVariables[location].baseType,
                            TintComponentTypeToInterStageComponentType(output_var.component_type));
                        DAWN_TRY_ASSIGN(metadata->interStageVariables[location].componentCount,
                                        TintCompositionTypeToInterStageComponentCount(
                                            output_var.composition_type));
                        DAWN_TRY_ASSIGN(metadata->interStageVariables[location].interpolationType,
                                        TintInterpolationTypeToInterpolationType(
                                            output_var.interpolation_type));
                        DAWN_TRY_ASSIGN(
                            metadata->interStageVariables[location].interpolationSampling,
                            TintInterpolationSamplingToInterpolationSamplingType(
                                output_var.interpolation_sampling));

                        totalInterStageShaderComponents +=
                            metadata->interStageVariables[location].componentCount;
                    }

                    if (DAWN_UNLIKELY(totalInterStageShaderComponents >
                                      kMaxInterStageShaderComponents)) {
                        return DAWN_VALIDATION_ERROR(
                            "Total vertex output components count exceeds limits");
                    }
                }

                if (metadata->stage == SingleShaderStage::Fragment) {
                    uint32_t totalInterStageShaderComponents = 0;
                    for (const auto& input_var : entryPoint.input_variables) {
                        if (!input_var.has_location_decoration) {
                            return DAWN_VALIDATION_ERROR(
                                "Need location decoration on fragment input");
                        }
                        uint32_t location = input_var.location_decoration;
                        if (DAWN_UNLIKELY(location > kMaxInterStageShaderLocation)) {
                            std::stringstream ss;
                            ss << "Fragment input location (" << location << ") over limits";
                            return DAWN_VALIDATION_ERROR(ss.str());
                        }
                        metadata->usedInterStageVariables.set(location);
                        DAWN_TRY_ASSIGN(
                            metadata->interStageVariables[location].baseType,
                            TintComponentTypeToInterStageComponentType(input_var.component_type));
                        DAWN_TRY_ASSIGN(metadata->interStageVariables[location].componentCount,
                                        TintCompositionTypeToInterStageComponentCount(
                                            input_var.composition_type));
                        DAWN_TRY_ASSIGN(
                            metadata->interStageVariables[location].interpolationType,
                            TintInterpolationTypeToInterpolationType(input_var.interpolation_type));
                        DAWN_TRY_ASSIGN(
                            metadata->interStageVariables[location].interpolationSampling,
                            TintInterpolationSamplingToInterpolationSamplingType(
                                input_var.interpolation_sampling));

                        totalInterStageShaderComponents +=
                            metadata->interStageVariables[location].componentCount;
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
                    if (totalInterStageShaderComponents > kMaxInterStageShaderComponents) {
                        return DAWN_VALIDATION_ERROR(
                            "Total fragment input components count exceeds limits");
                    }

                    for (const auto& output_var : entryPoint.output_variables) {
                        if (!output_var.has_location_decoration) {
                            return DAWN_VALIDATION_ERROR(
                                "Need location decoration on fragment output");
                        }

                        uint32_t unsanitizedAttachment = output_var.location_decoration;
                        if (unsanitizedAttachment >= kMaxColorAttachments) {
                            return DAWN_VALIDATION_ERROR(
                                "Fragment output index must be less than max number of color "
                                "attachments");
                        }
                        ColorAttachmentIndex attachment(
                            static_cast<uint8_t>(unsanitizedAttachment));
                        DAWN_TRY_ASSIGN(
                            metadata->fragmentOutputVariables[attachment].baseType,
                            TintComponentTypeToTextureComponentType(output_var.component_type));
                        uint32_t componentCount;
                        DAWN_TRY_ASSIGN(componentCount,
                                        TintCompositionTypeToInterStageComponentCount(
                                            output_var.composition_type));
                        // componentCount should be no larger than 4u
                        ASSERT(componentCount <= 4u);
                        metadata->fragmentOutputVariables[attachment].componentCount =
                            componentCount;
                        metadata->fragmentOutputsWritten.set(attachment);
                    }
                }

                for (const tint::inspector::ResourceBinding& resource :
                     inspector.GetResourceBindings(entryPoint.name)) {
                    BindingNumber bindingNumber(resource.binding);
                    BindGroupIndex bindGroupIndex(resource.bind_group);
                    if (bindGroupIndex >= kMaxBindGroupsTyped) {
                        return DAWN_VALIDATION_ERROR("Shader has bind group index over limits");
                    }

                    const auto& it = metadata->bindings[bindGroupIndex].emplace(
                        bindingNumber, EntryPointMetadata::ShaderBindingInfo{});
                    if (!it.second) {
                        return DAWN_VALIDATION_ERROR("Shader has duplicate bindings");
                    }

                    EntryPointMetadata::ShaderBindingInfo* info = &it.first->second;
                    info->bindingType = TintResourceTypeToBindingInfoType(resource.resource_type);

                    switch (info->bindingType) {
                        case BindingInfoType::Buffer:
                            info->buffer.minBindingSize = resource.size_no_padding;
                            DAWN_TRY_ASSIGN(info->buffer.type, TintResourceTypeToBufferBindingType(
                                                                   resource.resource_type));
                            break;
                        case BindingInfoType::Sampler:
                            switch (resource.resource_type) {
                                case tint::inspector::ResourceBinding::ResourceType::kSampler:
                                    info->sampler.isComparison = false;
                                    break;
                                case tint::inspector::ResourceBinding::ResourceType::
                                    kComparisonSampler:
                                    info->sampler.isComparison = true;
                                    break;
                                default:
                                    UNREACHABLE();
                            }
                            break;
                        case BindingInfoType::Texture:
                            info->texture.viewDimension =
                                TintTextureDimensionToTextureViewDimension(resource.dim);
                            if (resource.resource_type ==
                                    tint::inspector::ResourceBinding::ResourceType::kDepthTexture ||
                                resource.resource_type ==
                                    tint::inspector::ResourceBinding::ResourceType::
                                        kDepthMultisampledTexture) {
                                info->texture.compatibleSampleTypes = SampleTypeBit::Depth;
                            } else {
                                info->texture.compatibleSampleTypes =
                                    TintSampledKindToSampleTypeBit(resource.sampled_kind);
                            }
                            info->texture.multisampled =
                                resource.resource_type == tint::inspector::ResourceBinding::
                                                              ResourceType::kMultisampledTexture ||
                                resource.resource_type ==
                                    tint::inspector::ResourceBinding::ResourceType::
                                        kDepthMultisampledTexture;

                            break;
                        case BindingInfoType::StorageTexture:
                            DAWN_TRY_ASSIGN(
                                info->storageTexture.access,
                                TintResourceTypeToStorageTextureAccess(resource.resource_type));
                            info->storageTexture.format =
                                TintImageFormatToTextureFormat(resource.image_format);
                            info->storageTexture.viewDimension =
                                TintTextureDimensionToTextureViewDimension(resource.dim);

                            break;
                        case BindingInfoType::ExternalTexture:
                            break;
                        default:
                            return DAWN_VALIDATION_ERROR("Unknown binding type in Shader");
                    }
                }

                std::vector<tint::inspector::SamplerTexturePair> samplerTextureUses =
                    inspector.GetSamplerTextureUses(entryPoint.name);
                metadata->samplerTexturePairs.reserve(samplerTextureUses.size());
                std::transform(
                    samplerTextureUses.begin(), samplerTextureUses.end(),
                    std::back_inserter(metadata->samplerTexturePairs),
                    [](const tint::inspector::SamplerTexturePair& pair) {
                        EntryPointMetadata::SamplerTexturePair result;
                        result.sampler = {BindGroupIndex(pair.sampler_binding_point.group),
                                          BindingNumber(pair.sampler_binding_point.binding)};
                        result.texture = {BindGroupIndex(pair.texture_binding_point.group),
                                          BindingNumber(pair.texture_binding_point.binding)};
                        return result;
                    });

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
        TintSource(ARGS&&... args) : file(std::forward<ARGS>(args)...) {
        }

        tint::Source::File file;
    };

    MaybeError ValidateShaderModuleDescriptor(DeviceBase* device,
                                              const ShaderModuleDescriptor* descriptor,
                                              ShaderModuleParseResult* parseResult,
                                              OwnedCompilationMessages* outMessages) {
        ASSERT(parseResult != nullptr);

        const ChainedStruct* chainedDescriptor = descriptor->nextInChain;
        if (chainedDescriptor == nullptr) {
            return DAWN_VALIDATION_ERROR("Shader module descriptor missing chained descriptor");
        }
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
            std::vector<uint32_t> spirv(spirvDesc->code, spirvDesc->code + spirvDesc->codeSize);
            tint::Program program;
            DAWN_TRY_ASSIGN(program, ParseSPIRV(spirv, outMessages));

            tint::writer::wgsl::Options options;
            auto result = tint::writer::wgsl::Generate(&program, options);
            if (!result.success) {
                std::ostringstream errorStream;
                errorStream << "Tint WGSL failure:" << std::endl;
                errorStream << "Generator: " << result.error << std::endl;
                return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
            }

            newWgslCode = std::move(result.wgsl);
            newWgslDesc.source = newWgslCode.c_str();

            spirvDesc = nullptr;
            wgslDesc = &newWgslDesc;
        }

        if (spirvDesc) {
            if (device->IsToggleEnabled(Toggle::DisallowSpirv)) {
                return DAWN_VALIDATION_ERROR("SPIR-V is disallowed.");
            }

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
        if (!output.program.IsValid()) {
            std::string err = "Tint program failure: " + output.program.Diagnostics().str();
            return DAWN_VALIDATION_ERROR(err.c_str());
        }
        if (outputs != nullptr) {
            *outputs = std::move(output.data);
        }
        return std::move(output.program);
    }

    void AddVertexPullingTransformConfig(const VertexState& vertexState,
                                         const std::string& entryPoint,
                                         BindGroupIndex pullingBufferBindingSet,
                                         tint::transform::DataMap* transformInputs) {
        tint::transform::VertexPulling::Config cfg;
        cfg.entry_point_name = entryPoint;
        cfg.pulling_group = static_cast<uint32_t>(pullingBufferBindingSet);
        for (uint32_t i = 0; i < vertexState.bufferCount; ++i) {
            const auto& vertexBuffer = vertexState.buffers[i];
            tint::transform::VertexBufferLayoutDescriptor layout;
            layout.array_stride = vertexBuffer.arrayStride;
            layout.step_mode = ToTintVertexStepMode(vertexBuffer.stepMode);

            for (uint32_t j = 0; j < vertexBuffer.attributeCount; ++j) {
                const auto& attribute = vertexBuffer.attributes[j];
                tint::transform::VertexAttributeDescriptor attr;
                attr.format = ToTintVertexFormat(attribute.format);
                attr.offset = attribute.offset;
                attr.shader_location = attribute.shaderLocation;

                layout.attributes.push_back(std::move(attr));
            }

            cfg.vertex_state.push_back(std::move(layout));
        }
        transformInputs->Add<tint::transform::VertexPulling::Config>(cfg);
    }

    MaybeError ValidateCompatibilityWithPipelineLayout(DeviceBase* device,
                                                       const EntryPointMetadata& entryPoint,
                                                       const PipelineLayoutBase* layout) {
        for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            DAWN_TRY(ValidateCompatibilityWithBindGroupLayout(device, group, entryPoint,
                                                              layout->GetBindGroupLayout(group)));
        }

        for (BindGroupIndex group : IterateBitSet(~layout->GetBindGroupLayoutsMask())) {
            if (entryPoint.bindings[group].size() > 0) {
                std::ostringstream ostream;
                ostream << "No bind group layout entry matches the declaration set "
                        << static_cast<uint32_t>(group) << " in the shader module";
                return DAWN_VALIDATION_ERROR(ostream.str());
            }
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

            if (textureInfo.texture.sampleType == wgpu::TextureSampleType::UnfilterableFloat) {
                return DAWN_VALIDATION_ERROR(
                    "unfilterable-float texture bindings cannot be sampled with a "
                    "filtering sampler");
            }
        }

        return {};
    }

    // ShaderModuleBase

    ShaderModuleBase::ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor* descriptor)
        : CachedObject(device), mType(Type::Undefined) {
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

    ShaderModuleBase::ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : CachedObject(device, tag), mType(Type::Undefined) {
    }

    ShaderModuleBase::~ShaderModuleBase() {
        if (IsCachedReference()) {
            GetDevice()->UncacheShaderModule(this);
        }
    }

    // static
    Ref<ShaderModuleBase> ShaderModuleBase::MakeError(DeviceBase* device) {
        return AcquireRef(new ShaderModuleBase(device, ObjectBase::kError));
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
        return a->mType == b->mType && a->mOriginalSpirv == b->mOriginalSpirv &&
               a->mWgsl == b->mWgsl;
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

        callback(WGPUCompilationInfoRequestStatus_Success,
                 mCompilationMessages->GetCompilationInfo(), userdata);
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

}  // namespace dawn_native
