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

#include "dawn_native/opengl/ShaderModuleGL.h"

#include "common/Assert.h"
#include "common/Platform.h"
#include "dawn_native/SpirvUtils.h"
#include "dawn_native/opengl/DeviceGL.h"

#include <spirv_glsl.hpp>

#include <sstream>

namespace dawn_native { namespace opengl {

    std::string GetBindingName(BindGroupIndex group, BindingNumber bindingNumber) {
        std::ostringstream o;
        o << "dawn_binding_" << static_cast<uint32_t>(group) << "_"
          << static_cast<uint32_t>(bindingNumber);
        return o.str();
    }

    bool operator<(const BindingLocation& a, const BindingLocation& b) {
        return std::tie(a.group, a.binding) < std::tie(b.group, b.binding);
    }

    bool operator<(const CombinedSampler& a, const CombinedSampler& b) {
        return std::tie(a.samplerLocation, a.textureLocation) <
               std::tie(b.samplerLocation, b.textureLocation);
    }

    std::string CombinedSampler::GetName() const {
        std::ostringstream o;
        o << "dawn_combined";
        o << "_" << static_cast<uint32_t>(samplerLocation.group) << "_"
          << static_cast<uint32_t>(samplerLocation.binding);
        o << "_with_" << static_cast<uint32_t>(textureLocation.group) << "_"
          << static_cast<uint32_t>(textureLocation.binding);
        return o.str();
    }

    // static
    ResultOrError<ShaderModule*> ShaderModule::Create(Device* device,
                                                      const ShaderModuleDescriptor* descriptor) {
        Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor));
        DAWN_TRY(module->InitializeBase());
        return module.Detach();
    }

    ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
        : ShaderModuleBase(device, descriptor) {
    }

    std::string ShaderModule::TranslateToGLSL(const char* entryPointName,
                                              SingleShaderStage stage,
                                              CombinedSamplerInfo* combinedSamplers) const {
        // If these options are changed, the values in DawnSPIRVCrossGLSLFastFuzzer.cpp need to
        // be updated.
        spirv_cross::CompilerGLSL::Options options;

        // The range of Z-coordinate in the clipping volume of OpenGL is [-w, w], while it is
        // [0, w] in D3D12, Metal and Vulkan, so we should normalize it in shaders in all
        // backends. See the documentation of
        // spirv_cross::CompilerGLSL::Options::vertex::fixup_clipspace for more details.
        options.vertex.flip_vert_y = true;
        options.vertex.fixup_clipspace = true;

        // TODO(cwallez@chromium.org): discover the backing context version and use that.
#if defined(DAWN_PLATFORM_APPLE)
        options.version = 410;
#else
        options.version = 440;
#endif

        spirv_cross::CompilerGLSL compiler(GetSpirv());
        compiler.set_common_options(options);
        compiler.set_entry_point(entryPointName, ShaderStageToExecutionModel(stage));

        // Extract bindings names so that it can be used to get its location in program.
        // Now translate the separate sampler / textures into combined ones and store their info. We
        // need to do this before removing the set and binding decorations.
        compiler.build_combined_image_samplers();

        for (const auto& combined : compiler.get_combined_image_samplers()) {
            combinedSamplers->emplace_back();

            CombinedSampler* info = &combinedSamplers->back();
            info->samplerLocation.group = BindGroupIndex(
                compiler.get_decoration(combined.sampler_id, spv::DecorationDescriptorSet));
            info->samplerLocation.binding =
                BindingNumber(compiler.get_decoration(combined.sampler_id, spv::DecorationBinding));
            info->textureLocation.group = BindGroupIndex(
                compiler.get_decoration(combined.image_id, spv::DecorationDescriptorSet));
            info->textureLocation.binding =
                BindingNumber(compiler.get_decoration(combined.image_id, spv::DecorationBinding));
            compiler.set_name(combined.combined_id, info->GetName());
        }

        const EntryPointMetadata::BindingInfo& bindingInfo =
            GetEntryPoint(entryPointName, stage).bindings;

        // Change binding names to be "dawn_binding_<group>_<binding>".
        // Also unsets the SPIRV "Binding" decoration as it outputs "layout(binding=)" which
        // isn't supported on OSX's OpenGL.
        for (BindGroupIndex group(0); group < kMaxBindGroupsTyped; ++group) {
            for (const auto& it : bindingInfo[group]) {
                BindingNumber bindingNumber = it.first;
                const auto& info = it.second;

                uint32_t resourceId;
                switch (info.type) {
                    // When the resource is a uniform or shader storage block, we should change the
                    // block name instead of the instance name.
                    case wgpu::BindingType::ReadonlyStorageBuffer:
                    case wgpu::BindingType::StorageBuffer:
                    case wgpu::BindingType::UniformBuffer:
                        resourceId = info.base_type_id;
                        break;
                    default:
                        resourceId = info.id;
                        break;
                }

                compiler.set_name(resourceId, GetBindingName(group, bindingNumber));
                compiler.unset_decoration(info.id, spv::DecorationBinding);
                compiler.unset_decoration(info.id, spv::DecorationDescriptorSet);
            }
        }

        return compiler.compile();
    }

}}  // namespace dawn_native::opengl
