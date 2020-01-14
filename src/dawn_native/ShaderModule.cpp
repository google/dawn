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

#include "common/HashUtils.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Device.h"
#include "dawn_native/Pipeline.h"
#include "dawn_native/PipelineLayout.h"

#include <spirv-tools/libspirv.hpp>
#include <spirv_cross.hpp>

#include <sstream>

namespace dawn_native {

    namespace {
        Format::Type SpirvCrossBaseTypeToFormatType(spirv_cross::SPIRType::BaseType spirvBaseType) {
            switch (spirvBaseType) {
                case spirv_cross::SPIRType::Float:
                    return Format::Float;
                case spirv_cross::SPIRType::Int:
                    return Format::Sint;
                case spirv_cross::SPIRType::UInt:
                    return Format::Uint;
                default:
                    UNREACHABLE();
                    return Format::Other;
            }
        }

        wgpu::TextureViewDimension SpirvDimToTextureViewDimension(spv::Dim dim, bool arrayed) {
            switch (dim) {
                case spv::Dim::Dim1D:
                    return wgpu::TextureViewDimension::e1D;
                case spv::Dim::Dim2D:
                    if (arrayed) {
                        return wgpu::TextureViewDimension::e2DArray;
                    } else {
                        return wgpu::TextureViewDimension::e2D;
                    }
                case spv::Dim::Dim3D:
                    return wgpu::TextureViewDimension::e3D;
                case spv::Dim::DimCube:
                    if (arrayed) {
                        return wgpu::TextureViewDimension::CubeArray;
                    } else {
                        return wgpu::TextureViewDimension::Cube;
                    }
                default:
                    UNREACHABLE();
                    return wgpu::TextureViewDimension::Undefined;
            }
        }

        wgpu::TextureViewDimension ToWGPUTextureViewDimension(
            shaderc_spvc_texture_view_dimension dim) {
            switch (dim) {
                case shaderc_spvc_texture_view_dimension_undefined:
                    return wgpu::TextureViewDimension::Undefined;
                case shaderc_spvc_texture_view_dimension_e1D:
                    return wgpu::TextureViewDimension::e1D;
                case shaderc_spvc_texture_view_dimension_e2D:
                    return wgpu::TextureViewDimension::e2D;
                case shaderc_spvc_texture_view_dimension_e2D_array:
                    return wgpu::TextureViewDimension::e2DArray;
                case shaderc_spvc_texture_view_dimension_cube:
                    return wgpu::TextureViewDimension::Cube;
                case shaderc_spvc_texture_view_dimension_cube_array:
                    return wgpu::TextureViewDimension::CubeArray;
                case shaderc_spvc_texture_view_dimension_e3D:
                    return wgpu::TextureViewDimension::e3D;
            }
            UNREACHABLE();
        }

        Format::Type ToDawnFormatType(shaderc_spvc_texture_format_type type) {
            switch (type) {
                case shaderc_spvc_texture_format_type_float:
                    return Format::Type::Float;
                case shaderc_spvc_texture_format_type_sint:
                    return Format::Type::Sint;
                case shaderc_spvc_texture_format_type_uint:
                    return Format::Type::Uint;
                case shaderc_spvc_texture_format_type_other:
                    return Format::Type::Other;
            }
            UNREACHABLE();
        }

        wgpu::BindingType ToWGPUBindingType(shaderc_spvc_binding_type type) {
            switch (type) {
                case shaderc_spvc_binding_type_uniform_buffer:
                    return wgpu::BindingType::UniformBuffer;
                case shaderc_spvc_binding_type_storage_buffer:
                    return wgpu::BindingType::StorageBuffer;
                case shaderc_spvc_binding_type_readonly_storage_buffer:
                    return wgpu::BindingType::ReadonlyStorageBuffer;
                case shaderc_spvc_binding_type_sampler:
                    return wgpu::BindingType::Sampler;
                case shaderc_spvc_binding_type_sampled_texture:
                    return wgpu::BindingType::SampledTexture;
                case shaderc_spvc_binding_type_storage_texture:
                    return wgpu::BindingType::StorageTexture;
            }
            UNREACHABLE();
        }

