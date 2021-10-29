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
#include "dawn_native/SpirvValidation.h"
#include "dawn_native/TintUtils.h"
#include "dawn_native/opengl/DeviceGL.h"
#include "dawn_native/opengl/PipelineLayoutGL.h"
#include "dawn_native/opengl/SpirvUtils.h"

#include <spirv_glsl.hpp>

// Tint include must be after spirv_glsl.hpp, because spirv-cross has its own
// version of spirv_headers. We also need to undef SPV_REVISION because SPIRV-Cross
// is at 3 while spirv-headers is at 4.
#undef SPV_REVISION
#include <tint/tint.h>
#include <spirv-tools/libspirv.hpp>

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

    ResultOrError<std::unique_ptr<BindingInfoArray>> ExtractSpirvInfo(
        const DeviceBase* device,
        const spirv_cross::Compiler& compiler,
        const std::string& entryPointName,
        SingleShaderStage stage) {
        const auto& resources = compiler.get_shader_resources();

        // Fill in bindingInfo with the SPIRV bindings
        auto ExtractResourcesBinding =
            [](const DeviceBase* device,
               const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
               const spirv_cross::Compiler& compiler, BindingInfoType bindingType,
               BindingInfoArray* bindings, bool isStorageBuffer = false) -> MaybeError {
            for (const auto& resource : resources) {
                DAWN_INVALID_IF(
                    !compiler.get_decoration_bitset(resource.id).get(spv::DecorationBinding),
                    "No Binding decoration set for resource");

                DAWN_INVALID_IF(
                    !compiler.get_decoration_bitset(resource.id).get(spv::DecorationDescriptorSet),
                    "No Descriptor Decoration set for resource");

                BindingNumber bindingNumber(
                    compiler.get_decoration(resource.id, spv::DecorationBinding));
                BindGroupIndex bindGroupIndex(
                    compiler.get_decoration(resource.id, spv::DecorationDescriptorSet));

                DAWN_INVALID_IF(bindGroupIndex >= kMaxBindGroupsTyped,
                                "Bind group index over limits in the SPIRV");

                const auto& it =
                    (*bindings)[bindGroupIndex].emplace(bindingNumber, ShaderBindingInfo{});
                DAWN_INVALID_IF(!it.second, "Shader has duplicate bindings");

                ShaderBindingInfo* info = &it.first->second;
                info->id = resource.id;
                info->base_type_id = resource.base_type_id;
                info->bindingType = bindingType;

                switch (bindingType) {
                    case BindingInfoType::Texture: {
                        spirv_cross::SPIRType::ImageType imageType =
                            compiler.get_type(info->base_type_id).image;
                        spirv_cross::SPIRType::BaseType textureComponentType =
                            compiler.get_type(imageType.type).basetype;

                        info->texture.viewDimension =
                            SpirvDimToTextureViewDimension(imageType.dim, imageType.arrayed);
                        info->texture.multisampled = imageType.ms;
                        info->texture.compatibleSampleTypes =
                            SpirvBaseTypeToSampleTypeBit(textureComponentType);

                        if (imageType.depth) {
                            DAWN_INVALID_IF(
                                (info->texture.compatibleSampleTypes & SampleTypeBit::Float) == 0,
                                "Depth textures must have a float type");
                            info->texture.compatibleSampleTypes = SampleTypeBit::Depth;
                        }

                        DAWN_INVALID_IF(imageType.ms && imageType.arrayed,
                                        "Multisampled array textures aren't supported");
                        break;
                    }
                    case BindingInfoType::Buffer: {
                        // Determine buffer size, with a minimum of 1 element in the runtime
                        // array
                        spirv_cross::SPIRType type = compiler.get_type(info->base_type_id);
                        info->buffer.minBindingSize =
                            compiler.get_declared_struct_size_runtime_array(type, 1);

                        // Differentiate between readonly storage bindings and writable ones
                        // based on the NonWritable decoration.
                        // TODO(dawn:527): Could isStorageBuffer be determined by calling
                        // compiler.get_storage_class(resource.id)?
                        if (isStorageBuffer) {
                            spirv_cross::Bitset flags =
                                compiler.get_buffer_block_flags(resource.id);
                            if (flags.get(spv::DecorationNonWritable)) {
                                info->buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
                            } else {
                                info->buffer.type = wgpu::BufferBindingType::Storage;
                            }
                        } else {
                            info->buffer.type = wgpu::BufferBindingType::Uniform;
                        }
                        break;
                    }
                    case BindingInfoType::StorageTexture: {
                        spirv_cross::Bitset flags = compiler.get_decoration_bitset(resource.id);
                        DAWN_INVALID_IF(!flags.get(spv::DecorationNonReadable),
                                        "Read-write storage textures are not supported.");
                        info->storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;

                        spirv_cross::SPIRType::ImageType imageType =
                            compiler.get_type(info->base_type_id).image;
                        wgpu::TextureFormat storageTextureFormat =
                            SpirvImageFormatToTextureFormat(imageType.format);
                        DAWN_INVALID_IF(storageTextureFormat == wgpu::TextureFormat::Undefined,
                                        "Invalid image format declaration on storage image.");

                        const Format& format = device->GetValidInternalFormat(storageTextureFormat);
                        DAWN_INVALID_IF(!format.supportsStorageUsage,
                                        "The storage texture format (%s) is not supported.",
                                        storageTextureFormat);

                        DAWN_INVALID_IF(imageType.ms,
                                        "Multisampled storage textures aren't supported.");

                        DAWN_INVALID_IF(imageType.depth,
                                        "Depth storage textures aren't supported.");

                        info->storageTexture.format = storageTextureFormat;
                        info->storageTexture.viewDimension =
                            SpirvDimToTextureViewDimension(imageType.dim, imageType.arrayed);
                        break;
                    }
                    case BindingInfoType::Sampler: {
                        info->sampler.isComparison = false;
                        break;
                    }
                    case BindingInfoType::ExternalTexture: {
                        return DAWN_FORMAT_VALIDATION_ERROR("External textures are not supported.");
                    }
                }
            }
            return {};
        };

        std::unique_ptr<BindingInfoArray> resultBindings = std::make_unique<BindingInfoArray>();
        BindingInfoArray* bindings = resultBindings.get();
        DAWN_TRY(ExtractResourcesBinding(device, resources.uniform_buffers, compiler,
                                         BindingInfoType::Buffer, bindings));
        DAWN_TRY(ExtractResourcesBinding(device, resources.separate_images, compiler,
                                         BindingInfoType::Texture, bindings));
        DAWN_TRY(ExtractResourcesBinding(device, resources.separate_samplers, compiler,
                                         BindingInfoType::Sampler, bindings));
        DAWN_TRY(ExtractResourcesBinding(device, resources.storage_buffers, compiler,
                                         BindingInfoType::Buffer, bindings, true));
        // ReadonlyStorageTexture is used as a tag to do general storage texture handling.
        DAWN_TRY(ExtractResourcesBinding(device, resources.storage_images, compiler,
                                         BindingInfoType::StorageTexture, resultBindings.get()));

        return {std::move(resultBindings)};
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

    // static
    ResultOrError<BindingInfoArrayTable> ShaderModule::ReflectShaderUsingSPIRVCross(
        DeviceBase* device,
        const std::vector<uint32_t>& spirv) {
        BindingInfoArrayTable result;
        spirv_cross::Compiler compiler(spirv);
        for (const spirv_cross::EntryPoint& entryPoint : compiler.get_entry_points_and_stages()) {
            ASSERT(result.count(entryPoint.name) == 0);

            SingleShaderStage stage = ExecutionModelToShaderStage(entryPoint.execution_model);
            compiler.set_entry_point(entryPoint.name, entryPoint.execution_model);

            std::unique_ptr<BindingInfoArray> bindings;
            DAWN_TRY_ASSIGN(bindings, ExtractSpirvInfo(device, compiler, entryPoint.name, stage));
            result[entryPoint.name] = std::move(bindings);
        }
        return std::move(result);
    }

    MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult) {
        ScopedTintICEHandler scopedICEHandler(GetDevice());

        DAWN_TRY(InitializeBase(parseResult));
        // Tint currently does not support emitting GLSL, so when provided a Tint program need to
        // generate SPIRV and SPIRV-Cross reflection data to be used in this backend.
        tint::writer::spirv::Options options;
        options.disable_workgroup_init = GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);
        auto result = tint::writer::spirv::Generate(GetTintProgram(), options);
        DAWN_INVALID_IF(!result.success, "An error occured while generating SPIR-V: %s.",
                        result.error);

        DAWN_TRY_ASSIGN(mGLBindings, ReflectShaderUsingSPIRVCross(GetDevice(), result.spirv));

        return {};
    }

    ResultOrError<std::string> ShaderModule::TranslateToGLSL(const char* entryPointName,
                                                             SingleShaderStage stage,
                                                             CombinedSamplerInfo* combinedSamplers,
                                                             const PipelineLayout* layout,
                                                             bool* needsDummySampler) const {
        tint::transform::SingleEntryPoint singleEntryPointTransform;

        tint::transform::DataMap transformInputs;
        transformInputs.Add<tint::transform::SingleEntryPoint::Config>(entryPointName);

        tint::Program program;
        DAWN_TRY_ASSIGN(program, RunTransforms(&singleEntryPointTransform, GetTintProgram(),
                                               transformInputs, nullptr, nullptr));

        tint::writer::spirv::Options tintOptions;
        tintOptions.disable_workgroup_init =
            GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);
        auto result = tint::writer::spirv::Generate(&program, tintOptions);
        DAWN_INVALID_IF(!result.success, "An error occured while generating SPIR-V: %s.",
                        result.error);

        std::vector<uint32_t> spirv = std::move(result.spirv);
        DAWN_TRY(
            ValidateSpirv(GetDevice(), spirv, GetDevice()->IsToggleEnabled(Toggle::DumpShaders)));

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

        const BindingInfoArray& bindingInfo = *(mGLBindings.at(entryPointName));

        // Change binding names to be "dawn_binding_<group>_<binding>".
        // Also unsets the SPIRV "Binding" decoration as it outputs "layout(binding=)" which
        // isn't supported on OSX's OpenGL.
        const PipelineLayout::BindingIndexInfo& indices = layout->GetBindingIndexInfo();

        // Modify the decoration of variables so that SPIRV-Cross outputs only
        //  layout(binding=<index>) for interface variables.
        //
        // Tint is used for the reflection of bindings for the implicit pipeline layout and
        // pipeline/layout validation, but bindingInfo is set to mGLEntryPoints which is the
        // SPIRV-Cross reflection. Tint reflects bindings used more precisely than SPIRV-Cross so
        // some bindings in bindingInfo might not exist in the layout and querying the layout for
        // them would cause an ASSERT. That's why we defensively check that bindings are in the
        // layout before modifying them. This slight hack is ok because in the long term we will use
        // Tint to produce GLSL.
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

        std::string glsl = compiler.compile();

        if (GetDevice()->IsToggleEnabled(Toggle::DumpShaders)) {
            std::ostringstream dumpedMsg;
            dumpedMsg << "/* Dumped generated GLSL */" << std::endl << glsl;

            GetDevice()->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
        }

        return glsl;
    }

}}  // namespace dawn_native::opengl
