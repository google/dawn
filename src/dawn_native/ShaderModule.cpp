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

#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/Device.h"
#include "dawn_native/Pipeline.h"
#include "dawn_native/PipelineLayout.h"

#include <spirv-cross/spirv_cross.hpp>
#include <spirv-tools/libspirv.hpp>

namespace dawn_native {

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

    ShaderModuleBase::ShaderModuleBase(DeviceBase* device, const ShaderModuleDescriptor*)
        : ObjectBase(device) {
    }

    void ShaderModuleBase::ExtractSpirvInfo(const spirv_cross::Compiler& compiler) {
        DeviceBase* device = GetDevice();
        // TODO(cwallez@chromium.org): make errors here builder-level
        // currently errors here do not prevent the shadermodule from being used
        const auto& resources = compiler.get_shader_resources();

        switch (compiler.get_execution_model()) {
            case spv::ExecutionModelVertex:
                mExecutionModel = dawn::ShaderStage::Vertex;
                break;
            case spv::ExecutionModelFragment:
                mExecutionModel = dawn::ShaderStage::Fragment;
                break;
            case spv::ExecutionModelGLCompute:
                mExecutionModel = dawn::ShaderStage::Compute;
                break;
            default:
                UNREACHABLE();
        }

        // Extract push constants
        mPushConstants.mask.reset();
        mPushConstants.sizes.fill(0);
        mPushConstants.types.fill(PushConstantType::Int);

        if (resources.push_constant_buffers.size() > 0) {
            auto interfaceBlock = resources.push_constant_buffers[0];

            const auto& blockType = compiler.get_type(interfaceBlock.type_id);
            ASSERT(blockType.basetype == spirv_cross::SPIRType::Struct);

            for (uint32_t i = 0; i < blockType.member_types.size(); i++) {
                ASSERT(compiler.get_member_decoration_bitset(blockType.self, i)
                           .get(spv::DecorationOffset));

                uint32_t offset =
                    compiler.get_member_decoration(blockType.self, i, spv::DecorationOffset);
                ASSERT(offset % 4 == 0);
                offset /= 4;

                auto memberType = compiler.get_type(blockType.member_types[i]);
                PushConstantType constantType;
                if (memberType.basetype == spirv_cross::SPIRType::Int) {
                    constantType = PushConstantType::Int;
                } else if (memberType.basetype == spirv_cross::SPIRType::UInt) {
                    constantType = PushConstantType::UInt;
                } else {
                    ASSERT(memberType.basetype == spirv_cross::SPIRType::Float);
                    constantType = PushConstantType::Float;
                }

                // TODO(cwallez@chromium.org): check for overflows and make the logic better take
                // into account things like the array of types with padding.
                uint32_t size = memberType.vecsize * memberType.columns;
                // Handle unidimensional arrays
                if (!memberType.array.empty()) {
                    size *= memberType.array[0];
                }

                if (offset + size > kMaxPushConstants) {
                    device->HandleError("Push constant block too big in the SPIRV");
                    return;
                }

                mPushConstants.mask.set(offset);
                mPushConstants.names[offset] =
                    interfaceBlock.name + "." + compiler.get_member_name(blockType.self, i);
                mPushConstants.sizes[offset] = size;
                mPushConstants.types[offset] = constantType;
            }
        }

        // Fill in bindingInfo with the SPIRV bindings
        auto ExtractResourcesBinding = [this](const std::vector<spirv_cross::Resource>& resources,
                                              const spirv_cross::Compiler& compiler,
                                              dawn::BindingType bindingType) {
            for (const auto& resource : resources) {
                ASSERT(compiler.get_decoration_bitset(resource.id).get(spv::DecorationBinding));
                ASSERT(
                    compiler.get_decoration_bitset(resource.id).get(spv::DecorationDescriptorSet));

                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

                if (binding >= kMaxBindingsPerGroup || set >= kMaxBindGroups) {
                    GetDevice()->HandleError("Binding over limits in the SPIRV");
                    continue;
                }

                auto& info = mBindingInfo[set][binding];
                info.used = true;
                info.id = resource.id;
                info.base_type_id = resource.base_type_id;
                info.type = bindingType;
            }
        };

        ExtractResourcesBinding(resources.uniform_buffers, compiler,
                                dawn::BindingType::UniformBuffer);
        ExtractResourcesBinding(resources.separate_images, compiler,
                                dawn::BindingType::SampledTexture);
        ExtractResourcesBinding(resources.separate_samplers, compiler, dawn::BindingType::Sampler);
        ExtractResourcesBinding(resources.storage_buffers, compiler,
                                dawn::BindingType::StorageBuffer);

        // Extract the vertex attributes
        if (mExecutionModel == dawn::ShaderStage::Vertex) {
            for (const auto& attrib : resources.stage_inputs) {
                ASSERT(compiler.get_decoration_bitset(attrib.id).get(spv::DecorationLocation));
                uint32_t location = compiler.get_decoration(attrib.id, spv::DecorationLocation);

                if (location >= kMaxVertexAttributes) {
                    device->HandleError("Attribute location over limits in the SPIRV");
                    return;
                }

                mUsedVertexAttributes.set(location);
            }

            // Without a location qualifier on vertex outputs, spirv_cross::CompilerMSL gives them
            // all the location 0, causing a compile error.
            for (const auto& attrib : resources.stage_outputs) {
                if (!compiler.get_decoration_bitset(attrib.id).get(spv::DecorationLocation)) {
                    device->HandleError("Need location qualifier on vertex output");
                    return;
                }
            }
        }

        if (mExecutionModel == dawn::ShaderStage::Fragment) {
            // Without a location qualifier on vertex inputs, spirv_cross::CompilerMSL gives them
            // all the location 0, causing a compile error.
            for (const auto& attrib : resources.stage_inputs) {
                if (!compiler.get_decoration_bitset(attrib.id).get(spv::DecorationLocation)) {
                    device->HandleError("Need location qualifier on fragment input");
                    return;
                }
            }
        }
    }

    const ShaderModuleBase::PushConstantInfo& ShaderModuleBase::GetPushConstants() const {
        return mPushConstants;
    }

    const ShaderModuleBase::ModuleBindingInfo& ShaderModuleBase::GetBindingInfo() const {
        return mBindingInfo;
    }

    const std::bitset<kMaxVertexAttributes>& ShaderModuleBase::GetUsedVertexAttributes() const {
        return mUsedVertexAttributes;
    }

    dawn::ShaderStage ShaderModuleBase::GetExecutionModel() const {
        return mExecutionModel;
    }

    bool ShaderModuleBase::IsCompatibleWithPipelineLayout(const PipelineLayoutBase* layout) {
        for (size_t group = 0; group < kMaxBindGroups; ++group) {
            if (!IsCompatibleWithBindGroupLayout(group, layout->GetBindGroupLayout(group))) {
                return false;
            }
        }
        return true;
    }

    bool ShaderModuleBase::IsCompatibleWithBindGroupLayout(size_t group,
                                                           const BindGroupLayoutBase* layout) {
        const auto& layoutInfo = layout->GetBindingInfo();
        for (size_t i = 0; i < kMaxBindingsPerGroup; ++i) {
            const auto& moduleInfo = mBindingInfo[group][i];

            if (!moduleInfo.used) {
                continue;
            }

            if (moduleInfo.type != layoutInfo.types[i]) {
                return false;
            }
            if ((layoutInfo.visibilities[i] & StageBit(mExecutionModel)) == 0) {
                return false;
            }
        }

        return true;
    }

}  // namespace dawn_native
