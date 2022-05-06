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

#include <sstream>
#include <utility>

#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

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
    return std::tie(a.usePlaceholderSampler, a.samplerLocation, a.textureLocation) <
           std::tie(b.usePlaceholderSampler, a.samplerLocation, b.textureLocation);
}

std::string CombinedSampler::GetName() const {
    std::ostringstream o;
    o << "dawn_combined";
    if (usePlaceholderSampler) {
        o << "_placeholder_sampler";
    } else {
        o << "_" << static_cast<uint32_t>(samplerLocation.group) << "_"
          << static_cast<uint32_t>(samplerLocation.binding);
    }
    o << "_with_" << static_cast<uint32_t>(textureLocation.group) << "_"
      << static_cast<uint32_t>(textureLocation.binding);
    return o.str();
}

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const ShaderModuleDescriptor* descriptor,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor));
    DAWN_TRY(module->Initialize(parseResult, compilationMessages));
    return module;
}

ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
    : ShaderModuleBase(device, descriptor) {}

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    ScopedTintICEHandler scopedICEHandler(GetDevice());

    DAWN_TRY(InitializeBase(parseResult, compilationMessages));

    return {};
}

ResultOrError<std::string> ShaderModule::TranslateToGLSL(const char* entryPointName,
                                                         SingleShaderStage stage,
                                                         CombinedSamplerInfo* combinedSamplers,
                                                         const PipelineLayout* layout,
                                                         bool* needsPlaceholderSampler) const {
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
    // "usePlaceholderSampler" set to true, and only the texture binding point
    // will be used in naming them. In addition, Dawn will bind a
    // non-filtering sampler for them (see PipelineGL).
    auto uses = inspector.GetSamplerTextureUses(entryPointName, placeholderBindingPoint);
    for (const auto& use : uses) {
        combinedSamplers->emplace_back();

        CombinedSampler* info = &combinedSamplers->back();
        if (use.sampler_binding_point == placeholderBindingPoint) {
            info->usePlaceholderSampler = true;
            *needsPlaceholderSampler = true;
        } else {
            info->usePlaceholderSampler = false;
        }
        info->samplerLocation.group = BindGroupIndex(use.sampler_binding_point.group);
        info->samplerLocation.binding = BindingNumber(use.sampler_binding_point.binding);
        info->textureLocation.group = BindGroupIndex(use.texture_binding_point.group);
        info->textureLocation.binding = BindingNumber(use.texture_binding_point.binding);
        tintOptions.binding_map[use] = info->GetName();
    }
    if (*needsPlaceholderSampler) {
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
    DAWN_INVALID_IF(!result.success, "An error occured while generating GLSL: %s.", result.error);
    std::string glsl = std::move(result.glsl);

    if (GetDevice()->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated GLSL */" << std::endl << glsl;

        GetDevice()->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    return glsl;
}

}  // namespace dawn::native::opengl
