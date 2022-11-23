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

#include "dawn/native/metal/ShaderModuleMTL.h"

#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/CacheRequest.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/PipelineLayoutMTL.h"
#include "dawn/native/metal/RenderPipelineMTL.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include <tint/tint.h>

#include <sstream>

namespace dawn::native::metal {
namespace {

using OptionalVertexPullingTransformConfig = std::optional<tint::transform::VertexPulling::Config>;

#define MSL_COMPILATION_REQUEST_MEMBERS(X)                                                  \
    X(SingleShaderStage, stage)                                                             \
    X(const tint::Program*, inputProgram)                                                   \
    X(tint::transform::BindingRemapper::BindingPoints, bindingPoints)                       \
    X(tint::transform::MultiplanarExternalTexture::BindingsMap, externalTextureBindings)    \
    X(OptionalVertexPullingTransformConfig, vertexPullingTransformConfig)                   \
    X(std::optional<tint::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(LimitsForCompilationRequest, limits)                                                  \
    X(std::string, entryPointName)                                                          \
    X(uint32_t, sampleMask)                                                                 \
    X(bool, emitVertexPointSize)                                                            \
    X(bool, isRobustnessEnabled)                                                            \
    X(bool, disableSymbolRenaming)                                                          \
    X(bool, disableWorkgroupInit)                                                           \
    X(CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*>, tracePlatform)

DAWN_MAKE_CACHE_REQUEST(MslCompilationRequest, MSL_COMPILATION_REQUEST_MEMBERS);
#undef MSL_COMPILATION_REQUEST_MEMBERS

using WorkgroupAllocations = std::vector<uint32_t>;

#define MSL_COMPILATION_MEMBERS(X)                \
    X(std::string, msl)                           \
    X(std::string, remappedEntryPointName)        \
    X(bool, needsStorageBufferLength)             \
    X(bool, hasInvariantAttribute)                \
    X(WorkgroupAllocations, workgroupAllocations) \
    X(Extent3D, localWorkgroupSize)

DAWN_SERIALIZABLE(struct, MslCompilation, MSL_COMPILATION_MEMBERS){};
#undef MSL_COMPILATION_MEMBERS

}  // namespace
}  // namespace dawn::native::metal

namespace dawn::native::metal {

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

ShaderModule::~ShaderModule() = default;

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    ScopedTintICEHandler scopedICEHandler(GetDevice());
    return InitializeBase(parseResult, compilationMessages);
}

namespace {

ResultOrError<CacheResult<MslCompilation>> TranslateToMSL(
    DeviceBase* device,
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    const PipelineLayout* layout,
    ShaderModule::MetalFunctionData* out,
    uint32_t sampleMask,
    const RenderPipeline* renderPipeline) {
    ScopedTintICEHandler scopedICEHandler(device);

    std::ostringstream errorStream;
    errorStream << "Tint MSL failure:" << std::endl;

    // Remap BindingNumber to BindingIndex in WGSL shader
    using BindingRemapper = tint::transform::BindingRemapper;
    using BindingPoint = tint::transform::BindingPoint;
    BindingRemapper::BindingPoints bindingPoints;

    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayoutBase::BindingMap& bindingMap =
            layout->GetBindGroupLayout(group)->GetBindingMap();
        for (const auto [bindingNumber, bindingIndex] : bindingMap) {
            const BindingInfo& bindingInfo =
                layout->GetBindGroupLayout(group)->GetBindingInfo(bindingIndex);

            if (!(bindingInfo.visibility & StageBit(stage))) {
                continue;
            }

            uint32_t shaderIndex = layout->GetBindingIndexInfo(stage)[group][bindingIndex];

            BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                         static_cast<uint32_t>(bindingNumber)};
            BindingPoint dstBindingPoint{0, shaderIndex};
            if (srcBindingPoint != dstBindingPoint) {
                bindingPoints.emplace(srcBindingPoint, dstBindingPoint);
            }
        }
    }

    auto externalTextureBindings = BuildExternalTextureTransformBindings(layout);

    std::optional<tint::transform::VertexPulling::Config> vertexPullingTransformConfig;
    if (stage == SingleShaderStage::Vertex &&
        device->IsToggleEnabled(Toggle::MetalEnableVertexPulling)) {
        vertexPullingTransformConfig =
            BuildVertexPullingTransformConfig(*renderPipeline, kPullingBufferBindingSet);

        for (VertexBufferSlot slot : IterateBitSet(renderPipeline->GetVertexBufferSlotsUsed())) {
            uint32_t metalIndex = renderPipeline->GetMtlVertexBufferIndex(slot);

            // Tell Tint to map (kPullingBufferBindingSet, slot) to this MSL buffer index.
            BindingPoint srcBindingPoint{static_cast<uint32_t>(kPullingBufferBindingSet),
                                         static_cast<uint8_t>(slot)};
            BindingPoint dstBindingPoint{0, metalIndex};
            if (srcBindingPoint != dstBindingPoint) {
                bindingPoints.emplace(srcBindingPoint, dstBindingPoint);
            }
        }
    }

