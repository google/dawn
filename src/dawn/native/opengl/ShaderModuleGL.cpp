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

#include "dawn/native/opengl/ShaderModuleGL.h"

#include "dawn/common/Assert.h"
#include "dawn/common/Platform.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/SpirvValidation.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/opengl/SpirvUtils.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include <spirv_glsl.hpp>

// Tint include must be after spirv_glsl.hpp, because spirv-cross has its own
// version of spirv_headers. We also need to undef SPV_REVISION because SPIRV-Cross
// is at 3 while spirv-headers is at 4.
#undef SPV_REVISION
#include <tint/tint.h>
#include <spirv-tools/libspirv.hpp>

#include <sstream>

namespace dawn::native::opengl {

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

                const auto& [entry, inserted] =
                    (*bindings)[bindGroupIndex].emplace(bindingNumber, ShaderBindingInfo{});
                DAWN_INVALID_IF(!inserted, "Shader has duplicate bindings");

                ShaderBindingInfo* info = &entry->second;
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
        TRACE_EVENT0(GetDevice()->GetPlatform(), General, "TranslateToGLSL");
        tint::transform::Manager transformManager;
        tint::transform::DataMap transformInputs;

        AddExternalTextureTransform(layout, &transformManager, &transformInputs);

        tint::Program program;
        DAWN_TRY_ASSIGN(program, RunTransforms(&transformManager, GetTintProgram(), transformInputs,
                                               nullptr, nullptr));
        const OpenGLVersion& version = ToBackend(GetDevice())->gl.GetVersion();

        tint::writer::glsl::Options tintOptions;
        using Version = tint::writer::glsl::Version;
        tintOptions.version =
            Version(version.IsDesktop() ? Version::Standard::kDesktop : Version::Standard::kES,
                    version.GetMajor(), version.GetMinor());

        using tint::transform::BindingPoint;
        // When textures are accessed without a sampler (e.g., textureLoad()),
        // GetSamplerTextureUses() will return this sentinel value.
        BindingPoint placeholderBindingPoint{static_cast<uint32_t>(kMaxBindGroupsTyped), 0};

        tint::inspector::Inspector inspector(&program);
        // Find all the sampler/texture pairs for this entry point, and create
        // CombinedSamplers for them. CombinedSampler records the binding points
        // of the original texture and sampler, and generates a unique name. The
        // corresponding uniforms will be retrieved by these generated names
        // in PipelineGL. Any texture-only references will have
        // "useDummySampler" set to true, and only the texture binding point
        // will be used in naming them. In addition, Dawn will bind a
        // non-filtering sampler for them (see PipelineGL).
        auto uses = inspector.GetSamplerTextureUses(entryPointName, placeholderBindingPoint);
        for (const auto& use : uses) {
            combinedSamplers->emplace_back();

            CombinedSampler* info = &combinedSamplers->back();
            if (use.sampler_binding_point == placeholderBindingPoint) {
                info->useDummySampler = true;
                *needsDummySampler = true;
            } else {
                info->useDummySampler = false;
            }
            info->samplerLocation.group = BindGroupIndex(use.sampler_binding_point.group);
            info->samplerLocation.binding = BindingNumber(use.sampler_binding_point.binding);
            info->textureLocation.group = BindGroupIndex(use.texture_binding_point.group);
            info->textureLocation.binding = BindingNumber(use.texture_binding_point.binding);
            tintOptions.binding_map[use] = info->GetName();
        }
        if (*needsDummySampler) {
            tintOptions.placeholder_binding_point = placeholderBindingPoint;
        }

        // Since (non-Vulkan) GLSL does not support descriptor sets, generate a
        // mapping from the original group/binding pair to a binding-only
        // value. This mapping will be used by Tint to remap all global
        // variables to the 1D space.
        for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            const BindGroupLayoutBase::BindingMap& bindingMap =
                layout->GetBindGroupLayout(group)->GetBindingMap();
            for (const auto& it : bindingMap) {
                BindingNumber bindingNumber = it.first;
                BindingIndex bindingIndex = it.second;
                const BindingInfo& bindingInfo =
                    layout->GetBindGroupLayout(group)->GetBindingInfo(bindingIndex);
                if (!(bindingInfo.visibility & StageBit(stage))) {
                    continue;
                }

                uint32_t shaderIndex = layout->GetBindingIndexInfo()[group][bindingIndex];
                BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                             static_cast<uint32_t>(bindingNumber)};
                BindingPoint dstBindingPoint{0, shaderIndex};
                tintOptions.binding_points.emplace(srcBindingPoint, dstBindingPoint);
            }
            tintOptions.allow_collisions = true;
        }
        auto result = tint::writer::glsl::Generate(&program, tintOptions, entryPointName);
        DAWN_INVALID_IF(!result.success, "An error occured while generating GLSL: %s.",
                        result.error);
        std::string glsl = std::move(result.glsl);

        if (GetDevice()->IsToggleEnabled(Toggle::DumpShaders)) {
            std::ostringstream dumpedMsg;
            dumpedMsg << "/* Dumped generated GLSL */" << std::endl << glsl;

            GetDevice()->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
        }

        return glsl;
    }

}  // namespace dawn::native::opengl