        // TODO(rharrison): Convert this to ResultOrError once ExtractSPIRVInfo turns an error
        SingleShaderStage ToSingleShaderStage(shaderc_spvc_execution_model execution_model) {
            switch (execution_model) {
                case shaderc_spvc_execution_model_vertex:
                    return SingleShaderStage::Vertex;
                case shaderc_spvc_execution_model_fragment:
                    return SingleShaderStage::Fragment;
                case shaderc_spvc_execution_model_glcompute:
                    return SingleShaderStage::Compute;
                default:
                    // This will be converted to an error return when the whole
                    // calling stack is changed to passing errors.
                    UNREACHABLE();
                    return SingleShaderStage::Vertex;
            }
        }
    }  // anonymous namespace

    MaybeError ValidateShaderModuleDescriptor(DeviceBase*,
                                              const ShaderModuleDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        spvtools::SpirvTools spirvTools(SPV_ENV_VULKAN_1_1);

        std::ostringstream errorStream;
        errorStream << "SPIRV Validation failure:" << std::endl;

        spirvTools.SetMessageConsumer([&errorStream](spv_message_level_t level, const char*,
                                                     const spv_position_t& position,
                                                     const char* message) {
            switch (level) {
                case SPV_MSG_FATAL:
                case SPV_MSG_INTERNAL_ERROR:
                case SPV_MSG_ERROR:
                    errorStream << "error: line " << position.index << ": " << message << std::endl;
                    break;
                case SPV_MSG_WARNING:
                    errorStream << "warning: line " << position.index << ": " << message
                                << std::endl;
                    break;
                case SPV_MSG_INFO:
                    errorStream << "info: line " << position.index << ": " << message << std::endl;
                    break;
                default:
                    break;
            }
        });

        if (!spirvTools.Validate(descriptor->code, descriptor->codeSize)) {
            return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
        }

        return {};
    }

    // ShaderModuleBase

