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
#include "dawn/native/CacheRequest.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native {
namespace {

GLenum GLShaderType(SingleShaderStage stage) {
    switch (stage) {
        case SingleShaderStage::Vertex:
            return GL_VERTEX_SHADER;
        case SingleShaderStage::Fragment:
            return GL_FRAGMENT_SHADER;
        case SingleShaderStage::Compute:
            return GL_COMPUTE_SHADER;
    }
    UNREACHABLE();
}

tint::writer::glsl::Version::Standard ToTintGLStandard(opengl::OpenGLVersion::Standard standard) {
    switch (standard) {
        case opengl::OpenGLVersion::Standard::Desktop:
            return tint::writer::glsl::Version::Standard::kDesktop;
        case opengl::OpenGLVersion::Standard::ES:
            return tint::writer::glsl::Version::Standard::kES;
    }
    UNREACHABLE();
}

using BindingMap = std::unordered_map<tint::sem::BindingPoint, tint::sem::BindingPoint>;

#define GLSL_COMPILATION_REQUEST_MEMBERS(X)                                                 \
    X(const tint::Program*, inputProgram)                                                   \
    X(std::string, entryPointName)                                                          \
    X(SingleShaderStage, stage)                                                             \
    X(tint::transform::MultiplanarExternalTexture::BindingsMap, externalTextureBindings)    \
    X(BindingMap, glBindings)                                                               \
    X(std::optional<tint::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(LimitsForCompilationRequest, limits)                                                  \
    X(opengl::OpenGLVersion::Standard, glVersionStandard)                                   \
    X(uint32_t, glVersionMajor)                                                             \
    X(uint32_t, glVersionMinor)

DAWN_MAKE_CACHE_REQUEST(GLSLCompilationRequest, GLSL_COMPILATION_REQUEST_MEMBERS);
#undef GLSL_COMPILATION_REQUEST_MEMBERS

#define GLSL_COMPILATION_MEMBERS(X)  \
    X(std::string, glsl)             \
    X(bool, needsPlaceholderSampler) \
    X(opengl::CombinedSamplerInfo, combinedSamplerInfo)

DAWN_SERIALIZABLE(struct, GLSLCompilation, GLSL_COMPILATION_MEMBERS){};
#undef GLSL_COMPILATION_MEMBERS

}  // namespace
}  // namespace dawn::native

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

