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
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/SpirvUtils.h"
#include "dawn_native/TintUtils.h"
#include "dawn_native/opengl/DeviceGL.h"
#include "dawn_native/opengl/PipelineLayoutGL.h"

#include <spirv_glsl.hpp>

// Tint include must be after spirv_glsl.hpp, because spirv-cross has its own
// version of spirv_headers. We also need to undef SPV_REVISION because SPIRV-Cross
// is at 3 while spirv-headers is at 4.
#undef SPV_REVISION
#include <tint/tint.h>

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
        return std::tie(a.useDummySampler, a.samplerLocation, a.textureLocation) <
               std::tie(b.useDummySampler, a.samplerLocation, b.textureLocation);
    }

    std::string CombinedSampler::GetName() const {
        std::ostringstream o;
        o << "dawn_combined";
        if (useDummySampler) {
            o << "_dummy_sampler";
        } else {
            o << "_" << static_cast<uint32_t>(samplerLocation.group) << "_"
              << static_cast<uint32_t>(samplerLocation.binding);
        }
        o << "_with_" << static_cast<uint32_t>(textureLocation.group) << "_"
          << static_cast<uint32_t>(textureLocation.binding);
        return o.str();
    }

    // static
    ResultOrError<Ref<ShaderModule>> ShaderModule::Create(Device* device,
                                                          const ShaderModuleDescriptor* descriptor,
                                                          ShaderModuleParseResult* parseResult) {
        Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor));
        DAWN_TRY(module->Initialize(parseResult));
        return module;
    }

    ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
        : ShaderModuleBase(device, descriptor) {
    }

    MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult) {
        ScopedTintICEHandler scopedICEHandler(GetDevice());

        DAWN_TRY(InitializeBase(parseResult));
        // Tint currently does not support emitting GLSL, so when provided a Tint program need to
        // generate SPIRV and SPIRV-Cross reflection data to be used in this backend.
        if (GetDevice()->IsToggleEnabled(Toggle::UseTintGenerator)) {
            tint::writer::spirv::Options options;
            options.disable_workgroup_init =
                GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);
            auto result = tint::writer::spirv::Generate(GetTintProgram(), options);
            if (!result.success) {
                std::ostringstream errorStream;
                errorStream << "Generator: " << result.error << std::endl;
                return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
            }

            DAWN_TRY_ASSIGN(mGLEntryPoints,
                            ReflectShaderUsingSPIRVCross(GetDevice(), result.spirv));
        }

        return {};
    }

    ResultOrError<std::string> ShaderModule::TranslateToGLSL(const char* entryPointName,
                                                             SingleShaderStage stage,
                                                             CombinedSamplerInfo* combinedSamplers,
                                                             const PipelineLayout* layout,
                                                             bool* needsDummySampler) const {
        // If these options are changed, the values in DawnSPIRVCrossGLSLFastFuzzer.cpp need to
        // be updated.
        spirv_cross::CompilerGLSL::Options options;

        // The range of Z-coordinate in the clipping volume of OpenGL is [-w, w], while it is
        // [0, w] in D3D12, Metal and Vulkan, so we should normalize it in shaders in all
        // backends. See the documentation of
        // spirv_cross::CompilerGLSL::Options::vertex::fixup_clipspace for more details.
        options.vertex.flip_vert_y = true;
        options.vertex.fixup_clipspace = true;

        const OpenGLVersion& version = ToBackend(GetDevice())->gl.GetVersion();
        if (version.IsDesktop()) {
            // The computation of GLSL version below only works for 3.3 and above.
            ASSERT(version.IsAtLeast(3, 3));
        }
        options.es = version.IsES();
        options.version = version.GetMajor() * 100 + version.GetMinor() * 10;

        std::vector<uint32_t> spirv;
        if (GetDevice()->IsToggleEnabled(Toggle::UseTintGenerator)) {
            tint::transform::SingleEntryPoint singleEntryPointTransform;

            tint::transform::DataMap transformInputs;
            transformInputs.Add<tint::transform::SingleEntryPoint::Config>(entryPointName);

            tint::Program program;
            DAWN_TRY_ASSIGN(program, RunTransforms(&singleEntryPointTransform, GetTintProgram(),
                                                   transformInputs, nullptr, nullptr));

            tint::writer::spirv::Options options;
            options.disable_workgroup_init =
                GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);
            auto result = tint::writer::spirv::Generate(&program, options);
            if (!result.success) {
                std::ostringstream errorStream;
                errorStream << "Generator: " << result.error << std::endl;
                return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
            }

            spirv = std::move(result.spirv);
        } else {
            spirv = GetSpirv();
        }
        spirv_cross::CompilerGLSL compiler(std::move(spirv));
        compiler.set_common_options(options);
        compiler.set_entry_point(entryPointName, ShaderStageToExecutionModel(stage));

        // Analyzes all OpImageFetch opcodes and checks if there are instances where
        // said instruction is used without a combined image sampler.
        // GLSL does not support texelFetch without a sampler.
        // To workaround this, we must inject a dummy sampler which can be used to form a sampler2D
        // at the call-site of texelFetch as necessary.
        spirv_cross::VariableID dummySamplerId = compiler.build_dummy_sampler_for_combined_images();

        // Extract bindings names so that it can be used to get its location in program.
        // Now translate the separate sampler / textures into combined ones and store their info. We
        // need to do this before removing the set and binding decorations.
        compiler.build_combined_image_samplers();

        for (const auto& combined : compiler.get_combined_image_samplers()) {
            combinedSamplers->emplace_back();

            CombinedSampler* info = &combinedSamplers->back();
            if (combined.sampler_id == dummySamplerId) {
                *needsDummySampler = true;
                info->useDummySampler = true;
                info->samplerLocation = {};
            } else {
                info->useDummySampler = false;
                info->samplerLocation.group = BindGroupIndex(
                    compiler.get_decoration(combined.sampler_id, spv::DecorationDescriptorSet));
                info->samplerLocation.binding = BindingNumber(
                    compiler.get_decoration(combined.sampler_id, spv::DecorationBinding));
            }
            info->textureLocation.group = BindGroupIndex(
                compiler.get_decoration(combined.image_id, spv::DecorationDescriptorSet));
            info->textureLocation.binding =
                BindingNumber(compiler.get_decoration(combined.image_id, spv::DecorationBinding));
            compiler.set_name(combined.combined_id, info->GetName());
        }

        const EntryPointMetadata::BindingInfoArray& bindingInfo =
            GetDevice()->IsToggleEnabled(Toggle::UseTintGenerator)
                ? (*mGLEntryPoints.at(entryPointName)).bindings
                : GetEntryPoint(entryPointName).bindings;

        // Change binding names to be "dawn_binding_<group>_<binding>".
        // Also unsets the SPIRV "Binding" decoration as it outputs "layout(binding=)" which
        // isn't supported on OSX's OpenGL.
        const PipelineLayout::BindingIndexInfo& indices = layout->GetBindingIndexInfo();

        // Modify the decoration of variables so that SPIRV-Cross outputs only
        //  layout(binding=<index>) for interface variables.
        //
        // When the use_tint_generator toggle is on, Tint is used for the reflection of bindings
        // for the implicit pipeline layout and pipeline/layout validation, but bindingInfo is set
        // to mGLEntryPoints which is the SPIRV-Cross reflection. Tint reflects bindings used more
        // precisely than SPIRV-Cross so some bindings in bindingInfo might not exist in the layout
        // and querying the layout for them would cause an ASSERT. That's why we defensively check
        // that bindings are in the layout before modifying them. This slight hack is ok because in
        // the long term we will use Tint to produce GLSL.
        for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            for (const auto& it : bindingInfo[group]) {
                const BindGroupLayoutBase* bgl = layout->GetBindGroupLayout(group);
                BindingNumber bindingNumber = it.first;
                const auto& info = it.second;

                if (!bgl->HasBinding(bindingNumber)) {
                    continue;
                }

                // Remove the name of the base type. This works around an issue where if the SPIRV
                // has two uniform/storage interface variables that point to the same base type,
                // then SPIRV-Cross would emit two bindings with type names that conflict:
                //
                //   layout(binding=0) uniform Buf {...} binding0;
                //   layout(binding=1) uniform Buf {...} binding1;
                compiler.set_name(info.base_type_id, "");

                BindingIndex bindingIndex = bgl->GetBindingIndex(bindingNumber);

                compiler.unset_decoration(info.id, spv::DecorationDescriptorSet);
                compiler.set_decoration(info.id, spv::DecorationBinding,
                                        indices[group][bindingIndex]);
            }
        }

        return compiler.compile();
    }

}}  // namespace dawn_native::opengl