    std::optional<tint::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

    MslCompilationRequest req = {};
    req.stage = stage;
    req.inputProgram = programmableStage.module->GetTintProgram();
    req.bindingPoints = std::move(bindingPoints);
    req.externalTextureBindings = std::move(externalTextureBindings);
    req.vertexPullingTransformConfig = std::move(vertexPullingTransformConfig);
    req.substituteOverrideConfig = std::move(substituteOverrideConfig);
    req.entryPointName = programmableStage.entryPoint.c_str();
    req.sampleMask = sampleMask;
    req.emitVertexPointSize =
        stage == SingleShaderStage::Vertex &&
        renderPipeline->GetPrimitiveTopology() == wgpu::PrimitiveTopology::PointList;
    req.isRobustnessEnabled = device->IsRobustnessEnabled();
    req.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.tracePlatform = UnsafeUnkeyedValue(device->GetPlatform());

    const CombinedLimits& limits = device->GetLimits();
    req.limits = LimitsForCompilationRequest::Create(limits.v1);

    CacheResult<MslCompilation> mslCompilation;
    DAWN_TRY_LOAD_OR_RUN(
        mslCompilation, device, std::move(req), MslCompilation::FromBlob,
        [](MslCompilationRequest r) -> ResultOrError<MslCompilation> {
            tint::transform::Manager transformManager;
            tint::transform::DataMap transformInputs;

            // We only remap bindings for the target entry point, so we need to strip all other
            // entry points to avoid generating invalid bindings for them.
            // Run before the renamer so that the entry point name matches `entryPointName` still.
            transformManager.Add<tint::transform::SingleEntryPoint>();
            transformInputs.Add<tint::transform::SingleEntryPoint::Config>(r.entryPointName);

            // Needs to run before all other transforms so that they can use builtin names safely.
            transformManager.Add<tint::transform::Renamer>();
            if (r.disableSymbolRenaming) {
                // We still need to rename MSL reserved keywords
                transformInputs.Add<tint::transform::Renamer::Config>(
                    tint::transform::Renamer::Target::kMslKeywords);
            }

            if (!r.externalTextureBindings.empty()) {
                transformManager.Add<tint::transform::MultiplanarExternalTexture>();
                transformInputs.Add<tint::transform::MultiplanarExternalTexture::NewBindingPoints>(
                    std::move(r.externalTextureBindings));
            }

            if (r.vertexPullingTransformConfig) {
                transformManager.Add<tint::transform::VertexPulling>();
                transformInputs.Add<tint::transform::VertexPulling::Config>(
                    std::move(r.vertexPullingTransformConfig).value());
            }

            if (r.substituteOverrideConfig) {
                // This needs to run after SingleEntryPoint transform which removes unused overrides
                // for current entry point.
                transformManager.Add<tint::transform::SubstituteOverride>();
                transformInputs.Add<tint::transform::SubstituteOverride::Config>(
                    std::move(r.substituteOverrideConfig).value());
            }

            if (r.isRobustnessEnabled) {
                transformManager.Add<tint::transform::Robustness>();
            }
            transformManager.Add<BindingRemapper>();
            transformInputs.Add<BindingRemapper::Remappings>(std::move(r.bindingPoints),
                                                             BindingRemapper::AccessControls{},
                                                             /* mayCollide */ true);

            tint::Program program;
            tint::transform::DataMap transformOutputs;
            {
                TRACE_EVENT0(r.tracePlatform.UnsafeGetValue(), General, "RunTransforms");
                DAWN_TRY_ASSIGN(program,
                                RunTransforms(&transformManager, r.inputProgram, transformInputs,
                                              &transformOutputs, nullptr));
            }

            std::string remappedEntryPointName;
            if (auto* data = transformOutputs.Get<tint::transform::Renamer::Data>()) {
                auto it = data->remappings.find(r.entryPointName);
                if (it != data->remappings.end()) {
                    remappedEntryPointName = it->second;
                } else {
                    DAWN_INVALID_IF(!r.disableSymbolRenaming,
                                    "Could not find remapped name for entry point.");

                    remappedEntryPointName = r.entryPointName;
                }
            } else {
                return DAWN_VALIDATION_ERROR("Transform output missing renamer data.");
            }

            Extent3D localSize{0, 0, 0};
            if (r.stage == SingleShaderStage::Compute) {
                // Validate workgroup size after program runs transforms.
                DAWN_TRY_ASSIGN(localSize, ValidateComputeStageWorkgroupSize(
                                               program, remappedEntryPointName.data(), r.limits));
            }

            tint::writer::msl::Options options;
            options.buffer_size_ubo_index = kBufferLengthBufferSlot;
            options.fixed_sample_mask = r.sampleMask;
            options.disable_workgroup_init = r.disableWorkgroupInit;
            options.emit_vertex_point_size = r.emitVertexPointSize;
            TRACE_EVENT0(r.tracePlatform.UnsafeGetValue(), General, "tint::writer::msl::Generate");
            auto result = tint::writer::msl::Generate(&program, options);
            DAWN_INVALID_IF(!result.success, "An error occured while generating MSL: %s.",
                            result.error);

            // Metal uses Clang to compile the shader as C++14. Disable everything in the -Wall
            // category. -Wunused-variable in particular comes up a lot in generated code, and some
            // (old?) Metal drivers accidentally treat it as a MTLLibraryErrorCompileError instead
            // of a warning.
            result.msl = R"(
                #ifdef __clang__
                #pragma clang diagnostic ignored "-Wall"
                #endif
            )" + result.msl;

            auto workgroupAllocations =
                std::move(result.workgroup_allocations[remappedEntryPointName]);
            return MslCompilation{{
                std::move(result.msl),
                std::move(remappedEntryPointName),
                result.needs_storage_buffer_sizes,
                result.has_invariant_attribute,
                std::move(workgroupAllocations),
                localSize,
            }};
        });

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated MSL */" << std::endl << mslCompilation->msl;
        device->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    return mslCompilation;
}

}  // namespace

