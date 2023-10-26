// Copyright 2017 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/opengl/ShaderModuleGL.h"

#include <sstream>
#include <utility>

#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/CacheRequest.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/PipelineLayoutGL.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
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
    DAWN_UNREACHABLE();
}

tint::glsl::writer::Version::Standard ToTintGLStandard(opengl::OpenGLVersion::Standard standard) {
    switch (standard) {
        case opengl::OpenGLVersion::Standard::Desktop:
            return tint::glsl::writer::Version::Standard::kDesktop;
        case opengl::OpenGLVersion::Standard::ES:
            return tint::glsl::writer::Version::Standard::kES;
    }
    DAWN_UNREACHABLE();
}

using BindingMap = std::unordered_map<tint::BindingPoint, tint::BindingPoint>;

opengl::CombinedSampler* AppendCombinedSampler(opengl::CombinedSamplerInfo* info,
                                               tint::inspector::SamplerTexturePair pair,
                                               tint::BindingPoint placeholderBindingPoint) {
    info->emplace_back();
    opengl::CombinedSampler* combinedSampler = &info->back();
    combinedSampler->usePlaceholderSampler = pair.sampler_binding_point == placeholderBindingPoint;
    combinedSampler->samplerLocation.group = BindGroupIndex(pair.sampler_binding_point.group);
    combinedSampler->samplerLocation.binding = BindingNumber(pair.sampler_binding_point.binding);
    combinedSampler->textureLocation.group = BindGroupIndex(pair.texture_binding_point.group);
    combinedSampler->textureLocation.binding = BindingNumber(pair.texture_binding_point.binding);
    return combinedSampler;
}

