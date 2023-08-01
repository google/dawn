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
#include "dawn/native/metal/UtilsMetal.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include <tint/tint.h>

#include <sstream>

namespace dawn::native::metal {
namespace {

using OptionalVertexPullingTransformConfig =
    std::optional<tint::ast::transform::VertexPulling::Config>;

#define MSL_COMPILATION_REQUEST_MEMBERS(X)                                                       \
    X(SingleShaderStage, stage)                                                                  \
    X(const tint::Program*, inputProgram)                                                        \
    X(tint::ArrayLengthFromUniformOptions, arrayLengthFromUniform)                               \
    X(tint::BindingRemapperOptions, bindingRemapper)                                             \
    X(tint::ExternalTextureOptions, externalTextureOptions)                                      \
    X(OptionalVertexPullingTransformConfig, vertexPullingTransformConfig)                        \
    X(std::optional<tint::ast::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(LimitsForCompilationRequest, limits)                                                       \
    X(std::string, entryPointName)                                                               \
    X(uint32_t, sampleMask)                                                                      \
    X(bool, emitVertexPointSize)                                                                 \
    X(bool, isRobustnessEnabled)                                                                 \
    X(bool, disableSymbolRenaming)                                                               \
    X(bool, disableWorkgroupInit)                                                                \
    X(CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*>, platform)

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
    using BindingPoint = tint::BindingPoint;

    tint::BindingRemapperOptions bindingRemapper;
    bindingRemapper.allow_collisions = true;

    tint::ArrayLengthFromUniformOptions arrayLengthFromUniform;
    arrayLengthFromUniform.ubo_binding = {0, kBufferLengthBufferSlot};

    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayoutInternalBase::BindingMap& bindingMap =
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
                bindingRemapper.binding_points.emplace(srcBindingPoint, dstBindingPoint);
            }

            // Use the ShaderIndex as the indices for the buffer size lookups in the array length
            // uniform transform. This is used to compute the size of variable length arrays in
            // storage buffers.
            if (bindingInfo.buffer.type == wgpu::BufferBindingType::Storage ||
                bindingInfo.buffer.type == wgpu::BufferBindingType::ReadOnlyStorage ||
                bindingInfo.buffer.type == kInternalStorageBufferBinding) {
                arrayLengthFromUniform.bindpoint_to_size_index.emplace(dstBindingPoint,
                                                                       dstBindingPoint.binding);
            }
        }
    }

    std::optional<tint::ast::transform::VertexPulling::Config> vertexPullingTransformConfig;
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
                bindingRemapper.binding_points.emplace(srcBindingPoint, dstBindingPoint);
            }

            // Use the ShaderIndex as the indices for the buffer size lookups in the array
            // length uniform transform.
            arrayLengthFromUniform.bindpoint_to_size_index.emplace(dstBindingPoint,
                                                                   dstBindingPoint.binding);
        }
    }

    std::optional<tint::ast::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

    MslCompilationRequest req = {};
    req.stage = stage;
    req.inputProgram = programmableStage.module->GetTintProgram();
    req.bindingRemapper = std::move(bindingRemapper);
    req.externalTextureOptions = BuildExternalTextureTransformBindings(layout);
    req.vertexPullingTransformConfig = std::move(vertexPullingTransformConfig);
    req.substituteOverrideConfig = std::move(substituteOverrideConfig);
    req.entryPointName = programmableStage.entryPoint.c_str();
    req.sampleMask = sampleMask;
    req.emitVertexPointSize =
        stage == SingleShaderStage::Vertex &&
        renderPipeline->GetPrimitiveTopology() == wgpu::PrimitiveTopology::PointList;
    req.isRobustnessEnabled = device->IsRobustnessEnabled();
    req.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.platform = UnsafeUnkeyedValue(device->GetPlatform());
    req.arrayLengthFromUniform = std::move(arrayLengthFromUniform);

    const CombinedLimits& limits = device->GetLimits();
    req.limits = LimitsForCompilationRequest::Create(limits.v1);

    CacheResult<MslCompilation> mslCompilation;
    DAWN_TRY_LOAD_OR_RUN(
        mslCompilation, device, std::move(req), MslCompilation::FromBlob,
        [](MslCompilationRequest r) -> ResultOrError<MslCompilation> {
            tint::ast::transform::Manager transformManager;
            tint::ast::transform::DataMap transformInputs;

            // We only remap bindings for the target entry point, so we need to strip all other
            // entry points to avoid generating invalid bindings for them.
            // Run before the renamer so that the entry point name matches `entryPointName` still.
            transformManager.Add<tint::ast::transform::SingleEntryPoint>();
            transformInputs.Add<tint::ast::transform::SingleEntryPoint::Config>(r.entryPointName);

            // Needs to run before all other transforms so that they can use builtin names safely.
            transformManager.Add<tint::ast::transform::Renamer>();
            if (r.disableSymbolRenaming) {
                // We still need to rename MSL reserved keywords
                transformInputs.Add<tint::ast::transform::Renamer::Config>(
                    tint::ast::transform::Renamer::Target::kMslKeywords);
            }

            if (r.vertexPullingTransformConfig) {
                transformManager.Add<tint::ast::transform::VertexPulling>();
                transformInputs.Add<tint::ast::transform::VertexPulling::Config>(
                    std::move(r.vertexPullingTransformConfig).value());
            }

            if (r.substituteOverrideConfig) {
                // This needs to run after SingleEntryPoint transform which removes unused overrides
                // for current entry point.
                transformManager.Add<tint::ast::transform::SubstituteOverride>();
                transformInputs.Add<tint::ast::transform::SubstituteOverride::Config>(
                    std::move(r.substituteOverrideConfig).value());
            }

            tint::Program program;
            tint::ast::transform::DataMap transformOutputs;
            {
                TRACE_EVENT0(r.platform.UnsafeGetValue(), General, "RunTransforms");
                DAWN_TRY_ASSIGN(program,
                                RunTransforms(&transformManager, r.inputProgram, transformInputs,
                                              &transformOutputs, nullptr));
            }

            std::string remappedEntryPointName;
            if (auto* data = transformOutputs.Get<tint::ast::transform::Renamer::Data>()) {
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

            tint::msl::writer::Options options;
            options.disable_robustness = !r.isRobustnessEnabled;
            options.buffer_size_ubo_index = kBufferLengthBufferSlot;
            options.fixed_sample_mask = r.sampleMask;
            options.disable_workgroup_init = r.disableWorkgroupInit;
            options.emit_vertex_point_size = r.emitVertexPointSize;
            options.array_length_from_uniform = r.arrayLengthFromUniform;
            options.binding_remapper_options = r.bindingRemapper;
            options.external_texture_options = r.externalTextureOptions;

            TRACE_EVENT0(r.platform.UnsafeGetValue(), General, "tint::msl::writer::Generate");
            auto result = tint::msl::writer::Generate(&program, options);
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
        },
        "Metal.CompileShaderToMSL");

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

    std::ostringstream labelStream;
    labelStream << GetLabel() << "::" << entryPointName;
    SetDebugName(GetDevice(), out->function.Get(), "Dawn_ShaderModule", labelStream.str());
    GetDevice()->GetBlobCache()->EnsureStored(mslCompilation);

    if (GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling) &&
        GetEntryPoint(entryPointName).usedVertexInputs.any()) {
        out->needsStorageBufferLength = true;
    }

    return {};
}
}  // namespace dawn::native::metal