MaybeError ShaderModule::CreateFunction(SingleShaderStage stage,
                                        const ProgrammableStage& programmableStage,
                                        const PipelineLayout* layout,
                                        ShaderModule::MetalFunctionData* out,
                                        uint32_t sampleMask,
                                        const RenderPipeline* renderPipeline) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "ShaderModuleMTL::CreateFunction");

    ASSERT(!IsError());
    ASSERT(out);

    const char* entryPointName = programmableStage.entryPoint.c_str();

    // Vertex stages must specify a renderPipeline
    if (stage == SingleShaderStage::Vertex) {
        ASSERT(renderPipeline != nullptr);
    }

    CacheResult<MslCompilation> mslCompilation;
    DAWN_TRY_ASSIGN(mslCompilation, TranslateToMSL(GetDevice(), programmableStage, stage, layout,
                                                   out, sampleMask, renderPipeline));
    out->needsStorageBufferLength = mslCompilation->needsStorageBufferLength;
    out->workgroupAllocations = std::move(mslCompilation->workgroupAllocations);
    out->localWorkgroupSize = MTLSizeMake(mslCompilation->localWorkgroupSize.width,
                                          mslCompilation->localWorkgroupSize.height,
                                          mslCompilation->localWorkgroupSize.depthOrArrayLayers);

    NSRef<NSString> mslSource =
        AcquireNSRef([[NSString alloc] initWithUTF8String:mslCompilation->msl.c_str()]);

    NSRef<MTLCompileOptions> compileOptions = AcquireNSRef([[MTLCompileOptions alloc] init]);
    if (mslCompilation->hasInvariantAttribute) {
        if (@available(macOS 11.0, iOS 13.0, *)) {
            (*compileOptions).preserveInvariance = true;
        }
    }
    auto mtlDevice = ToBackend(GetDevice())->GetMTLDevice();
    NSError* error = nullptr;

    NSPRef<id<MTLLibrary>> library;
    {
        TRACE_EVENT0(GetDevice()->GetPlatform(), General, "MTLDevice::newLibraryWithSource");
        library = AcquireNSPRef([mtlDevice newLibraryWithSource:mslSource.Get()
                                                        options:compileOptions.Get()
                                                          error:&error]);
    }

    if (error != nullptr) {
        DAWN_INVALID_IF(error.code != MTLLibraryErrorCompileWarning,
                        "Unable to create library object: %s.",
                        [error.localizedDescription UTF8String]);
    }
    ASSERT(library != nil);

    NSRef<NSString> name = AcquireNSRef(
        [[NSString alloc] initWithUTF8String:mslCompilation->remappedEntryPointName.c_str()]);

    {
        TRACE_EVENT0(GetDevice()->GetPlatform(), General, "MTLLibrary::newFunctionWithName");
        out->function = AcquireNSPRef([*library newFunctionWithName:name.Get()]);
    }

    GetDevice()->GetBlobCache()->EnsureStored(mslCompilation);

    if (GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling) &&
        GetEntryPoint(entryPointName).usedVertexInputs.any()) {
        out->needsStorageBufferLength = true;
    }

    return {};
}
}  // namespace dawn::native::metal