#define GLSL_COMPILATION_REQUEST_MEMBERS(X)                                                      \
    X(const tint::Program*, inputProgram)                                                        \
    X(std::string, entryPointName)                                                               \
    X(SingleShaderStage, stage)                                                                  \
    X(std::optional<tint::ast::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(LimitsForCompilationRequest, limits)                                                       \
    X(tint::glsl::writer::Options, tintOptions)                                                  \
    X(CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*>, platform)

DAWN_MAKE_CACHE_REQUEST(GLSLCompilationRequest, GLSL_COMPILATION_REQUEST_MEMBERS);
#undef GLSL_COMPILATION_REQUEST_MEMBERS

#define GLSL_COMPILATION_MEMBERS(X)     \
    X(std::string, glsl)                \
    X(bool, needsInternalUniformBuffer) \
    X(tint::TextureBuiltinsFromUniformOptions::BindingPointToFieldAndOffset, bindingPointToData)

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

ResultOrError<GLuint> ShaderModule::CompileShader(
    const OpenGLFunctions& gl,
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    CombinedSamplerInfo* combinedSamplers,
    const PipelineLayout* layout,
    bool* needsPlaceholderSampler,
    bool* needsTextureBuiltinUniformBuffer,
    tint::TextureBuiltinsFromUniformOptions::BindingPointToFieldAndOffset* bindingPointToData)
    const {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "TranslateToGLSL");

    const OpenGLVersion& version = ToBackend(GetDevice())->GetGL().GetVersion();

    using tint::BindingPoint;
    // Since (non-Vulkan) GLSL does not support descriptor sets, generate a
    // mapping from the original group/binding pair to a binding-only
    // value. This mapping will be used by Tint to remap all global
    // variables to the 1D space.
    const BindingInfoArray& moduleBindingInfo =
        GetEntryPoint(programmableStage.entryPoint).bindings;
    BindingMap glBindings;
    BindingMap externalTextureExpansionMap;
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        uint32_t groupAsInt = static_cast<uint32_t>(group);
        const BindGroupLayoutInternalBase* bgl = layout->GetBindGroupLayout(group);
        const auto& indices = layout->GetBindingIndexInfo()[group];
        const auto& groupBindingInfo = moduleBindingInfo[group];
        for (const auto& [bindingNumber, bindingInfo] : groupBindingInfo) {
            BindingIndex bindingIndex = bgl->GetBindingIndex(bindingNumber);
            GLuint shaderIndex = indices[bindingIndex];
            BindingPoint srcBindingPoint{groupAsInt, static_cast<uint32_t>(bindingNumber)};
            BindingPoint dstBindingPoint{0, shaderIndex};
            if (srcBindingPoint != dstBindingPoint) {
                glBindings.emplace(srcBindingPoint, dstBindingPoint);
            }
        }

        for (const auto& [_, expansion] : bgl->GetExternalTextureBindingExpansionMap()) {
            uint32_t plane1Slot = indices[bgl->GetBindingIndex(expansion.plane1)];
            uint32_t paramsSlot = indices[bgl->GetBindingIndex(expansion.params)];
            BindingPoint plane0{groupAsInt, static_cast<uint32_t>(expansion.plane0)};
            BindingPoint plane1{groupAsInt, static_cast<uint32_t>(expansion.plane1)};
            BindingPoint params{groupAsInt, static_cast<uint32_t>(expansion.params)};
            glBindings.emplace(plane1, BindingPoint{0u, plane1Slot});
            glBindings.emplace(params, BindingPoint{0u, paramsSlot});
            externalTextureExpansionMap[plane0] = plane1;
        }
    }

    // Some texture builtin functions are unsupported on GLSL ES. These are emulated with internal
    // uniforms.
    tint::TextureBuiltinsFromUniformOptions textureBuiltinsFromUniform;
    textureBuiltinsFromUniform.ubo_binding = {kMaxBindGroups + 1, 0};
    // Remap the internal ubo binding as well.
    glBindings.emplace(textureBuiltinsFromUniform.ubo_binding,
                       BindingPoint{0, layout->GetInternalUniformBinding()});

    std::optional<tint::ast::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

    const CombinedLimits& limits = GetDevice()->GetLimits();

    GLSLCompilationRequest req = {};
    req.inputProgram = GetTintProgram();
    req.stage = stage;
    req.entryPointName = programmableStage.entryPoint;
    req.substituteOverrideConfig = std::move(substituteOverrideConfig);
    req.limits = LimitsForCompilationRequest::Create(limits.v1);
    req.platform = UnsafeUnkeyedValue(GetDevice()->GetPlatform());

    req.tintOptions.version = tint::glsl::writer::Version(ToTintGLStandard(version.GetStandard()),
                                                          version.GetMajor(), version.GetMinor());

    req.tintOptions.disable_robustness = false;

    req.tintOptions.external_texture_options = BuildExternalTextureTransformBindings(layout);
    req.tintOptions.binding_remapper_options.binding_points = std::move(glBindings);
    req.tintOptions.texture_builtins_from_uniform = std::move(textureBuiltinsFromUniform);

    // When textures are accessed without a sampler (e.g., textureLoad()),
    // GetSamplerTextureUses() will return this sentinel value.
    BindingPoint placeholderBindingPoint{static_cast<uint32_t>(kMaxBindGroupsTyped), 0};

    *needsPlaceholderSampler = false;
    tint::inspector::Inspector inspector(*req.inputProgram);
    // Find all the sampler/texture pairs for this entry point, and create
    // CombinedSamplers for them. CombinedSampler records the binding points
    // of the original texture and sampler, and generates a unique name. The
    // corresponding uniforms will be retrieved by these generated names
    // in PipelineGL. Any texture-only references will have
    // "usePlaceholderSampler" set to true, and only the texture binding point
    // will be used in naming them. In addition, Dawn will bind a
    // non-filtering sampler for them (see PipelineGL).
    auto uses =
        inspector.GetSamplerTextureUses(programmableStage.entryPoint, placeholderBindingPoint);
    CombinedSamplerInfo combinedSamplerInfo;
    for (const auto& use : uses) {
        CombinedSampler* info =
            AppendCombinedSampler(&combinedSamplerInfo, use, placeholderBindingPoint);

        if (info->usePlaceholderSampler) {
            *needsPlaceholderSampler = true;
            req.tintOptions.placeholder_binding_point = placeholderBindingPoint;
        }
        req.tintOptions.binding_map[use] = info->GetName();

        // If the texture has an associated plane1 texture (ie., it's an external texture),
        // append a new combined sampler with the same sampler and the plane1 texture.
        BindingMap::iterator plane1Texture =
            externalTextureExpansionMap.find(use.texture_binding_point);
        if (plane1Texture != externalTextureExpansionMap.end()) {
            tint::inspector::SamplerTexturePair plane1Use{use.sampler_binding_point,
                                                          plane1Texture->second};
            CombinedSampler* plane1Info =
                AppendCombinedSampler(&combinedSamplerInfo, plane1Use, placeholderBindingPoint);
            req.tintOptions.binding_map[plane1Use] = plane1Info->GetName();
        }
    }

    CacheResult<GLSLCompilation> compilationResult;
    DAWN_TRY_LOAD_OR_RUN(
        compilationResult, GetDevice(), std::move(req), GLSLCompilation::FromBlob,
        [](GLSLCompilationRequest r) -> ResultOrError<GLSLCompilation> {
            tint::ast::transform::Manager transformManager;
            tint::ast::transform::DataMap transformInputs;

            transformManager.Add<tint::ast::transform::SingleEntryPoint>();
            transformInputs.Add<tint::ast::transform::SingleEntryPoint::Config>(r.entryPointName);

            if (r.substituteOverrideConfig) {
                // This needs to run after SingleEntryPoint transform which removes unused overrides
                // for current entry point.
                transformManager.Add<tint::ast::transform::SubstituteOverride>();
                transformInputs.Add<tint::ast::transform::SubstituteOverride::Config>(
                    std::move(r.substituteOverrideConfig).value());
            }

            tint::Program program;
            tint::ast::transform::DataMap transformOutputs;
            DAWN_TRY_ASSIGN(program, RunTransforms(&transformManager, r.inputProgram,
                                                   transformInputs, &transformOutputs, nullptr));

            if (r.stage == SingleShaderStage::Compute) {
                // Validate workgroup size after program runs transforms.
                Extent3D _;
                DAWN_TRY_ASSIGN(_, ValidateComputeStageWorkgroupSize(
                                       program, r.entryPointName.c_str(), r.limits));
            }

            auto result = tint::glsl::writer::Generate(program, r.tintOptions, r.entryPointName);
            DAWN_INVALID_IF(!result, "An error occurred while generating GLSL:\n%s",
                            result.Failure().reason.str());

            return GLSLCompilation{{std::move(result->glsl), result->needs_internal_uniform_buffer,
                                    result->bindpoint_to_data}};
        },
        "OpenGL.CompileShaderToGLSL");

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
    *needsTextureBuiltinUniformBuffer = compilationResult->needsInternalUniformBuffer;

    // Since the TextureBuiltinsFromUniform transform runs before BindingRemapper,
    // we need to take care of their binding remappings here.
    for (const auto& e : compilationResult->bindingPointToData) {
        tint::BindingPoint bindingPoint = e.first;

        const BindGroupLayoutInternalBase* bgl =
            layout->GetBindGroupLayout(BindGroupIndex{bindingPoint.group});
        bindingPoint.binding =
            static_cast<uint32_t>(bgl->GetBindingIndex(BindingNumber{bindingPoint.binding}));

        bindingPointToData->emplace(bindingPoint, e.second);
    }

    *combinedSamplers = std::move(combinedSamplerInfo);
    return shader;
}

}  // namespace dawn::native::opengl