    ShaderModuleBase::ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor* descriptor)
        : CachedObject(device), mCode(descriptor->code, descriptor->code + descriptor->codeSize) {
        mFragmentOutputFormatBaseTypes.fill(Format::Other);
    }

    ShaderModuleBase::ShaderModuleBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : CachedObject(device, tag) {
    }

    ShaderModuleBase::~ShaderModuleBase() {
        if (IsCachedReference()) {
            GetDevice()->UncacheShaderModule(this);
        }
    }

    // static
    ShaderModuleBase* ShaderModuleBase::MakeError(DeviceBase* device) {
        return new ShaderModuleBase(device, ObjectBase::kError);
    }

    void ShaderModuleBase::ExtractSpirvInfo(const spirv_cross::Compiler& compiler) {
        ASSERT(!IsError());
        if (GetDevice()->IsToggleEnabled(Toggle::UseSpvc)) {
            ExtractSpirvInfoWithSpvc(compiler);
        } else {
            ExtractSpirvInfoWithSpirvCross(compiler);
        }
    }

    void ShaderModuleBase::ExtractSpirvInfoWithSpvc(const spirv_cross::Compiler& compiler) {
        shaderc_spvc_execution_model execution_model;
        if (!CheckSpvcSuccess(mSpvcContext.GetExecutionModel(&execution_model),
                              "Unable to get execution model for shader.")) {
            return;
        }

        mExecutionModel = ToSingleShaderStage(execution_model);

        size_t push_constant_buffers_count;
        if (!CheckSpvcSuccess(mSpvcContext.GetPushConstantBufferCount(&push_constant_buffers_count),
                              "Unable to get push constant buffer count for shader.")) {
            return;
        }

        // TODO(rharrison): This should be handled by spirv-val pass in spvc,
        // but need to confirm.
        if (push_constant_buffers_count > 0) {
            GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                     "Push constants aren't supported.");
            return;
        }

        // Fill in bindingInfo with the SPIRV bindings
        auto ExtractResourcesBinding = [this](std::vector<shaderc_spvc_binding_info> bindings) {
            for (const auto& binding : bindings) {
                if (binding.binding >= kMaxBindingsPerGroup || binding.set >= kMaxBindGroups) {
                    GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                             "Binding over limits in the SPIRV");
                    continue;
                }

                BindingInfo* info = &mBindingInfo[binding.set][binding.binding];
                *info = {};
                info->used = true;
                info->id = binding.id;
                info->base_type_id = binding.base_type_id;
                if (binding.binding_type == shaderc_spvc_binding_type_sampled_texture) {
                    info->multisampled = binding.multisampled;
                    info->textureDimension = ToWGPUTextureViewDimension(binding.texture_dimension);
                    info->textureComponentType = ToDawnFormatType(binding.texture_component_type);
                }
                info->type = ToWGPUBindingType(binding.binding_type);
            }
        };

        std::vector<shaderc_spvc_binding_info> resource_bindings;
        if (!CheckSpvcSuccess(mSpvcContext.GetBindingInfo(
                                  shaderc_spvc_shader_resource_uniform_buffers,
                                  shaderc_spvc_binding_type_uniform_buffer, &resource_bindings),
                              "Unable to get binding info for uniform buffers from shader")) {
            return;
        }
        ExtractResourcesBinding(resource_bindings);

        if (!CheckSpvcSuccess(mSpvcContext.GetBindingInfo(
                                  shaderc_spvc_shader_resource_separate_images,
                                  shaderc_spvc_binding_type_sampled_texture, &resource_bindings),
                              "Unable to get binding info for sampled textures from shader")) {
            return;
        }
        ExtractResourcesBinding(resource_bindings);

        if (!CheckSpvcSuccess(
                mSpvcContext.GetBindingInfo(shaderc_spvc_shader_resource_separate_samplers,
                                            shaderc_spvc_binding_type_sampler, &resource_bindings),
                "Unable to get binding info for samples from shader")) {
            return;
        }
        ExtractResourcesBinding(resource_bindings);

        if (!CheckSpvcSuccess(mSpvcContext.GetBindingInfo(
                                  shaderc_spvc_shader_resource_storage_buffers,
                                  shaderc_spvc_binding_type_storage_buffer, &resource_bindings),
                              "Unable to get binding info for storage buffers from shader")) {
            return;
        }
        ExtractResourcesBinding(resource_bindings);

        std::vector<shaderc_spvc_resource_location_info> input_stage_locations;
        if (!CheckSpvcSuccess(mSpvcContext.GetInputStageLocationInfo(&input_stage_locations),
                              "Unable to get input stage location information from shader")) {
            input_stage_locations.clear();
            return;
        }

        for (const auto& input : input_stage_locations) {
            if (mExecutionModel == SingleShaderStage::Vertex) {
                if (input.location >= kMaxVertexAttributes) {
                    GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                             "Attribute location over limits in the SPIRV");
                    return;
                }
                mUsedVertexAttributes.set(input.location);
            } else if (mExecutionModel == SingleShaderStage::Fragment) {
                // Without a location qualifier on vertex inputs, spirv_cross::CompilerMSL gives
                // them all the location 0, causing a compile error.
                if (!input.has_location) {
                    GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                             "Need location qualifier on fragment input");
                    return;
                }
            }
        }

        std::vector<shaderc_spvc_resource_location_info> output_stage_locations;
        if (!CheckSpvcSuccess(mSpvcContext.GetOutputStageLocationInfo(&output_stage_locations),
                              "Unable to get output stage location information from shader")) {
            output_stage_locations.clear();
            return;
        }

        for (const auto& output : output_stage_locations) {
            if (mExecutionModel == SingleShaderStage::Vertex) {
                // Without a location qualifier on vertex outputs, spirv_cross::CompilerMSL
                // gives them all the location 0, causing a compile error.
                if (!output.has_location) {
                    GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                             "Need location qualifier on vertex output");
                    return;
                }
            } else if (mExecutionModel == SingleShaderStage::Fragment) {
                if (output.location >= kMaxColorAttachments) {
                    GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                             "Fragment output location over limits in the SPIRV");
                    return;
                }
            }
        }

        if (mExecutionModel == SingleShaderStage::Fragment) {
            std::vector<shaderc_spvc_resource_type_info> output_types;
            if (!CheckSpvcSuccess(mSpvcContext.GetOutputStageTypeInfo(&output_types),
                                  "Unable to get output stage type information from shader")) {
                output_stage_locations.clear();
                return;
            }

            for (const auto& output : output_types) {
                ASSERT(output.type != shaderc_spvc_texture_format_type_other);
                mFragmentOutputFormatBaseTypes[output.location] = ToDawnFormatType(output.type);
            }
        }
    }

    void ShaderModuleBase::ExtractSpirvInfoWithSpirvCross(const spirv_cross::Compiler& compiler) {
        // TODO(cwallez@chromium.org): make errors here creation errors
        // currently errors here do not prevent the shadermodule from being used
        const auto& resources = compiler.get_shader_resources();

        switch (compiler.get_execution_model()) {
            case spv::ExecutionModelVertex:
                mExecutionModel = SingleShaderStage::Vertex;
                break;
            case spv::ExecutionModelFragment:
                mExecutionModel = SingleShaderStage::Fragment;
                break;
            case spv::ExecutionModelGLCompute:
                mExecutionModel = SingleShaderStage::Compute;
                break;
            default:
                UNREACHABLE();
        }

        if (resources.push_constant_buffers.size() > 0) {
            GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                     "Push constants aren't supported.");
        }

        // Fill in bindingInfo with the SPIRV bindings
        auto ExtractResourcesBinding =
            [this](const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
                   const spirv_cross::Compiler& compiler, wgpu::BindingType bindingType) {
                for (const auto& resource : resources) {
                    ASSERT(compiler.get_decoration_bitset(resource.id).get(spv::DecorationBinding));
                    ASSERT(compiler.get_decoration_bitset(resource.id)
                               .get(spv::DecorationDescriptorSet));

                    uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                    uint32_t set =
                        compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

                    if (binding >= kMaxBindingsPerGroup || set >= kMaxBindGroups) {
                        GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                                 "Binding over limits in the SPIRV");
                        continue;
                    }

                    BindingInfo* info = &mBindingInfo[set][binding];
                    *info = {};
                    info->used = true;
                    info->id = resource.id;
                    info->base_type_id = resource.base_type_id;
                    switch (bindingType) {
                        case wgpu::BindingType::SampledTexture: {
                            spirv_cross::SPIRType::ImageType imageType =
                                compiler.get_type(info->base_type_id).image;
                            spirv_cross::SPIRType::BaseType textureComponentType =
                                compiler.get_type(imageType.type).basetype;

                            info->multisampled = imageType.ms;
                            info->textureDimension =
                                SpirvDimToTextureViewDimension(imageType.dim, imageType.arrayed);
                            info->textureComponentType =
                                SpirvCrossBaseTypeToFormatType(textureComponentType);
                            info->type = bindingType;
                        } break;
                        case wgpu::BindingType::StorageBuffer: {
                            // Differentiate between readonly storage bindings and writable ones
                            // based on the NonWritable decoration
                            spirv_cross::Bitset flags =
                                compiler.get_buffer_block_flags(resource.id);
                            if (flags.get(spv::DecorationNonWritable)) {
                                info->type = wgpu::BindingType::ReadonlyStorageBuffer;
                            } else {
                                info->type = wgpu::BindingType::StorageBuffer;
                            }
                        } break;
                        default:
                            info->type = bindingType;
                    }
                }
            };

        ExtractResourcesBinding(resources.uniform_buffers, compiler,
                                wgpu::BindingType::UniformBuffer);
        ExtractResourcesBinding(resources.separate_images, compiler,
                                wgpu::BindingType::SampledTexture);
        ExtractResourcesBinding(resources.separate_samplers, compiler, wgpu::BindingType::Sampler);
        ExtractResourcesBinding(resources.storage_buffers, compiler,
                                wgpu::BindingType::StorageBuffer);

        // Extract the vertex attributes
        if (mExecutionModel == SingleShaderStage::Vertex) {
            for (const auto& attrib : resources.stage_inputs) {
                ASSERT(compiler.get_decoration_bitset(attrib.id).get(spv::DecorationLocation));
                uint32_t location = compiler.get_decoration(attrib.id, spv::DecorationLocation);

                if (location >= kMaxVertexAttributes) {
                    GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                             "Attribute location over limits in the SPIRV");
                    return;
                }

                mUsedVertexAttributes.set(location);
            }

            // Without a location qualifier on vertex outputs, spirv_cross::CompilerMSL gives
            // them all the location 0, causing a compile error.
            for (const auto& attrib : resources.stage_outputs) {
                if (!compiler.get_decoration_bitset(attrib.id).get(spv::DecorationLocation)) {
                    GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                             "Need location qualifier on vertex output");
                    return;
                }
            }
        }

        if (mExecutionModel == SingleShaderStage::Fragment) {
            // Without a location qualifier on vertex inputs, spirv_cross::CompilerMSL gives
            // them all the location 0, causing a compile error.
            for (const auto& attrib : resources.stage_inputs) {
                if (!compiler.get_decoration_bitset(attrib.id).get(spv::DecorationLocation)) {
                    GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                             "Need location qualifier on fragment input");
                    return;
                }
            }

            for (const auto& fragmentOutput : resources.stage_outputs) {
                ASSERT(
                    compiler.get_decoration_bitset(fragmentOutput.id).get(spv::DecorationLocation));
                uint32_t location =
                    compiler.get_decoration(fragmentOutput.id, spv::DecorationLocation);
                if (location >= kMaxColorAttachments) {
                    GetDevice()->HandleError(wgpu::ErrorType::Validation,
                                             "Fragment output location over limits in the SPIRV");
                    return;
                }

                spirv_cross::SPIRType::BaseType shaderFragmentOutputBaseType =
                    compiler.get_type(fragmentOutput.base_type_id).basetype;
                Format::Type formatType =
                    SpirvCrossBaseTypeToFormatType(shaderFragmentOutputBaseType);
                ASSERT(formatType != Format::Type::Other);
                mFragmentOutputFormatBaseTypes[location] = formatType;
            }
        }
    }

    const ShaderModuleBase::ModuleBindingInfo& ShaderModuleBase::GetBindingInfo() const {
        ASSERT(!IsError());
        return mBindingInfo;
    }

    const std::bitset<kMaxVertexAttributes>& ShaderModuleBase::GetUsedVertexAttributes() const {
        ASSERT(!IsError());
        return mUsedVertexAttributes;
    }

    const ShaderModuleBase::FragmentOutputBaseTypes& ShaderModuleBase::GetFragmentOutputBaseTypes()
        const {
        ASSERT(!IsError());
        return mFragmentOutputFormatBaseTypes;
    }

    SingleShaderStage ShaderModuleBase::GetExecutionModel() const {
        ASSERT(!IsError());
        return mExecutionModel;
    }

    bool ShaderModuleBase::IsCompatibleWithPipelineLayout(const PipelineLayoutBase* layout) const {
        ASSERT(!IsError());

        for (uint32_t group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            if (!IsCompatibleWithBindGroupLayout(group, layout->GetBindGroupLayout(group))) {
                return false;
            }
        }

        for (uint32_t group : IterateBitSet(~layout->GetBindGroupLayoutsMask())) {
            for (size_t i = 0; i < kMaxBindingsPerGroup; ++i) {
                if (mBindingInfo[group][i].used) {
                    return false;
                }
            }
        }

        return true;
    }

    bool ShaderModuleBase::IsCompatibleWithBindGroupLayout(
        size_t group,
        const BindGroupLayoutBase* layout) const {
        ASSERT(!IsError());

        const auto& layoutInfo = layout->GetBindingInfo();
        for (size_t i = 0; i < kMaxBindingsPerGroup; ++i) {
            const auto& moduleInfo = mBindingInfo[group][i];
            const auto& layoutBindingType = layoutInfo.types[i];

            if (!moduleInfo.used) {
                continue;
            }

            if (layoutBindingType != moduleInfo.type) {
                // Binding mismatch between shader and bind group is invalid. For example, a
                // writable binding in the shader with a readonly storage buffer in the bind group
                // layout is invalid. However, a readonly binding in the shader with a writable
                // storage buffer in the bind group layout is valid.
                bool validBindingConversion =
                    layoutBindingType == wgpu::BindingType::StorageBuffer &&
                    moduleInfo.type == wgpu::BindingType::ReadonlyStorageBuffer;
                if (!validBindingConversion) {
                    return false;
                }
            }

            if ((layoutInfo.visibilities[i] & StageBit(mExecutionModel)) == 0) {
                return false;
            }

            if (layoutBindingType == wgpu::BindingType::SampledTexture) {
                Format::Type layoutTextureComponentType =
                    Format::TextureComponentTypeToFormatType(layoutInfo.textureComponentTypes[i]);
                if (layoutTextureComponentType != moduleInfo.textureComponentType) {
                    return false;
                }

                if (layoutInfo.textureDimensions[i] != moduleInfo.textureDimension) {
                    return false;
                }
            }
        }

        return true;
    }

    size_t ShaderModuleBase::HashFunc::operator()(const ShaderModuleBase* module) const {
        size_t hash = 0;

        for (uint32_t word : module->mCode) {
            HashCombine(&hash, word);
        }

        return hash;
    }

    bool ShaderModuleBase::EqualityFunc::operator()(const ShaderModuleBase* a,
                                                    const ShaderModuleBase* b) const {
        return a->mCode == b->mCode;
    }

    bool ShaderModuleBase::CheckSpvcSuccess(shaderc_spvc_status status, const char* error_msg) {
        if (status != shaderc_spvc_status_success) {
            GetDevice()->HandleError(wgpu::ErrorType::Validation, error_msg);
            return false;
        }
        return true;
    }

}  // namespace dawn_native
