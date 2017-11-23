// Copyright 2017 The NXT Authors
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

#include "backend/ShaderModule.h"

#include "backend/BindGroupLayout.h"
#include "backend/Device.h"
#include "backend/Pipeline.h"
#include "backend/PipelineLayout.h"

#include <spirv-cross/spirv_cross.hpp>

namespace backend {

    ShaderModuleBase::ShaderModuleBase(ShaderModuleBuilder* builder)
        : mDevice(builder->mDevice) {
    }

    DeviceBase* ShaderModuleBase::GetDevice() const {
        return mDevice;
    }

    void ShaderModuleBase::ExtractSpirvInfo(const spirv_cross::Compiler& compiler) {
        // TODO(cwallez@chromium.org): make errors here builder-level
        // currently errors here do not prevent the shadermodule from being used
        const auto& resources = compiler.get_shader_resources();

        switch (compiler.get_execution_model()) {
            case spv::ExecutionModelVertex:
                mExecutionModel = nxt::ShaderStage::Vertex;
                break;
            case spv::ExecutionModelFragment:
                mExecutionModel = nxt::ShaderStage::Fragment;
                break;
            case spv::ExecutionModelGLCompute:
                mExecutionModel = nxt::ShaderStage::Compute;
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
                ASSERT(compiler.get_member_decoration_mask(blockType.self, i) & 1ull << spv::DecorationOffset);
                uint32_t offset = compiler.get_member_decoration(blockType.self, i, spv::DecorationOffset);
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

                // TODO(cwallez@chromium.org): check for overflows and make the logic better take into account
                // things like the array of types with padding.
                uint32_t size = memberType.vecsize * memberType.columns;
                // Handle unidimensional arrays
                if (!memberType.array.empty()) {
                    size *= memberType.array[0];
                }

                if (offset + size > kMaxPushConstants) {
                    mDevice->HandleError("Push constant block too big in the SPIRV");
                    return;
                }

                mPushConstants.mask.set(offset);
                mPushConstants.names[offset] = interfaceBlock.name + "." + compiler.get_member_name(blockType.self, i);
                mPushConstants.sizes[offset] = size;
                mPushConstants.types[offset] = constantType;
            }
        }

        // Fill in bindingInfo with the SPIRV bindings
        auto ExtractResourcesBinding = [this](const std::vector<spirv_cross::Resource>& resources,
                                              const spirv_cross::Compiler& compiler, nxt::BindingType bindingType) {
            constexpr uint64_t requiredBindingDecorationMask = (1ull << spv::DecorationBinding) | (1ull << spv::DecorationDescriptorSet);

            for (const auto& resource : resources) {
                ASSERT((compiler.get_decoration_mask(resource.id) & requiredBindingDecorationMask) == requiredBindingDecorationMask);
                uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                uint32_t set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);

                if (binding >= kMaxBindingsPerGroup || set >= kMaxBindGroups) {
                    mDevice->HandleError("Binding over limits in the SPIRV");
                    continue;
                }

                auto& info = mBindingInfo[set][binding];
                info.used = true;
                info.id = resource.id;
                info.base_type_id = resource.base_type_id;
                info.type = bindingType;
            }
        };

        ExtractResourcesBinding(resources.uniform_buffers, compiler, nxt::BindingType::UniformBuffer);
        ExtractResourcesBinding(resources.separate_images, compiler, nxt::BindingType::SampledTexture);
        ExtractResourcesBinding(resources.separate_samplers, compiler, nxt::BindingType::Sampler);
        ExtractResourcesBinding(resources.storage_buffers, compiler, nxt::BindingType::StorageBuffer);

        // Extract the vertex attributes
        if (mExecutionModel == nxt::ShaderStage::Vertex) {
            for (const auto& attrib : resources.stage_inputs) {
                ASSERT(compiler.get_decoration_mask(attrib.id) & (1ull << spv::DecorationLocation));
                uint32_t location = compiler.get_decoration(attrib.id, spv::DecorationLocation);

                if (location >= kMaxVertexAttributes) {
                    mDevice->HandleError("Attribute location over limits in the SPIRV");
                    return;
                }

                mUsedVertexAttributes.set(location);
            }

            // Without a location qualifier on vertex outputs, spirv_cross::CompilerMSL gives them all
            // the location 0, causing a compile error.
            for (const auto& attrib : resources.stage_outputs) {
                if (!(compiler.get_decoration_mask(attrib.id) & (1ull << spv::DecorationLocation))) {
                    mDevice->HandleError("Need location qualifier on vertex output");
                    return;
                }
            }
        }

        if (mExecutionModel == nxt::ShaderStage::Fragment) {
            // Without a location qualifier on vertex inputs, spirv_cross::CompilerMSL gives them all
            // the location 0, causing a compile error.
            for (const auto& attrib : resources.stage_inputs) {
                if (!(compiler.get_decoration_mask(attrib.id) & (1ull << spv::DecorationLocation))) {
                    mDevice->HandleError("Need location qualifier on fragment input");
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

    nxt::ShaderStage ShaderModuleBase::GetExecutionModel() const {
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

    bool ShaderModuleBase::IsCompatibleWithBindGroupLayout(size_t group, const BindGroupLayoutBase* layout) {
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

    ShaderModuleBuilder::ShaderModuleBuilder(DeviceBase* device) : Builder(device) {
    }

    std::vector<uint32_t> ShaderModuleBuilder::AcquireSpirv() {
        return std::move(mSpirv);
    }

    ShaderModuleBase* ShaderModuleBuilder::GetResultImpl() {
        if (mSpirv.size() == 0) {
            HandleError("Shader module needs to have the source set");
            return nullptr;
        }

        return mDevice->CreateShaderModule(this);
    }

    void ShaderModuleBuilder::SetSource(uint32_t codeSize, const uint32_t* code) {
        mSpirv.assign(code, code + codeSize);
    }

}
