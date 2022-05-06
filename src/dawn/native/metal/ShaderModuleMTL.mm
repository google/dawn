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
#include "dawn/native/TintUtils.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/PipelineLayoutMTL.h"
#include "dawn/native/metal/RenderPipelineMTL.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include <tint/tint.h>

#include <sstream>

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

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    ScopedTintICEHandler scopedICEHandler(GetDevice());
    return InitializeBase(parseResult, compilationMessages);
}

ResultOrError<std::string> ShaderModule::TranslateToMSL(
    const char* entryPointName,
    SingleShaderStage stage,
    const PipelineLayout* layout,
    uint32_t sampleMask,
    const RenderPipeline* renderPipeline,
    std::string* remappedEntryPointName,
    bool* needsStorageBufferLength,
    bool* hasInvariantAttribute,
    std::vector<uint32_t>* workgroupAllocations) {
    ScopedTintICEHandler scopedICEHandler(GetDevice());

    std::ostringstream errorStream;
    errorStream << "Tint MSL failure:" << std::endl;

    // Remap BindingNumber to BindingIndex in WGSL shader
    using BindingRemapper = tint::transform::BindingRemapper;
    using BindingPoint = tint::transform::BindingPoint;
    BindingRemapper::BindingPoints bindingPoints;
    BindingRemapper::AccessControls accessControls;

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

    tint::transform::Manager transformManager;
    tint::transform::DataMap transformInputs;

    // We only remap bindings for the target entry point, so we need to strip all other entry
    // points to avoid generating invalid bindings for them.
    transformManager.Add<tint::transform::SingleEntryPoint>();
    transformInputs.Add<tint::transform::SingleEntryPoint::Config>(entryPointName);

    AddExternalTextureTransform(layout, &transformManager, &transformInputs);

    if (stage == SingleShaderStage::Vertex &&
        GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling)) {
        transformManager.Add<tint::transform::VertexPulling>();
        AddVertexPullingTransformConfig(*renderPipeline, entryPointName, kPullingBufferBindingSet,
                                        &transformInputs);

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
    if (GetDevice()->IsRobustnessEnabled()) {
        transformManager.Add<tint::transform::Robustness>();
    }
    transformManager.Add<tint::transform::BindingRemapper>();
    transformManager.Add<tint::transform::Renamer>();

    if (GetDevice()->IsToggleEnabled(Toggle::DisableSymbolRenaming)) {
        // We still need to rename MSL reserved keywords
        transformInputs.Add<tint::transform::Renamer::Config>(
            tint::transform::Renamer::Target::kMslKeywords);
    }

    transformInputs.Add<BindingRemapper::Remappings>(std::move(bindingPoints),
                                                     std::move(accessControls),
                                                     /* mayCollide */ true);

    tint::Program program;
    tint::transform::DataMap transformOutputs;
    {
        TRACE_EVENT0(GetDevice()->GetPlatform(), General, "RunTransforms");
        DAWN_TRY_ASSIGN(program, RunTransforms(&transformManager, GetTintProgram(), transformInputs,
                                               &transformOutputs, nullptr));
    }

    if (auto* data = transformOutputs.Get<tint::transform::Renamer::Data>()) {
        auto it = data->remappings.find(entryPointName);
        if (it != data->remappings.end()) {
            *remappedEntryPointName = it->second;
        } else {
            DAWN_INVALID_IF(!GetDevice()->IsToggleEnabled(Toggle::DisableSymbolRenaming),
                            "Could not find remapped name for entry point.");

            *remappedEntryPointName = entryPointName;
        }
    } else {
        return DAWN_FORMAT_VALIDATION_ERROR("Transform output missing renamer data.");
    }

    tint::writer::msl::Options options;
    options.buffer_size_ubo_index = kBufferLengthBufferSlot;
    options.fixed_sample_mask = sampleMask;
    options.disable_workgroup_init = GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    options.emit_vertex_point_size =
        stage == SingleShaderStage::Vertex &&
        renderPipeline->GetPrimitiveTopology() == wgpu::PrimitiveTopology::PointList;
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "tint::writer::msl::Generate");
    auto result = tint::writer::msl::Generate(&program, options);
    DAWN_INVALID_IF(!result.success, "An error occured while generating MSL: %s.", result.error);

    *needsStorageBufferLength = result.needs_storage_buffer_sizes;
    *hasInvariantAttribute = result.has_invariant_attribute;
    *workgroupAllocations = std::move(result.workgroup_allocations[*remappedEntryPointName]);

    return std::move(result.msl);
}

MaybeError ShaderModule::CreateFunction(const char* entryPointName,
                                        SingleShaderStage stage,
                                        const PipelineLayout* layout,
                                        ShaderModule::MetalFunctionData* out,
                                        id constantValuesPointer,
                                        uint32_t sampleMask,
                                        const RenderPipeline* renderPipeline) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "ShaderModuleMTL::CreateFunction");

    ASSERT(!IsError());
    ASSERT(out);

    // Vertex stages must specify a renderPipeline
    if (stage == SingleShaderStage::Vertex) {
        ASSERT(renderPipeline != nullptr);
    }

    std::string remappedEntryPointName;
    std::string msl;
    bool hasInvariantAttribute = false;
    DAWN_TRY_ASSIGN(msl, TranslateToMSL(entryPointName, stage, layout, sampleMask, renderPipeline,
                                        &remappedEntryPointName, &out->needsStorageBufferLength,
                                        &hasInvariantAttribute, &out->workgroupAllocations));

    // Metal uses Clang to compile the shader as C++14. Disable everything in the -Wall
    // category. -Wunused-variable in particular comes up a lot in generated code, and some
    // (old?) Metal drivers accidentally treat it as a MTLLibraryErrorCompileError instead
    // of a warning.
    msl = R"(
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
)" + msl;

    if (GetDevice()->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated MSL */" << std::endl << msl;
        GetDevice()->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    NSRef<NSString> mslSource = AcquireNSRef([[NSString alloc] initWithUTF8String:msl.c_str()]);

    NSRef<MTLCompileOptions> compileOptions = AcquireNSRef([[MTLCompileOptions alloc] init]);
    if (hasInvariantAttribute) {
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

    NSRef<NSString> name =
        AcquireNSRef([[NSString alloc] initWithUTF8String:remappedEntryPointName.c_str()]);

    {
        TRACE_EVENT0(GetDevice()->GetPlatform(), General, "MTLLibrary::newFunctionWithName");
        if (constantValuesPointer != nil) {
            if (@available(macOS 10.12, *)) {
                MTLFunctionConstantValues* constantValues = constantValuesPointer;
                out->function = AcquireNSPRef([*library newFunctionWithName:name.Get()
                                                             constantValues:constantValues
                                                                      error:&error]);
                if (error != nullptr) {
                    if (error.code != MTLLibraryErrorCompileWarning) {
                        return DAWN_VALIDATION_ERROR(std::string("Function compile error: ") +
                                                     [error.localizedDescription UTF8String]);
                    }
                }
                ASSERT(out->function != nil);
            } else {
                UNREACHABLE();
            }
        } else {
            out->function = AcquireNSPRef([*library newFunctionWithName:name.Get()]);
        }
    }

    if (GetDevice()->IsToggleEnabled(Toggle::MetalEnableVertexPulling) &&
        GetEntryPoint(entryPointName).usedVertexInputs.any()) {
        out->needsStorageBufferLength = true;
    }

    return {};
}
}  // namespace dawn::native::metal