ResultOrError<GLuint> ShaderModule::CompileShader(const OpenGLFunctions& gl,
                                                  const ProgrammableStage& programmableStage,
                                                  SingleShaderStage stage,
                                                  CombinedSamplerInfo* combinedSamplers,
                                                  const PipelineLayout* layout,
                                                  bool* needsPlaceholderSampler) const {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "TranslateToGLSL");

    const OpenGLVersion& version = ToBackend(GetDevice())->GetGL().GetVersion();

    using tint::transform::BindingPoint;
    // Since (non-Vulkan) GLSL does not support descriptor sets, generate a
    // mapping from the original group/binding pair to a binding-only
    // value. This mapping will be used by Tint to remap all global
    // variables to the 1D space.
    const BindingInfoArray& moduleBindingInfo =
        GetEntryPoint(programmableStage.entryPoint).bindings;
    std::unordered_map<tint::sem::BindingPoint, tint::sem::BindingPoint> glBindings;
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayoutBase* bgl = layout->GetBindGroupLayout(group);
        const auto& groupBindingInfo = moduleBindingInfo[group];
        for (const auto& [bindingNumber, bindingInfo] : groupBindingInfo) {
            BindingIndex bindingIndex = bgl->GetBindingIndex(bindingNumber);
            GLuint shaderIndex = layout->GetBindingIndexInfo()[group][bindingIndex];
            BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                         static_cast<uint32_t>(bindingNumber)};
            BindingPoint dstBindingPoint{0, shaderIndex};
            if (srcBindingPoint != dstBindingPoint) {
                glBindings.emplace(srcBindingPoint, dstBindingPoint);
            }
        }
    }

    std::optional<tint::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

    const CombinedLimits& limits = GetDevice()->GetLimits();

    GLSLCompilationRequest req = {};
    req.inputProgram = GetTintProgram();
    req.stage = stage;
    req.entryPointName = programmableStage.entryPoint;
    req.externalTextureBindings = BuildExternalTextureTransformBindings(layout);
    req.glBindings = std::move(glBindings);
    req.substituteOverrideConfig = std::move(substituteOverrideConfig);
    req.limits = LimitsForCompilationRequest::Create(limits.v1);
    req.glVersionStandard = version.GetStandard();
    req.glVersionMajor = version.GetMajor();
    req.glVersionMinor = version.GetMinor();

    CacheResult<GLSLCompilation> compilationResult;
    DAWN_TRY_LOAD_OR_RUN(
        compilationResult, GetDevice(), std::move(req), GLSLCompilation::FromBlob,
        [](GLSLCompilationRequest r) -> ResultOrError<GLSLCompilation> {
            tint::transform::Manager transformManager;
            tint::transform::DataMap transformInputs;

            if (!r.externalTextureBindings.empty()) {
                transformManager.Add<tint::transform::MultiplanarExternalTexture>();
                transformInputs.Add<tint::transform::MultiplanarExternalTexture::NewBindingPoints>(
                    std::move(r.externalTextureBindings));
            }

            if (r.substituteOverrideConfig) {
                transformManager.Add<tint::transform::SingleEntryPoint>();
                transformInputs.Add<tint::transform::SingleEntryPoint::Config>(r.entryPointName);
                // This needs to run after SingleEntryPoint transform which removes unused overrides
                // for current entry point.
                transformManager.Add<tint::transform::SubstituteOverride>();
                transformInputs.Add<tint::transform::SubstituteOverride::Config>(
                    std::move(r.substituteOverrideConfig).value());
            }

            tint::Program program;
            DAWN_TRY_ASSIGN(program, RunTransforms(&transformManager, r.inputProgram,
                                                   transformInputs, nullptr, nullptr));

            if (r.stage == SingleShaderStage::Compute) {
                // Validate workgroup size after program runs transforms.
                Extent3D _;
                DAWN_TRY_ASSIGN(_, ValidateComputeStageWorkgroupSize(
                                       program, r.entryPointName.c_str(), r.limits));
            }

            tint::writer::glsl::Options tintOptions;
            tintOptions.version = tint::writer::glsl::Version(ToTintGLStandard(r.glVersionStandard),
                                                              r.glVersionMajor, r.glVersionMinor);

            // When textures are accessed without a sampler (e.g., textureLoad()),
            // GetSamplerTextureUses() will return this sentinel value.
            BindingPoint placeholderBindingPoint{static_cast<uint32_t>(kMaxBindGroupsTyped), 0};

            bool needsPlaceholderSampler = false;
            tint::inspector::Inspector inspector(&program);
            // Find all the sampler/texture pairs for this entry point, and create
            // CombinedSamplers for them. CombinedSampler records the binding points
            // of the original texture and sampler, and generates a unique name. The
            // corresponding uniforms will be retrieved by these generated names
            // in PipelineGL. Any texture-only references will have
            // "usePlaceholderSampler" set to true, and only the texture binding point
            // will be used in naming them. In addition, Dawn will bind a
            // non-filtering sampler for them (see PipelineGL).
            auto uses = inspector.GetSamplerTextureUses(r.entryPointName, placeholderBindingPoint);
            CombinedSamplerInfo combinedSamplerInfo;
            for (const auto& use : uses) {
                combinedSamplerInfo.emplace_back();

                CombinedSampler* info = &combinedSamplerInfo.back();
                if (use.sampler_binding_point == placeholderBindingPoint) {
                    info->usePlaceholderSampler = true;
                    needsPlaceholderSampler = true;
                    tintOptions.placeholder_binding_point = placeholderBindingPoint;
                } else {
                    info->usePlaceholderSampler = false;
                }
                info->samplerLocation.group = BindGroupIndex(use.sampler_binding_point.group);
                info->samplerLocation.binding = BindingNumber(use.sampler_binding_point.binding);
                info->textureLocation.group = BindGroupIndex(use.texture_binding_point.group);
                info->textureLocation.binding = BindingNumber(use.texture_binding_point.binding);
                tintOptions.binding_map[use] = info->GetName();
            }
            tintOptions.binding_points = std::move(r.glBindings);
            tintOptions.allow_collisions = true;

            auto result = tint::writer::glsl::Generate(&program, tintOptions, r.entryPointName);
            DAWN_INVALID_IF(!result.success, "An error occured while generating GLSL: %s.",
                            result.error);

            return GLSLCompilation{
                {std::move(result.glsl), needsPlaceholderSampler, std::move(combinedSamplerInfo)}};
        });

    if (GetDevice()->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated GLSL */" << std::endl << compilationResult->glsl;

        GetDevice()->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    GLuint shader = gl.CreateShader(GLShaderType(stage));
    const char* source = compilationResult->glsl.c_str();
    gl.ShaderSource(shader, 1, &source, nullptr);
    gl.CompileShader(shader);

    GLint compileStatus = GL_FALSE;
    gl.GetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLint infoLogLength = 0;
        gl.GetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 1) {
            std::vector<char> buffer(infoLogLength);
            gl.GetShaderInfoLog(shader, infoLogLength, nullptr, &buffer[0]);
            gl.DeleteShader(shader);
            return DAWN_VALIDATION_ERROR("%s\nProgram compilation failed:\n%s", source,
                                         buffer.data());
        }
    }

    GetDevice()->GetBlobCache()->EnsureStored(compilationResult);
    *needsPlaceholderSampler = compilationResult->needsPlaceholderSampler;
    *combinedSamplers = std::move(compilationResult->combinedSamplerInfo);
    return shader;
}

}  // namespace dawn::native::opengl
